#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "nnet_types.h"
#include "nnet_actv.h"
#include "function.h"


/******************************************************************************
 *                                                                            *
 *                      PRIVATE OPERATIONS AND VARIABLES                      *
 *                                                                            *
 ******************************************************************************/

/*
 * Activation Function Definitions:
 * . number of parameters
 * . parameter names
 * . parameter default values
 */

/*
 * nnet_actv_fcn_passthrough
 *
 * Pass-through activation function
 * The input is copied to the output.
 * No parameters required.
 */
static RValue nnet_actv_fcn_passthrough
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  return nnet_unit_activation;
}



/*
 * nnet_actv_fcn_threshold
 *
 * Threshold activation function
 * - nnet_actv_par_1: threshold point
 * - nnet_actv_par_2: value on the left side of the threshold point
 * - nnet_actv_par_3: value on the right side of the threshold point
 */
static RValue nnet_actv_fcn_threshold
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  RValue retval, tpoint, lvalue, rvalue, a;

  /* Initialization */
  a = nnet_unit_activation;
  tpoint = nnet_actv_par[0];
  lvalue = nnet_actv_par[1];
  rvalue = nnet_actv_par[2];

  /* Return value */
  retval = (a >= tpoint ? rvalue : lvalue);

  return retval;
}



/*
 * nnet_actv_fcn_linear
 *
 * Linear activation function
 * - nnet_actv_par_1: angular coefficient
 * - nnet_actv_par_2: linear coefficient
 */
static RValue nnet_actv_fcn_linear
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  RValue retval, angc, linc, a;

  /* Initialization */
  a = nnet_unit_activation;
  angc = nnet_actv_par[0];
  linc = nnet_actv_par[1];

  /* Return value */
  retval = linc + angc * a;

  return retval;
}



/*
 * nnet_actv_fcn_pcwlinear
 *
 * Piecewise-linear activation function
 * - nnet_actv_par_1: left threshold point
 * - nnet_actv_par_2: right threshold point
 * - nnet_actv_par_3: value on the left side of the left threshold
 * - nnet_actv_par_4: value on the right side of the right threshold
 *
 * Definition
 *
 * f (activation) =
 *   left value , if activation <= left threshold;
 *   right value, if activation >= right threshold
 *   linear     , if left threshold < activation < right threshold
 *
 * If right value = left value, threshold function will be called.
 * If right value < left value, values will be shifted.
 */
static RValue nnet_actv_fcn_pcwlinear
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  RValue retval, lthresh, rthresh, lvalue, rvalue, a;


  /* Initialization */
  a = nnet_unit_activation;
  if (nnet_actv_par[0] < nnet_actv_par[1])
    {
      lthresh = nnet_actv_par[0];
      rthresh = nnet_actv_par[1];
    }
  else
    {
      lthresh = nnet_actv_par[1];
      rthresh = nnet_actv_par[0];
    }
  lvalue = nnet_actv_par[2];
  rvalue = nnet_actv_par[3];

  /* Return value */
  if (a <= lthresh)
    {
      retval = lvalue;
    }
  else if (a >= rthresh)
    {
      retval = rvalue;
    }
  else
    {
      retval =
        ((a - lthresh) * (rvalue - lvalue)) / (rthresh - lthresh) + lvalue;
    }

  return retval;
}



/*
 * nnet_actv_fcn_sigmoidal
 *
 * Sigmoidal activation function
 * - nnet_actv_par_1: scale coefficient
 * - nnet_actv_par_2: exponent coefficient
 */
static RValue nnet_actv_fcn_sigmoidal
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  RValue retval, k1, k2, a;

  /* Initialization */
  a = nnet_unit_activation;
  k1 = nnet_actv_par[0];
  k2 = nnet_actv_par[1];


  /* Return value */
  retval = k1 / (1.0 + exp (-k2 * a));

  return retval;
}



/*
 * nnet_actv_fcn_tanh
 *
 * Hyperbolic Tangent activation function
 * - nnet_actv_par_1: scale coefficient
 * - nnet_actv_par_2: angle coefficient
 */
static RValue nnet_actv_fcn_tanh
  (const RValue nnet_unit_activation, const RFunctionParameters nnet_actv_par)
{
  RValue retval, k1, k2, a;

  /* Initialization */
  a = nnet_unit_activation;
  k1 = nnet_actv_par[0];
  k2 = nnet_actv_par[1];

  /* Return value */
  retval = k1 * tanh (k2 * a);

  return retval;
}



/******************************************************************************
 *                                                                            *
 *                               PUBLIC VARIABLES                             *
 *                                                                            *
 ******************************************************************************/

/*
 * Activation Function Classes
 */
/* Pass-through function class */
function_class_type nnet_actv_struct_passthrough = {
  NNET_ACTV_PASSTHROUGH,
  "Pass-through",
  nnet_actv_fcn_passthrough,
  0,
  {}
  ,
  {}
  ,
};

ActivationClass nnet_actv_class_passthrough = &nnet_actv_struct_passthrough;


