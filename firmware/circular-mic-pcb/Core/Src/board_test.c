// Board bring-up test. No PC needed: the LED ring is the display. Repeats forever:
//
// 1. LED check (~8 s), does not depend on sound:
//      a. All 16 LEDs fully on for 3 s. Any LED that is dark here needs fixing.
//      b. One LED at a time, 300 ms each, OUT0 -> OUT15. Exactly one LED should be lit at each step.
//         Two lit at once = short between neighbouring pins. g_led_now holds the D number that should be lit.
// 2. Mic meter (15 s): the two LEDs next to each mic show that mic's level. Quiet room = dark.
//      - Tap or scratch next to one mic: only the LEDs beside it should flash bright.
//      - A dead mic stays dark when tapped, and its g_mic_ok bit stays 0.
//
// Also printed on USART3 (J3, 115200 baud), and visible in the debugger (globals below).
// All "slot" indices are TDM slots 0..7 = ADAU7118 channels 0..7.

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
#define ALIVE_P2P       50                          // a live mic's own noise is a few hundred counts
#define MIC_METER_MS    15000U
#define ALL_ON_MS       3000U
#define ONE_LED_MS      300U

// Driver output OUTk drives LED D(d_of_out[k]) (from the schematic; OUTk and Dk are not the same number).
static const uint8_t d_of_out[TLC59116_NUM_LEDS] = {1, 2, 3, 4, 5, 7, 6, 8, 9, 13, 10, 14, 11, 15, 12, 16};

// Slot -> board position. MK3 is at 0 deg, angles counter-clockwise, component side up
// (from firmware/final/README.md; the tap test confirms it).
//   slot:     0    1    2    3    4    5    6    7
//   mic:      MK2  MK1  MK4  MK3  MK6  MK5  MK8  MK7
static const uint8_t  pos_of_slot[NUM_SLOTS] = {1, 2, 7, 0, 5, 6, 3, 4};   // position p = angle 45*p deg
static const char    *name_of_slot[NUM_SLOTS] = {"MK2", "MK1", "MK4", "MK3", "MK6", "MK5", "MK8", "MK7"};

// Debugger watch list.
volatile uint32_t g_led_init_fails;       // counts up while the TLC59116F does not answer on I2C
volatile uint32_t g_led_i2c_errors;       // I2C writes to the driver that failed after start-up (should stay 0)
volatile uint8_t  g_led_now;              // during the one-at-a-time step: D number that should be lit (0 = none)
volatile uint32_t g_sai_halves;           // counts up every 10 ms while audio is arriving
volatile uint32_t g_sai_errors;           // SAI errors (overrun etc.)
volatile int32_t  g_mic_p2p[NUM_SLOTS];   // peak-to-peak level per slot over the last 50 ms, 24-bit counts
volatile uint8_t  g_mic_ok;               // bit s = 1 once slot s has shown a signal

static int32_t dma_buf[2U * HALF_WORDS];  // in AXI SRAM (.bss), which DMA1 can reach
static volatile int32_t run_min[NUM_SLOTS];
static volatile int32_t run_max[NUM_SLOTS];

// Runs in the DMA interrupt: track min and max of each slot.
static void scan_half(const int32_t *x)
{
    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        int32_t lo = run_min[s];
        int32_t hi = run_max[s];
        for (uint32_t n = 0; n < HALF_FRAMES; n++) {
            int32_t v = x[n * NUM_SLOTS + s] >> 8;   // 24-bit sample sits in the top of the 32-bit slot
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

static void reset_min_max(void)
{
    for (uint32_t s = 0; s < NUM_SLOTS; s++) {
        run_min[s] = INT32_MAX;
        run_max[s] = INT32_MIN;
    }
}

static void uart_print(const char *text, int len)
{
    HAL_UART_Transmit(&huart3, (const uint8_t *)text, (uint16_t)len, 50);
}

static void led_set(uint8_t k, uint8_t brightness)
{
    if (tlc59116_set(&hi2c1, k, brightness) != HAL_OK) {
        g_led_i2c_errors++;
    }
}

static void led_set_all(uint8_t brightness)
{
    for (uint8_t k = 0; k < TLC59116_NUM_LEDS; k++) {
        led_set(k, brightness);
    }
}

static void led_check(void)
{
    char line[48];
    int len;

    len = snprintf(line, sizeof(line), "\r\nLED check: all on\r\n");
    uart_print(line, len);
    led_set_all(255);
    HAL_Delay(ALL_ON_MS);
    led_set_all(0);
    HAL_Delay(ONE_LED_MS);

    for (uint8_t k = 0; k < TLC59116_NUM_LEDS; k++) {
        g_led_now = d_of_out[k];
        len = snprintf(line, sizeof(line), "OUT%u -> D%u\r\n", k, d_of_out[k]);
        uart_print(line, len);
        led_set(k, 255);
        HAL_Delay(ONE_LED_MS);
        led_set(k, 0);
    }
    g_led_now = 0;

    len = snprintf(line, sizeof(line), "Mic meter (I2C errors so far: %lu)\r\n", (unsigned long)g_led_i2c_errors);
    uart_print(line, len);
}

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
}

static void mic_meter_step(void)
{
    static uint32_t ticks;
    char line[128];
    int len = 0;

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
        if (p2p > ALIVE_P2P) {
            g_mic_ok |= (uint8_t)(1U << s);
        }

        // Quiet room ~ off, speech ~ medium, tap = full.
        int32_t b = p2p >> 8;
        uint8_t brightness = (b > 255) ? 255U : (uint8_t)b;

        // LED k is at 281.25 + 22.5*k deg, so the mic at 45*p deg sits between LEDs 2p+3 and 2p+4.
        uint8_t k = (uint8_t)(2U * pos_of_slot[s] + 3U);
        led_set(k % 16U, brightness);
        led_set((k + 1U) % 16U, brightness);
    }

    // Once a second: one line with every mic's level.
    if (++ticks >= 1000U / METER_PERIOD_MS) {
        ticks = 0;
        for (uint32_t s = 0; s < NUM_SLOTS; s++) {
            len += snprintf(&line[len], sizeof(line) - (size_t)len, "%s %7ld  ", name_of_slot[s], (long)g_mic_p2p[s]);
        }
        len += snprintf(&line[len], sizeof(line) - (size_t)len, "ok=0x%02X\r\n", g_mic_ok);
        uart_print(line, len);
    }
}

void board_test_loop(void)
{
    led_check();

    __disable_irq();
    reset_min_max();   // drop whatever piled up during the LED check
    __enable_irq();

    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < MIC_METER_MS) {
        mic_meter_step();
    }
    led_set_all(0);
}
