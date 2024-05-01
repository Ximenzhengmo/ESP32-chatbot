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

typedef struct{
    int ans_len;
    char ans[ANS_BUFFER_MAX + 5];
} AnsBuffer;

#define USE_BAIDU

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


void gpt_ask_init();
void gpt_ask_uninit();
esp_err_t gpt_post_response(char* question);
void resTxtState_reset();
void ansBuffer_reset();
void ansBuffer_append(char c);
char ansBuffer_top();
void ansBuffer_rewind();
GPT_resTxtState get_gptResTxtState(GPT_resTxtState state, char c);


#endif