/*
 * recorder.h
 * Records REC_SECONDS of audio into rec_buf.
 * rec_buf is 16-bit, interleaved m0 m1 m2 m3 m0 ... Save it from the gdb console with:
 *   dump binary value <file>.bin rec_buf
 */
#ifndef RECORDER_H
#define RECORDER_H

#include <stdint.h>
#include "array_config.h"

enum {
    REC_IDLE      = 0,
    REC_RECORDING = 1,
    REC_DONE      = 2,
};

/* Start a new recording (overwrites the last one) */
void recorder_start(void);

/* Add one block to the recording. Called by mic_capture for every block. */
void recorder_push(const int16_t *block);

extern int16_t rec_buf[REC_SAMPLES * NUM_MICS];
extern volatile uint8_t  g_rec_state;    /* REC_IDLE, REC_RECORDING or REC_DONE */
extern volatile uint32_t g_rec_pos;      /* samples written so far (all mics) */

#endif /* RECORDER_H */
