#ifndef _AUDIO_BOARD_DEFINITION_H_
#define _AUDIO_BOARD_DEFINITION_H_

#include "driver/touch_pad.h"

/**
 * @brief SDCARD Function Definition
 */
#define FUNC_SDCARD_EN            (1)
#define SDCARD_OPEN_FILE_NUM_MAX  5
#define SDCARD_INTR_GPIO          GPIO_NUM_34

#define ESP_SD_PIN_CLK            (-1)
#define ESP_SD_PIN_CMD            (-1)
#define ESP_SD_PIN_D0             (-1)
#define ESP_SD_PIN_D1             (-1)
#define ESP_SD_PIN_D2             (-1)
#define ESP_SD_PIN_D3             (-1)


/**
 * @brief LED Function Definition
 */
#define FUNC_SYS_LEN_EN           (1)
#define GREEN_LED_GPIO            GPIO_NUM_22


/**
 * @brief Audio Codec Chip Function Definition
 */
#define FUNC_AUDIO_CODEC_EN       (1)
#define AUXIN_DETECT_GPIO         GPIO_NUM_12
#define HEADPHONE_DETECT          GPIO_NUM_19
#define PA_ENABLE_GPIO            GPIO_NUM_27
#define CODEC_ADC_I2S_PORT        (0)
#define CODEC_ADC_BITS_PER_SAMPLE (16) /* 16bit */
#define CODEC_ADC_SAMPLE_RATE     (48000)
#define RECORD_HARDWARE_AEC       (false)
#define BOARD_PA_GAIN             (-24) /* Power amplifier gain defined by board (dB) */

extern audio_hal_func_t AUDIO_CODEC_ES8388_DEFAULT_HANDLE;
#define AUDIO_CODEC_DEFAULT_CONFIG(){                   \
        .adc_input  = AUDIO_HAL_ADC_INPUT_LINE1,        \
        .dac_output = AUDIO_HAL_DAC_OUTPUT_ALL,         \
        .codec_mode = AUDIO_HAL_CODEC_MODE_BOTH,        \
        .i2s_iface = {                                  \
            .mode = AUDIO_HAL_MODE_SLAVE,               \
            .fmt = AUDIO_HAL_I2S_NORMAL,                \
            .samples = AUDIO_HAL_48K_SAMPLES,           \
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,        \
        },                                              \
};


/**
 * @brief Button Function Definition
 */
#define FUNC_BUTTON_EN            (1)
#define INPUT_KEY_NUM             1
#define BUTTON_REC_ID             GPIO_NUM_2
#define BUTTON_MODE_ID            (-1)
#define BUTTON_SPEAK_ID           GPIO_NUM_15

#define BUTTON_SET_ID             (-1) // TOUCH_PAD_NUM9
#define BUTTON_PLAY_ID            (-1) // TOUCH_PAD_NUM8
#define BUTTON_VOLUP_ID           (-1) // TOUCH_PAD_NUM7
#define BUTTON_VOLDOWN_ID         (-1) // TOUCH_PAD_NUM4

#define INPUT_KEY_DEFAULT_INFO() {                      \
     {                                                  \
        .type = PERIPH_ID_BUTTON,                       \
        .user_id = INPUT_KEY_USER_ID_REC,               \
        .act_id = BUTTON_REC_ID,                        \
    },                                                  \
}

int8_t get_input_speak_id(void);

#endif
