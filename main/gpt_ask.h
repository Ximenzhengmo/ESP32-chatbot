#ifndef _GPT_ASK_H_
#define _GPT_ASK_H_

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "tts.h"
#include "baidu_access_token.h"

#define RAW_RESPONSE_BUFFER_MAX 2048
#define ANS_BUFFER_MAX 1024

/*
 * @brief      GPT answer buffer
 */
typedef struct{
    int ans_len;
    char ans[ANS_BUFFER_MAX + 5];
} AnsBuffer;

#define USE_BAIDU


/*
 * @brief      GPT response text state
 */
typedef enum{
    START,
    FOUND_quotation,    // "
    FOUND_r,            // "r
    FOUND_e,            // "re
    FOUND_s,            // "res
    FOUND_u,            // "resu
    FOUND_l,            // "resul
    FOUND_t,            // "result
    FOUND_quotation2,   // "result"
    FOUND_colon,        // "result":
    FOUND_quotation3,   // "result":"
    ACCEPT
}GPT_resTxtState;

/*
 * @brief      Initialize GPT ASK
 */
void gpt_ask_init();

/*
 * @brief      Uninitialize GPT ASK
 */
void gpt_ask_uninit();

/*
 * @brief      Post a question to GPT
 *
 * @param      question  The question
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t gpt_post_response(char* question);

/*
 * @brief      Reset the GPT response text state
 */
void resTxtState_reset();

/*
 * @brief      Reset the GPT answer buffer
 */
void ansBuffer_reset();

/*
 * @brief      Append a character to the GPT answer buffer
 *
 * @param[in]  c     The character (1 Byte)
 */
void ansBuffer_append(char c);

/*
 * @brief      Get the top character of the GPT answer buffer
 *
 * @return     The top character
 */
char ansBuffer_top();

/*
 * @brief      Pop the top character of the GPT answer buffer
 */
void ansBuffer_rewind();

/*
 * @brief      renew the GPT response text state
 *
 * @param[in]  state  The current state
 * @param[in]  c      The new character
 *
 * @return     The next state
 */
GPT_resTxtState get_gptResTxtState(GPT_resTxtState state, char c);


#endif