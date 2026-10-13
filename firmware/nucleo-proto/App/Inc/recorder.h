/*
 * recorder.h
 *
 * Button-triggered RAM recording of REC_SECONDS of 4-channel audio into rec_buf
 * (interleaved int16, m0 m1 m2 m3 m0 ...). Export rec_buf from the debugger's Memory Browser
 * as RAW Binary: REC_SAMPLES * NUM_MICS * 2 bytes.
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

extern int16_t rec_buf[REC_SAMPLES * NUM_MICS];
extern volatile uint8_t  g_rec_state;   /* REC_IDLE / REC_RECORDING / REC_DONE */
extern volatile uint32_t g_rec_pos;     /* int16 values written so far */

/* Start a new recording (main loop). Overwrites the previous one. */
void recorder_arm(void);

/* Append one interleaved block of BLOCK_SAMPLES * NUM_MICS samples (DMA callback) */
void recorder_push(const int16_t *block);

#endif /* RECORDER_H */
