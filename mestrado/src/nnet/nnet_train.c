#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "nnet_types.h"
#include "nnet_train.h"
#include "../function/function.h"


/******************************************************************************
 *                                                                            *
 *                      PRIVATE VARIABLES AND OPERATIONS                      *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lrate_fcn_fixed
 *
 * Fixed learning rate function. Input (time) is ignored.
 *
 * - parameter 1: learning rate (etha)
 */
static RValue nnet_lrate_fcn_fixed
  (const RValue input, const RFunctionParameters param)
{
  return param[0];
}



/*
 * nnet_lrate_fcn_percstep
 *
 * Percentual step time decay function
 *
 * - parameter 1: initial (time = 0) learning rate
 * - parameter 2: epoch duration
 * - parameter 3: percentual decay after each epoch
 */
static RValue nnet_lrate_fcn_percstep
  (const RValue input, const RFunctionParameters param)
{
  RValue retval, lrate_0, perc_decay;
  DTime T_epoch;
  DTime t;
  UsLgIntValue nu_epochs;
  RValue cur_decay;

  /* Initialization */
  lrate_0 = param[0];

  if (lrate_0 < DBL_EPSILON)
    lrate_0 = 0.0;

  if (lrate_0 > 1.0 - DBL_EPSILON)
    lrate_0 = 1.0;

  T_epoch = (DTime) param[1];

  perc_decay = 1.0 - param[2] / 100.0;

  if (perc_decay < DBL_EPSILON)
    perc_decay = 0.0;

  if (perc_decay > 1.0 - DBL_EPSILON)
    perc_decay = 1.0;

  /* Calculation */
  t = (DTime) input;
  nu_epochs = t / T_epoch;
  cur_decay = pow ((double) perc_decay, (double) nu_epochs);
  retval = lrate_0 * cur_decay;

  return retval;
}



/*
 * nnet_lrate_fcn_expdecay
 *
 * Exponential time decay learning rate
 *
 * - parameter 1: initial (time = 0) learning rate
 * - parameter 2: time constant
 */
static RValue nnet_lrate_fcn_expdecay
  (const RValue input, const RFunctionParameters param)
{
  RValue retval, etha_0, T;

  /* Initialization */
  etha_0 = param[0];

  if (etha_0 < DBL_EPSILON)
    etha_0 = 0.0;

  if (etha_0 > 1.0 - DBL_EPSILON)
    etha_0 = 1.0;

  T = param[1];

  if (T < DBL_EPSILON)
    T = 1.0;

  retval = etha_0 * exp ((double) -(input / T));

  return retval;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * Learning Rate Function Classes
 */

/* Fixed Value */
static function_class_type nnet_train_struct_fixed = {
  NNET_LRATE_FIXED,
  "Fixed Value",
  nnet_lrate_fcn_fixed,
  1,
  {"Learning Rate"},
  {0.05}
};

LRateFunctionClass nnet_train_lrate_fixed = &nnet_train_struct_fixed;



/* Percentual Step */
static function_class_type nnet_train_struct_percstep = {
  NNET_LRATE_PERCSTEP,
  "Percentual Step",
  nnet_lrate_fcn_percstep,
  3,
  {"Initial Learning Rate", "Epoch Duration", "Percentual Decay"},
  {0.1, 1000.0, 90.0}
};

LRateFunctionClass nnet_train_lrate_percstep = &nnet_train_struct_percstep;



/* Exponential Decay */
static function_class_type nnet_train_struct_expdecay = {
  NNET_LRATE_EXPDECAY,
  "Exponential Decay",
  nnet_lrate_fcn_expdecay,
  2,
  {"Initial Learning Rate", "Time Constant"},
  {0.1, 1000.0}
};

LRateFunctionClass nnet_train_lrate_expdecay = &nnet_train_struct_expdecay;;



/* Set of Learning Rate Function Classes */
RFunctionClassSet nnet_train_lrate_classes = {
  3,
  {
   &nnet_train_struct_fixed,
   &nnet_train_struct_percstep,
   &nnet_train_struct_expdecay}
};



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_train_lrate_class_by_name
 *
 * Returns the learning rate function class by the given name
 */
LRateFunctionClass
nnet_train_lrate_class_by_name (Name name)
{
  return function_class_by_name (nnet_train_lrate_classes, name);
}



/*
 * nnet_train_lrate_function_create
 *
 * Creates a new learning rate function instance
 */
LRateFunction
nnet_train_lrate_create (const LRateFunctionClass lrate_class,
                         const LRateFunctionParameters parameters)
{
  LRateFunction new_function;   /* new learning rate function */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function class was passed */
  if (lrate_class == NULL)
    {
      fprintf (stderr,
               "nnet_train_lrate_create: no learning rate function class passed\n");
      return NULL;
    }

  /* Instantiates the new function */
  new_function = function_create (lrate_class);

  if (new_function == NULL)
    {
      fprintf (stderr,
               "nnet_train_lrate_create: error creating learning rate function\n");
      return NULL;
    }

  /* If a parameter vector was passed, sets the parameters */
  if (parameters != NULL)
    {
      exit_status = function_set_parameter_vector (new_function, parameters);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_train_lrate_create: error setting parameter vector\n");
          function_destroy (&new_function);
          return NULL;
        }
    }

  return new_function;
}



/*
 * nnet_train_lrate_destroy
 *
 * Destroys a previously created learning rate function instance
 */
int
nnet_train_lrate_destroy (LRateFunction * lrate_function)
{
  /* Checks if the function was actually passed */
  if (lrate_function == NULL || *lrate_function == NULL)
    {
      fprintf (stderr, "nnet_train_lrate_destroy: no function to destroy\n");
      return EXIT_FAILURE;
    }

  /* Destroys the function */
  function_destroy (lrate_function);

  /* Makes it point to NULL */
  *lrate_function = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_train_lrate_value
 *
 * Returns the learning rate at the given time
 */
RValue
nnet_train_lrate_value (const LRateFunction lrate_function, const DTime time)
{
  /* Dispatches the execution */
  return function_value (lrate_function, (RValue) time);
}



/*
 * nnet_train_lrate_set_parameters
 *
 * Sets the learning rate function parameter vector
 */
int
nnet_train_lrate_set_parameters (LRateFunction lrate_function,
                                 const LRateFunctionParameters parameters)
{
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function and the parameters were passed */
  if (lrate_function == NULL)
    {
      fprintf (stderr,
               "nnet_train_lrate_set_parameters: no function passed\n");
      return EXIT_FAILURE;
    }

  if (parameters == NULL)
    {
      fprintf (stderr,
               "nnet_train_lrate_set_parameters: no parameter vector passed\n");
      return EXIT_FAILURE;
    }

  /* Dispatches the execution */
  exit_status = function_set_parameter_vector (lrate_function, parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_train_lrate_set_parameters: error setting parameter vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
