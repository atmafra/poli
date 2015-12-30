#ifndef __INCSTAT_H_
#define __INCSTAT_H_ 1

#include "../common/types.h"


/*
 * STATISTIC PARAMETERS
 */

/*
 * istt_clear_stat
 *
 * Initializes all the internal statistics
 */
extern void istt_clear_stat (void);


/*
 * istt_add_stat
 *
 * Adds a new observation
 */
extern void istt_add_stat (RValue x, RValue y);


/*
 * istt_samples
 *
 * Returns the current number of observations
 */
extern UsLgIntValue istt_samples (void);


/*
 * istt_average_x
 *
 * Returns the average of the x observations
 */
extern RValue istt_average_x (void);


/*
 * istt_average_y
 *
 * Returns the average of the y observations
 */
extern RValue istt_average_y (void);


/*
 * istt_variance_x
 *
 * Returns the variance of the x observations
 */
extern RValue istt_variance_x (void);


/*
 * istt_variance_y
 *
 * Returns the variance of the y observations
 */
extern RValue istt_variance_y (void);


/*
 * istt_stddev_x
 *
 * Returns the standard deviation of the x observations
 */
extern RValue istt_stddev_x (void);


/*
 * istt_stddev_y
 *
 * Returns the standard deviation of the y observations
 */
extern RValue istt_stddev_y (void);


/*
 * istt_max_x
 *
 * Returns the maximum value of the x observations
 */
extern RValue istt_max_x (void);


/*
 * istt_max_x
 *
 * Returns the minimum value of the x observations
 */
extern RValue istt_min_x (void);


/*
 * istt_max_y
 *
 * Returns the maximum value of the y observations
 */
extern RValue istt_max_y (void);


/*
 * istt_max_y
 *
 * Returns the minimum value of the y observations
 */
extern RValue istt_min_y (void);


/*
 * istt_sum_x
 *
 * Returns the sum of all the x observations
 */
extern RValue istt_sum_x (void);


/*
 * istt_sum_y
 *
 * Returns the sum of all the y observations
 */
extern RValue istt_sum_y (void);


/*
 * istt_sum_sqr_x
 *
 * Returns the sum of the squared x observations
 */
extern RValue istt_sum_sqr_x (void);


/*
 * istt_sum_sqr_y
 *
 * Returns the sum of the squared y observations
 */
extern RValue istt_sum_sqr_y (void);


/*
 * istt_sum_sqr_diff_xy
 *
 * Returns the sum of the squared (x - y) for all observations
 */
extern RValue istt_sum_sqr_diff_xy (void);


/*
 * istt_sum_xy
 *
 * Returns the sum of the x*y products for all observations
 */
extern RValue istt_sum_xy (void);


/*
 * istt_linear_regression
 *
 * Returns the linear and angular coefficients of the linear regression
 * for the current sample
 */
extern void istt_linear_regression (RValue * lin_coeff, RValue * ang_coeff);


/*
 * istt_linear_correlation_coeff
 *
 * Returns the linear correlation coefficient between the x and y observations
 */
extern RValue istt_linear_correlation_coeff (void);



/*
 * RANDOM NUMBER GENERATORS
 */

/*
 * istt_uniform_random
 *
 * Returns a random value from an uniform distribution between 'low' and 'high'
 */
extern UsLgIntValue istt_uniform_random (UsLgIntValue low, UsLgIntValue high);



/*
 * istt_uniform_random_real
 *
 * Returns a random value from an uniform distribution between 'low' and 'high'
 */
extern RValue istt_uniform_random_real (RValue low, RValue high);



/*
 * istt_gaussian_random
 *
 * Returns a random value from a Gaussian distribution
 * with average 'avg' and standard deviation 'stddev'
 *
 * Input Parameters
 * - avg: the average of the Gaussian distribution
 * - stddev: the standard deviation of the Gaussian distribution
 * - randomize: indicates that a new random seed should be used
 * - seed: new random seed to be used. If the value '0' is passed, the seed
 *   will be generated from the system clock
 */
extern RValue istt_gaussian_random
  (RValue avg, RValue stddev, BoolValue randomize, UsIntValue seed);



/*
 * istt_gaussian_randomize
 *
 * Defines a new random seed for the pseudo-random number generator,
 * based on the system clock
 */
extern void istt_gaussian_randomize (void);


#endif /* incstat.h */
