#include "asr.h"

static const char* TAG = "ASR";

//* BAIDU ASR API Key and Secret
#define BAIDU_API_ACCESS_KEY CONFIG_BAIDU_ASR_ACCESS_KEY
#define BAIDU_API_ACCESS_SECERT CONFIG_BAIDU_ASR_ACCESS_SECERT

esp_periph_handle_t led_handle = NULL; // * LED handle
esp_periph_set_handle_t set = NULL;  // * Peripherals set handle
audio_event_iface_handle_t evt = NULL;  // * Audio event handle
baidu_sr_handle_t sr = NULL;  // * Baidu ASR handle

//* Callback function when the `sr` starts
void baidu_sr_begin(baidu_sr_handle_t sr) {
    if (led_handle) {
        periph_led_blink(led_handle, get_green_led_gpio(), 500, 500, true, -1, 0);
    }
    ESP_LOGW(TAG, "Start speaking now");
}

// * Clear the envet queue
void clear_msg(){
    audio_event_iface_discard(evt);
}

void asr_init() {
    // * Initialize NVS flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
#else
    tcpip_adapter_init();
#endif
    ESP_LOGI(TAG, "[ 1 ] Initialize Buttons & Connect to Wi-Fi network, ssid=%s", "BUAA-Mobile");
    // * Initialize peripherals management
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    set = esp_periph_set_init(&periph_cfg);

    //* Initialize Wi-Fi
    periph_wifi_cfg_t wifi_cfg = WIFI_CONFIG_DEFAULT();
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);

    //* Initialize the `record button`
    periph_button_cfg_t btn_cfg = {
        .gpio_mask = (1ULL << get_input_rec_id()),
    };
    esp_periph_handle_t button_handle = periph_button_init(&btn_cfg);

    //* Initialize the LED
    periph_led_cfg_t led_cfg = {
        .led_speed_mode = LEDC_LOW_SPEED_MODE,
        .led_duty_resolution = LEDC_TIMER_10_BIT,
        .led_timer_num = LEDC_TIMER_0,
        .led_freq_hz = 5000,
    };
    led_handle = periph_led_init(&led_cfg);


    //* Start wifi & button & led peripheral
    esp_periph_start(set, button_handle);
    esp_periph_start(set, wifi_handle);
    esp_periph_start(set, led_handle);
    // * Wait for Wi-Fi connection
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    // * Initialize the onboard Audio Codec ES8388
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_STOP);
    vTaskDelay(100 / portTICK_RATE_MS);
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 3 ] Create and start audio_pipeline");
    //* Initialize the Baidu ASR
    baidu_sr_config_t sr_config = {
        .api_key = BAIDU_API_ACCESS_KEY,
        .secret_key = BAIDU_API_ACCESS_SECERT,
        .record_sample_rates = ASR_SAMPLE_RATE,
        .on_begin = baidu_sr_begin,
        .buffer_size = DEFAULT_SR_BUFFER_SIZE,
    };
    sr = baidu_sr_init(&sr_config);

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    //* Initialize the event interface and set the listener
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);
    ESP_LOGI(TAG, "[4.1] Listening event from the pipeline");
    baidu_sr_set_listener(sr, evt);
    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);
}


void asr_uninit() {
    ESP_LOGI(TAG, "Stop audio_pipeline");
    baidu_sr_destroy(sr);
    /* Stop all periph before removing the listener */
    esp_periph_set_stop_all(set);
    if( set != NULL && evt != NULL) {
        audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);
    }
    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);
    esp_periph_set_destroy(set);
}

char* asr_get_text() {
    ESP_LOGI(TAG, "[ 5 ] Listen for all pipeline events");
    char* original_text = NULL; // * The original text, if failed to get the text, it will be NULL

    // * Start the UDP server, it will work and listen to the client until the user press the `record button` ,
    // * and then it will resume after the answer audio is over.
    udp_server_start_process();
    while (1) {
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(evt, &msg, portMAX_DELAY) != ESP_OK) {
            ESP_LOGW(TAG, "[ * ] Event process failed: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
            continue;
        }
        //* Get the event message
        ESP_LOGI(TAG, "[ * ] Event received: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
            msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);

        // * If the event is not from the `record button` or the `record button` is not pressed, continue
        if (msg.source_type != PERIPH_ID_BUTTON) {
            continue;
        }
        if ((int)msg.data != get_input_rec_id()) {
            continue;
        }
        // * If the event is from the `record button` and the button is pressed, stop the UDP server and start the ASR
        if (msg.cmd == PERIPH_BUTTON_PRESSED) {
            ESP_LOGI(TAG, "[ * ] Resuming pipeline");
            udp_server_stop_process();
            baidu_sr_start(sr);
        // * If the event is from the `record button` and the button is released, get the text from the ASR
        } else if (msg.cmd == PERIPH_BUTTON_RELEASE || msg.cmd == PERIPH_BUTTON_LONG_RELEASE) {
            ESP_LOGI(TAG, "[ * ] Stop pipeline");
            periph_led_stop(led_handle, get_green_led_gpio());
            // * Get the text from the ASR
            original_text = baidu_sr_stop(sr);
            if (original_text == NULL) {
                ESP_LOGI(TAG, "get text error");
            } else {
                ESP_LOGI(TAG, "Original text = %s", original_text);
                if( original_text[0] == '\0'){
                    ESP_LOGI(TAG, "get empty text");
                    free(original_text);
                    original_text = NULL;
                }
            }
            break;
        }
    }
    return original_text;
}

