/*
 * app_main.c
 */
#include "app_main.h"
#include "main.h"
#include "mic_capture.h"
#include "stream.h"
#include "doa.h"

#define BLINK_BLOCKS    500U    // toggle the green LED every 500 blocks = 0.5 s

void app_init(void)
{
	if (doa_init() != 0) {
		Error_Handler();
	}
    if (mic_capture_start() != 0) {
        Error_Handler();        // red LED on
    }
}

void app_loop(void)
{
	static uint32_t last_blink;

	stream_task();
	doa_task();

    if (g_cb_count - last_blink >= BLINK_BLOCKS) {    // green LED blinks while capture runs
        last_blink = g_cb_count;
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
}
