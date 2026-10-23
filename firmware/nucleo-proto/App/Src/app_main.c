/*
 * app_main.c
 */
#include "app_main.h"
#include "main.h"
#include "mic_capture.h"
#include "stream.h"
#include "doa.h"
#include "beamformer.h"

#define BLINK_BLOCKS    500U    // toggle the green LED every 500 blocks = 0.5 s

volatile uint32_t g_beam_follow_doa;	// set to 1 in Live Expressions: beam points where the DOA says

void app_init(void)
{
	// D-cache: our DSP buffers live in AXI SRAM, which is slow uncached. The only DMA buffer
	// (pdm_buf) is invalidated before each read in mic_capture.c; USB FS uses no DMA.
	SCB_EnableDCache();

	if (doa_init() != 0 || beam_init() != 0) {
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
	if (g_beam_follow_doa && g_doa_valid) {
		g_beam_az = g_doa_az;
	}
	beam_task();

    if (g_cb_count - last_blink >= BLINK_BLOCKS) {    // green LED blinks while capture runs
        last_blink = g_cb_count;
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
}
