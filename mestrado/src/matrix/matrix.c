#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "errorh.h"

#include "matrix.h"

/******************************************************************************
 *                                                                            *
 *                             PRIVATE OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/
typedef enum
{
  MATRIX_SET,
  MATRIX_SUM,
  MATRIX_SUBTRACTION,
  MATRIX_PRODUCT,
  MATRIX_DIVISION,
  MATRIX_LOG
}
MatrixOperation;


/*
 * matrix_value_operation
 *
 * Executes the requested operation between one matrix element and the given
 * value
 */
static int
matrix_value_operation (Matrix matrix, const MIndex row, const MIndex column,
                        const RValue value, const MatrixOperation operation)
{
  /* Checks if the matrix was actually passed */
  if (matrix == NULL)
    return error_failure ("matrix_value_operation", "no matrix passed\n");

  /* Checks the indexes */
  if (row < 1 || row > matrix->rows)
    return error_failure ("matrix_value_operation",
                          "invalid row index (%ld)\n", row);

  if (column < 1 || column > matrix->columns)
    return error_failure ("matrix_value_operation",
                          "invalid column index (%ld)\n", column);

  /* Executes the selected operation */
  errno = 0;

  switch (operation)
    {
    case MATRIX_SET:
      matrix->elements[row - 1][column - 1] = value;
      break;

    case MATRIX_SUM:
      matrix->elements[row - 1][column - 1] += value;
      break;

    case MATRIX_SUBTRACTION:
      matrix->elements[row - 1][column - 1] -= value;
      break;

    case MATRIX_PRODUCT:
      matrix->elements[row - 1][column - 1] *= value;
      break;

    case MATRIX_DIVISION:
      matrix->elements[row - 1][column - 1] /= value;
      break;

    case MATRIX_LOG:
      matrix->elements[row - 1][column - 1] =
        log (matrix->elements[row - 1][column - 1]);
      break;

    default:
      return error_failure ("matrix_value_operation",
                            "invalid operation requested\n");
    }

  if (errno != 0)
    return error_failure ("matrix_value_operation", "math error: %s\n",
                          strerror (errno));

  return EXIT_SUCCESS;
}



/*
 * matrix_operation
 *
 * Operations between two matrices, resulting another matrix
 */
