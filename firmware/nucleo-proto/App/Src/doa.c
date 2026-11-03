/*
 * doa.c
 *
 *  Created on: Oct 19, 2026
 *      Author: jasonr8
 */

// TODO(final board): parabola is cheap stand in for exact (sinc) interpolation

#include <string.h>
#include <math.h>
#include "doa.h"
#include "main.h"
#include "arm_math.h"

#define DOA_EPS			1e-12f		// prevents dividing by 0 in silent bins
#define TEST_BIN		32U

/* Debug values */
volatile float 	  g_doa_az;
/* volatile float g_doa_az_ls; */				// old least squares, not used
float 			  g_srp_map[SRP_N_ANGLES];
volatile uint32_t g_doa_valid;
/* volatile float g_doa_tau_us[NUM_PAIRS]; */	// old least squares, not used
volatile float 	  g_doa_level_db;
volatile float 	  g_doa_floor_db;
volatile uint32_t g_doa_updates;
volatile uint32_t g_doa_cycles;
volatile uint32_t g_doa_overruns;

/* Framing: interrupt fills one buffer, main loop uses other */
static float frames[2][NUM_MICS][FRAME_LEN];
static uint32_t fill_buf, fill_n;
static volatile int32_t ready_buf = -1;

/* FFT */
static arm_rfft_fast_instance_f32 fft;
static float fft_in[N_FFT];
static float spec[N_FFT];
static float cc[N_FFT];

/* GCC-PHAT */
static float W[NUM_MICS][2U * DOA_NB];
static float acc[NUM_PAIRS][2U * DOA_NB];
static uint8_t pair_i[NUM_PAIRS], pair_j[NUM_PAIRS];
static uint32_t n_frames;

/* Old least squares, not used: u_xy = ls * tau */
/* static float ls[2][NUM_PAIRS]; */

/* SRP-PHAT */
static float pair_ax[NUM_PAIRS], pair_ay[NUM_PAIRS];	// tau_p = pair_ax * ux + pair_ay * uy (s)
static float corr[NUM_PAIRS][DOA_WIN];					// each pair's correlation at lags -14..+14
static float srp_idx[SRP_N_ANGLES][NUM_PAIRS];			// where angle d says pair p's peak should be (index into corr)

/* Energy gate */
static float level_sum;
static float floor_db;
static uint32_t have_floor;


/* Framing */

void doa_push(const int16_t *block)			// runs in DMA interrupt, once per 1 ms
{
	for (uint32_t n=0; n<BLOCK_SAMPLES; n++) {
		for (uint32_t m=0; m<NUM_MICS; m++) {
			int16_t s = block[n * NUM_MICS + m];			// block is m0 m1 m2 m3 m0 m1 ...
			frames[fill_buf][m][fill_n] = s / 32768.0f;		// normalizing to -1,+1
		}
		fill_n += 1;
		if (fill_n == FRAME_LEN) {	// count samples
			if (ready_buf >= 0) {
				g_doa_overruns++;		// main loop never took last frame
			}
			__DMB();						// frame fully written before main loop can see
			ready_buf = (int32_t)fill_buf;	// tell main loop which bucket
			fill_buf ^= 1U;					// switch to other bucket
			fill_n = 0;						// start from beginning
		}
	}
}


/* GCC-PHAT, every frame */

// W = X / |X| over the band. Returns sum |X|^2 over the band (for the energy gate)
// Dividing each bin by its own size and keep only its direction (phase)
static float whiten(const float *X, float*W_m)
{
	float mag[DOA_NB];						// 79 arrow lengths
	float energy;
	const float *Xb = X + 2U * DOA_K_LO;	// X starting at bin 7

	arm_cmplx_mag_f32(Xb, mag, DOA_NB);		// mag[k] = length of arrow k
	arm_power_f32(mag, DOA_NB, &energy);	// energy = sum of length^2
	for (uint32_t k=0; k<DOA_NB; k++) {
		mag[k] = 1.0f / (mag[k] + DOA_EPS);	// 1 / length
	}
	arm_cmplx_mult_real_f32(Xb, mag, W_m, DOA_NB);	// each arrow x (1 / its length)

	return energy;
}

