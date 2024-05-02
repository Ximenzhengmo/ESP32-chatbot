#ifndef _TTS_H_
#define _TTS_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "tts_stream.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "board.h"
#include "esp_partition.h"
#include "esp_tts_voice.h"
#include "esp_tts_voice_template.h"


/**
 * @brief      Initialize TTS
 */
void esp_tts_init();

/**
 * @brief      Uninitialize TTS
 */
void esp_tts_uninit();

/**
 * @brief      Create TTS task
 */
void esp_tts_task_create();

/**
 * @brief      Destory TTS task
 */
void esp_tts_task_destory();

/**
 * @brief      Send text to TTS queue
 *
 * @param[in]  txt   The text
 */
void esp_tts_send_txt(char* txt);

/**
 * @brief      TTS receive task, receive text from queue and play it
 *
 * @param      param  The parameter
 */
void esp_tts_receive_task(void* param);

/**
 * @brief      Check if TTS receive task is working
 *
 * @return     ESP_OK if working, otherwise ESP_FAIL
 */
esp_err_t is_esp_tts_receive_task_working();

/**
 * @brief      parse the text and play it
 *
 * @param      txt          The text
 * @param[in]  if_free_txt  If free text
 *
 * @return     ESP_OK if success, otherwise ESP_FAIL
 */
esp_err_t tts_read_txt(char* txt, bool if_free_txt);
#endif