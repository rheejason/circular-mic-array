/*
 * app_main.c
 *
 * LEDs:
 *   LD1 green   blinks ~1 Hz while capture is running (toggles every 500 blocks)
 *   LD2 yellow  on while recording; goes off when rec_buf is full
 *   LD3 red     on = Error_Handler() was hit
 * Button B1 (blue): start a new recording.
 */
#include "app_main.h"
#include "main.h"
#include "mic_capture.h"
#include "recorder.h"

#define HEARTBEAT_BLOCKS    500U

void app_init(void)
{
    if (mic_capture_start() != 0) {
        Error_Handler();
    }
}

void app_loop(void)
{
    static GPIO_PinState last_btn = GPIO_PIN_RESET;
    static uint32_t last_beat;

    /* B1 reads high while pressed. Act on the press edge only; bounces during a recording are ignored. */
    GPIO_PinState btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
    if (btn == GPIO_PIN_SET && last_btn == GPIO_PIN_RESET && g_rec_state != REC_RECORDING) {
        recorder_arm();
    }
    last_btn = btn;

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                      (g_rec_state == REC_RECORDING) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (g_cb_count - last_beat >= HEARTBEAT_BLOCKS) {
        last_beat = g_cb_count;
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
}
