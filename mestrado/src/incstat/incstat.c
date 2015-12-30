#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include "incstat.h"

#define __ISTT_SQR_(x) (x)*(x)

/*
 * Internal static statistic parameters
 */
static UsLgIntValue int_samples = 0;
static RValue int_sum_x = 0.0, int_sum_y = 0.0;
static RValue int_avg_x = 0.0, int_avg_x_old = 0.0;
static RValue int_avg_y = 0.0, int_avg_y_old = 0.0;
static RValue int_sqr_diff_x = 0.0, int_sqr_diff_y = 0.0;
static RValue int_max_x = 0.0, int_min_x = 0.0;
static RValue int_max_y = 0.0, int_min_y = 0.0;
static RValue int_sum_sqr_x = 0.0, int_sum_sqr_y = 0.0;
static RValue int_sum_sqr_diff_xy = 0.0;
static RValue int_sum_xy = 0.0;


/*
 * istt_clear_stat
 *
 * Initializes all the internal statistics
 */
void
istt_clear_stat (void)
{
  int_samples = 0;
  int_sum_x = 0.0;
  int_sum_y = 0.0;
  int_avg_x = 0.0;
  int_avg_x_old = 0.0;
  int_avg_y = 0.0;
  int_avg_y_old = 0.0;
  int_sqr_diff_x = 0.0;
  int_sqr_diff_y = 0.0;
  int_sum_sqr_x = 0.0;
  int_sum_sqr_y = 0.0;
  int_sum_sqr_diff_xy = 0.0;
  int_sum_xy = 0.0;
  int_max_x = 0.0;
  int_min_x = 0.0;
  int_max_y = 0.0;
  int_min_y = 0.0;
}


/*
 * istt_add_stat
 *
 * Adds a new observation
 */
void
istt_add_stat (RValue x, RValue y)
{
  /*
   * Averages
   */
  int_avg_x_old = int_avg_x;
  int_avg_y_old = int_avg_y;
  int_avg_x = (int_samples * int_avg_x + x) / (int_samples + 1);
  int_avg_y = (int_samples * int_avg_y + y) / (int_samples + 1);

  /*
   * Accumulators
   */
  int_sum_x += x;
  int_sum_y += y;
  int_sum_sqr_x += __ISTT_SQR_ (x);
  int_sum_sqr_y += __ISTT_SQR_ (y);
  int_sum_xy += x * y;
  int_sum_sqr_diff_xy += __ISTT_SQR_ (x - y);

  /*
   * Square Differences
   */
  int_sqr_diff_x +=
    int_samples / __ISTT_SQR_ (int_samples + 1)
    * __ISTT_SQR_ (int_avg_x_old - x) + __ISTT_SQR_ (int_avg_x - x);

  int_sqr_diff_y +=
    +int_samples / __ISTT_SQR_ (int_samples + 1)
    * __ISTT_SQR_ (int_avg_y_old - y) + __ISTT_SQR_ (int_avg_y - y);

  /*
   * Maxima and minima
   */
  if (int_samples == 0)
    {
      int_min_x = x;
      int_max_x = x;
      int_min_y = y;
      int_max_y = y;
    }
  else
    {
      int_min_x = (int_min_x - x < DBL_EPSILON ? int_min_x : x);
      int_max_x = (int_max_x - x > DBL_EPSILON ? int_max_x : x);
      int_min_y = (int_min_y - y < DBL_EPSILON ? int_min_y : y);
      int_max_y = (int_max_y - y > DBL_EPSILON ? int_max_y : y);
    }

  /*
   * Number of observations
   */
  int_samples++;
}



/*
 * istt_samples
 *
 * Returns the current number of observations
 */
UsLgIntValue
istt_samples (void)
{
  return int_samples;
}



/*
 * istt_average_x
 *
 * Returns the average of the x observations
 */
RValue
istt_average_x (void)
{
  return int_avg_x;
}



/*
 * istt_average_y
 *
 * Returns the average of the y observations
 */
RValue
istt_average_y (void)
{
  return int_avg_y;
}



/*
 * istt_variance_x
 *
 * Returns the variance of the x observations
 */
RValue
istt_variance_x (void)
{
  if (int_samples > 0)
    return int_sqr_diff_x / int_samples;
  else
    return 0.0;
}



/*
 * istt_variance_y
 *
 * Returns the variance of the y observations
 */
RValue
istt_variance_y (void)
{
  if (int_samples > 0)
    return int_sqr_diff_y / int_samples;
  else
    return 0.0;
}



/*
 * istt_stddev_x
 *
 * Returns the standard deviation of the x observations
 */
RValue
istt_stddev_x (void)
{
  if (int_samples > 0)
    return sqrt (int_sqr_diff_x / int_samples);
  else
    return 0.0;
}



/*
 * istt_stddev_y
 *
 * Returns the standard deviation of the y observations
 */
RValue
istt_stddev_y (void)
{
  if (int_samples > 0)
    return sqrt (int_sqr_diff_y / int_samples);
  else
    return 0.0;
}



/*
 * istt_max_x
 *
 * Returns the maximum value of the x observations
 */
RValue
istt_max_x (void)
{
  return int_max_x;
}



/*
 * istt_min_x
 *
 * Returns the minimum value of the x observations
 */
RValue
istt_min_x (void)
{
  return int_min_x;
}



/*
 * istt_max_y
 *
 * Returns the maximum value of the y observations
 */
RValue
istt_max_y (void)
{
  return int_max_y;
}



/*
 * istt_min_y
 *
 * Returns the minimum value of the y observations
 */
RValue
istt_min_y (void)
{
  return int_min_y;
}



/*
 * istt_sum_x
 *
 * Returns the sum of all the x observations
 */
