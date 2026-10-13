/*
 * mic_capture.c
 *
 * SAI1 clocks all 4 mics on PE2 and reads two data lines: PE6 (mics 0, 1) and PE4 (mics 2, 3).
 * DMA copies the PDM bits into g_pdm_buf, one byte per mic at a time.
 * g_pdm_buf has two halves of 1 ms each: DMA fills one half while we filter the other.
 */
#include "mic_capture.h"
#include "recorder.h"
#include "sai.h"
#include "pdm2pcm_glo.h"

#define PDM_GAIN_DB         0
#define PDM_HPF_TAP         2122358088U     /* high-pass that removes DC (ST's value) */
#define PEAK_BLOCKS         100U            /* g_peak updates every 100 blocks = 100 ms */

/* DMA buffer size in 16-bit words: two halves, one block each */
#define HALF_WORDS          (PDM_BLOCK_BYTES / 2U)
#define BUF_WORDS           (2U * HALF_WORDS)

_Static_assert(PDM_DECIMATION == 32U, "also change PDM_FILTER_DEC_FACTOR_32 below");

/* Debug values */
volatile uint32_t g_cb_count;
volatile int16_t  g_peak[NUM_MICS];
volatile uint32_t g_proc_cycles;
volatile uint32_t g_sai_err_count;

/* DMA buffer. Must not be in DTCM (DMA1 can't reach it); the linker puts it in AXI SRAM. */
uint16_t g_pdm_buf[BUF_WORDS] __attribute__((aligned(32)));

static PDM_Filter_Handler_t filter[NUM_MICS];
static PDM_Filter_Config_t  filter_cfg[NUM_MICS];
static int16_t  pcm_block[BLOCK_SAMPLES * NUM_MICS];
static int16_t  peak_hold[NUM_MICS];
static uint32_t peak_count;


/* ---------- Private functions ---------- */

/* Start the CPU cycle counter used for g_proc_cycles */
static void cycle_counter_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;      /* unlock (needed on Cortex-M7) */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Set up one PDM-to-PCM filter per mic. Needs the CRC clock on (MX_CRC_Init). */
static int filters_init(void)
{
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        filter[m].bit_order        = PDM_FILTER_BIT_ORDER_MSB;
        filter[m].endianness       = PDM_FILTER_ENDIANNESS_LE;
        filter[m].high_pass_tap    = PDM_HPF_TAP;
        filter[m].in_ptr_channels  = NUM_MICS;     /* read every 4th byte */
        filter[m].out_ptr_channels = NUM_MICS;     /* write every 4th sample */
        if (PDM_Filter_Init(&filter[m]) != 0) {
            return -1;
        }

        filter_cfg[m].decimation_factor     = PDM_FILTER_DEC_FACTOR_32;
        filter_cfg[m].output_samples_number = BLOCK_SAMPLES;
        filter_cfg[m].mic_gain              = PDM_GAIN_DB;
        if (PDM_Filter_setConfig(&filter[m], &filter_cfg[m]) != 0) {
            return -1;
        }
    }
    return 0;
}

/* Track the loudest sample per mic and publish it to g_peak every 100 ms */
static void update_peaks(void)
{
    for (uint32_t n = 0; n < BLOCK_SAMPLES; n++) {
        for (uint32_t m = 0; m < NUM_MICS; m++) {
            int32_t s = pcm_block[n * NUM_MICS + m];
            if (s < 0) {
                s = -s;
            }
            if (s > 32767) {
                s = 32767;
            }
            if (s > peak_hold[m]) {
                peak_hold[m] = (int16_t)s;
            }
        }
    }

    if (++peak_count >= PEAK_BLOCKS) {
        for (uint32_t m = 0; m < NUM_MICS; m++) {
            g_peak[m] = peak_hold[m];
            peak_hold[m] = 0;
        }
        peak_count = 0;
    }
}

/* Convert one 1 ms block of PDM data to PCM. Runs in the DMA interrupt. */
static void process_block(uint16_t *half)
{
    uint32_t start = DWT->CYCCNT;
    uint8_t *pdm = (uint8_t *)half;

    /* On each data line the SAI stores the SEL=3V3 mic first, so the bytes come in as
       mic 1, 0, 3, 2. Reading byte (m ^ 1) puts them back in order 0, 1, 2, 3. */
    for (uint32_t m = 0; m < NUM_MICS; m++) {
        PDM_Filter(&pdm[m ^ 1U], &pcm_block[m], &filter[m]);
    }

    update_peaks();
    recorder_push(pcm_block);

    g_cb_count++;
    g_proc_cycles = DWT->CYCCNT - start;
}


/* ---------- Public functions ---------- */

int mic_capture_start(void)
{
    cycle_counter_init();

    if (filters_init() != 0) {
        return -1;
    }

    /* Circular DMA: keeps refilling g_pdm_buf forever. Size is in 16-bit words. */
    if (HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)g_pdm_buf, BUF_WORDS) != HAL_OK) {
        return -2;
    }
    return 0;
}


/* ---------- HAL callbacks (replace the HAL's empty weak versions) ---------- */

/* First half of g_pdm_buf is full */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        process_block(&g_pdm_buf[0]);
    }
}

/* Second half of g_pdm_buf is full */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        process_block(&g_pdm_buf[HALF_WORDS]);
    }
}

/* SAI error, e.g. overrun */
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI1_Block_A) {
        g_sai_err_count++;
    }
}
