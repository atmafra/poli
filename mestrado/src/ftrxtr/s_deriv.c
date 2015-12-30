#include <math.h>
#include <float.h>
#include <stdio.h>

#include "incstat.h"
#include "errorh.h"

#include "s_samples.h"
#include "s_deriv.h"

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
cmp_real
sder_derivative (const cmp_real x_current,
                 const cmp_real x_previous,
                 const cmp_real x_next,
                 const smp_yes_no has_previous, const smp_yes_no has_next)
{
  /* linear regression coefficients */
  cmp_real lin_coeff = 0.0, ang_coeff = 0.0;


  if (has_previous == SMP_NO && has_next == SMP_NO)
    return 0.0;

  istt_clear_stat ();

  if (has_previous == SMP_YES)
    istt_add_stat (0.0, x_previous);

  istt_add_stat (1.0, x_current);

  if (has_next == SMP_YES)
    istt_add_stat (2.0, x_next);

  istt_linear_regression (&lin_coeff, &ang_coeff);

  return ang_coeff;
}



/*
 * sder_list_derivative
 *
 * Sets the second-half elements of the given list to be the
 * derivatives of the first-half elements
 */
int
sder_list_derivative (sample_list_type * list_cur,
                      sample_list_type * list_prev,
                      sample_list_type * list_next)
{
  cmp_real x_cur = 0.0;         /* current sample value: x[n] */
  cmp_real x_prev = 0.0;        /* x[n-1] */
  cmp_real x_next = 0.0;        /* x[n+1] */
  cmp_complex cur_value;        /* current complex value */
  cmp_complex drv_value;        /* derivative value */
  smp_num_samples cur_sample;   /* current sample counter */
  smp_num_samples half_sample;  /* middlepoint list sample */
  smp_yes_no has_prev = SMP_NO; /* current list has a previous list */
  smp_yes_no has_next = SMP_NO; /* current list has a next list */


  /* list must have an even number of elements */
  if (list_cur->samples % 2 != 0)
    return error_failure ("sder_list_derivative",
                          "list must have an even number of elemens\n");

  /* initialization */
  half_sample = list_cur->samples / 2;
  drv_value.re = 0.0;
  drv_value.im = 0.0;
  cur_value.re = 0.0;
  cur_value.im = 0.0;

  /* derivation loop */
  for (cur_sample = 1; cur_sample <= half_sample; cur_sample++)
    {
      if (get_list_value (*list_cur, cur_sample, &cur_value) != EXIT_SUCCESS)
        return error_failure ("sder_list_derivative",
                              "error getting %ld list value\n", cur_sample);

      x_cur = cur_value.re;

      if (list_prev != NULL)
        {
          if (get_list_value (*list_prev, cur_sample, &cur_value)
              != EXIT_SUCCESS)
            return error_failure ("sder_list_derivative",
                                  "error getting %ld previous list value\n",
                                  cur_sample);

          x_prev = cur_value.re;
          has_prev = SMP_YES;
        }

      if (list_next != NULL)
        {
          if (get_list_value (*list_next, cur_sample, &cur_value)
              != EXIT_SUCCESS)
            return error_failure ("sder_list_derivative",
                                  "error getting %ld previous list value\n",
                                  cur_sample);

          x_next = cur_value.re;
          has_next = SMP_YES;
        }

      drv_value.re = sder_derivative (x_cur, x_prev, x_next,
                                      has_prev, has_next);

      if (set_list_value (list_cur, half_sample + cur_sample, drv_value)
          != EXIT_SUCCESS)

        return error_failure ("sder_list_derivative",
                              "error setting derivative for first sample\n");
    }

  return EXIT_SUCCESS;
}



/*
 * sder_index_derivative
 *
 * Calculates the derivatives for all lists in the given index
 */
int
sder_index_derivative (index_list_type index)
{
  index_entry_type cur;         /* current index entry */
  sample_list_type *cur_list;   /* current list */
  sample_list_type *prev_list;  /* previous list */
  sample_list_type *next_list;  /* next list */

  cur = index.head;

  while (cur != NULL)
    {
      cur_list = cur->list;

      if (cur->previous != NULL)
        prev_list = cur->previous->list;
      else
        prev_list = NULL;

      if (cur->next != NULL)
        next_list = cur->next->list;
      else
        next_list = NULL;

      if (sder_list_derivative (cur_list, prev_list, next_list)
          != EXIT_SUCCESS)

        return error_failure ("sder_index_derivative",
                              "error calculating derivatives for list %s\n",
                              cur->name);

      cur = cur->next;
    }

  return EXIT_SUCCESS;
}