static int
matrix_operation (const Matrix m1, const Matrix m2, Matrix * result,
                  const MatrixOperation operation)
{
  MIndex rows, cols;            /* auxiliary number of rows and columns */
  MIndex cur_row, cur_col;      /* auxiliary row and column counters */


  /* Checks if the matrices were actually passed */
  if (m1 == NULL)
    {
      fprintf (stderr, "matrix_operation: first matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  if (m2 == NULL)
    {
      fprintf (stderr, "matrix_operation: second matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  if (result == NULL)
    {
      fprintf (stderr,
               "matrix_operation: no result matrix to use for result storage\n");
      return EXIT_FAILURE;
    }

  /* Checks if the matrices are compatible */
  rows = m1->rows;
  cols = m1->columns;

  if (m2->rows != rows || m2->columns != cols)
    {
      fprintf (stderr,
               "matrix_operation: incompatible dimensions for adding matrices M1(%ldx%ld) and M2(%ldx%ld)\n",
               rows, cols, m2->rows, m2->columns);
      return EXIT_FAILURE;
    }

  /* Checks if the result matrix should be created */
  if (*result == NULL)
    {
      *result = matrix_create (rows, cols, FALSE, 0.0);
      if (*result == NULL)
        {
          fprintf (stderr,
                   "matrix_operation: error creating new result matrix\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      /* Checks the compatibility of the result matrix passed */
      if ((*result)->rows != rows || (*result)->columns != cols)
        {
          fprintf (stderr,
                   "matrix_operation: incompatible dimension for result matrix M(%ldx%ld) and R(%ldx%ld)\n",
                   rows, cols, (*result)->rows, (*result)->columns);
          return EXIT_FAILURE;
        }
    }

  /* Executes the sum */
  for (cur_row = 1; cur_row <= rows; cur_row++)
    for (cur_col = 1; cur_col <= cols; cur_col++)
      {
        /* Executes the sum of the current rows */
        switch (operation)
          {
          case MATRIX_SUM:
            (*result)->elements[cur_row - 1][cur_col - 1] =
              m1->elements[cur_row - 1][cur_col - 1] +
              m2->elements[cur_row - 1][cur_col - 1];
            break;

          case MATRIX_SUBTRACTION:
            (*result)->elements[cur_row - 1][cur_col - 1] =
              m1->elements[cur_row - 1][cur_col - 1] -
              m2->elements[cur_row - 1][cur_col - 1];
            break;

          default:
            fprintf (stderr, "matrix_operation: invalid operation\n");
            return EXIT_FAILURE;
          }
      }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * matrix_create
 *
 * Creates a new bidimensional matrix
 */
Matrix
matrix_create (const MIndex rows, const MIndex columns,
               const BoolValue initialize, const RValue initial_value)
{
  Matrix new_matrix = NULL;     /* new matrix */
  MMatrix row_array = NULL;     /* array of rows */
  MIndex cur_row;               /* current row index */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the numbers of rows and columns are correct */
  if (rows < 1)
    {
      fprintf (stderr, "matrix_create: invalid number of rows (%ld)\n", rows);
      return NULL;
    }

  if (columns < 1)
    {
      fprintf (stderr, "matrix_create: invalid number of columns (%ld)\n",
               columns);
      return NULL;
    }

  /* Creates the array of rows */
  row_array = (MMatrix) malloc (rows * sizeof (RValue *));
  if (row_array == NULL)
    {
      fprintf (stderr,
               "matrix_create: virtual memory exhausted creating rows array\n");
      return NULL;
    }

  /* Allocates the row vectors */
  for (cur_row = 1; cur_row <= rows; cur_row++)
    {
      row_array[cur_row - 1] = (RValue *) malloc (columns * sizeof (RValue));
      if (row_array[cur_row - 1] == NULL)
        {
          fprintf (stderr,
                   "matrix_create: virtual memory exhausted creating row %ld\n",
                   cur_row);
          return NULL;
        }
    }

  /* Creates the matrix */
  new_matrix = (Matrix) malloc (sizeof (matrix_type));
  if (new_matrix == NULL)
    {
      fprintf (stderr,
               "matrix_create: virtual memory exhausted creating new matrix\n");
      return NULL;
    }

  /* Initializes the new matrix attributes */
  new_matrix->rows = rows;
  new_matrix->columns = columns;
  new_matrix->elements = row_array;

  /* Optionally initializes the new matrix */
  if (initialize == TRUE)
    {
      exit_status = matrix_init (new_matrix, initial_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "matrix_create: error initializing new matrix\n");
          return NULL;
        }
    }

  return new_matrix;
}



/*
 * matrix_destroy
 *
 * Destroys a previously created matrix
 */
int
matrix_destroy (Matrix * matrix)
{
  MIndex cur_row;               /* current row index */


  /* Checks if the matrix was actually passed */
  if (matrix == NULL || *matrix == NULL)
    {
      fprintf (stderr, "matrix_destroy: no matrix to destroy\n");
      return EXIT_FAILURE;
    }

  /* Row destruction loop */
  for (cur_row = 1; cur_row <= (*matrix)->rows; cur_row++)
    free ((*matrix)->elements[cur_row - 1]);

  /* Destroys the row array and the matrix */
  free ((*matrix)->elements);
  free (*matrix);
  *matrix = NULL;

  return EXIT_SUCCESS;
}



/*
 * matrix_get_value
 *
 * Gets the value of an element in the matrix, given its row and column
 */
int
matrix_get_value (const Matrix matrix, const MIndex row, const MIndex column,
                  RValue * value)
{
  /* Checks if the matrix was actually passed */
  if (matrix == NULL)
    {
      fprintf (stderr, "matrix_get_value: no matrix passed\n");
      return EXIT_FAILURE;
    }

  /* Checks the indexes */
  if (row < 1 || row > matrix->rows)
    {
      fprintf (stderr, "matrix_get_value: invalid row index (%ld)\n", row);
      return EXIT_FAILURE;
    }

  if (column < 1 || column > matrix->columns)
    {
      fprintf (stderr, "matrix_get_value: invalid column index (%ld)\n",
               column);
      return EXIT_FAILURE;
    }

  /* Gets the value */
  *value = matrix->elements[row - 1][column - 1];

  return EXIT_SUCCESS;
}



/*
 * matrix_set_value
 *
 * Sets the value of an element in the matrix, given its row and column
 */
int
matrix_set_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value)
{
  return matrix_value_operation (matrix, row, column, value, MATRIX_SET);
}



/*
 * matrix_init
 *
 * Initializes all elements in the given matrix to the given value
 */
int
matrix_init (Matrix matrix, const RValue value)
{
  MIndex row, col;              /* auxiliary current row and column */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the matrix was actually passed */
  if (matrix == NULL)
    {
      fprintf (stderr, "matrix_init: no matrix passed to initialization\n");
      return EXIT_FAILURE;
    }

  /* Initializes the matrix */
  for (row = 1; row <= matrix->rows; row++)
    for (col = 1; col <= matrix->columns; col++)
      {
        exit_status = matrix_set_value (matrix, row, col, value);
        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr,
                     "matrix_init: error initializing element (%ld,%ld)\n",
                     row, col);
            return EXIT_FAILURE;
          }
      }

  return EXIT_SUCCESS;
}



/*
 * matrix_sum_value
 *
 * Sums 'value' to the element given by 'row' and 'column'
 */
int
matrix_sum_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value)
{
  return matrix_value_operation (matrix, row, column, value, MATRIX_SUM);
}



/*
 * matrix_subt_value
 *
 * Subtracts 'value' from the element given by 'row' and 'column'
 */
int
matrix_subt_value (Matrix matrix, const MIndex row, const MIndex column,
                   const RValue value)
{
  return matrix_value_operation (matrix, row, column, value,
                                 MATRIX_SUBTRACTION);
}



/*
 * matrix_mult_value
 *
 * Multiplies the element given by 'row' and 'column' by 'value'
 */
int
matrix_mult_value (Matrix matrix, const MIndex row, const MIndex column,
                   const RValue value)
{
  return matrix_value_operation (matrix, row, column, value, MATRIX_PRODUCT);
}



/*
 * matrix_div_value
 *
 * Divides the element given by 'row' and 'column' by 'value'
 */
int
matrix_div_value (Matrix matrix, const MIndex row, const MIndex column,
                  const RValue value)
{
  return matrix_value_operation (matrix, row, column, value, MATRIX_DIVISION);
}


/*
 * matrix_log_value
 *
 * Applies natural logarithm to the element given by 'row' and 'column'
 */
int
matrix_log_value (Matrix matrix, const MIndex row, const MIndex column)
{
  return matrix_value_operation (matrix, row, column, 0.0, MATRIX_LOG);
}



/*
 * matrix_sum_all
 *
 * Sums 'value' to all the elements in the given matrix
 */
int
matrix_sum_all (Matrix matrix, const RValue value)
{
  MIndex row, col;              /* auxiliary current row and column */

  if (matrix == NULL)
    return error_failure ("matrix log", "no matrix passed\n");

  for (row = 1; row <= matrix->rows; row++)
    for (col = 1; col <= matrix->columns; col++)
      if (matrix_sum_value (matrix, row, col, value) != EXIT_SUCCESS)
        return error_failure ("matrix_log",
                              "error summing to element (%ld,%ld)\n",
                              row, col);

  return EXIT_SUCCESS;
}



/*
 * matrix_mult_all
 *
 * Multiplies all elements in the given matrix by 'value'
 */
int
matrix_mult_all (Matrix matrix, const RValue value)
{
  MIndex row, col;              /* auxiliary current row and column */

  if (matrix == NULL)
    return error_failure ("matrix log", "no matrix passed\n");

  for (row = 1; row <= matrix->rows; row++)
    for (col = 1; col <= matrix->columns; col++)
      if (matrix_mult_value (matrix, row, col, value) != EXIT_SUCCESS)
        return error_failure ("matrix_log",
                              "error multiplying element (%ld,%ld)\n",
                              row, col);

  return EXIT_SUCCESS;
}



/*
 * matrix_log_all
 *
 * Applies natural logarithm to all elements in the given matrix
 */
int
matrix_log_all (Matrix matrix)
{
  MIndex row, col;              /* auxiliary current row and column */

  if (matrix == NULL)
    return error_failure ("matrix log", "no matrix passed\n");

  for (row = 1; row <= matrix->rows; row++)
    for (col = 1; col <= matrix->columns; col++)
      if (matrix_log_value (matrix, row, col) != EXIT_SUCCESS)
        return error_failure ("matrix_log",
                              "error applying log to element (%ld,%ld)\n",
                              row, col);

  return EXIT_SUCCESS;
}



/*
 * matrix_identity
 *
 * Returns the identity matrix of the given order multiplied by 'value'
 */
Matrix
matrix_identity (const MIndex order, const RValue value)
{
  Matrix new_matrix = NULL;     /* new identity matrix */
  MIndex row, col;              /* row and column indexes */
  int exit_status;              /* auxiliary function return status */


  /* Creates the new matrix */
  new_matrix = matrix_create (order, order, FALSE, 0.0);
  if (new_matrix == NULL)
    {
      fprintf (stderr,
               "matrix_identity: error creating identity matrix of order %ld\n",
               order);
      return NULL;
    }

  /* Populates the identity matrix */
  for (row = 1; row <= order; row++)
    for (col = 1; col <= order; col++)
      {
        exit_status =
          matrix_set_value (new_matrix, row, col, (row == col ? value : 0.0));
        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr,
                     "matrix_identity: error constructing identity matrix\n");
            return NULL;
          }
      }

  return new_matrix;
}



