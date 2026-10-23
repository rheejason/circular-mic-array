/*
 * mic_capture.c
 */
#include "mic_capture.h"
#include "array_config.h"
#include "sai.h"
#include "pdm2pcm_glo.h"
#include "stream.h"
#include "doa.h"
#include "beamformer.h"

#define PDM_GAIN_DB     0
#define PDM_HPF_TAP     2122358088U             // high-pass that removes DC (ST's value)

#define HALF_WORDS      (PDM_BLOCK_BYTES / 2U)  // one block, in 16-bit DMA words
#define BUF_WORDS       (2U * HALF_WORDS)       // DMA fills one half while we filter the other

/* Debug values */
volatile uint32_t g_cb_count;
volatile uint32_t g_proc_cycles;
volatile uint32_t g_sai_err_count;

/* Buffers */
static uint16_t pdm_buf[BUF_WORDS] __attribute__((aligned(32)));    // DMA target (AXI SRAM, not DTCM)
static int16_t  pcm_block[BLOCK_SAMPLES * NUM_MICS];                // 1 ms of audio: m0 m1 m2 m3 m0 ...
static int16_t  beam_block[BLOCK_SAMPLES];                          // 1 ms of beam output (mono)

static PDM_Filter_Handler_t filter[NUM_MICS];
static PDM_Filter_Config_t  filter_cfg[NUM_MICS];


/* Private functions */

static void cycle_counter_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;          // unlock (needed on Cortex-M7)
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static int filters_init(void)       // needs the CRC clock on (MX_CRC_Init)
{
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        filter[m].bit_order        = PDM_FILTER_BIT_ORDER_MSB;
        filter[m].endianness       = PDM_FILTER_ENDIANNESS_LE;
        filter[m].high_pass_tap    = PDM_HPF_TAP;
        filter[m].in_ptr_channels  = NUM_MICS;      // read every 4th byte
        filter[m].out_ptr_channels = NUM_MICS;      // write every 4th sample
        if (PDM_Filter_Init(&filter[m]) != 0) {
            return -1;
        }

        filter_cfg[m].decimation_factor     = PDM_FILTER_DEC_FACTOR_32;    // must match PDM_DECIMATION
        filter_cfg[m].output_samples_number = BLOCK_SAMPLES;
        filter_cfg[m].mic_gain              = PDM_GAIN_DB;
        if (PDM_Filter_setConfig(&filter[m], &filter_cfg[m]) != 0) {
            return -1;
        }
    }
    return 0;
}

static void process_block(uint16_t *half)   // runs in the DMA interrupt, once per 1 ms
{
    uint32_t start = DWT->CYCCNT;
    uint8_t *pdm = (uint8_t *)half;

    // DMA wrote this half straight to RAM; drop any stale cached copy before reading it
    // (half is 32-byte aligned and 768 bytes = 24 cache lines)
    SCB_InvalidateDCache_by_Addr((uint32_t *)half, PDM_BLOCK_BYTES);

    // Bytes arrive as mic 1, 0, 3, 2 (SEL=3V3 mic first on each line); m ^ 1 puts them in order
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        PDM_Filter(&pdm[m ^ 1U], &pcm_block[m], &filter[m]);
    }

    beam_push(pcm_block);                   // feed the beamformer
    beam_pop(beam_block);                   // 1 ms of its output (~20 ms behind the mics)
    stream_push(pcm_block, beam_block);
    doa_push(pcm_block);

    g_cb_count++;
    g_proc_cycles = DWT->CYCCNT - start;
}


/* Public functions */

int mic_capture_start(void)
{
    cycle_counter_init();

    if (filters_init() != 0) {
        return -1;
    }

    // Circular DMA: refills pdm_buf forever. Size is in 16-bit words.
    if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)pdm_buf, BUF_WORDS) != HAL_OK) {
        return -2;
    }
    return 0;
}


/* HAL callbacks (replace the HAL's empty weak versions) */

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)   // first half of pdm_buf is full
{
    if (hsai->Instance == SAI1_Block_A) {
        process_block(&pdm_buf[0]);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)       // second half of pdm_buf is full
{
    if (hsai->Instance == SAI1_Block_A) {
        process_block(&pdm_buf[HALF_WORDS]);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)        // e.g. overrun
{
    if (hsai->Instance == SAI1_Block_A) {
        g_sai_err_count++;
    }
}
