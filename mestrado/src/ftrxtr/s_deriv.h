#ifndef __S_DERIV_H_
#define __S_DERIV_H_ 1

#include "s_complex.h"
#include "s_samples.h"

/*
 * sder_derivative
 *
 * Returns the derivative estimate value of x[n],
 * given x[n-1], x[n] and x[n+1], according to linear regression.
 *
 * If x[n-1] is undefined, ignores it
 * If x[n+1] is undefined, ignores it
 * If both x[n-1] and x[n+1] are undefined, returns 0.0
 *
 * Parameters
 * - alpha: the alpha coefficient of the filter
 * - x_current: x[n]
 * - x_previous: x[n-1]
 */
extern cmp_real
sder_derivative (const cmp_real x_current,
                 const cmp_real x_previous,
                 const cmp_real x_next,
                 const smp_yes_no has_previous, const smp_yes_no has_next);



/*
 * sder_list_derivative
 *
 * Sets the second-half elements of the given list to be the
 * derivatives of the first-half elements
 */
extern int
sder_list_derivative (sample_list_type * list_cur,
                      sample_list_type * list_prev,
                      sample_list_type * list_next);



/*
 * sder_index_derivative
 *
 * Calculates the derivatives for all lists in the given index
 */
extern int sder_index_derivative (index_list_type index);



#endif /* __S_DERIV_H_ */
