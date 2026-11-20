// Board bring-up test: mic level meter on the LED ring.
//
// The two LEDs next to each mic show how much louder that mic is than its own quiet level, so every mic
// lights the same way even if some are quieter than others. Quiet room = dark, talking = medium, tap = full.
//
// Debugger live expressions (index = TDM slot, see the table below):
//   g_sai_halves   counts up ~100 per second while audio arrives (stuck = no capture)
//   g_sai_errors   should stay 0
//   g_mic_p2p[8]   level over the last 50 ms (peak-to-peak, 24-bit counts)
//   g_mic_floor[8] each mic's quiet level. All 8 should be similar; one far off = suspect mic.
// The same numbers are printed on USART3 (J3, 115200 baud) once a second.

#include "board_test.h"
#include "main.h"
#include "tlc59116.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
extern SAI_HandleTypeDef hsai_BlockA1;
extern UART_HandleTypeDef huart3;

#define NUM_SLOTS       8U
#define HALF_FRAMES     480U                        // 10 ms at 48 kHz
#define HALF_WORDS      (HALF_FRAMES * NUM_SLOTS)   // one word = one 32-bit TDM slot
#define METER_PERIOD_MS 50U
#define LEVEL_PER_6DB   36U                         // brightness added for each doubling above the quiet level

// Slot s -> mic name and ring position p (mic at 45*p deg CCW from MK3, component side up).
//   slot:  0    1    2    3    4    5    6    7
//   mic:   MK2  MK1  MK4  MK3  MK6  MK5  MK8  MK7
static const uint8_t  pos_of_slot[NUM_SLOTS]  = {1, 2, 7, 0, 5, 6, 3, 4};
static const char    *name_of_slot[NUM_SLOTS] = {"MK2", "MK1", "MK4", "MK3", "MK6", "MK5", "MK8", "MK7"};

volatile uint32_t g_led_init_fails;
volatile uint32_t g_sai_halves;
volatile uint32_t g_sai_errors;
volatile int32_t  g_mic_p2p[NUM_SLOTS];
volatile int32_t  g_mic_floor[NUM_SLOTS];

static int32_t dma_buf[2U * HALF_WORDS];   // AXI SRAM (.bss), reachable by DMA1
static volatile int32_t run_min[NUM_SLOTS];
static volatile int32_t run_max[NUM_SLOTS];

static void reset_min_max(void)
{
    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        run_min[s] = INT32_MAX;
        run_max[s] = INT32_MIN;
    }
}

// DMA interrupt: track min and max of each slot.
static void scan_half(const int32_t *x)
{
    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        int32_t lo = run_min[s];
        int32_t hi = run_max[s];
        for (uint32_t n = 0; n < HALF_FRAMES; n++) {
            int32_t v = x[n * NUM_SLOTS + s] >> 8;   // 24-bit sample in the top of the 32-bit slot
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        run_min[s] = lo;
        run_max[s] = hi;
    }
    g_sai_halves++;
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) { scan_half(&dma_buf[0]); }
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)     { scan_half(&dma_buf[HALF_WORDS]); }
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)      { g_sai_errors++; }

void board_test_init(void)
{
    while (tlc59116_init(&hi2c1) != HAL_OK) {
        g_led_init_fails++;
        HAL_Delay(500);
    }
    reset_min_max();
    if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)dma_buf, 2U * HALF_WORDS) != HAL_OK) {
        Error_Handler();
    }
    HAL_Delay(200);   // let the ADAU7118 start up before the first reading
}

void board_test_loop(void)
{
    static uint32_t ticks;

    HAL_Delay(METER_PERIOD_MS);

    // Take the last 50 ms of min/max and start a new window.
    __disable_irq();
    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        g_mic_p2p[s] = (run_max[s] > run_min[s]) ? (run_max[s] - run_min[s]) : 0;
    }
    reset_min_max();
    __enable_irq();

    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        int32_t p2p = g_mic_p2p[s];

        // Quiet level: drops straight to any quieter reading, creeps up slowly (~10 s) otherwise.
        int32_t floor = g_mic_floor[s];
        if (floor == 0 || p2p < floor) {
            floor = p2p;
        } else {
            floor += (p2p - floor) >> 8;
        }
        if (floor < 1) {
            floor = 1;
        }
        g_mic_floor[s] = floor;

        // Brightness = number of doublings (6 dB steps) above the quiet level.
        uint32_t ratio = (uint32_t)p2p / (uint32_t)floor;
        uint32_t doublings = (ratio > 1U) ? (31U - __CLZ(ratio)) : 0U;
        uint32_t b = doublings * LEVEL_PER_6DB;
        uint8_t brightness = (b > 255U) ? 255U : (uint8_t)b;

        // LED k is at 281.25 + 22.5*k deg, so the mic at 45*p deg sits between LEDs 2p+3 and 2p+4.
        uint8_t k = (uint8_t)(2U * pos_of_slot[s] + 3U);
        tlc59116_set(&hi2c1, k % 16U, brightness);
        tlc59116_set(&hi2c1, (k + 1U) % 16U, brightness);
    }

    // Once a second: "MK2 p2p/floor ..." for every mic.
    if (++ticks >= 1000U / METER_PERIOD_MS) {
        char line[160];
        int len = 0;
        ticks = 0;
        for (uint32_t s = 0; s < NUM_SLOTS; s++) {
            len += snprintf(&line[len], sizeof(line) - (size_t)len, "%s %ld/%ld  ",
                            name_of_slot[s], (long)g_mic_p2p[s], (long)g_mic_floor[s]);
        }
        len += snprintf(&line[len], sizeof(line) - (size_t)len, "\r\n");
        HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len, 50);
    }
}
