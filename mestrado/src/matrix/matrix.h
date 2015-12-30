#ifndef __MATRIX_H_
#define __MATRIX_H_ 1

#include <stdio.h>
#include "../common/types.h"
#include "../vector/vector.h"



/******************************************************************************
 *                                                                            *
 *                         BIDIMENSIONAL MATRIX DATATYPES                     *
 *                                                                            *
 ******************************************************************************/

/* Symbolic types for rows and columns */
typedef Vector MRow;
typedef Vector MColumn;

/* Rows and columns indexes */
typedef UsLgIntValue MIndex;

/* Matrix as an array of vectors (rows) */
typedef RValue **MMatrix;

/* Matrix data type */
typedef struct
{
  MIndex rows;
  MIndex columns;
  MMatrix elements;
}
matrix_type;

typedef matrix_type *Matrix;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * matrix_create
 *
 * Creates a new bidimensional matrix
 */
extern Matrix
matrix_create (const MIndex rows, const MIndex columns,
               const BoolValue initialize, const RValue initial_value);



/*
 * matrix_destroy
 *
 * Destroys a previously created matrix
 */
extern int matrix_destroy (Matrix * matrix);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * matrix_get_value
 *
 * Gets the value of an element in the matrix, given its row and column
 */
extern int
matrix_get_value (const Matrix matrix, const MIndex row, const MIndex column,
                  RValue * value);



/*
 * matrix_set_value
 *
 * Sets the value of an element in the matrix, given its row and column
 */
extern int
matrix_set_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value);



/*
 * matrix_init
 *
 * Initializes all elements in the given matrix to the given value
 */
extern int matrix_init (Matrix matrix, const RValue value);



/*
 * matrix_sum_value
 *
 * Sums 'value' to the element given by 'row' and 'column'
 */
extern int
matrix_sum_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value);



/*
 * matrix_subt_value
 *
 * Subtracts 'value' from the element given by 'row' and 'column'
 */
extern int
matrix_subt_value (Matrix matrix, const MIndex row, const MIndex column,
                   const RValue value);



/*
 * matrix_mult_value
 *
 * Multiplies the element given by 'row' and 'column' by 'value'
 */
extern int
matrix_mult_value (Matrix matrix, const MIndex row, const MIndex column,
                   const RValue value);



/*
 * matrix_div_value
 *
 * Divides the element given by 'row' and 'column' by 'value'
 */
extern int
matrix_div_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value);



/*
 * matrix_identity
 *
 * Returns the identity matrix of the given order, multiplied by 'value'
 */
extern Matrix matrix_identity (const MIndex order, const RValue value);



/*
 * matrix_get_row
 *
 * Writes the matrix row given by the index in the given vector
 */
extern int
matrix_get_row (const Matrix matrix, const MIndex row_index, Vector row);



/*
 * matrix_get_column
 *
 * Writes the matrix column given by the index in the given vector
 */
extern int
matrix_get_column (const Matrix matrix, const MIndex col_index,
                   Vector column);



/*
 * matrix_raw_info
 *
 * Writes the matrix to the given output stream, element by element,
 * column->row order, jumping one line at the end of each row
 */
extern void matrix_raw_info (const Matrix matrix, FILE * output_fd);



/******************************************************************************
 *                                                                            *
 *                              MATRIX OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * If the result matrix doesn't point to NULL, results will be stored in it.
 * If the result matrix points to NULL, a new matrix will be created.
 */

/*
 * matrix_sum
 *
 * Returns the sum of the two given matrices.
 */
extern int matrix_sum (const Matrix m1, const Matrix m2, Matrix * result);



/*
 * matrix_subtract
 *
 * Returns the difference between the two given matrices (m1 - m2).
 */
extern int
matrix_subtract (const Matrix m1, const Matrix m2, Matrix * result);



/*
 * matrix_product
 *
 * Returns the matrix product m1 * m2
 */
extern int matrix_product (const Matrix m1, const Matrix m2, Matrix * result);



/*
 * matrix_scalar_multiply
 *
 * Returns the matrix multiplied by the given scalar
 */
extern int
matrix_scalar_multiply (const Matrix m, const RValue a, Matrix * result);



/*
 * matrix_transpose
 *
 * Returns the transposed matrix from m
 */
extern int matrix_transpose (const Matrix m, Matrix * result);



#endif /* __MATRIX_H_ */
