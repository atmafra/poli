#ifndef __VECTOR_H_
#define __VECTOR_H_ 1

#include <stdio.h>

#include "types.h"

#define __EUCLIDEAN_STRING_ "Euclidean"
#define __INNER_PRODUCT_STRING_ "Inner Product"

/******************************************************************************
 *                                                                            *
 *                              VECTOR DATATYPES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * vector_metric_type
 *
 * Supported vector metrics
 */
typedef enum
{
  VECTOR_METR_EUCLIDEAN = 1,
  VECTOR_METR_INNER_PRODUCT = 2
}
vector_metric_type;

typedef vector_metric_type VectorMetric;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * vector_create
 *
 * Creates a new vector
 */
extern Vector vector_create (const UsLgIntValue dimension);



/*
 * vector_destroy
 *
 * Destroys a previously created vector
 */
extern int vector_destroy (Vector * vector);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * vector_str_to_metric
 *
 * Converts the given string into its associated metrics
 */
extern int vector_str_to_metric (const char *str, VectorMetric * metric);



/*
 * vector_metric_to_str
 *
 * Returns the string representation associated to the metrics
 */
extern char *vector_metric_to_str (const VectorMetric metric);



/*
 * vector_get_value
 *
 * Gets the value of a component of the vector. 1's logic.
 */
extern int
vector_get_value (const Vector vector, const UsLgIntValue component,
                  RValue * value);



/*
 * vector_set_value
 *
 * Sets the value of a particular component. 1's logic.
 */
extern int
vector_set_value (Vector vector, const UsLgIntValue component,
                  const RValue value);



/*
 * vector_reset
 *
 * Sets all components of the vector to a fixed value
 */
extern int vector_reset (Vector vector, const RValue value);



/*
 * vector_sum_value
 *
 * Sums 'value' to the given component
 */
extern int
vector_sum_value (Vector vector, const UsLgIntValue component,
                  const RValue value);



/*
 * vector_subt_value
 *
 * Subtracts 'value' from the given component
 */
extern int
vector_subt_value (Vector vector, const UsLgIntValue component,
                   const RValue value);



/*
 * vector_mult_value
 *
 * Multiplies component by the given 'value'
 */
extern int
vector_mult_value (Vector vector, const UsLgIntValue component,
                   const RValue value);



/*
 * vector_div_value
 *
 * Divides component by the given 'value'
 */
extern int
vector_div_value (Vector vector, const UsLgIntValue component,
                  const RValue value);



/*
 * vector_copy
 *
 * Copies the components of the first vector to the second vector
 */
extern int vector_copy (const Vector v_orig, Vector v_dest);



/*
 * vector_create_and_load
 *
 * Creates a new vector and loads its elements
 */
extern Vector
vector_create_and_load (const UsLgIntValue dimension, const RValue * source);



/*
 * vector_info
 *
 * Outputs vector information
 */
extern void vector_info (const Vector vector, FILE * output_fd);



/*
 * vector_raw_info
 *
 * Outputs vector information at a one value per line basis
 */
extern void vector_raw_info (const Vector vector, FILE * output_fd);



/*
 * vector_list_info
 *
 * Writes the vector as a list of samples
 */
extern void
vector_list_info (const Vector vector, const RValue element_index,
                  FILE * output_fd);



/******************************************************************************
 *                                                                            *
 *                             VECTOR OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * vector_sum
 *
 * Returns the sum of the two input vectors
 */
extern int vector_sum (const Vector v1, const Vector v2, Vector result);



/*
 * vector_subtract
 *
 * Returns the difference between the two input vectors
 */
extern int vector_subtract (const Vector v1, const Vector v2, Vector result);



/*
 * vector_multiply
 *
 * Returns the component-by-component product of the two input vectors
 */
extern int vector_multiply (const Vector v1, const Vector v2, Vector result);



/*
 * vector_divide
 *
 * Returns the component-by-component division of the two input vectors
 */
extern int vector_divide (const Vector v1, const Vector v2, Vector result);



/*
 * vector_scalar_multiply
 *
 * Returns the vector multiplied by the given scalar
 */
extern int
vector_scalar_multiply (const Vector v, const RValue k, Vector result);



/*
 * vector_sqrt
 *
 * Returns the vector of the square roots of each component
 */
extern int vector_sqrt (const Vector v, Vector result);



/******************************************************************************
 *                                                                            *
 *                               VECTOR METRICS                               *
 *                                                                            *
 ******************************************************************************/

/*
 * vector_normalize
 *
 * Simultaneously normalize (make unitary modulus) two vectors
 */
extern int vector_normalize (Vector v1, Vector v2);



/*
 * vector_metric
 *
 * Computes the requested metric between v1 and v2, optionally ponderated
 * by pond_vector
 */
extern int
vector_metric (const Vector v1, const Vector v2,
               const Vector pond_vector, const VectorMetric metric,
               RValue * result);



/*
 * vector_metr_euclidean
 *
 * Computes the euclidean distance between v1 and v2
 */
extern int
vector_metr_euclidean (const Vector v1, const Vector v2, RValue * result);



/*
 * vector_metr_inner_product
 *
 * Computes the inner product v1 and v2
 */
extern int
vector_metr_inner_product (const Vector v1, const Vector v2, RValue * result);



#endif /* __VECTOR_H_ */
