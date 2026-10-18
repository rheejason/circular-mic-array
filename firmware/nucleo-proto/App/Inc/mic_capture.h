/*
 * mic_capture.h
 */
#ifndef MIC_CAPTURE_H
#define MIC_CAPTURE_H

#include <stdint.h>

int mic_capture_start(void);    // start capture, returns 0 on success

/* Debug values (Live Expressions) */
extern volatile uint32_t g_cb_count;        // blocks processed, goes up 1000 per second
extern volatile uint32_t g_proc_cycles;     // CPU cycles for the last block (480000 = 1 ms)
extern volatile uint32_t g_sai_err_count;   // SAI errors, should stay 0

#endif /* MIC_CAPTURE_H */
