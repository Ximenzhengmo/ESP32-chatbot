#ifndef _ASR_H_
#define _ASR_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "i2s_stream.h"

#include "esp_http_client.h"
#include "sdkconfig.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "esp_peripherals.h"
#include "periph_button.h"
#include "periph_wifi.h"
#include "periph_led.h"
#include "baidu_sr.h"
#include "board.h"
#include "tts_stream.h"

#include "audio_idf_version.h"

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
#include "esp_netif.h"
#else
#include "tcpip_adapter.h"
#endif

#include "udp_server.h"

#define ASR_SAMPLE_RATE (16000)

#if CONFIG_WIFI_WPA2_ENTERPRISE_ENABLE
#define WIFI_CONFIG_DEFAULT() {                         \
        .wifi_config = {                                \
            .sta = {                                    \
                .ssid = CONFIG_WIFI_SSID,               \
            },                                          \
        },                                              \
        .wpa2_e_cfg = {                                 \
            .eap_id = CONFIG_WAP2_ENTERPRISE_ID,        \
            .diasble_wpa2_e = true,                     \
            .eap_method = 1,  /* PEAP */                \
            .eap_username = CONFIG_WAP2_ENTERPRISE_ID,  \
            .eap_password = CONFIG_WIFI_PASSWORD,       \
        },                                              \
    }
#else
#define WIFI_CONFIG_DEFAULT() {                         \
        .wifi_config = {                                \
            .sta = {                                    \
                .ssid = CONFIG_WIFI_SSID,               \
                .password = CONFIG_WIFI_PASSWORD,       \
            },                                          \
        },                                              \
    }
#endif


/*
 * @brief Initialize ASR
 */
void asr_init();

/*
 * @brief Uninitialize ASR
 */
void asr_uninit();

/*
 * @brief Get the text from ASR
 *
 * @return The text from ASR
 *          - NULL if failed
 */
char* asr_get_text();

/*
 * @brief Clear the msg from the aduio-pipeline listener
 *          in case of the multiple processing of the same msg
 */
void clear_msg();

#endif