#include <stdlib.h>
#include <float.h>
#include "nnet_lvq_window.h"
#include "../nnet_units.h"
#include "../../errorh/errorh.h"
#include "../../vector/vector.h"

/******************************************************************************
 *                                                                            *
 *                              PRIVATE OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_winfunction
 *
 * Evaluates if the input vector is inside the window defined
 * between the two winning units weight vectors
 */
BoolValue
nnet_lvq_winfunction (const Vector x, const Vector w1, const Vector w2,
                      const RValue w)
{
  RValue d1;                    /* distance between input an winner 1 */
  RValue d2;                    /* distance between input an winner 2 */
  RValue d12;                   /* ratio between d1 and d2 */
  RValue d21;                   /* ratio between d2 and d1 */
  RValue d_min_ratio;           /* minimum ratio between d1 and d2 */

  /* calculates the distance between input vector and both weight vectors */
  if (error_if_failure
      (vector_metr_euclidean (x, w1, &d1), "nnet_lvq_window",
       "error calculating distance between input and first winner"))
    return EXIT_FAILURE;

  if (error_if_failure
      (vector_metr_euclidean (x, w2, &d2), "nnet_lvq_window",
       "error calculating distance between input and second winner"))
    return EXIT_FAILURE;

  /* calculates the minumum ratio between the two distances */
  if (d1 <= DBL_EPSILON || d2 <= DBL_EPSILON)
    return FALSE;
  else
    {
      d12 = d1 / d2;
      d21 = d2 / d1;
      d_min_ratio = (d12 < d21 ? d12 : d21);
      return (d_min_ratio > (1.0 - w) / (1.0 + w) ? TRUE : FALSE);
    }
}



/******************************************************************************
 *                                                                            *
 *                               PUBLIC OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_window
 *
 * Evaluates if the input vector is inside the window defined
 * between the two winners, according to the window width
 */
int
nnet_lvq_window (const Vector input_vector,
                 const Unit winner1, const Unit winner2,
                 const RValue window_width, BoolValue * result)
{
  UsLgIntValue dim = 0;         /* input vector dimension */
  Vector w1 = NULL;             /* winner 1 input connection vector */
  Vector w2 = NULL;             /* winner 2 input connection vector */


  /* checks if the input vector was actually passed */
  if (error_if_null (input_vector, "nnet_lvq_window",
                     "no input vector passed"))
    return EXIT_FAILURE;

  /* checks if the winning unit was actually passed */
  if (error_if_null (winner1, "nnet_lvq_window", "no winning unit passed"))
    return EXIT_FAILURE;

  /* checks the window width interval */
  if (window_width <= DBL_EPSILON || window_width >= 1.0 - DBL_EPSILON)
    return error_failure ("nnet_lvq_window",
                          "invalid value for w: %f", window_width);

  /* checks dimensional compatibility */
  dim = input_vector->dimension;
  if (winner1->nu_inputs != dim)
    return error_failure
      ("nnet_lvq_window",
       "incompatible dimensions between input vector (%ld) and winner inputs (%ld)",
       dim, input_vector->dimension);

  /* trivial case: second winner undefined */
  if (winner2 == NULL)
    {
      *result = FALSE;
      return EXIT_SUCCESS;
    }

  /* gets the weight vectors for both winners */
  if (error_if_failure
      (nnet_unit_get_weight_vector (winner1, w1), "nnet_lvq_window",
       "error getting weight vector for first winner"))
    return EXIT_FAILURE;

  if (error_if_failure
      (nnet_unit_get_weight_vector (winner2, w2), "nnet_lvq_window",
       "error getting weight vector for second winner"))
    return EXIT_FAILURE;

  /* evaluates the window function */
  *result = nnet_lvq_winfunction (input_vector, w1, w2, window_width);

  return EXIT_SUCCESS;
}
