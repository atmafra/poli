#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../common/types.h"
#include "function.h"


/******************************************************************************
 *                                                                            *
 *                               PUBLIC OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * function_class_create
 *
 * Creates and initializes function class variables
 */
int
function_class_create (RFunctionClass cls,
                       const RFunctionType function_class,
                       const RFunctionImplementation function,
                       const size_t nu_parameters,
                       const char *parameter_name[],
                       const RFunctionParameters parameter_default)
{
  size_t parm;                  /* Parameter counter */

  /* Class function */
  cls->function = function;

  if (nu_parameters < 0)
    {
      fprintf (stderr,
               "function_class_create: negative number of parameters\n");
      return EXIT_FAILURE;
    }
  else
    {
      cls->nu_parameters = nu_parameters;
    }

  /* RFunction parameter names and default values */
  for (parm = 0; parm < nu_parameters; parm++)
    {
      sprintf (cls->parameter_name[parm], "%s", parameter_name[parm]);
      cls->parameter_default[parm] = parameter_default[parm];
    }

  return EXIT_SUCCESS;
}



/*
 * function_class_destroy
 *
 * Frees up the memory allocated for an function class
 */
void
function_class_destroy (RFunctionClass cls)
{
  size_t parm;                  /* parameter counter */

  /* Releases the parameter names */
  for (parm = 0; parm < cls->nu_parameters; parm++)
    free (cls->parameter_name[parm]);

  /* Releases the parameter names and default values lists */
  free (cls->parameter_name);
  free (cls->parameter_default);

  return;
}



/*
 * function_create
 *
 * Creates and initializes a new function given the function class.
 * The function parameters are set to the defaults for the class.
 */
RFunction
function_create (const RFunctionClass function_class)
{
  RFunction function;           /* returned new function */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the function class was actually passed */
  if (function_class == NULL)
    {
      fprintf (stderr, "function_create: no function class passed\n");
      return NULL;
    }

  /* Allocates the new function */
  function = (RFunction) malloc (sizeof (function_type));
  if (function == NULL)
    {
      fprintf (stderr, "function_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Sets the function class */
  function->function_class = function_class;

  /* Allocates and initializes the parameters with defaults */
  function->parameters = (RValue *)
    malloc (function_class->nu_parameters * sizeof (RValue));

  if (function->parameters == NULL)
    {
      fprintf (stderr, "function_create: virtual memory exhausted\n");
      free (function);
      return NULL;
    }

  /* Sets the parameters according to the function class defaults */
  exit_status =
    function_set_parameter_vector (function,
                                   function_class->parameter_default);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "function_create: error setting parameter vector\n");
      free (function);
      free (function->parameters);
      return NULL;
    }

  return function;
}



/*
 * function_destroy
 *
 * Destroys a previously created function
 */
void
function_destroy (RFunction * function)
{
  free ((*function)->parameters);
  free (*function);
  *function = NULL;

  return;
}



/*
 * function_class_by_name
 *
 * Returns the function class given its name
 */
RFunctionClass
function_class_by_name (const RFunctionClassSet class_set, const Name name)
{
  UsIntValue cur_class;         /* current function class */

  /* Searches the class */
  for (cur_class = 0; cur_class < class_set.nu_classes; cur_class++)
    {
      if (strncmp
          (name, class_set.function_class[cur_class]->name,
           sizeof (Name)) == 0)
        return class_set.function_class[cur_class];
    }

  /* Function class not found */
  fprintf (stderr,
           "function_class_by_name: no function class with the given name: '%s'\n",
           name);
  return NULL;
}



/*
 * function_value
 *
 * Returns the function value for the given input with the current parameters
 */
RValue
function_value (const RFunction function, const RValue input)
{
  return function->function_class->function (input, function->parameters);
}



/*
 * function_set_parameter
 *
 * Sets one parameter of the function
 */
int
function_set_parameter (RFunction function,
                        const UsIntValue parameter_index, const RValue value)
{
  /* Checks if the function was actually passed */
  if (function == NULL)
    {
      fprintf (stderr, "function_set_parameter: no function passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the parameter index is valid */
  if (parameter_index < 0
      || parameter_index >= function->function_class->nu_parameters)
    {
      fprintf (stderr,
               "function_set_parameter: invalid parameter index: %d (function has %d parameters\n",
               parameter_index, function->function_class->nu_parameters);
      return EXIT_FAILURE;
    }

  /* Sets the parameter value */
  function->parameters[parameter_index] = value;

  return EXIT_SUCCESS;
}



/*
 * function_set_parameter_vector
 *
 * Sets the parameters of the function to the given parameter vector
 */
int
function_set_parameter_vector (RFunction function,
                               const RFunctionParameters parameters)
{
  size_t parm;                  /* parameter counter */
  int exit_status;              /* auxiliary function return status */

  for (parm = 0; parm < function->function_class->nu_parameters; parm++)
    {
      exit_status = function_set_parameter (function, parm, parameters[parm]);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "function_set_parameter_vector: error setting parameter %d\n",
                   parm);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * function_class_info
 *
 * Outputs function class information
 */
void
function_class_info (const RFunctionClass function_class, FILE * output_fd)
{
  size_t cur_param;             /* auxiliary parameter counter */

  if (output_fd == NULL)
    return;

  fprintf (output_fd, "  Number of Parameters: %d\n",
           function_class->nu_parameters);

  for (cur_param = 0; cur_param < function_class->nu_parameters; cur_param++)
    {
      fprintf (output_fd, "    parameter %d: %s (default %f)\n", cur_param,
               function_class->parameter_name[cur_param],
               function_class->parameter_default[cur_param]);
    }

  return;
}



/*
 * function_info
 *
 * Outputs function information
 */
void
function_info (const RFunction function, FILE * output_fd)
{
  size_t cur_param;             /* auxiliary parameter counter */

  if (output_fd == NULL)
    return;

  for (cur_param = 0; cur_param < function->function_class->nu_parameters;
       cur_param++)
    fprintf (output_fd, "%s: %f\n",
             function->function_class->parameter_name[cur_param],
             function->parameters[cur_param]);

  return;
}