static void accumulate(float *acc_p, const float *Wi, const float *Wj)
{
	for (uint32_t b=0; b<DOA_NB; b++) {				//b = 0..78 -> FFT bin k = DOA_K_LO + b
		float ar = Wi[2*b], ai = Wi[2*b + 1];			// mic i's arrow for this bin
		float br = Wj[2*b], bi = Wj[2*b + 1];			// mic j's arrow for this bin
		acc_p[2*b] 	   += ar * br + ai * bi;			// Re{ W_i[k] * conj(W_j[k]) }
		acc_p[2*b + 1] += ai * br - ar * bi;			// Im{ W_i[k] * conj(W_j[k]) }
	}
}

static void add_frame(float frame[NUM_MICS][FRAME_LEN])
{
	for (uint32_t m=0; m<NUM_MICS; m++) {
		memcpy(fft_in, frame[m], FRAME_LEN * sizeof(float));					// 512 samples of mic m
		memset(fft_in + FRAME_LEN, 0, (N_FFT - FRAME_LEN) * sizeof(float));	    // 512 zeros after
		arm_rfft_fast_f32(&fft, fft_in, spec, 0);
		level_sum += whiten(spec, W[m]);
	}

	for (uint32_t p=0; p<NUM_PAIRS; p++) {
		accumulate(acc[p], W[pair_i[p]], W[pair_j[p]]);
	}
	n_frames++;
}

// One pair: inverse FFT of 9-frame sum -> correlation at lags -14..+14
static void pair_corr(const float *acc_p, float *corr_p)
{
	// 1. Rebuild full packed spectrum: acc_p for bins 7..85 and zeros else
	memset(fft_in, 0, sizeof fft_in);
	memcpy(fft_in + 2U * DOA_K_LO, acc_p, 2U * DOA_NB * sizeof(float));

	// 2. Inverse FFT: cc[n] = how well mic i and mic j match at lag n
	arm_rfft_fast_f32(&fft, fft_in, cc, 1);

	// 3. Copy lags -14..+14 into corr_p[0..28] in order
	for (uint32_t w=0; w<DOA_WIN; w++) {
		int32_t lag = (int32_t)w - (int32_t)DOA_MAX_SHIFT;
		corr_p[w] = cc[(lag + (int32_t)N_FFT) % (int32_t)N_FFT];		// negative lags sit at end of cc
	}
}

// Parabola through a peak and its two neighbours -> how far the real top is from the middle (-0.5..+0.5)
static float parabola_offset(float y_left, float y_mid, float y_right)
{
	float den = y_left - 2.0f * y_mid + y_right;
	if (den == 0.0f) {
		return 0.0f;							// flat top: stay on the middle point
	}
	return 0.5f * (y_left - y_right) / den;
}

static float wrap_360(float az)
{
	if (az < 0.0f) {
		az += 360.0f;
	} else if (az >= 360.0f) {
		az -= 360.0f;
	}
	return az;
}

/* Old least squares, not used: one pair's highest point -> delay in seconds
static float pair_delay(const float *corr_p)
{
	float peak;
	uint32_t w_peak;

	// 1. Highest point
	arm_max_f32(corr_p, DOA_WIN, &peak, &w_peak);

	// 2. Parabola -> fraction of a sample (needs a neighbour on both sides)
	float delta = 0.0f;
	if (w_peak > 0U && w_peak < DOA_WIN - 1U) {
		delta = parabola_offset(corr_p[w_peak - 1U], corr_p[w_peak], corr_p[w_peak + 1U]);
	}

	// 3. Back to lag in samples, then seconds
	float lag_peak = (float)w_peak - (float)DOA_MAX_SHIFT + delta;

	return lag_peak / (float)FS_HZ;
}
*/

