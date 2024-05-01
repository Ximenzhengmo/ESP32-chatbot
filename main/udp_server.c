#include "udp_server.h"

TaskHandle_t udp_server_receive_handle = NULL;
bool udp_server_stopped = false;
static const char *TAG = "udp_server";


static void udp_server_receive_task(void *pvParameters)
{
    static char rx_buffer[TRX_BUFFER_SIZE];
    static char tx_buffer[TRX_BUFFER_SIZE];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;
    // char ans[]= "hello";
    while (1) {
        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");
        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        while (1) {
            if ( udp_server_stopped ){
                vTaskDelay(100 / portTICK_PERIOD_MS);
                continue;
            }
// ------------------   print log    -------------------
            // ESP_LOGI(TAG, "Waiting for data");
// ------------------   print log    -------------------
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str)-1);
                } else if (source_addr.ss_family == PF_INET6) {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str)-1);
                }

// ------------------   print log    -------------------
                // ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
// ------------------   print log    -------------------
                // ESP_LOGI(TAG, "%s", rx_buffer);
                static size_t i2s_bytes_write = 0;
                static size_t i2s_bytes_read = 0;
                i2s_write(I2S_NUM_0, rx_buffer, len, &i2s_bytes_write , 100 / portTICK_RATE_MS);
                if ( gpio_get_level( (gpio_num_t)get_input_speak_id() ) == 0 ){
                    i2s_read(I2S_NUM_0, tx_buffer, sizeof(tx_buffer), &i2s_bytes_read, 100 / portTICK_RATE_MS);
                    int err = sendto(sock, tx_buffer, i2s_bytes_read , 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }
                }else{
                    static uint8_t silence[4] = {0xFF, 0xFF, 0xFF, 0xFF};
                    int err = sendto(sock, silence , 4 , 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void create_udp_server(void)
{
    if( udp_server_receive_handle != NULL ){
        return;
    }
    xTaskCreate(udp_server_receive_task, "udp_server", 4096 * 2, (void*)AF_INET, 5, &udp_server_receive_handle);
    ESP_LOGI(TAG, "udp server created");
}

void distory_udp_server(void){
    if( udp_server_receive_handle ){
        vTaskDelete(udp_server_receive_handle);
        udp_server_receive_handle = NULL;
        ESP_LOGW(TAG, "udp server deleted");
    }
}

void udp_server_suspend(){
    if( udp_server_receive_handle ){
        vTaskSuspend(udp_server_receive_handle);
        ESP_LOGW(TAG, "udp server suspended");
    }
}

void udp_server_wait_for_state_change( eTaskState state ){
    while( udp_server_receive_handle && eTaskGetState(udp_server_receive_handle) != state ){
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void udp_server_resume(){
    if( udp_server_receive_handle ){
        if( eTaskGetState(udp_server_receive_handle) == eSuspended ){
            vTaskResume(udp_server_receive_handle);
            ESP_LOGW(TAG, "udp server resumed");
        }
    }
}