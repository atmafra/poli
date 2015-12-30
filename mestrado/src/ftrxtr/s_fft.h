/*
 * FFT - Fast Fourier Transform support for samples
 */

#ifndef __SFFT_H_
#define __SFFT_H_ 1

#include "s_smptypes.h"

/* Definition of the constant PI */
#undef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI 3.1415926535897932384626434
#endif

/* Exponent type domain */
typedef size_t smp_exp_type;

/* Internal type for bit-reversion */
typedef unsigned char *smp_bit_vector;

/* In-place computing */
typedef enum
{ SFFT_IN_PLACE, SFFT_OFF_PLACE }
sfft_place_type;

/*
 * Transform type
 * - SFFT_FFT: Fast Discrete Fourier Transform
 * - SFFT_FCT: Fast Discrete Cosine Transform
 */
typedef enum
{ SFFT_FFT, SFFT_FCT }
sfft_transform_type;

/* DFT domain type */
typedef enum
{ SFFT_REAL, SFFT_COMPLEX }
sfft_domain_type;

/* DFT direction */
typedef enum
{ SFFT_DIRECT, SFFT_INVERSE }
sfft_direction_type;



/*
 * sfft_sup_power
 *
 * Returns the minimum power of 'basis', and its corresponding exponent,
 * that is bigger than 'x'.
 */
int
sfft_sup_power (const smp_num_samples basis, const smp_num_samples x,
                smp_num_samples * power, smp_num_samples * exponent);


/*
 * sfft_bit_reverse
 *
 * Returns the N bit-reversed sample index, required for the
 * initialization of the FFT algorithm.
 * - index: is the sample index
 * - N: integer positive exponent, representing the sample size as two to the
 *      N-th power
 * - reversed_index: the return value, containing the N bit-reversed
 *   sample index
 */
int
sfft_bit_reverse (const smp_num_samples index, const smp_num_samples N,
                  smp_num_samples * reversed_index);


/*
 * sfft_W
 *
 * Returns the FFT's twiddle factors W.
 *
 * A twiddle factor is defined as
 * W(k,N) = - exp (-j*2*PI*k/N) = - cos (2*PI*k/N) + j * sin (2*PI*k/N)
 * Note that the twiddle factors are N-periodic. So W(k+N,N) = W(k,N).
 *
 * Input values
 * - k: discrete frequency value
 * - N: the number of samples
 *
 * Output value
 * - W: the complex twiddle factor W at frequency k
 */
int sfft_W (const smp_num_samples k, const smp_num_samples N,
            cmp_complex * W);


/*
 * sfft_exec
 *
 * Executes the FFT of the signal on the current list of
 * the given 'in_index'.
 * The returning list is the list at position 'out_pos'
 * of the index 'out_index'.
 * If the flag 'in_place' is set to yes, then the in-place calculation
 * will be done:
 *   - 'out_index' will return in_index';
 *   - 'out_pos' will return 'in_pos'.
 * The flag 'inverse' makes the inverse FFT to be calculated: the coefficients
 * will be rescaled by a 1/N factor.
 * If 'domain' is set to 'SMP_REAL', then the real FFT will be computed;
 * otherwise ('SMP_COMPLEX'), the complex transform will be computed.
 */
int
sfft_exec (index_list_type * in_index,
           index_list_type * out_index, smp_index_pos * out_pos,
           const sfft_place_type place, const sfft_direction_type direction,
           const sfft_domain_type domain);



/*
 * sfft_dct_exec
 *
 * Executes the FFT of the signal on list at position 'in_pos' of the given
 * 'in_index'
 * The returning list is the list at position 'out_pos'
 * of the index 'out_index'
 * If the flag 'in_place' is set to yes, then the in-place calculation
 * will be done:
 *   - 'out_index' will return in_index';
 *   - 'out_pos' will return 'in_pos'
 * The flag 'inverse' makes the inverse FFT to be calculated: the coefficients
 * will be rescaled by a 1/N factor
 */
int
sfft_dct_exec (index_list_type * in_index,
               index_list_type * out_index, smp_index_pos * out_pos,
               const sfft_place_type place,
               const sfft_direction_type direction);



/*
 * sfft_exec_index
 *
 * Executes the FFT for all the signal lists
 * at the given input index 'in_index'.
 * A new index 'out_index' is created with the transformed signals.
 * The input 'domain' specifies wheather the real FFT (SFFT_REAL)
 * or the complex * FFT (SFFT_COMPLEX) should be performed.
 */
int
sfft_exec_index (index_list_type * in_index, index_list_type * out_index,
                 const sfft_transform_type transform,
                 const sfft_place_type place,
                 const sfft_direction_type direction,
                 const sfft_domain_type domain);

#endif /* ! __SFFT_H_ */