/*
 * matrix_get_row
 *
 * Writes the matrix row given by the index in the given vector
 */
int
matrix_get_row (const Matrix matrix, const MIndex row_index, Vector row)
{
  MIndex cur_col;               /* current column being copied */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the matrix was actually passed */
  if (matrix == NULL)
    {
      fprintf (stderr, "matrix_get_row: matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  /* Checks if the vector was actually passed */
  if (row == NULL)
    {
      fprintf (stderr, "matrix_get_row: no vector to write row in\n");
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility */
  if (row->dimension != matrix->columns)
    {
      fprintf (stderr,
               "matrix_get_row: incompatible dimensions between matrix columns (%ld) and row vector dimension (%ld)\n",
               matrix->columns, row->dimension);
      return EXIT_FAILURE;
    }

  /* Checks the row index interval */
  if (row_index < 1 || row_index > matrix->rows)
    {
      fprintf (stderr, "matrix_get_row: invalid row index: %ld\n", row_index);
      return EXIT_FAILURE;
    }

  /* Copies the row to the vector */
  for (cur_col = 1; cur_col <= matrix->columns; cur_col++)
    {
      exit_status =
        vector_set_value (row, cur_col,
                          matrix->elements[row_index - 1][cur_col - 1]);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "matrix_get_row: error copying element (%ld,%ld) to the given vector\n",
                   row_index, cur_col);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * matrix_get_column
 *
 * Writes the matrix column given by the index in the given vector
 */
int
matrix_get_column (const Matrix matrix, const MIndex col_index, Vector column)
{
  MIndex cur_row;               /* current row */
  RValue cur_value;             /* current row value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the matrix was actually passed */
  if (matrix == NULL)
    {
      fprintf (stderr, "matrix_get_column: matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  /* Checks if the vector was actually passed */
  if (column == NULL)
    {
      fprintf (stderr, "matrix_get_column: no vector to write row in\n");
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility */
  if (column->dimension != matrix->rows)
    {
      fprintf (stderr,
               "matrix_get_column: incompatible dimensions between matrix rows (%ld) and column vector dimension (%ld)\n",
               matrix->rows, column->dimension);
      return EXIT_FAILURE;
    }

  /* Checks the row index interval */
  if (col_index < 1 || col_index > matrix->columns)
    {
      fprintf (stderr, "matrix_get_column: invalid column index: %ld\n",
               col_index);
      return EXIT_FAILURE;
    }

  /* Copies the column to the vector */
  for (cur_row = 1; cur_row <= matrix->rows; cur_row++)
    {
      /* Gets the value of the element */
      exit_status = matrix_get_value (matrix, cur_row, col_index, &cur_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "matrix_get_column: error getting value of element (%ld,%ld)\n",
                   cur_row, col_index);
          return EXIT_FAILURE;
        }

      /* Copies it to the column vector */
      exit_status = vector_set_value (column, cur_row, cur_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "matrix_get_column: error setting value of column component %ld\n",
                   cur_row);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * matrix_info
 *
 * Writes the matrix to the given output stream in matrix format
 */
void
matrix_info (const Matrix matrix, FILE * output_fd)
{
  MIndex row, col;              /* auxiliary current row and column */
  RValue cur_val;               /* current element value */

  if (matrix == NULL)
    return;

  for (row = 1; row <= matrix->rows; row++)
    {
      for (col = 1; col <= matrix->columns; col++)
        {
          if (matrix_get_value (matrix, row, col, &cur_val) != EXIT_SUCCESS)
            {
              error_failure ("matrix_raw_info",
                             "error getting element (%ld,%ld) from matrix\n",
                             row, col);
              return;
            }
          fprintf (output_fd, "%f", cur_val);

          if (col < matrix->columns)
            fprintf (output_fd, " ");
        }

      if (row < matrix->rows)
        fprintf (output_fd, "\n");
    }
}



/*
 * matrix_raw_info
 *
 * Writes the matrix to the given output stream, element by element,
 * column->row order, jumping one line at the end of each row
 */
void
matrix_raw_info (const Matrix matrix, FILE * output_fd)
{
  MIndex row, col;              /* auxiliary current row and column */
  RValue cur_val;               /* current element value */

  if (matrix == NULL)
    return;

  for (row = 1; row <= matrix->rows; row++)
    {
      for (col = 1; col <= matrix->columns; col++)
        {
          if (matrix_get_value (matrix, row, col, &cur_val) != EXIT_SUCCESS)
            {
              error_failure ("matrix_raw_info",
                             "error getting element (%ld,%ld) from matrix\n",
                             row, col);
              return;
            }

          fprintf (output_fd, "%f\n", cur_val);
        }

      if (row < matrix->rows)
        fprintf (output_fd, "\n");
    }
}



/*
 * matrix_vector_info
 *
 * Writes the matrix to the given stream as a single vector
 */
void
matrix_vector_info (const Matrix matrix, const RValue element_index,
                    FILE * output_fd)
{
  MIndex row, col;              /* auxiliary current row and column */
  RValue cur_dim;               /* auxiliary vector dimension */
  RValue cur_val;               /* current element value */

  if (matrix == NULL)
    return;

  for (row = 1; row <= matrix->rows; row++)
    for (col = 1; col <= matrix->columns; col++)
      {
        if (matrix_get_value (matrix, row, col, &cur_val) != EXIT_SUCCESS)
          {
            error_failure ("matrix_raw_info",
                           "error getting element (%ld,%ld) from matrix\n",
                           row, col);
            return;
          }

        cur_dim = (RValue) (row - 1.0) * matrix->columns + col - 1.0;
        fprintf (output_fd, "%f %f %f\n", element_index, cur_dim, cur_val);
      }
}



/*
 * matrix_sum
 *
 * Returns the sum of the two given matrices.
 * If the result matrix doesn't point to NULL, results will be stored in it.
 * If the result matrix points to NULL, a new matrix will be created.
 */
int
matrix_sum (const Matrix m1, const Matrix m2, Matrix * result)
{
  return matrix_operation (m1, m2, result, MATRIX_SUM);
}



/*
 * matrix_subtract
 *
 * Returns the difference between the two given matrices (m1 - m2).
 * If the result matrix doesn't point to NULL, results will be stored in it.
 * If the result matrix points to NULL, a new matrix will be created.
 */
int
matrix_subtract (const Matrix m1, const Matrix m2, Matrix * result)
{
  return matrix_operation (m1, m2, result, MATRIX_SUBTRACTION);
}


/*
 * matrix_product
 *
 * Returns the matrix product m1 * m2
 * If the result matrix doesn't point to NULL, results will be stored in it.
 * If the result matrix points to NULL, a new matrix will be created.
 */
int
matrix_product (const Matrix m1, const Matrix m2, Matrix * result)
{
  MIndex rows, cols;            /* auxiliary number of rows and columns */
  MIndex cur_rowid, cur_colid;  /* auxiliary current row and column */
  Vector cur_row = NULL;        /* auxiliary row */
  Vector cur_col = NULL;        /* auxiliary column */
  RValue cur_value;             /* auxiliary result element value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the matrices were actually passed */
  if (m1 == NULL)
    {
      fprintf (stderr, "matrix_product: first matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  if (m2 == NULL)
    {
      fprintf (stderr, "matrix_product: second matrix passed NULL\n");
      return EXIT_FAILURE;
    }

  if (result == NULL)
    {
      fprintf (stderr,
               "matrix_product: no result matrix to use for result storage\n");
      return EXIT_FAILURE;
    }

  /* Checks if the matrices are compatible */
  if (m1->columns != m2->rows)
    {
      fprintf (stderr,
               "matrix_product: matrices have incompatible dimensions: M1(%ld,%ld) x M2(%ld,%ld)\n",
               m1->rows, m1->columns, m2->rows, m2->columns);
      return EXIT_FAILURE;
    }

  /* Initializes the result matrix dimension */
  rows = m1->rows;
  cols = m2->columns;

  /* Checks if the result matrix should be created */
  if (*result == NULL)
    {
      *result = matrix_create (rows, cols, FALSE, 0.0);
      if (*result == NULL)
        {
          fprintf (stderr,
                   "matrix_product: error creating new result matrix\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      /* Checks the compatibility of the result matrix passed */
      if ((*result)->rows != rows || (*result)->columns != cols)
        {
          fprintf (stderr,
                   "matrix_product: incompatible dimension for result matrix M(%ldx%ld) and R(%ldx%ld)\n",
                   rows, cols, (*result)->rows, (*result)->columns);
          return EXIT_FAILURE;
        }
    }

  /* Allocates memory for the auxiliary row and column */
  cur_row = vector_create (cols);
  cur_col = vector_create (rows);
  if (cur_row == NULL || cur_col == NULL)
    {
      fprintf (stderr,
               "matrix_product: error allocating auxiliary row and column\n");
      return EXIT_FAILURE;
    }

  /* Executes the product */
  for (cur_rowid = 1; cur_rowid <= rows; cur_rowid++)
    {
      exit_status = matrix_get_row (m1, cur_rowid, cur_row);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "matrix_product: error getting row %ld from m1\n",
                   cur_rowid);
          return EXIT_FAILURE;
        }

      for (cur_colid = 1; cur_colid <= cols; cur_colid++)
        {
          exit_status = matrix_get_column (m2, cur_colid, cur_col);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "matrix_product: error getting column %ld from m2\n",
                       cur_colid);
              return EXIT_FAILURE;
            }

          /* Multiplies row by column */
          exit_status =
            vector_metr_inner_product (cur_row, cur_col, &cur_value);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "matrix_product: error executing inner product between m1 row %ld and m2 column %ld\n",
                       cur_rowid, cur_colid);
              return EXIT_FAILURE;
            }

          /* Sets the value of the element in the result matrix */
          exit_status =
            matrix_set_value (*result, cur_rowid, cur_colid, cur_value);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "matrix_product: error setting value of result vector element (%ld,%ld)\n",
                       cur_rowid, cur_colid);
              return EXIT_FAILURE;
            }
        }
    }

  /* Destroys the auxiliary vectors */
  exit_status = vector_destroy (&cur_row);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "matrix_product: error destroying auxiliary row vector\n");
      return EXIT_FAILURE;
    }

  exit_status = vector_destroy (&cur_col);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "matrix_product: error destroying auxiliary column vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