// Value of curve y at a fractional index x, from a smooth curve through the 4 nearest samples (Catmull-Rom)
// A straight line between 2 samples was up to 2 deg off; this matches the exact answer
static float read_between(const float *y, float x)
{
	uint32_t i = (uint32_t)x;							// sample just left of x
	float t = x - (float)i;								// how far toward the next one (0..1)
	float y0 = y[i - 1U], y1 = y[i], y2 = y[i + 1U], y3 = y[i + 2U];

	float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
	float b =         y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
	float c = -0.5f * y0              + 0.5f * y2;
	return ((a * t + b) * t + c) * t + y1;
}

// SRP-PHAT: score every angle by adding up what each pair's curve says there, keep the best
static float srp_azimuth(void)
{
	float peak;
	uint32_t d_peak;

	// 1. Score each angle d
	for (uint32_t d=0; d<SRP_N_ANGLES; d++) {
		float score = 0.0f;
		for (uint32_t p=0; p<NUM_PAIRS; p++) {
			score += read_between(corr[p], srp_idx[d][p]);		// this pair's curve where angle d says its peak is
		}
		g_srp_map[d] = score;
	}

	// 2. Best angle
	arm_max_f32(g_srp_map, SRP_N_ANGLES, &peak, &d_peak);

	// 3. Parabola for between grid angles. Map is a circle: left of 0 deg is 358 deg
	float y_left =  g_srp_map[(d_peak + SRP_N_ANGLES - 1U) % SRP_N_ANGLES];
	float y_right = g_srp_map[(d_peak + 1U) % SRP_N_ANGLES];
	float delta = parabola_offset(y_left, peak, y_right);

	// 4. Grid index -> degrees
	return wrap_360(((float)d_peak + delta) * (360.0f / (float)SRP_N_ANGLES));
}

static void estimate(void)
{
	// 1. Energy gate: how loud was this update compared to noise floor?
	float level_db = 10.0f * log10f(level_sum / (float)(DOA_K * NUM_MICS) + 1e-20f);
	if (!have_floor || level_db < floor_db) {
		floor_db = level_db;					// quiter than the floor: floor drops to it now
		have_floor = 1U;
	} else {
		floor_db += DOA_FLOOR_RISE_DB;			// otherwise floor creeps up slowly
	}
	uint32_t valid = (level_db > floor_db + DOA_GATE_DB);

	if (valid) {
		// 2. One correlation curve per pair
		for (uint32_t p=0; p<NUM_PAIRS; p++) {
			pair_corr(acc[p], corr[p]);
		}

		// 3. SRP-PHAT: all pairs vote on every angle together
		g_doa_az = srp_azimuth();

		/* Old least squares, not used: one delay per pair, combined into one direction
		float ux = 0.0f, uy = 0.0f;
		for (uint32_t p=0; p<NUM_PAIRS; p++) {
			float tau = pair_delay(corr[p]);
			g_doa_tau_us[p] = tau * 1e6f;
			ux += ls[0][p] * tau;
			uy += ls[1][p] * tau;
		}
		g_doa_az_ls = wrap_360(atan2f(uy, ux) * (180.0f / PI));
		*/
	}

	// 4. Publish, then start the next DOA_K frames
	g_doa_valid = valid;
	g_doa_level_db = level_db;
	g_doa_floor_db = floor_db;
	g_doa_updates++;
	memset(acc, 0, sizeof acc);
	level_sum = 0.0f;
	n_frames = 0;
}


/* Setup */

