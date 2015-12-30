#ifndef __NNET_ACTV_H_
#define __NNET_ACTV_H_ 1

#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/* Activation Function Classes */

/* Pass-through function class */
extern ActivationClass nnet_actv_class_passthrough;

/* Threshold function class */
extern ActivationClass nnet_actv_class_threshold;

/* Linear function class */
extern ActivationClass nnet_actv_class_linear;

/* Piecewise-linear function class */
extern ActivationClass nnet_actv_class_pcwlinear;

/* Sigmoidal function class */
extern ActivationClass nnet_actv_class_sigmoidal;

/* Hyperbolic Tangent function class */
extern ActivationClass nnet_actv_class_tanh;


/* Set of Activation Function Classes */
extern RFunctionClassSet nnet_actv_classes;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_actv_class_by_name
 *
 * Returns the activation class variable associated to the given name
 */
extern ActivationClass nnet_actv_class_by_name (Name name);



/*
 * nnet_actv_create_function
 *
 * Creates and initializes a new activation function given the
 * activation function class.
 *
 * The function parameters are set to the defaults for the class
 */
extern ActivationFunction
nnet_actv_create_function (const ActivationClass activation_class);



/*
 * nnet_actv_destroy_function
 *
 * Destroys a previously created activation function
 */
extern void nnet_actv_destroy_function (ActivationFunction * function);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_actv_value
 *
 * Returns the function value for the given input with the current parameters
 */
extern RValue nnet_actv_value
  (const ActivationFunction function, const RValue input);



/*
 * nnet_actv_set_parameters
 *
 * Sets the parameters for the given function.
 * The number of parameters is defined in the function class.
 */
extern int nnet_actv_set_parameters
  (ActivationFunction function, const ActivationParameters parameters);



/*
 * nnet_actv_class_info
 *
 * Outputs activation function class information
 */
extern void nnet_actv_class_info
  (const ActivationClass activation_class, FILE * output_fd);



/*
 * nnet_actv_function_info
 *
 * Outputs activation function information
 */
extern void nnet_actv_function_info
  (const ActivationFunction function, FILE * output_fd);


#endif /*  nnet_actv.h  */
