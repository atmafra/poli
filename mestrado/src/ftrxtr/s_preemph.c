#include <math.h>
#include <float.h>
#include <stdio.h>

#include "errorh.h"
#include "s_preemph.h"
#include "s_samples.h"

/*
 * spre_preemphasis
 *
 * Returns the preemphasized value x'[n] of x[n], according to the filter
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
spre_real
spre_preemphasis (const spre_filter_coeff_type alpha,
                  const spre_real x_current, const spre_real x_previous)
{
  spre_filter_coeff_type def_alpha;     /* Value of alpha to be used */


  /* Certifies for a valid alpha */
  if (alpha <= DBL_EPSILON || (spre_real) 1.0 - alpha <= DBL_EPSILON)
    def_alpha = __SPRE_DEFAULT_ALPHA_;
  else
    def_alpha = alpha;

  return x_current - def_alpha * x_previous;
}



/*
 * spre_list_preemphasis
 *
 * Applies the pre-emphasis to all of the samples in the given list
 */
int
spre_list_preemphasis (sample_list_type * list, const spre_real alpha)
{
  smp_num_samples cur_sample;   /* Current sample counter */

  /* Auxiliary current, saved and last list values */
  cmp_complex cur_value, saved_value, last_value;


  /* 'last_value' will be initialized with the value of the last sample */
  if (get_list_value (*list, list->samples, &last_value) != EXIT_SUCCESS)

    return error_failure ("spre_list_preemphasis",
                          "error reading last sample from list\n");

  /* Pre-emphasis loop */
  for (cur_sample = 1; cur_sample <= list->samples; cur_sample++)
    {
      /* Gets the value at the current position */
      if (get_list_value ((*list), cur_sample, &cur_value) != EXIT_SUCCESS)

        return error_failure ("spre_list_preemphasis",
                              "error getting %ld list value\n", cur_sample);

      /* Saves the current value */
      saved_value.re = cur_value.re;

      if (list->data_type == SMP_COMPLEX)
        saved_value.im = cur_value.im;
      else
        saved_value.im = 0.0;

      /* Calculates the new current value */
      cur_value.re = spre_preemphasis (alpha, cur_value.re, last_value.re);

      if (list->data_type == SMP_COMPLEX)
        cur_value.im = spre_preemphasis (alpha, cur_value.im, last_value.im);

      /* Sets the new current value */
      if (set_list_value (list, cur_sample, cur_value) != EXIT_SUCCESS)

        return error_failure ("spre_list_preemphasis",
                              "error setting %ld list value\n", cur_sample);

      /* Updates the last value */
      last_value.re = saved_value.re;

      if (list->data_type == SMP_COMPLEX)
        last_value.im = saved_value.im;
    }

  return EXIT_SUCCESS;
}
