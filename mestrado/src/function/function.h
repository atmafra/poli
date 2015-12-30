#ifndef __function_H_
#define __function_H_ 1

#include <stdio.h>
#include "../common/types.h"

#ifndef RFUNC_NAME_SIZE
#define RFUNC_NAME_SIZE 64
#endif

#ifndef RFUNC_MAX_PARAMETERS
#define RFUNC_MAX_PARAMETERS 10
#endif

#ifndef RFUNC_MAX_CLASSES
#define RFUNC_MAX_CLASSES 10
#endif


/******************************************************************************
 *                                                                            *
 *                         GENERIC FUNCTION DATATYPE                          *
 *                                                                            *
 ******************************************************************************/

/* Basic Types */
typedef int RFunctionType;
typedef RValue *RFunctionParameters;


/*
 * RFunctionImplementation
 *
 * Generic real to real function implementation.
 * Takes a real number as input and outputs a real number.
 * May have a list of parameters.
 *
 * f(x,p1,..., pN): R -> R
 */
typedef RValue (*RFunctionImplementation)
  (const RValue input, const RFunctionParameters parameters);


/*
 * function_class_type
 *
 * Generic function class datatype
 *
 * - function_class: activation function class identification
 * - function: pointer to the activation function
 * - nu_parameters: number of parameters the function takes
 * - parameter_name: list of descriptions of the parameters
 * - parameter_default: default values for the parameters
 */
typedef struct
{
  RFunctionType function_type;
  Name name;
  RFunctionImplementation function;
  UsIntValue nu_parameters;
  Name parameter_name[RFUNC_MAX_PARAMETERS];
  RValue parameter_default[RFUNC_MAX_PARAMETERS];
}
function_class_type;

/* Symbolic Types */
typedef function_class_type *RFunctionClass;



/*
 * function_type
 *
 * Activation function implementations.
 *
 * - activation_class: pointer to an activation function class
 * - parameters: list parameter values for the instance
 */
typedef struct
{
  RFunctionClass function_class;
  RFunctionParameters parameters;
}
function_type;

/* Symbolic Type */
typedef function_type *RFunction;



/*
 * RFunctionClassSet
 *
 * Set of available function classes
 */
typedef struct
{
  UsIntValue nu_classes;
  RFunctionClass function_class[RFUNC_MAX_CLASSES];
}
RFunctionClassSet;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * function_class_create
 *
 * Creates and initializes function class variables
 */
extern int
function_class_create (RFunctionClass cls,
                       const RFunctionType function_class,
                       const RFunctionImplementation function,
                       const size_t nu_parameters,
                       const char *parameter_name[],
                       const RFunctionParameters parameter_default);



/*
 * function_class_destroy
 *
 * Frees up the memory allocated for a function class
 */
extern void function_class_destroy (RFunctionClass cls);



/*
 * function_create
 *
 * Creates and initializes a new function given the function class.
 * The function parameters are set to the defaults for the class.
 */
extern RFunction function_create (const RFunctionClass function_class);



/*
 * function_destroy
 *
 * Destroys a previously created function
 */
extern void function_destroy (RFunction * function);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * function_class_by_name
 *
 * Returns the function class given its name
 */
extern RFunctionClass
function_class_by_name (const RFunctionClassSet class_set, const Name name);



/*
 * function_value
 *
 * Returns the function value for the given input with the current parameters
 */
extern RValue function_value (const RFunction function, const RValue input);



/*
 * function_set_parameter
 *
 * Sets one parameter of the function
 */
extern int
function_set_parameter (RFunction function,
                        const UsIntValue parameter_index, const RValue value);



/*
 * function_set_parameter_vector
 *
 * Sets the parameters of the function to the given parameter vector
 */
extern int
function_set_parameter_vector (RFunction function,
                               const RFunctionParameters parameters);



/*
 * function_class_info
 *
 * Outputs function class information
 */
extern void
function_class_info (const RFunctionClass function_class, FILE * output_fd);



/*
 * function_info
 *
 * Outputs function information
 */
extern void function_info (const RFunction function, FILE * output_fd);


#endif /*  function.h  */
