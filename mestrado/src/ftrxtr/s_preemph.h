#ifndef __S_PREEMPH_H_
#define __S_PREEMPH_H_ 1

#include "s_complex.h"
#include "s_samples.h"

/* Default value for the alpha coefficient */
#define __SPRE_DEFAULT_ALPHA_ 0.95

/* Data type for filter coefficient */
typedef cmp_real spre_filter_coeff_type;

/* Real data type */
typedef cmp_real spre_real;



/*
 * spre_preemphasis
 *
 * Returns the pre-emphasized value x'[n] of x[n], according to the filter
 * x'[n] = x[n] - alpha * x[n-1]
 * where alpha is a constant, 0 <= alpha <= 1
 *
 * If an invalid alpha is passed (out of range), then the default value for alpha
 * will be used
 *
 * Parameters
 * - alpha: the alpha coefficient of the filter
 * - x_current: x[n]
 * - x_previous: x[n-1]
 */
spre_real spre_preemphasis (const spre_filter_coeff_type alpha,
                            const spre_real x_current,
                            const spre_real x_previous);



/*
 * spre_list_preemphasis
 *
 * Applies the pre-emphasis to all of the samples in the given list
 */
int spre_list_preemphasis (sample_list_type * list, const spre_real alpha);



#endif /* __S_PREEMPH_H_ */
