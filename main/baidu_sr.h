/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _BAIDU_SR_H_
#define _BAIDU_SR_H_

#include "esp_err.h"
#include "audio_event_iface.h"
#include "baidu_access_token.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
 *       if it occurs error to ASR, you can open the macro to 
 *   open a debug uri to get the error message from the server.
 *       The debug uri is a http server in your local network. you
 *   can use `python_script/ESP_http_debug.py` to start the server 
 *   for example.
 */
// #define HTTP_DEBUG_URI "http://10.193.41.222:8000/"


#define DEFAULT_SR_BUFFER_SIZE (1024*8)

typedef struct baidu_sr* baidu_sr_handle_t;
typedef void (*baidu_sr_event_handle_t)(baidu_sr_handle_t sr);

/**
 * Baidu ASR configurations
 */
typedef struct {
    const char *api_key;                /*!< API Key */
    const char *secret_key;             /*!< Secert Key*/
    const char *api_token;              /*!< api_token*/
    int record_sample_rates;            /*!< Audio recording sample rate */
    int buffer_size;                    /*!< Processing buffer size */
    baidu_sr_event_handle_t on_begin;  /*!< Begin send audio data to server */
} baidu_sr_config_t;




/**
 * @brief      initialize Baidu ASR, this function will return an ASR context
 *
 * @param      config  The Baidu ASR configuration
 *
 * @return     The ASR context
 */
baidu_sr_handle_t baidu_sr_init(baidu_sr_config_t *config);

/**
 * @brief      Start recording and sending audio to Baidu ASR
 *
 * @param[in]  sr   The ASR context
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t baidu_sr_start(baidu_sr_handle_t sr);

/**
 * @brief      Stop sending audio to Baidu ASR and get the result text
 *
 * @param[in]  sr   The ASR context
 *
 * @return     Baidu ASR server response
 */
char *baidu_sr_stop(baidu_sr_handle_t sr);

/**
 * @brief      Cleanup the ASR object
 *
 * @param[in]  sr   The ASR context
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_sr_destroy(baidu_sr_handle_t sr);

/**
 * @brief      Register listener for the ASR context
 *
 * @param[in]   sr   The ASR context
 * @param[in]  listener  The listener
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_sr_set_listener(baidu_sr_handle_t sr, audio_event_iface_handle_t listener);


/**
 * @brief      Reset the token of Baidu ASR
 *
 * @param[in]  sr    The ASR context
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_sr_reset_token(baidu_sr_handle_t sr);


#ifdef __cplusplus
}
#endif

#endif
