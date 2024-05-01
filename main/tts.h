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
#include "asr.h"

void esp_tts_init();
void esp_tts_uninit();
void esp_tts_task_create();
void esp_tts_task_destory();
void esp_tts_send_txt(char* txt);
void esp_tts_receive_task(void* param);
esp_err_t is_esp_tts_receive_task_working();
esp_err_t tts_read_txt(char* txt, bool if_free_txt);
#endif