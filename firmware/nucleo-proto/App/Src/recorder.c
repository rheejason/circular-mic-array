/*
 * recorder.c
 */
#include <string.h>
#include "recorder.h"

#define BLOCK_VALUES    (BLOCK_SAMPLES * NUM_MICS)
#define REC_VALUES      (REC_SAMPLES * NUM_MICS)

_Static_assert(REC_SAMPLES % BLOCK_SAMPLES == 0, "recording must be a whole number of blocks");

/* 384 KB at 1 s; lands in AXI SRAM (RAM_D1) with the rest of .bss */
int16_t rec_buf[REC_VALUES];

volatile uint8_t  g_rec_state = REC_IDLE;
volatile uint32_t g_rec_pos;

void recorder_arm(void)
{
    /* Reset the position before changing state: the DMA callback only writes while RECORDING */
    g_rec_pos = 0;
    g_rec_state = REC_RECORDING;
}

void recorder_push(const int16_t *block)
{
    if (g_rec_state != REC_RECORDING) {
        return;
    }

    memcpy(&rec_buf[g_rec_pos], block, BLOCK_VALUES * sizeof(int16_t));
    g_rec_pos += BLOCK_VALUES;

    if (g_rec_pos >= REC_VALUES) {
        g_rec_state = REC_DONE;
    }
}
