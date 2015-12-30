#ifndef __NNET_WEIGHTS_H_
#define __NNET_WEIGHTS_H_ 1

#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/* Weight Initialization Functions */

/* Fixed Weight */
extern WeightInitClass nnet_wght_init_fixed_weight;

/* Random Weight */
extern WeightInitClass nnet_wght_init_uniform_random;

/* Gaussian Random Weight */
extern WeightInitClass nnet_wght_init_gaussian_random;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_wght_class_by_name
 *
 * Returns a weight initialization function class by its class name
 */
extern WeightInitClass nnet_wght_class_by_name (Name name);



/*
 * nnet_wght_create_function
 *
 * Creates and initializes a new weight initialization function given the
 * weight initialization function class.
 *
 * The function parameters are set to the defaults for the class
 */
extern WeightInitFunction
nnet_wght_create_function (const WeightInitClass wght_class);



/*
 * nnet_wght_destroy_function
 *
 * Destroy a previously created weight initialization
 */
extern void nnet_wght_destroy_function (WeightInitFunction * function);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_wght_value
 *
 * Returns the function value for the given input with the current parameters
 */
extern RValue nnet_wght_value (const WeightInitFunction function);



/*
 * nnet_wght_set_parameters
 *
 * Sets the parameters for the given function.
 * The number of parameters is defined in the function class.
 */
extern int nnet_wght_set_parameters
  (WeightInitFunction function, const WeightInitParameters parameters);


#endif /* __NNET_WEIGHTS_H_ */
