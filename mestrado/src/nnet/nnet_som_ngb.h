#ifndef __NNET_SOM_NGB_H_
#define __NNET_SOM_NGB_H_ 1

#include <stdio.h>
#include "nnet_types.h"
#include "function.h"
#include "vector.h"


/******************************************************************************
 *                                                                            *
 *                              PUBLIC DATATYPES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * NgbFunctionType
 *
 * Neighborhood function types
 */
typedef enum
{
  NNET_SOM_NGB_RECTANGLE = 0,
  NNET_SOM_NGB_GAUSSIAN = 1
}
NgbFunctionType;



/*
 * nnet_som_ngb_class_type
 *
 * Neighborhood function classes
 */
typedef struct
{
  RFunctionClass ngb_class;
  VectorMetric vector_metric;
}
nnet_som_ngb_class_type;

/* Symbolic Type */
typedef nnet_som_ngb_class_type *NgbFunctionClass;


/*
 * NgbFunction
 *
 * Neighborhood Functions between two vectors
 */
typedef struct
{
  NgbFunctionClass function_class;
  RFunction function;
}
nnet_som_ngb_function_type;


/* Symbolic Types */
typedef nnet_som_ngb_function_type *NgbFunction;
typedef RFunctionParameters NgbRFunctionParameters;



/******************************************************************************
 *                                                                            *
 *                              PUBLIC VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * Neighborhood Function Classes
 */

/* Rectangular neighborhood function */
extern NgbFunctionClass nnet_som_ngb_rectangular;

/* Gaussian neighborhood function */
extern NgbFunctionClass nnet_som_ngb_gaussian;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_som_ngb_init_all_classes
 *
 * Initializes all neighborhood function classes
 */
extern int nnet_som_ngb_init_all_classes ();



/*
 * nnet_som_ngb_destroy_all_classes
 *
 * Destroys all neighborhood function classes
 */
extern void nnet_som_ngb_destroy_all_classes ();



/*
 * nnet_som_ngb_class_by_name
 *
 * Returns the layer class variable associated to the given name
 */
extern NgbFunctionClass nnet_som_ngb_class_by_name (Name name);



/*
 * nnet_som_ngb_create_function
 *
 * Creates a new neighborhood function with default parameter vector
 */
extern NgbFunction
nnet_som_ngb_create_function (const NgbFunctionClass ngb_class,
                              const NgbRFunctionParameters parameters);



/*
 * nnet_som_ngb_destroy_function
 *
 * Destroys a previously created neighborhood function
 */
extern int nnet_som_ngb_destroy_function (NgbFunction * function);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_som_ngb_value
 *
 * Evaluates the neighborhood function between the two given vectors
 */
extern int
nnet_som_ngb_value (const NgbFunction function,
                    const Vector v1, const Vector v2, RValue * value);



/*
 * nnet_som_ngb_set_parameter
 *
 * Sets the neighborhood function parameters
 */
extern int
nnet_som_ngb_set_parameter (NgbFunction function,
                            const UsIntValue parameter_index,
                            const RValue value);



/*
 * nnet_som_ngb_set_parameter_vector
 *
 * Sets the neighborhood function parameter vector
 */
extern int
nnet_som_ngb_set_parameter_vector (NgbFunction function,
                                   const NgbRFunctionParameters parameters);



/*
 * nnet_som_ngb_function_info
 *
 * Outputs neighborhood function info
 */
extern void
nnet_som_ngb_function_info (NgbFunction function, FILE * output_fd);



#endif
