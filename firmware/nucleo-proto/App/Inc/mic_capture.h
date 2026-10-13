/*
 * mic_capture.h
 * Reads the 4 PDM mics through SAI1 + DMA and converts them to 48 kHz PCM.
 * Output is one 1 ms block at a time, interleaved: m0 m1 m2 m3 m0 m1 ...
 */
#ifndef MIC_CAPTURE_H
#define MIC_CAPTURE_H

#include <stdint.h>
#include "array_config.h"

/* Start capturing. Returns 0 on success. */
int mic_capture_start(void);

/* Debug values (watch in Live Expressions) */
extern volatile uint32_t g_cb_count;         /* blocks processed, goes up 1000 per second */
extern volatile int16_t  g_peak[NUM_MICS];   /* loudest sample per mic over the last 100 ms */
extern volatile uint32_t g_proc_cycles;      /* CPU cycles for the last block (480000 = 1 ms) */
extern volatile uint32_t g_sai_err_count;    /* SAI errors, should stay 0 */
extern uint16_t g_pdm_buf[];                 /* raw PDM data from the DMA */

#endif /* MIC_CAPTURE_H */
