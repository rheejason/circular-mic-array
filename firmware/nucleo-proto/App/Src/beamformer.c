/*
 * beamformer.c
 *
 *  Created on: Oct 22, 2026
 *      Author: jasonr8
 */

#include <string.h>
#include <math.h>
#include "beamformer.h"
#include "main.h"
#include "arm_math.h"

#define OUT_LEN			1024U				// output FIFO, samples (power of 2 so indices wrap cleanly)
#define OUT_PREFILL		(2U * BF_HOP)		// start playing once 2 hops are waiting

/* Debug values */
volatile float	  g_beam_az;
volatile uint32_t g_beam_cycles;
volatile uint32_t g_beam_overruns;
volatile uint32_t g_beam_underruns;

/* Input framing: interrupt fills one hop buffer, main loop uses the other */
static float hops[2][NUM_MICS][BF_HOP];
static uint32_t fill_buf, fill_n;
static volatile int32_t ready_buf = -1;

/* Steering */
static float px[NUM_MICS], py[NUM_MICS];	// mic positions (m)
static float weights[NUM_MICS][BF_FFT];		// packed like the FFT: slot k = (re, im) at [2k], [2k+1]
static float steered_az = -1.0f;			// angle the weights are for (-1 = not yet)

/* STFT */
static arm_rfft_fast_instance_f32 fft;
static float win[BF_FFT];					// sqrt-Hann
static float prev[NUM_MICS][BF_HOP];		// last hop of each mic = first half of next frame
static float fft_in[BF_FFT];
static float spec[BF_FFT];					// X_m, one mic at a time
static float prod[BF_FFT];					// X_m * weights_m
static float Y[BF_FFT];						// sum over mics
static float y[BF_FFT];						// IFFT(Y), windowed
static float tail[BF_HOP];					// second half of last y, added to the next hop
static float out[BF_HOP];
static int16_t out_q15[BF_HOP];

/* Output FIFO: main loop writes 256 per hop, interrupt reads 48 per ms */
static int16_t out_fifo[OUT_LEN];
static volatile uint32_t out_wr, out_rd;	// total samples written / read; index = count % OUT_LEN
static uint32_t primed;						// 1 while output is playing (interrupt only)