RValue
istt_sum_x (void)
{
  return int_sum_x;
}



/*
 * istt_sum_y
 *
 * Returns the sum of all the y observations
 */
RValue
istt_sum_y (void)
{
  return int_sum_y;
}



/*
 * istt_sum_sqr_x
 *
 * Returns the sum of the squared x observations
 */
RValue
istt_sum_sqr_x (void)
{
  return int_sum_sqr_x;
}



/*
 * istt_sum_sqr_y
 *
 * Returns the sum of the squared y observations
 */
RValue
istt_sum_sqr_y (void)
{
  return int_sum_sqr_y;
}



/*
 * istt_sum_sqr_diff_xy
 *
 * Returns the sum of the squared (x - y) for all observations
 */
RValue
istt_sum_sqr_diff_xy (void)
{
  return int_sum_sqr_diff_xy;
}



/*
 * istt_sum_xy
 *
 * Returns the sum of the x*y products for all observations
 */
RValue
istt_sum_xy (void)
{
  return int_sum_xy;
}



/*
 * istt_linear_regression
 *
 * Returns the linear and angular coefficients of the linear regression
 * for the current sample
 */
void
istt_linear_regression (RValue * lin_coeff, RValue * ang_coeff)
{
  /* Linear regression linear coefficient */
  *lin_coeff = ((int_sum_y * int_sum_sqr_x) - (int_sum_x * int_sum_xy)) /
    ((int_samples * int_sum_sqr_x) - __ISTT_SQR_ (int_sum_x));

  /* Linear regression angular coefficient */
  *ang_coeff = ((int_sum_x * int_sum_y) - (int_samples * int_sum_xy)) /
    (__ISTT_SQR_ (int_sum_x) - (int_samples * int_sum_sqr_x));
}



/*
 * istt_linear_correlation_coeff
 *
 * Returns the linear correlation coefficient between the x and y observations
 */
RValue
istt_linear_correlation_coeff (void)
{
  return ((int_samples * int_sum_xy) - (int_sum_x * int_sum_y)) /
    sqrt (((int_samples * int_sum_sqr_x) - __ISTT_SQR_ (int_sum_x)) *
          ((int_samples * int_sum_sqr_y) - __ISTT_SQR_ (int_sum_y)));
}



/*
 * RANDOM NUMBER GENERATORS
 */


/*
 * istt_uniform_random
 *
 * Returns a random value from an uniform distribution between 'low' and 'high'
 */
UsLgIntValue
istt_uniform_random (UsLgIntValue low, UsLgIntValue high)
{
  UsLgIntValue interval;
  UsLgIntValue aux_rand;
  RValue dbl_rand;

  if (low > high)
    {
      fprintf (stderr,
               "istt_uniform_random: lower bound greater than upper bound\n");
      return high;
    }

  interval = high - low;
  aux_rand = (UsLgIntValue) rand ();
  dbl_rand = (RValue) aux_rand *(interval / (RValue) RAND_MAX);
  dbl_rand += low;

  return (long) dbl_rand;
}



/*
 * istt_uniform_random_real
 *
 * Returns a random value from an uniform distribution between 'low' and 'high'
 */
RValue
istt_uniform_random_real (RValue low, RValue high)
{
  RValue interval;
  RValue aux_rand;
  RValue dbl_rand;

  if (low - high > DBL_EPSILON)
    {
      fprintf (stderr,
               "istt_uniform_random real: lower bound greater than upper bound\n");
      return high;
    }

  interval = high - low;
  aux_rand = (RValue) rand ();
  dbl_rand = (RValue) aux_rand *(interval / (RValue) RAND_MAX);
  dbl_rand += low;

  return dbl_rand;
}



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
 * - seed: new random seed to be used. If the value '0' is passed,
 *   the seed will be generated from the system clock
 */
RValue
istt_gaussian_random (RValue avg, RValue stddev, BoolValue randomize,
                      unsigned int seed)
{
  unsigned aux_seed;            /* auxiliary for the random seed */

  /* Auxiliary for the random number calculation */
  RValue v1 = 0.0, v2 = 0.0;
  RValue rsq = 2.0;
  RValue aux_rnd_1;
  RValue aux_rnd_2;
  RValue fac;

  /* Defines the random seed to be used */
  if (randomize == TRUE)
    {
      if (seed == 0)
        {
          /* If seed was passed 0, then the seed should be taken randomly */
          aux_seed = (unsigned int) time (NULL);
        }
      else
        aux_seed = seed;

      /* Sets the pseudo-random number generator seed */
      srand (aux_seed);
    }

  /* Keep searching for a pair of values inside an unitary circle */
  while (rsq - 1.0 > DBL_EPSILON)
    {
      /*
       * The pair (v1,v2) is not inside the unitary circle yet
       * Get a new pseudo-random value for aux_rnd_1 and aux_rnd_2
       */
      aux_rnd_1 = (RValue) rand ();
      aux_rnd_2 = (RValue) rand ();

      /* Normalize the values to a -1..+1 scale */
      v1 = 2.0 * (aux_rnd_1 / RAND_MAX) - 1.0;
      v2 = 2.0 * (aux_rnd_2 / RAND_MAX) - 1.0;

      /* Calculate the sum of squares */
      rsq = __ISTT_SQR_ (v1) + __ISTT_SQR_ (v2);
    }

  /* Generate the random value */
  fac = sqrt (-2 * (log (rsq) / rsq));

  /* Return the transformed random value */
  return (v1 * fac) * stddev + avg;
}



/*
 * istt_gaussian_randomize
 *
 * Defines a new random seed for the pseudo-random number generator,
 * based on the system clock
 */
void
istt_gaussian_randomize (void)
{
  srand ((unsigned int) time (NULL));
}
