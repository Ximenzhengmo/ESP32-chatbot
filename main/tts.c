#include "tts.h"

static const char* TAG = "TTS";

xQueueHandle qhandle = NULL;
esp_tts_voice_t* voice = NULL;
spi_flash_mmap_handle_t mmap;
TaskHandle_t tts_receive_task_handle = NULL;
#define QUEUE_SIZE 100
#define QUEUE_ITEM_SIZE sizeof(char*)

void esp_tts_init(){
    const esp_partition_t* part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "voice_data");
    AUDIO_MEM_CHECK(TAG, part, {
        ESP_LOGE(TAG, "Couldn't find voice data partition!");
        return;
    });
    uint16_t* voicedata = NULL;
    esp_err_t err = esp_partition_mmap(part, 0, 3 * 1024 * 1024, SPI_FLASH_MMAP_DATA, (const void**)&voicedata, &mmap);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't map voice data partition!");
        return;
    }
    voice = esp_tts_voice_set_init(&esp_tts_voice_template, voicedata);
    AUDIO_MEM_CHECK(TAG, voice, {
        ESP_LOGE(TAG, "Couldn't init tts voice set!");
        return;
    });
}

void esp_tts_uninit(){
    esp_tts_voice_set_free(voice);
    spi_flash_munmap(mmap);
}

void esp_tts_task_create() {
    if (qhandle != NULL) {
        xQueueReset(qhandle);
    } else {
        qhandle = xQueueCreate(QUEUE_SIZE, QUEUE_ITEM_SIZE);
    }
    xTaskCreate(esp_tts_receive_task, "tts2", 2 * 4096, NULL, 5, &tts_receive_task_handle);
}

void esp_tts_task_destory() {
    while ( is_esp_tts_receive_task_working() == ESP_OK ) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    tts_receive_task_handle = NULL;
    ESP_LOGI(TAG, "esp tts task finished");
}

esp_err_t is_esp_tts_receive_task_working() {
    if( tts_receive_task_handle == NULL ){
        return ESP_FAIL;
    }else{
        eTaskState state = eTaskGetState(tts_receive_task_handle);
        ESP_LOGW(TAG, "tts receive task state: %d", state);
        if( state <= eSuspended )
            return ESP_OK;
        else
            return ESP_FAIL;
    }
}

void esp_tts_send_txt(char* txt) {
    if( xQueueSend(qhandle, &txt, 500 / portTICK_PERIOD_MS) == pdTRUE ){
        ESP_LOGW(TAG, "Send txt success");
    }else {
        ESP_LOGE(TAG, "Send txt failed, queue length = %d", uxQueueMessagesWaiting(qhandle));
        free(txt);
    }
}

esp_err_t tts_read_txt(char* txt, bool if_free_txt) {
    int len;
    esp_tts_handle_t tts_handle = esp_tts_create(voice);
    if (esp_tts_parse_chinese(tts_handle, txt)) {
        ESP_LOGW(TAG, "%s", txt);
        if ( if_free_txt ) free(txt);
    } else {
        ESP_LOGE(TAG, "The Chinese string parse failed, txt: %s", txt);
        if ( if_free_txt ) free(txt);
        return ESP_FAIL;
    }
    while (1) {
        uint8_t* pcm_data = (uint8_t*)esp_tts_stream_play(tts_handle, &len, TTS_VOICE_SPEED_3);
        if (len == 0) {
            ESP_LOGI(TAG, "End of stream");
            vTaskDelay(10 / portTICK_PERIOD_MS);
            break;
        }
        len <<= 1;
        while (len) {
            size_t i2s_bytes_write = 0;
            i2s_write(I2S_NUM_0, pcm_data, len, &i2s_bytes_write, 1 / portTICK_RATE_MS);
            len -= i2s_bytes_write;
            pcm_data += i2s_bytes_write;
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    esp_tts_destroy(tts_handle);
    return ESP_OK;
}


void esp_tts_receive_task(void* param) {
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    while (1) {
        char* txt = NULL;
        if (xQueueReceive(qhandle, &txt, 3000 / portTICK_PERIOD_MS) == pdTRUE) {
            ESP_LOGI(TAG, "Get txt, queue length = %d", uxQueueMessagesWaiting(qhandle));
        } else {
            ESP_LOGE(TAG, "Get txt handle failed");
            tts_receive_task_handle = NULL;
            vTaskDelete(NULL);
            break;
        }
        if ( tts_read_txt(txt, true) == ESP_FAIL ){
            continue;
        }
    }

    ESP_LOGI(TAG, "tts receive task finished");
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    tts_receive_task_handle = NULL;
    vTaskDelete(NULL);
}