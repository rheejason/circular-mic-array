/*
 * app_main.c
 * Starts capture, then handles the button and LEDs.
 *
 *   Blue button   start a recording
 *   Green LED     blinks while capture is running
 *   Yellow LED    on while recording
 *   Red LED       error (Error_Handler in main.c)
 */
#include "app_main.h"
#include "main.h"
#include "mic_capture.h"
#include "recorder.h"

#define BLINK_BLOCKS    500U    /* toggle the green LED every 500 blocks = 0.5 s */


/* Start a recording when the button is pressed (on the press, not while held) */
static void check_button(void)
{
    static GPIO_PinState last = GPIO_PIN_RESET;
    GPIO_PinState now = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

    if (now == GPIO_PIN_SET && last == GPIO_PIN_RESET && g_rec_state != REC_RECORDING) {
        recorder_start();
    }
    last = now;
}

static void update_leds(void)
{
    static uint32_t last_blink;

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                      (g_rec_state == REC_RECORDING) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (g_cb_count - last_blink >= BLINK_BLOCKS) {
        last_blink = g_cb_count;
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
}


void app_init(void)
{
    if (mic_capture_start() != 0) {
        Error_Handler();
    }
}

void app_loop(void)
{
    check_button();
    update_leds();
}
