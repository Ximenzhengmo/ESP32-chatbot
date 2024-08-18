#include "tts.h"
#include "asr.h"
#include "gpt_ask.h"
#include "udp_server.h"

static const char *TAG = "MAIN";

/**
 * @brief   Main task, a infinite loop    
 */
void task(){
    // * Initialize ASR, GPT ASK and TTS
    asr_init();
    gpt_ask_init();
    esp_tts_init();
    // * Create UDP server to listen and response to the client
    create_udp_server();
    // * Set the volume of the onboard speaker
    audio_board_get_handle()->audio_hal->audio_codec_set_volume(30);
    // * Read the text "已开机" from the TTS
    tts_read_txt("已开机", false);

    // * Infinite loop to ask question
    while(1){
        esp_log_level_set("*", ESP_LOG_INFO);
        ESP_LOGI(TAG, "please ask question ....................");

        // * block until the user ask a question and get the text
        char* question = asr_get_text();

#ifndef HTTP_DEBUG_URI
/**        
 *         HTTP_DEBUG_URI is not defined, this means the program is running correctly on the board,
 *     so we can use the GPT to get the answer.
 *         If it occurs an error from `ASR` part, you can define the macro `HTTP_DEBUG_URI` in 
 *    `./main/baidu_sr.h` and use a `local server` to have an insight into the raw audio-stream with 
 *    the help of `./python_script/ESP32_http_debug.py`. In this case, the `.py` script will receive 
 *    the audio-stream and send it to the `Baidu ASR` server to get the text in your PC.
 */
        if( question != NULL ){
            // * Post the question to the GPT, get and play the answer audio
            gpt_post_response(question);
        }else{
            // * Something went wrong, read the text "对不起，我没有听清楚，请再说一遍" from the TTS
            tts_read_txt("对不起，我没有听清楚，请再说一遍", false);
            // * Clear the envet queue
            clear_msg();
        }
#endif
    }
    //* Uninitialize ASR, GPT ASK and TTS
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


