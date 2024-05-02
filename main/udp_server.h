#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include <string.h>
#include "sdkconfig.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

#if CONFIG_EXAMPLE_CONNECT_ETHERNET
#include "esp_eth.h"
#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif // CONFIG_ETH_USE_SPI_ETHERNET
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET
#include "esp_log.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "nvs_flash.h"

#include "i2s_stream.h"
#include "board.h"

#define PORT 5555
#define TRX_BUFFER_SIZE 1024

extern TaskHandle_t udp_server_receive_handle;
extern TaskHandle_t udp_server_send_handle; 

/**
 * @brief      Create UDP server
*/
void create_udp_server(void);

/**
 * @brief      Destory UDP server
*/
void distory_udp_server(void);

/**
 * @brief      UDP server stop receive and response
*/
void udp_server_stop_process(void);

/**
 * @brief      UDP server start receive and response
*/
void udp_server_start_process(void);


#endif