/*
 * mic_capture.h
 *
 * SAI1 PDM capture: DMA ping-pong buffer -> PDM2PCM filter per mic -> 1 ms interleaved PCM blocks
 * (m0 m1 m2 m3 m0 m1 ...), handed to the recorder from the DMA callbacks.
 */
#ifndef MIC_CAPTURE_H
#define MIC_CAPTURE_H

#include <stdint.h>
#include "array_config.h"

/* Debug values for Live Expressions */
extern volatile uint32_t g_cb_count;          /* +1 per DMA callback (one per 1 ms block) */
extern volatile int16_t  g_peak[NUM_MICS];    /* loudest |sample| per mic over the last ~100 ms */
extern volatile uint32_t g_proc_cycles;       /* CPU cycles spent in the last callback (480000 = 1 ms) */
extern volatile uint32_t g_sai_err_count;     /* SAI/DMA errors (e.g. overrun) */
extern uint16_t g_pdm_buf[];                  /* raw PDM DMA buffer */

/* Init the PDM filters and start SAI DMA. Returns 0 on success. */
int mic_capture_start(void);

#endif /* MIC_CAPTURE_H */