// Each pair gives tau_p = a_p . u with a_p = -(p_i - p_j) / c. SRP needs a_p
// Old least squares (commented out) also solved u = (A^T A)^-1 A^T tau here
static int ls_init(void)
{
	float px[NUM_MICS], py[NUM_MICS];				// mic positions (m)
	/* float sxx = 0.0f, sxy = 0.0f, syy = 0.0f; */	// A^T A = [sxx sxy; sxy syy]
	uint32_t p = 0;

	for (uint32_t m=0; m<NUM_MICS; m++) {			// mic 0 at +x, counter-clockwise
		float angle = 2.0f * PI * (float)m / (float)NUM_MICS;
		px[m] = MIC_RADIUS_M * cosf(angle);
		py[m] = MIC_RADIUS_M * sinf(angle);
	}

	for (uint32_t i=0; i<NUM_MICS; i++) {			// pairs (0,1) (0,2) (0,3) (1,2) (1,3) (2,3)
		for (uint32_t j=i+1U; j<NUM_MICS; j++) {
			pair_i[p] = (uint8_t)i;
			pair_j[p] = (uint8_t)j;
			pair_ax[p] = -(px[i] - px[j]) / SPEED_OF_SOUND;		// row p of A (s)
			pair_ay[p] = -(py[i] - py[j]) / SPEED_OF_SOUND;
			/*
			sxx += pair_ax[p] * pair_ax[p];
			sxy += pair_ax[p] * pair_ay[p];
			syy += pair_ay[p] * pair_ay[p];
			*/
			p++;
		}
	}

	/*
	float det = sxx * syy - sxy * sxy;
	if (det <= 0.0f) {
		return -1;									// mics all on one line: no unique direction
	}
	for (p=0; p<NUM_PAIRS; p++) {					// ls = (A^T A)^-1 A^T, one column per pair
		ls[0][p] = (syy * pair_ax[p] - sxy * pair_ay[p]) / det;
		ls[1][p] = (sxx * pair_ay[p] - sxy * pair_ax[p]) / det;
	}
	*/
	return 0;
}

// For each angle we try, where each pair's correlation peak should land
// Same model as least squares (tau_p = a_p . u), just run forwards: angle -> delays
static void srp_init(void)
{
	for (uint32_t d=0; d<SRP_N_ANGLES; d++) {
		float az = 2.0f * PI * (float)d / (float)SRP_N_ANGLES;		// angle d in radians
		float ux = cosf(az), uy = sinf(az);							// unit vector toward it
		for (uint32_t p=0; p<NUM_PAIRS; p++) {
			float tau = pair_ax[p] * ux + pair_ay[p] * uy;			// delay this pair would see (s)
			srp_idx[d][p] = tau * (float)FS_HZ + (float)DOA_MAX_SHIFT;	// -> samples -> index into corr (lag 0 is index 14)
		}
	}
}

// A tone exactly on bin TEST_BIN must come out of the FFT on that bin (bring-up check, delete later)
static int fft_selftest(void)
{
	float peak;
	uint32_t idx;

	for (uint32_t n=0; n<N_FFT; n++) {
		fft_in[n] = sinf(2.0f * PI * (float)(TEST_BIN * n) / (float)N_FFT);
	}
	arm_rfft_fast_f32(&fft, fft_in, spec, 0);
	arm_cmplx_mag_f32(spec + 2, cc, N_FFT / 2U - 1U);	// bins 1..511 (cc used as scratch)
	arm_max_f32(cc, N_FFT / 2U - 1U, &peak, &idx);
	return (idx + 1U == TEST_BIN) ? 0 : -1;				// idx 0 is bin 1
}

int doa_init(void)
{
	// search window must cover the largest delay across the array (12.59), +2 so the parabola
	// and read_between always have neighbours inside corr
	if (DOA_MAX_SHIFT < (uint32_t)floorf(2.0f * MIC_RADIUS_M / SPEED_OF_SOUND * FS_HZ) + 2U) {
		return -1;
	}
	if (arm_rfft_fast_init_f32(&fft, N_FFT) != ARM_MATH_SUCCESS) {
		return -2;
	}
	if (fft_selftest() != 0) {
		return -3;
	}
	if (ls_init() != 0) {
		return -4;
	}
	srp_init();										// needs pair_ax, pair_ay from ls_init
	return 0;
}


/* Main loop */

void doa_task(void)
{
	int32_t b = ready_buf;
	if (b < 0) {
		return;										// no frame waiting
	}
	__DMB();										// read the frame only after seeing ready_buf

	uint32_t start = DWT->CYCCNT;
	add_frame(frames[b]);
	ready_buf = -1;									// frame copied out, buffer free for the interrupt
	if (n_frames == DOA_K) {
		estimate();
	}
	g_doa_cycles = DWT->CYCCNT - start;
}



