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


/* Framing */

void beam_push(const int16_t *block)
{
	for (uint32_t n=0; n<BLOCK_SAMPLES; n++) {
		for (uint32_t m=0; m<NUM_MICS; m++) {
			int16_t s = block[n * NUM_MICS + m];
			hops[fill_buf][m][fill_n] = s / 32768.0f;
		}
		fill_n += 1;
		if (fill_n == BF_HOP) {
			if (ready_buf >= 0) {
				g_beam_overruns++;		// main loop never took last hop
			}
			__DMB();
			ready_buf = (int32_t)fill_buf;
			fill_buf ^= 1U;
			fill_n = 0;
		}
	}
}


/* Output Fifo */

static void out_write(const int16_t *src)
{
	if (OUT_LEN - (out_wr - out_rd) < BF_HOP) {
		return;			// no room, interrupt isn't reading
	}
	for (uint32_t n=0; n<BF_HOP; n++) {
		out_fifo[(out_wr + n) % OUT_LEN] = src[n];
	}
	__DMB();			// samples written before interrupt can see them
	out_wr += BF_HOP;
}

void beam_pop(int16_t *dst)
{
	uint32_t level = out_wr - out_rd;

	if (primed && level < BLOCK_SAMPLES) {
		g_beam_underruns++;
		primed = 0;
	}
	if (!primed && level >= OUT_PREFILL) {
		primed = 1;
	}
	if (!primed) {
		memset(dst, 0, BLOCK_SAMPLES * sizeof(int16_t));
		return;
	}
	for (uint32_t n=0; n<BLOCK_SAMPLES; n++) {
		dst[n] = out_fifo[(out_rd + n) % OUT_LEN];
	}
	out_rd += BLOCK_SAMPLES;
}


/* Delay-and-sum */

// Weights for a talker at az_deg
static void steer(float az_deg)
{
	float az = az_deg * (PI / 180.0f);
	float ux = cosf(az), uy = sinf(az);			// unit vector toward talker

	for (uint32_t m=0; m<NUM_MICS; m++) {
		float tau = (ux * px[m] + uy * py[m]) / SPEED_OF_SOUND;
		float *w = weights[m];

		w[0] = 1.0f / (float)NUM_MICS;
		w[1] = 0.0f;
		for (uint32_t k=1; k<BF_BINS; k++) {
			float f = (float)k * (float)FS_HZ / (float)BF_FFT;
			float phase = -2.0f * PI * f * tau;			// delay by tau
			w[2U*k]		 = cosf(phase) / (float)NUM_MICS;
			w[2U*k + 1U] = sinf(phase) / (float)NUM_MICS;
		}
	}
	steered_az = az_deg;
}

// One hop: 256 new samples per mic in, 256 beam samples out
static void process_hop(float hop[NUM_MICS][BF_HOP])
{
	// 1. Each mic: [previous hop | this hop] x window -> FFT -> x weights -> add into Y
	memset(Y, 0, sizeof Y);
	for (uint32_t m=0; m<NUM_MICS; m++) {
		for (uint32_t n=0; n<BF_HOP; n++) {
			fft_in[n]		   = prev[m][n] * win[n];
			fft_in[BF_HOP + n] = hop[m][n] * win[BF_HOP + n];
		}
		memcpy(prev[m], hop[m], sizeof prev[m]);

		arm_rfft_fast_f32(&fft, fft_in, spec, 0);
		arm_cmplx_mult_cmplx_f32(spec, weights[m], prod, BF_BINS);
		arm_add_f32(Y, prod, Y, BF_FFT);
	}

	// 2. Back to time, window again, overlap-add with last frame's second half
	arm_rfft_fast_f32(&fft, Y, y, 1);
	arm_mult_f32(y, win, y ,BF_FFT);
	for (uint32_t n=0; n<BF_HOP; n++) {
		out[n] = tail[n] + y[n];
		tail[n] = y[BF_HOP + n];
	}

	// 3. To int16 (clips at +1), into the output FIFO
	arm_float_to_q15(out, out_q15, BF_HOP);
	out_write(out_q15);
}


/* Setup */

int beam_init(void)
{
	if (arm_rfft_fast_init_f32(&fft, BF_FFT) != ARM_MATH_SUCCESS) {
		return -1;
	}
	for (uint32_t n=0; n<BF_FFT; n++) {		// sqrt(Hann) = half a sine; win^2 at 50% overlap sums to 1
		win[n] = sinf(PI * (float)n / (float)BF_FFT);
	}
	for (uint32_t m=0; m<NUM_MICS; m++) {	// mic 0 at +x, counter-clockwise (same as doa.c)
		float angle = 2.0f * PI * (float)m / (float)NUM_MICS;
		px[m] = MIC_RADIUS_M * cosf(angle);
		py[m] = MIC_RADIUS_M * sinf(angle);
	}
	steer(g_beam_az);
	return 0;
}


/* Main loop */

void beam_task(void)
{
	int32_t b = ready_buf;
	if (b < 0) {
		return;								// no hop waiting
	}
	__DMB();								// read the hop only after seeing ready_buf

	uint32_t start = DWT->CYCCNT;
	float az = g_beam_az;
	if (az != steered_az) {
		steer(az);							// new angle from the debugger or the DOA
	}
	process_hop(hops[b]);
	ready_buf = -1;							// hop buffer free for the interrupt
	g_beam_cycles = DWT->CYCCNT - start;
}