/* Threshold function class */
function_class_type nnet_actv_struct_threshold = {
  NNET_ACTV_THRESHOLD,
  "Threshold",
  nnet_actv_fcn_threshold,
  3,
  {"Threshold activation", "Left value", "Right value"}
  ,
  {0.0, 0.0, 1.0}
  ,
};

ActivationClass nnet_actv_class_threshold = &nnet_actv_struct_threshold;


/* Linear function class */
function_class_type nnet_actv_struct_linear = {
  NNET_ACTV_LINEAR,
  "Linear",
  nnet_actv_fcn_linear,
  2,
  {"Angular coefficient", "Linear coefficient"}
  ,
  {1.0, 0.0}
  ,
};

ActivationClass nnet_actv_class_linear = &nnet_actv_struct_linear;


/* Piecewise-linear function class */
function_class_type nnet_actv_struct_pcwlinear = {
  NNET_ACTV_PCWLINEAR,
  "Piecewise Linear",
  nnet_actv_fcn_pcwlinear,
  4,
  {"Left threshold activation", "Right threshold activation",
   "Left value", "Right value"}
  ,
  {-0.5, +0.5, 0.0, 1.0}
  ,
};

ActivationClass nnet_actv_class_pcwlinear = &nnet_actv_struct_pcwlinear;


/* Sigmoidal function class */
function_class_type nnet_actv_struct_sigmoidal = {
  NNET_ACTV_SIGMOIDAL,
  "Sigmoidal",
  nnet_actv_fcn_sigmoidal,
  2,
  {"Scale coefficient", "Exponent coefficient"}
  ,
  {1.0, 1.0}
  ,
};

ActivationClass nnet_actv_class_sigmoidal = &nnet_actv_struct_sigmoidal;


/* Hyperbolic Tangent function class */
function_class_type nnet_actv_struct_tanh = {
  NNET_ACTV_TANH,
  "Hyperbolic Tangent",
  nnet_actv_fcn_tanh,
  2,
  {"Scale coefficient", "Angle coefficient"}
  ,
  {1.0, 1.0}
  ,
};

ActivationClass nnet_actv_class_tanh = &nnet_actv_struct_tanh;



/* Set of Activation Function Classes */
RFunctionClassSet nnet_actv_classes = {
  6,
  {
   (ActivationClass) & nnet_actv_struct_passthrough,
   (ActivationClass) & nnet_actv_struct_threshold,
   (ActivationClass) & nnet_actv_struct_linear,
   (ActivationClass) & nnet_actv_struct_pcwlinear,
   (ActivationClass) & nnet_actv_struct_sigmoidal,
   (ActivationClass) & nnet_actv_struct_tanh,
   }
};



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_actv_class_by_name
 *
 * Returns the activation class variable associated to the given name
 */
ActivationClass
nnet_actv_class_by_name (Name name)
{
  return function_class_by_name (nnet_actv_classes, name);
}



/*
 * nnet_actv_create_function
 *
 * Creates an activation function of the given class
 */
ActivationFunction nnet_actv_create_function
  (const ActivationClass activation_class)
{
  return function_create (activation_class);
}



/*
 * nnet_actv_destroy_function
 *
 * Destroys a previously created activation function
 */
void
nnet_actv_destroy_function (ActivationFunction * function)
{
  function_destroy (function);
  return;
}



/*
 * nnet_actv_value
 *
 * Returns the function value for the given input with the current parameters
 */
RValue
nnet_actv_value (const ActivationFunction function, const RValue input)
{
  return function_value (function, input);
}



/*
 * nnet_actv_set_parameters
 *
 * Sets the parameters for the given function.
 * The number of parameters is defined in the function class.
 */
int nnet_actv_set_parameters
  (ActivationFunction function, const ActivationParameters parameters)
{
  int exit_status;              /* auxiliary function return status */

  /* Dispatches the execution */
  exit_status = function_set_parameter_vector (function, parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_actv_set_parameters: error setting parameter vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_actv_class_info
 *
 * Outputs activation function class information
 */
void nnet_actv_class_info
  (const ActivationClass activation_class, FILE * output_fd)
{
  size_t cur_param;             /* auxiliary parameter counter */

  if (output_fd == NULL)
    return;

  fprintf (output_fd, "Function Class: %s\n", activation_class->name);
  fprintf (output_fd, "  Number of Parameters: %d\n",
           activation_class->nu_parameters);

  for (cur_param = 0; cur_param < activation_class->nu_parameters;
       cur_param++)
    {
      fprintf (output_fd, "    parameter %lu: %s (default %f)\n", cur_param,
               activation_class->parameter_name[cur_param],
               activation_class->parameter_default[cur_param]);
    }

  return;
}



/*
 * nnet_actv_function_info
 *
 * Returns a string containing activation function information
 */
void nnet_actv_function_info
  (const ActivationFunction function, FILE * output_fd)
{
  size_t cur_param;             /* auxiliary parameter counter */

  if (output_fd == NULL)
    return;

  fprintf (output_fd, "Activation class: %s\n",
           function->function_class->name);

  for (cur_param = 0; cur_param < function->function_class->nu_parameters;
       cur_param++)
    fprintf (output_fd, "%s: %f\n",
             function->function_class->parameter_name[cur_param],
             function->parameters[cur_param]);

  return;
}
