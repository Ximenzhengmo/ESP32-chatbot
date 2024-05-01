#include "tts.h"
#include "asr.h"
#include "gpt_ask.h"
#include "udp_server.h"

static const char *TAG = "MAIN";

void task(){
    asr_init();
    gpt_ask_init();
    esp_tts_init();
    create_udp_server();
    audio_board_get_handle()->audio_hal->audio_codec_set_volume(30);
    tts_read_txt("已开机", false);
    while(1){
        esp_log_level_set("*", ESP_LOG_INFO);
        ESP_LOGI(TAG, "please ask a question ....................");
        char* question = asr_get_text();
#ifndef HTTP_DEBUG_URI
        if( question != NULL ){
            gpt_post_response(question);
        }else{
            tts_read_txt("对不起，我没有听清楚，请再说一遍", false);
            clear_msg();
        }
#endif
    }
    esp_tts_uninit();
    gpt_ask_uninit();
    asr_uninit();
    vTaskDelete(NULL);
}


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    xTaskCreate(task, "task", 2 * 4096, NULL, 6, NULL);
}


