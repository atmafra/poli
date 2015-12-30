#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "nnet_som_ngb.h"
#include "../nnet_units.h"
#include "../../function/function.h"
#include "../../vector/vector.h"

/******************************************************************************
 *                                                                            *
 *                       PRIVATE OPERATIONS AND VARIABLES                     *
 *                                                                            *
 ******************************************************************************/

/*
 * Neighborhood functions
 */

/*
 * nnet_som_rctngb
 *
 * Rectangular neighborhood function
 */
static RValue
nnet_som_rctngb (const RValue distance,
                 const RFunctionParameters nnet_ngb_par)
{
  RValue retval, w_0, w, T1;
  DTime t;
  static DTime last_t = 0;
  static RValue last_w = 0.0;
  static RValue last_T1 = 0.0;


  /* Initialization */
  t = (DTime) nnet_ngb_par[0];
  w_0 = nnet_ngb_par[1];
  T1 = nnet_ngb_par[2];

  /* Static variables */
  if (t != last_t || fabs (T1 - last_T1) > DBL_EPSILON)
    {
      /* Recalculate variance */
      w = w_0 * exp (-(RValue) t / T1);

      /* Update the statics */
      last_t = t;
      last_w = w;
      last_T1 = T1;
    }

  retval = (RValue)
    (distance > last_w - DBL_EPSILON
     || distance < -last_w + DBL_EPSILON ? 0.0 : 1.0);

  return retval;
}



/*
 * nnet_som_ngb_gaussian
 *
 * Gaussian neighborhood function
 */
static RValue
nnet_som_gssngb (const RValue distance,
                 const RFunctionParameters nnet_ngb_par)
{
  RValue retval, r_0, r, T1;
  DTime t;
  static DTime last_t = 0;
  static RValue last_r = 0.0;
  static RValue last_T1 = 0.0;

  /* Initialization */
  t = (DTime) nnet_ngb_par[0];
  r_0 = nnet_ngb_par[1];
  T1 = nnet_ngb_par[2];

  /* Static variables */
  if (t != last_t || fabs (T1 - last_T1) > DBL_EPSILON)
    {
      /* Recalculate variance */
      r = r_0 * exp (-((RValue) t) / T1);

      /* Update the statics */
      last_t = t;
      last_r = r;
      last_T1 = T1;
    }

  retval = (RValue) exp (-sqr (distance) / (2.0 * sqr (last_r)));

  return retval;
}



/******************************************************************************
 *                                                                            *
 *                               PUBLIC VARIABLES                             *
 *                                                                            *
 ******************************************************************************/

/*
 * Neighborhood Function Classes
 */

/* Rectangular */
static function_class_type nnet_som_ngb_struct_rectangular = {
  NNET_SOM_NGB_RECTANGLE,
  "Rectangular",
  nnet_som_rctngb,
  3,
  {"Time", "Initial Width", "Time Constant"},
  {0.0, 10.0, 1000.0}
};

static nnet_som_ngb_class_type nnet_som_ngb_rectangular_struct = {
  &nnet_som_ngb_struct_rectangular,
  VECTOR_METR_EUCLIDEAN
};

NgbFunctionClass nnet_som_ngb_rectangular = &nnet_som_ngb_rectangular_struct;



/* Gaussian */
static function_class_type nnet_som_ngb_struct_gaussian = {
  NNET_SOM_NGB_GAUSSIAN,
  "Gaussian",
  nnet_som_gssngb,
  3,
  {"Time", "Initial Radius", "Time Constant"},
  {0.0, 10.0, 1000.0}
};

static nnet_som_ngb_class_type nnet_som_ngb_gaussian_struct = {
  &nnet_som_ngb_struct_gaussian,
  VECTOR_METR_EUCLIDEAN
};

NgbFunctionClass nnet_som_ngb_gaussian = &nnet_som_ngb_gaussian_struct;



/* Set of Neighborhood Function classes */
RFunctionClassSet nnet_som_ngb_classes = {
  2,
  {&nnet_som_ngb_struct_rectangular,
   &nnet_som_ngb_struct_gaussian}
};



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_som_ngb_class_by_name
 *
 * Returns the layer class variable associated to the given name
 */
NgbFunctionClass
nnet_som_ngb_class_by_name (Name name)
{
  RFunctionClass ngb_function_class;    /* neighborhood function class */


  /* Fetches the function class */
  ngb_function_class = function_class_by_name (nnet_som_ngb_classes, name);

  if (ngb_function_class == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_class_by_name: neighborhood function class not found\n");
      return NULL;
    }

  /* Gets the neighborhood function class */
  switch (ngb_function_class->function_type)
    {
    case NNET_SOM_NGB_RECTANGLE:
      return nnet_som_ngb_rectangular;

    case NNET_SOM_NGB_GAUSSIAN:
      return nnet_som_ngb_gaussian;

    default:
      fprintf (stderr,
               "nnet_som_ngb_class_by_name: invalid neighborhood function type\n");
      return NULL;
    }
};



