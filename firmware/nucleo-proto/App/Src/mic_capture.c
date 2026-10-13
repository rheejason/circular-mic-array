/*
 * mic_capture.c
 *
 * How the data flows:
 *   - SAI1 clocks all 4 mics at 3.072 MHz (PE2) and reads 2 data lines (PE6 = mics 0/1, PE4 = mics 2/3).
 *   - The SAI de-interleaves the PDM bits and DMA writes them into g_pdm_buf, byte-interleaved by mic:
 *     byte 0 = mic 0, byte 1 = mic 1, byte 2 = mic 2, byte 3 = mic 3, byte 4 = mic 0, ...
 *   - g_pdm_buf holds two 1 ms halves. DMA fills one half while we filter the other (ping-pong).
 *     The HAL calls RxHalfCplt when the first half is full and RxCplt when the second half is full.
 *   - Each mic has its own PDM2PCM filter. in_ptr_channels = 4 makes the filter read every 4th byte;
 *     out_ptr_channels = 4 makes it write every 4th sample, so the output block comes out interleaved.
 */
#include "mic_capture.h"
#include "recorder.h"
#include "sai.h"
#include "pdm2pcm_glo.h"

/* Set to 0 for the first bring-up test (DMA only: g_cb_count should climb, g_pdm_buf should change) */
#define RUN_PDM_FILTER      1

/* If one scratch shows on two meters or the order comes out 1,0,3,2, try PDM_FILTER_ENDIANNESS_BE */
#define PDM_BIT_ORDER       PDM_FILTER_BIT_ORDER_MSB
#define PDM_ENDIANNESS      PDM_FILTER_ENDIANNESS_LE
#define PDM_MIC_GAIN_DB     24          /* lower this if g_peak sits at 32767 */
#define PDM_HPF_TAP         2122358088U /* DC-blocking high-pass, ST example value */

#define PEAK_HOLD_BLOCKS    100U        /* g_peak window: 100 blocks = 100 ms */

/* DMA buffer in 16-bit words (SAI data size is 16 bit): two halves of PDM_BLOCK_BYTES bytes each */
#define PDM_HALF_WORDS      (PDM_BLOCK_BYTES / 2U)    /* 768 */
#define PDM_BUF_WORDS       (2U * PDM_HALF_WORDS)     /* 1536 */

volatile uint32_t g_cb_count;
volatile int16_t  g_peak[NUM_MICS];
volatile uint32_t g_proc_cycles;
volatile uint32_t g_sai_err_count;

/* Global (not static) so it can be watched in Live Expressions. Lands in AXI SRAM, which DMA1 can reach. */
uint16_t g_pdm_buf[PDM_BUF_WORDS] __attribute__((aligned(32)));

static PDM_Filter_Handler_t pdm_handle[NUM_MICS];
static PDM_Filter_Config_t  pdm_config[NUM_MICS];
static int16_t pcm_block[BLOCK_SAMPLES * NUM_MICS];

static int16_t  peak_acc[NUM_MICS];
static uint32_t peak_blocks;

static void cycle_counter_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;              /* unlock DWT (needed on Cortex-M7) */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void update_peaks(void)
{
    for (uint32_t n = 0; n < BLOCK_SAMPLES; n++) {
        for (uint32_t m = 0; m < NUM_MICS; m++) {
            int32_t s = pcm_block[n * NUM_MICS + m];
            if (s < 0) s = -s;
            if (s > 32767) s = 32767;
            if (s > peak_acc[m]) peak_acc[m] = (int16_t)s;
        }
    }

    if (++peak_blocks >= PEAK_HOLD_BLOCKS) {
        for (uint32_t m = 0; m < NUM_MICS; m++) {
            g_peak[m] = peak_acc[m];
            peak_acc[m] = 0;
        }
        peak_blocks = 0;
    }
}

/* Runs in the DMA interrupt, once per 1 ms block */
static void process_half(uint16_t *half)
{
    uint32_t t0 = DWT->CYCCNT;

#if RUN_PDM_FILTER
    uint8_t *pdm = (uint8_t *)half;
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        PDM_Filter(&pdm[m], &pcm_block[m], &pdm_handle[m]);
    }
    update_peaks();
    recorder_push(pcm_block);
#else
    (void)half;
#endif

    g_cb_count++;
    g_proc_cycles = DWT->CYCCNT - t0;
}

int mic_capture_start(void)
{
    cycle_counter_init();

    /* The PDM library needs the CRC peripheral clocked; MX_CRC_Init() runs before this */
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        pdm_handle[m].bit_order        = PDM_BIT_ORDER;
        pdm_handle[m].endianness       = PDM_ENDIANNESS;
        pdm_handle[m].high_pass_tap    = PDM_HPF_TAP;
        pdm_handle[m].in_ptr_channels  = NUM_MICS;
        pdm_handle[m].out_ptr_channels = NUM_MICS;
        if (PDM_Filter_Init(&pdm_handle[m]) != 0) {
            return -1;
        }

        pdm_config[m].decimation_factor     = PDM_FILTER_DEC_FACTOR_64;
        pdm_config[m].output_samples_number = BLOCK_SAMPLES;
        pdm_config[m].mic_gain              = PDM_MIC_GAIN_DB;
        if (PDM_Filter_setConfig(&pdm_handle[m], &pdm_config[m]) != 0) {
            return -2;
        }
    }

    /* Size is counted in 16-bit data items, not bytes. Circular DMA: runs forever from here. */
    if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)g_pdm_buf, PDM_BUF_WORDS) != HAL_OK) {
        return -3;
    }
    return 0;
}

/* Override the HAL's empty __weak callbacks */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        process_half(&g_pdm_buf[0]);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        process_half(&g_pdm_buf[PDM_HALF_WORDS]);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        g_sai_err_count++;
    }
}
