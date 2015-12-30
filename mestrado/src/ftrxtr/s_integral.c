#include "s_integral.h"

/*
 * INTERNAL ERROR HANDLING FUNCTIONS
 */

/*
 * init_error_vector
 *
 * Initializes the error vector to a non-error status
 */
int
init_error_vector (int *error_status)
{
  /* Auxiliary error vector counter */
  int error_count;


  /* Validates pointer to error vector */
  if (error_status == NULL)
    {
      fprintf (stderr, "init_error_vector: unallocated error vector\n");
      return (EXIT_FAILURE);
    }

  /* Mark all error flags as FALSE */
  for (error_count = __INT_ERROR_FIRST; error_count <= __INT_ERROR_LAST;
       error_count++)
    error_status[error_count] = FALSE;

  /* Set the "no error" flag to TRUE */
  error_status[__INT_ERROR_NOERROR] = TRUE;

  return (EXIT_SUCCESS);
}


/*
 * display_error_vector
 *
 * Decodes the error vector into messages
 */
int
display_error_vector (int *error_status)
{
  /* Auxiliary error counter */
  int error_count;


  /* Validates pointer to error vector */
  if (error_status == NULL)
    {
      fprintf (stderr, "display_error_vector: unallocated error vector\n");
      return (EXIT_FAILURE);
    }

  if (error_status[__INT_ERROR_NOERROR] == FALSE)
    {
      /* At least one error found */
      fprintf (stderr, "integral: errors found.\n");

      /* Error vector decodification */
      for (error_count = __INT_ERROR_FIRST;
           error_count <= __INT_ERROR_LAST; error_count++)
        {
          if (error_status[error_count] == TRUE)
            {
              fprintf (stderr, "integral[%d]: ", error_count);

              /* Translate the error code into a message */
              switch (error_count)
                {
                case __INT_ERROR_BOUNDS:
                  fprintf (stderr, "invalid integration bounds.\n");
                  break;

                case __INT_ERROR_SEGMENTS:
                  fprintf (stderr, "invalid number of segments.\n");
                  break;

                case __INT_ERROR_METHOD:
                  fprintf (stderr, "unknown integration method.\n");
                  break;

                case __INT_ERROR_FUNCTION:
                  fprintf (stderr, "undefined integration function.\n");
                  break;

                case __INT_ERROR_SAMPLES:
                  fprintf (stderr, "undefined list of samples.\n");
                  break;
                }
            }
        }
      return (EXIT_FAILURE);
    }

  return (EXIT_SUCCESS);
}


/*
 * INTEGRATION FUNCTIONS
 */


/*
 * func_integral
 *
 * Numeric integral of function 'f', between the limits
 * 'lower_bound' and 'upper_bound'.
 * The parameter segments defines in how many parts the interval
 * between lower_bound and upper_bound will be broken into.
 * The higher the number of segments, the most accurate the interpolation
 * of the function.
 */
double
func_integral (const double lower_bound, const double upper_bound,
               const sint_segments_type segments,
               double (*f) (const double), const sint_method_type method)
{
  /* Error vector */
  int error_status[__INT_ERROR_LAST + 1];

  /* Integration step size */
  double increment;

  /* Auxiliary current segment */
  sint_segments_type curr_segment;

  /* Internal number of segments */
  sint_segments_type v_segments = segments;

  /* Internal integration method */
  sint_method_type v_int_method = method;

  /* Return value */
  double acc = 0.0;

  /* Auxiliary used during integration */
  double x;

  /* Simpson coefficient */
  double simpson_coeff = 0;

  /* Auxiliary function return status */
  int exit_status;


  /*
   * Initialization and defaults
   */

  /* Number of segments */
  if (v_segments == 0)
    v_segments = __INT_DEFAULT_SEGMENTS;

  /* Integration method */
  if (v_int_method == 0)
    v_int_method = __INT_DEFAULT_METHOD;

  /* Initialize the error vector */
  init_error_vector (error_status);

  /* Validates integration bounds */
  if (lower_bound > upper_bound)
    {
      error_status[__INT_ERROR_NOERROR] = FALSE;
      error_status[__INT_ERROR_BOUNDS] = TRUE;
    }

  /* Validates number of segments */
  /*
  if (v_segments < 0)
    {
      error_status[__INT_ERROR_NOERROR] = FALSE;
      error_status[__INT_ERROR_SEGMENTS] = TRUE;
    }
   */

  /* Validates integration method */
  switch (v_int_method)
    {
    case __INT_TRAPEZE:
      break;

    case __INT_SIMPSON:
      break;

    default:
      error_status[__INT_ERROR_NOERROR] = FALSE;
      error_status[__INT_ERROR_METHOD] = TRUE;
      break;
    }

  /* Validates pointer to integrating function */
  if (*f == NULL)
    {
      error_status[__INT_ERROR_NOERROR] = FALSE;
      error_status[__INT_ERROR_FUNCTION] = TRUE;
    }

  /* Handles exceptions */
  exit_status = display_error_vector (error_status);
  if (exit_status != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  /*
   * Initiates the numeric integration
   * If the bounds are equal, the initialization value of 'acc' will return.
   */

  if (lower_bound != upper_bound)
    {
      /* Defines the integration step size */
      increment = (upper_bound - lower_bound) / (double) v_segments;

      switch (v_int_method)
        {
          /* Trapeze integration method */
        case __INT_TRAPEZE:

          /* Initialization */
          acc += f (lower_bound) + f (upper_bound);

          /* Integration loop */
          for (curr_segment = 1; curr_segment < v_segments; curr_segment++)
            {
              x = lower_bound + curr_segment * increment;
              acc += 2 * f (x);
            }

          /* Finalization */
          acc *= increment / 2;

          break;

          /* Simpson integration method */
        case __INT_SIMPSON:

          /* Initialization */
          acc += f (lower_bound) + f (upper_bound);

          /* Integration loop */
          for (curr_segment = 1; curr_segment < segments; curr_segment++)
            {
              if (fmod (curr_segment, 2) == 0)
                simpson_coeff = 2;
              else
                simpson_coeff = 4;

              x = lower_bound + curr_segment * increment;
              acc += simpson_coeff * f (x);
            }

          /* Finalization */
          acc *= increment / 3.0;

          break;
        }
    }

  return (acc);
}
