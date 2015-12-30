#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nnet_weights.h"
#include "../function/function.h"
#include "../incstat/incstat.h"

/******************************************************************************
 *                                                                            *
 *                       PRIVATE OPERATIONS AND VARIABLES                     *
 *                                                                            *
 ******************************************************************************/

/*
 * Weight Initialization functions
 */

/*
 * nnet_wght_fcn_fxdwght
 *
 * Fixed weight function
 *
 * - nnet_wght_par 1: weight
 */
static RValue nnet_wght_fcn_fxdwght
  (const RValue input, const RFunctionParameters nnet_wght_par)
{
  return nnet_wght_par[0];
}



/*
 * nnet_wght_fcn_unfrand
 *
 * Uniform random function
 *
 * - nnet_wght_par 1: lower limit
 * - nnet_wght_par 2: upper limit
 */
static RValue nnet_wght_fcn_unfrand
  (const RValue input, const RFunctionParameters nnet_wght_par)
{
  RValue retval, lvalue, rvalue;

  /* Initialization */
  lvalue = nnet_wght_par[0];
  rvalue = nnet_wght_par[1];

  /* Return value */
  retval = (RValue) istt_uniform_random_real (lvalue, rvalue);

  return retval;
}



/*
 * nnet_wght_fcn_gssrand
 *
 * Gaussian random function
 *
 * - nnet_wght_par 1: average
 * - nnet_wght_par 2: standard deviation
 */
static RValue nnet_wght_fcn_gssrand
  (const RValue input, const RFunctionParameters nnet_wght_par)
{
  RValue retval, avg, stddev;

  /* Initialization */
  avg = nnet_wght_par[0];
  stddev = nnet_wght_par[1];

  /* Return value */
  retval = istt_gaussian_random (avg, stddev, FALSE, 0);

  return retval;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/* Weight Initialization function classes */

/* Fixed Weight initialization class */
static function_class_type nnet_wght_struct_fxdwght = {
  NNET_WGHT_FIXED,
  "Fixed Value",
  nnet_wght_fcn_fxdwght,
  1,
  {"Weight"},
  {1.0}
};



/* Uniform Random initialization function class */
static function_class_type nnet_wght_struct_unfrand = {
  NNET_WGHT_UNIFORM,
  "Uniform Random",
  nnet_wght_fcn_unfrand,
  2,
  {"Lower Limit", "Upper Limit"},
  {-1.0, +1.0}
};



/* Fixed Weight initialization class */
static function_class_type nnet_wght_struct_gssrand = {
  NNET_WGHT_GAUSSIAN,
  "Gaussian Random",
  nnet_wght_fcn_gssrand,
  2,
  {"Average", "Standard Deviation"},
  {0.0, +0.1}
};



/* Fixed Weight */
WeightInitClass nnet_wght_init_fixed_weight = &nnet_wght_struct_fxdwght;

/* Random Weight */
WeightInitClass nnet_wght_init_uniform_random = &nnet_wght_struct_unfrand;

/* Gaussian Random Weight */
WeightInitClass nnet_wght_init_gaussian_random = &nnet_wght_struct_gssrand;



/* Set of Weight Initialization Function Classes */
RFunctionClassSet nnet_wght_classes = {
  3,
  {
   &nnet_wght_struct_fxdwght,
   &nnet_wght_struct_unfrand,
   &nnet_wght_struct_gssrand}
};



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_wght_class_by_name
 *
 * Returns a weight initialization function class by its class name
 */
WeightInitClass
nnet_wght_class_by_name (Name name)
{
  return function_class_by_name (nnet_wght_classes, name);
}



/*
 * nnet_wght_create_function
 *
 * Creates and initializes a new weight initialization function given the
 * weight initialization function class.
 *
 * The function parameters are set to the defaults for the class
 */
WeightInitFunction
nnet_wght_create_function (const WeightInitClass wght_class)
{
  return function_create (wght_class);
}



/*
 * nnet_wght_destroy_function
 *
 * Destroys a previously created weight initialization
 */
void
nnet_wght_destroy_function (WeightInitFunction * function)
{
  function_destroy (function);
  return;
}



/*
 * nnet_wght_value
 *
 * Returns the function value for the given input with the current parameters
 */
RValue
nnet_wght_value (const WeightInitFunction function)
{
  /* Weight initialization functions don't have inputs */
  return function_value (function, 0.0);
}



/*
 * nnet_wght_set_parameters
 *
 * Sets the parameters for the given function.
 * The number of parameters is defined in the function class.
 */
int nnet_wght_set_parameters
  (WeightInitFunction function, const WeightInitParameters parameters)
{
  int exit_status;              /* auxiliary function return status */

  /* Dispatches the execution to 'function_set_parameter_vector' */
  exit_status = function_set_parameter_vector (function, parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_wght_set_parameters: error setting function parameter vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
