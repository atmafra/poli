#ifndef __NNET_TRAIN_H_
#define __NNET_TRAIN_H_ 1

#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * Learning Rate Function Classes
 */

/* Fixed Value */
extern LRateFunctionClass nnet_train_lrate_fixed;

/* Percentual Step */
extern LRateFunctionClass nnet_train_lrate_percstep;

/* Exponential Decay */
extern LRateFunctionClass nnet_train_lrate_expdecay;



/* Learning Rate function class names */
extern const char nnet_train_lrate_class_name[][];



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_train_lrate_class_by_name
 *
 * Returns the learning rate function class by the given name
 */
extern LRateFunctionClass nnet_train_lrate_class_by_name (Name name);



/*
 * nnet_train_lrate_create
 *
 * Creates a new learning rate function instance
 */
extern LRateFunction
nnet_train_lrate_create (const LRateFunctionClass lrate_class,
                         const LRateFunctionParameters parameters);



/*
 * nnet_train_lrate_destroy
 *
 * Destroys a previously created learning rate function instance
 */
extern int nnet_train_lrate_destroy (LRateFunction * lrate_function);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_train_lrate_value
 *
 * Returns the learning rate at the given time
 */
extern RValue
nnet_train_lrate_value (const LRateFunction lrate_function, const DTime time);



/*
 * nnet_train_lrate_set_parameters
 *
 * Sets the learning rate function parameter vector
 */
extern int
nnet_train_lrate_set_parameters (LRateFunction lrate_function,
                                 const LRateFunctionParameters parameters);



#endif /* __NNET_TRAIN_H_ */