/*
 * nnet_som_ngb_create_function
 *
 * Creates a new neighborhood function with default parameter vector
 */
NgbFunction
nnet_som_ngb_create_function (const NgbFunctionClass ngb_class,
                              const NgbRFunctionParameters parameters)
{
  NgbFunction new_function;     /* new neighborhood function */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the class was actually passed */
  if (ngb_class == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_create_function: no function class passed\n");
      return NULL;
    }

  /* Allocates the new function */
  new_function = (NgbFunction) malloc (sizeof (nnet_som_ngb_function_type));

  if (new_function == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_create_function: virtual memory exhausted\n");
      return NULL;
    }

  /* Sets the function class */
  new_function->function_class = ngb_class;

  /* Instantiates the new real function */
  new_function->function = function_create (ngb_class->ngb_class);

  if (new_function->function == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_create_function: error creating neighborhood real function\n");
      free (new_function);
      return NULL;
    }

  /* If the parameter vector was passed, sets it */
  if (parameters != NULL)
    {
      exit_status =
        nnet_som_ngb_set_parameter_vector (new_function, parameters);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_ngb_create_function: error setting parameter vector\n");
          function_destroy (&(new_function->function));
          free (new_function);
          return NULL;
        }
    }

  return new_function;
}



/*
 * nnet_som_ngb_destroy_function
 *
 * Destroys a previously created neighborhood function
 */
int
nnet_som_ngb_destroy_function (NgbFunction * function)
{
  /* Check if the function was passed */
  if (function == NULL || *function == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_destroy_function: no function passed\n");
      return EXIT_FAILURE;
    }

  /* Destroys the internal real function */
  function_destroy (&((*function)->function));

  /* Frees the function itself */
  free (*function);

  /* Makes it point to NULL */
  *function = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_som_ngb_value
 *
 * Evaluates the neighborhood function between the two given vectors
 */
int
nnet_som_ngb_value (const NgbFunction function,
                    const Vector v1, const Vector v2, RValue * value)
{
  RValue distance;              /* metrics between the two vectors */
  RValue neighborhood;          /* neighborhood function value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function was actually passed */
  if (function == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_value: no neighborhood function\n");
      return EXIT_FAILURE;
    }

  /* Checks if the vectors were actually passed */
  if (v1 == NULL || v2 == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_value: missing vectors\n");
      return EXIT_FAILURE;
    }

  /* Calculates the metrics between the two vectors */
  exit_status = vector_metric
    (v1, v2, NULL, function->function_class->vector_metric, &distance);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "nnet_som_ngb_value: error calculating distance\n");
      return EXIT_FAILURE;
    }

  /* Calculates the value of the neighborhood function */
  neighborhood = function_value (function->function, distance);

  *value = neighborhood;

  return EXIT_SUCCESS;
}



/*
 * nnet_som_ngb_set_parameter
 *
 * Sets the neighborhood function parameters
 */
int
nnet_som_ngb_set_parameter (NgbFunction function,
                            const UsIntValue parameter_index,
                            const RValue value)
{
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function was actually passed */
  if (function == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_set_parameters: no function passed\n");
      return EXIT_FAILURE;
    }

  /* Sets the neighborhood function parameters */
  exit_status =
    function_set_parameter (function->function, parameter_index, value);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_ngb_set_parameters: error setting parameter %d\n",
               parameter_index);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_ngb_set_parameter_vector
 *
 * Sets the neighborhood function parameter vector
 */
int
nnet_som_ngb_set_parameter_vector (NgbFunction function,
                                   const NgbRFunctionParameters parameters)
{
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function was actually passed */
  if (function == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_set_parameter_vector: no function passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the parameter vector was actually passed */
  if (parameters == NULL)
    {
      fprintf (stderr,
               "nnet_som_ngb_set_parameter_vector: no parameter vector passed\n");
      return EXIT_FAILURE;
    }

  /* Sets the real function parameter vector */
  exit_status =
    function_set_parameter_vector (function->function, parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_ngb_set_parameter_vector: error setting parameter vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_ngb_function_info
 *
 * Outputs neighborhood function info
 */
void
nnet_som_ngb_function_info (NgbFunction function, FILE * output_fd)
{
  if (output_fd == NULL)
    return;

  fprintf (output_fd, "Neighborhood function class: %s\n",
           function->function->function_class->name);

  function_class_info (function->function->function_class, output_fd);
  function_info (function->function, output_fd);

  return;
}
