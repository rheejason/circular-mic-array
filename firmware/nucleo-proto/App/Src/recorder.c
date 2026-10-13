/*
 * recorder.c
 */
#include <string.h>
#include "recorder.h"

#define BLOCK_LEN   (BLOCK_SAMPLES * NUM_MICS)
#define REC_LEN     (REC_SAMPLES * NUM_MICS)

_Static_assert(REC_SAMPLES % BLOCK_SAMPLES == 0, "recording must be a whole number of blocks");

int16_t rec_buf[REC_LEN];       /* 384 KB for 1 s, in AXI SRAM */

volatile uint8_t  g_rec_state = REC_IDLE;
volatile uint32_t g_rec_pos;

void recorder_start(void)
{
    g_rec_pos = 0;                  /* reset first: recorder_push only writes while RECORDING */
    g_rec_state = REC_RECORDING;
}

void recorder_push(const int16_t *block)
{
    if (g_rec_state != REC_RECORDING) {
        return;
    }

    memcpy(&rec_buf[g_rec_pos], block, BLOCK_LEN * sizeof(int16_t));
    g_rec_pos += BLOCK_LEN;

    if (g_rec_pos >= REC_LEN) {
        g_rec_state = REC_DONE;
    }
}
