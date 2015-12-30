#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "vector.h"
#include "../incstat/incstat.h"


/******************************************************************************
 *                                                                            *
 *                       PRIVATE DATATYPES AND OPERATIONS                     *
 *                                                                            *
 ******************************************************************************/

/*
 * VectorOperation
 *
 * Vector operation type
 */
typedef enum
{
  VECTOR_SET,
  VECTOR_SUM,
  VECTOR_SUBTRACTION,
  VECTOR_PRODUCT,
  VECTOR_DIVISION
}
VectorOperation;



/*
 * vector_value_operation
 *
 * Executes the requested operation between vector and the given real
 */
static int
vector_value_operation (Vector vector, const UsLgIntValue component,
                        const RValue value, const VectorOperation operation)
{
  /* Checks if the vector was actually passed */
  if (vector == NULL)
    {
      fprintf (stderr, "vector_value_operation: no vector to set value\n");
      return EXIT_FAILURE;
    }

  /* Validates the component */
  if (component < 1 || component > vector->dimension)
    {
      fprintf (stderr,
               "vector_value_operation: invalid component: %ld [1..%ld]\n",
               component, vector->dimension);
      return EXIT_FAILURE;
    }

  /* Executes the requested operation */
  switch (operation)
    {
    case VECTOR_SET:
      vector->value[component - 1] = value;
      break;

    case VECTOR_SUM:
      vector->value[component - 1] += value;
      break;

    case VECTOR_SUBTRACTION:
      vector->value[component - 1] -= value;
      break;

    case VECTOR_PRODUCT:
      vector->value[component - 1] *= value;
      break;

    case VECTOR_DIVISION:
      /* Avoids division by zero */
      if (value <= DBL_EPSILON)
        {
          fprintf (stderr, "vector_value_operation: division by zero\n");
          return EXIT_FAILURE;
        }
      vector->value[component - 1] /= value;
      break;

    default:
      fprintf (stderr,
               "vector_value_operation: invalid operation requested\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * vector_operation
 *
 * Executes the requested vector operation between the given pair of inputs
 */
static int
vector_operation (const Vector v1, const Vector v2, Vector result,
                  VectorOperation operation)
{
  UsLgIntValue cur_comp;        /* component counter */
  RValue comp1, comp2;          /* auxiliary component values */
  RValue comp_result;           /* auxiliary component result */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the input vectors were actually passed */
  if (v1 == NULL || v2 == NULL)
    {
      fprintf (stderr, "vector_operation: input vector passed null\n");
      return EXIT_FAILURE;
    }

  /* Checks if the result vector was actually passed */
  if (result == NULL)
    {
      fprintf (stderr, "vector_operation: no result vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility between the two input vectors */
  if (v1->dimension != v2->dimension)
    {
      fprintf (stderr, "vector_operation: \
        incompatible vector dimensions: dim (v1) = %ld, dim (v2) = %ld\n", v1->dimension, v2->dimension);
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility between the input vectors and the result
   * vector */
  if (v1->dimension != result->dimension)
    {
      fprintf (stderr, "vector_operation: \
        incompatible input and result dimensions: \
        dim (input) = %ld, dim (result) = %ld\n", v1->dimension, result->dimension);
      return EXIT_FAILURE;
    }

  /* Executes the requested operation */
  for (cur_comp = 1; cur_comp <= v1->dimension; cur_comp++)
    {
      /* Gets the value of the input components */
      exit_status = vector_get_value (v1, cur_comp, &comp1);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_operation: error getting value of component %ld of v1\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      exit_status = vector_get_value (v2, cur_comp, &comp2);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_operation: error getting value of component %ld of v2\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Performs the requested operation */
      switch (operation)
        {
        case VECTOR_SUM:
          comp_result = comp1 + comp2;
          break;

        case VECTOR_SUBTRACTION:
          comp_result = comp1 - comp2;
          break;

        case VECTOR_PRODUCT:
          comp_result = comp1 * comp2;
          break;

        case VECTOR_DIVISION:
          if (comp2 < DBL_EPSILON)
            {
              fprintf (stderr, "vector_operation: division by zero\n");
              return EXIT_FAILURE;
            }
          comp_result = comp1 / comp2;
          break;

        default:
          fprintf (stderr, "vector_operation: invalid operation\n");
          return EXIT_FAILURE;
        }

      /* Sets the result vector component */
      exit_status = vector_set_value (result, cur_comp, comp_result);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_operation: error setting result component %ld\n",
                   cur_comp);
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
 * vector_create
 *
 * Creates a new vector
 */
Vector
vector_create (const UsLgIntValue dimension)
{
  Vector new_vector;            /* new vector */


  /* Checks if the number of elements is valid */
  if (dimension < 1)
    {
      fprintf (stderr, "vector_create: invalid dimension: %ld\n", dimension);
      return NULL;
    }

  /* Allocates the new vector */
  new_vector = (Vector) malloc (sizeof (vector_type));

  if (new_vector == NULL)
    {
      fprintf (stderr, "vector_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Allocates memory space to store its elements */
  new_vector->value = (RValue *) malloc (dimension * sizeof (RValue));

  if (new_vector->value == NULL)
    {
      free (new_vector);
      fprintf (stderr, "vector_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Initializes the vector */
  new_vector->dimension = dimension;

  return new_vector;
}



/*
 * vector_destroy
 *
 * Destroys a previously created vector
 */
int
vector_destroy (Vector * vector)
{
  /* Checks if the vector was actually passed */
  if (vector == NULL || *vector == NULL)
    {
      fprintf (stderr, "vector_destroy: no vector to destroy\n");
      return EXIT_FAILURE;
    }

  /* Destroys the vector */
  if ((*vector)->value != NULL)
    free ((*vector)->value);

  free (*vector);
  *vector = NULL;

  return EXIT_SUCCESS;
}



/*
 * vector_get_value
 *
 * Gets the value of a component of the vector. 1's logic.
 */
int
vector_get_value (const Vector vector, const UsLgIntValue component,
                  RValue * value)
{
  /* Checks if the vector was actually passed */
  if (vector == NULL)
    {
      fprintf (stderr, "vector_get_value: no vector to get value from\n");
      return EXIT_FAILURE;
    }

  /* Validates the component */
  if (component < 1 || component > vector->dimension)
    {
      fprintf (stderr, "vector_get_value: invalid component: %ld\n",
               component);
      return EXIT_FAILURE;
    }

  /* Gets the value: 1's logic */
  *value = vector->value[component - 1];

  return EXIT_SUCCESS;
}



/*
 * vector_set_value
 *
 * Sets the value of a particular component. 1's logic.
 */
int
vector_set_value (Vector vector, const UsLgIntValue component,
                  const RValue value)
{
  return vector_value_operation (vector, component, value, VECTOR_SET);
}



/*
 * vector_sum_value
 *
 * Sums 'value' to the given component
 */
int
vector_sum_value (Vector vector, const UsLgIntValue component,
                  const RValue value)
{
  return vector_value_operation (vector, component, value, VECTOR_SUM);
}



/*
 * vector_subt_value
 *
 * Subtracts 'value' from the given component
 */
int
vector_subt_value (Vector vector, const UsLgIntValue component,
                   const RValue value)
{
  return vector_value_operation (vector, component, value,
                                 VECTOR_SUBTRACTION);
}



/*
 * vector_mult_value
 *
 * Multiplies component by the given 'value'
 */
int
vector_mult_value (Vector vector, const UsLgIntValue component,
                   const RValue value)
{
  return vector_value_operation (vector, component, value, VECTOR_PRODUCT);
}



/*
 * vector_div_value
 *
 * Divides component by the given 'value'
 */
int
vector_div_value (Vector vector, const UsLgIntValue component,
                  const RValue value)
{
  return vector_value_operation (vector, component, value, VECTOR_DIVISION);
}



/*
 * vector_copy
 *
 * Copies the components of the first vector to the second vector
 */
int
vector_copy (const Vector v_orig, Vector v_dest)
{
  UsLgIntValue cur_comp;        /* current component */
  RValue cur_value;             /* current component value */
  int exit_status;              /* auxiliary function return status */

  /* Checks if the origin vector was actually passed */
  if (v_orig == NULL)
    {
      fprintf (stderr, "vector_copy: no origin vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the destination vector was actually passed */
  if (v_dest == NULL)
    {
      fprintf (stderr, "vector_copy: no destination vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility */
  if (v_orig->dimension != v_dest->dimension)
    {
      fprintf (stderr,
               "vector_copy: incompatible dimensions: dim(orig) = %ld; dim(dest) = %ld\n",
               v_orig->dimension, v_dest->dimension);
      return EXIT_FAILURE;
    }

  /* Copies the components */
  for (cur_comp = 1; cur_comp <= v_orig->dimension; cur_comp++)
    {
      /* Gets the origin vector component value */
      exit_status = vector_get_value (v_orig, cur_comp, &cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_copy: error getting value of origin vector component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Sets the destination vector component value */
      exit_status = vector_set_value (v_dest, cur_comp, cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_copy: error setting value of destination vector component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * vector_create_and_load
 *
 * Creates a new vector and loads its elements
 */
Vector vector_create_and_load
  (const UsLgIntValue dimension, const RValue * source)
{
  Vector new_vector;            /* new vector */
  UsLgIntValue cur_comp;        /* component counter */
  int exit_status;              /* auxiliary function return status */


  /* Creates the new element */
  new_vector = vector_create (dimension);

  if (new_vector == NULL)
    {
      fprintf (stderr, "vector_create_and_load: error creating vector\n");
      return NULL;
    }

  /* Sets its values by the source */
  for (cur_comp = 0; cur_comp < dimension; cur_comp++)
    {
      exit_status = vector_set_value
        (new_vector, cur_comp, (RValue) source[cur_comp - 1]);

      if (exit_status != EXIT_SUCCESS)
        {
          free (new_vector);
          fprintf (stderr,
                   "vector_create_and_load: error setting value of component %ld\n",
                   cur_comp);
          return NULL;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * vector_info
 *
 * Outputs vector information
 */
void
vector_info (const Vector vector, FILE * output_fd)
{
  UsLgIntValue cur_index;

  if (vector == NULL || output_fd == NULL)
    return;

  for (cur_index = 0; cur_index < vector->dimension; cur_index++)
    fprintf (output_fd, " %-3.2f", vector->value[cur_index]);

  fprintf (output_fd, "\n");

  return;
}



/*
 * vector_raw_info
 *
 * Outputs vector information at a one value per line basis
 */
void
vector_raw_info (const Vector vector, FILE * output_fd)
{
  UsLgIntValue cur_index;

  if (vector == NULL || output_fd == NULL)
    return;

  for (cur_index = 0; cur_index < vector->dimension; cur_index++)
    fprintf (output_fd, "%f\n", vector->value[cur_index]);

  return;
}



/*
 * vector_normalize
 *
 * Normalize the array associated to the vector
 */
int
vector_normalize (Vector vector1, Vector vector2)
{
  UsLgIntValue dim;             /* greatest of vector dimensions */
  UsLgIntValue cur_dim;         /* value array elements */
  RValue cur_value1, cur_value2;        /* current dimension value */
  RValue norm1, norm2;          /* norm of the array */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vector was actually passed */
  if (vector1 == NULL && vector2 == NULL)
    {
      fprintf (stderr, "vector_normalize: no vector to normalize\n");
      return EXIT_FAILURE;
    }

  /* Determines the combined dimension */
  if (vector1 == NULL)
    dim = vector2->dimension;
  else if (vector2 == NULL)
    dim = vector1->dimension;
  else
    dim =
      (vector1->dimension >
       vector2->dimension ? vector1->dimension : vector2->dimension);

  /* Checks if the vectors have dimensions */
  if (dim < 1)
    return EXIT_SUCCESS;

  /* Trivial case: dimension 1 */
  if (dim == 1)
    {
      if (vector1 != NULL)
        {
          exit_status = vector_set_value (vector1, 1, 1.0);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "vector_normalize: error setting element value\n");
              return EXIT_FAILURE;
            }
        }

      if (vector2 != NULL)
        {
          exit_status = vector_set_value (vector2, 1, 1.0);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "vector_normalize: error setting element value\n");
              return EXIT_FAILURE;
            }
        }
      return EXIT_SUCCESS;
    }

  /* Initializes the statistics */
  istt_clear_stat ();

  /* Adds the vector components to the statistics */
  for (cur_dim = 1; cur_dim <= dim; cur_dim++)
    {
      /* Gets the value from vector 1 */
      cur_value1 = 0.0;
      if (vector1 != NULL)
        if (vector1->dimension >= cur_dim)
          {
            exit_status = vector_get_value (vector1, cur_dim, &cur_value1);

            if (exit_status != EXIT_SUCCESS)
              {
                fprintf (stderr,
                         "vector_normalize: error getting value from array 1\n");
                return EXIT_FAILURE;
              }
          }

      /* Gets the value from array 2 */
      cur_value2 = 0.0;
      if (vector2 != NULL)
        if (vector2->dimension >= cur_dim)
          {
            exit_status = vector_get_value (vector2, cur_dim, &cur_value2);

            if (exit_status != EXIT_SUCCESS)
              {
                fprintf (stderr,
                         "vector_normalize: error getting value from array 2\n");
                return EXIT_FAILURE;
              }
          }

      /* Adds to the statistics */
      istt_add_stat (cur_value1, cur_value2);
    }

  /* Gets the norm of the arrays */
  norm1 = (RValue) sqrt (istt_sum_sqr_x ());
  norm2 = (RValue) sqrt (istt_sum_sqr_y ());

  /* Sets the new values for the dimensions */
  for (cur_dim = 1; cur_dim <= dim; cur_dim++)
    {
      /* Divides the current dimension by the vector norm */
      if (vector1 != NULL && norm1 > DBL_EPSILON)
        if (vector1->dimension >= cur_dim)
          {
            exit_status = vector_div_value (vector1, cur_dim, norm1);

            if (exit_status != EXIT_SUCCESS)
              {
                fprintf (stderr,
                         "vector_normalize: error dividing by norm\n");
                return EXIT_FAILURE;
              }
          }

      /* Divides the current dimension by the vector norm */
      if (vector2 != NULL && norm2 > DBL_EPSILON)
        if (vector2->dimension >= cur_dim)
          {
            exit_status = vector_div_value (vector2, cur_dim, norm2);

            if (exit_status != EXIT_SUCCESS)
              {
                fprintf (stderr,
                         "vector_normalize: error dividing by norm\n");
                return EXIT_FAILURE;
              }
          }
    }

  return EXIT_SUCCESS;
}



/*
 * vector_sum
 *
 * Returns the sum of the two input vectors
 */
int
vector_sum (const Vector v1, const Vector v2, Vector result)
{
  /* Dispatches the execution to 'vector_operation' */
  return vector_operation (v1, v2, result, VECTOR_SUM);
}



/*
 * vector_subtract
 *
 * Returns the difference between the two input vectors
 */
int
vector_subtract (const Vector v1, const Vector v2, Vector result)
{
  /* Dispatches the execution to 'vector_operation' */
  return vector_operation (v1, v2, result, VECTOR_SUBTRACTION);
}



/*
 * vector_multiply
 *
 * Returns the component-by-component product of the two input vectors
 */
int
vector_multiply (const Vector v1, const Vector v2, Vector result)
{
  /* Dispatches the execution to 'vector_operation' */
  return vector_operation (v1, v2, result, VECTOR_PRODUCT);
}



/*
 * vector_divide
 *
 * Returns the component-by-component division of the two input vectors
 */
int
vector_divide (const Vector v1, const Vector v2, Vector result)
{
  /* Dispatches the execution to 'vector_operation' */
  return vector_operation (v1, v2, result, VECTOR_DIVISION);
}



/*
 * vector_scalar_multiply
 *
 * Returns the vector multiplied by the given scalar
 */
int
vector_scalar_multiply (const Vector v, const RValue k, Vector result)
{
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current vector component value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vectors were actually passed */
  if (v == NULL)
    {
      fprintf (stderr, "vector_scalar_multiply: no vector to multiply\n");
      return EXIT_FAILURE;
    }

  if (result == NULL)
    {
      fprintf (stderr, "vector_scalar_multiply: no result vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks dimension compatibility */
  if (v->dimension != result->dimension)
    {
      fprintf (stderr,
               "vector_scalar_multiply: incompatible dimensions: dim (input) = %ld, dim (result) = %ld\n",
               v->dimension, result->dimension);
      return EXIT_FAILURE;
    }

  /* Multiplies the components */
  for (cur_comp = 1; cur_comp <= v->dimension; cur_comp++)
    {
      /* Gets the old component value */
      exit_status = vector_get_value (v, cur_comp, &cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_scalar_multiply: error getting value of component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Sets the new component value */
      exit_status = vector_set_value (result, cur_comp, k * cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_scalar_multiply: error setting value of component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * vector_sqrt
 *
 * Returns the vector of the square roots of each component
 */
int
vector_sqrt (const Vector v, Vector result)
{
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current vector component value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vectors were actually passed */
  if (v == NULL)
    {
      fprintf (stderr, "vector_sqrt: no vector passed\n");
      return EXIT_FAILURE;
    }

  if (result == NULL)
    {
      fprintf (stderr, "vector_sqrt: no result vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks dimension compatibility */
  if (v->dimension != result->dimension)
    {
      fprintf (stderr,
               "vector_sqrt: incompatible dimensions: dim(input) = %ld, dim(result) = %ld\n",
               v->dimension, result->dimension);
      return EXIT_FAILURE;
    }

  /* Takes the square root of the components */
  for (cur_comp = 1; cur_comp <= v->dimension; cur_comp++)
    {
      /* Gets the old component value */
      exit_status = vector_get_value (v, cur_comp, &cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_sqrt: error getting value of component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Sets the new component value */
      exit_status = vector_set_value
        (result, cur_comp, (RValue) sqrt ((double) cur_value));

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_sqrt: error setting value of component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * vector_metric
 *
 * Computes metrics based on the 'incstat' library
 */
int
vector_metric (const Vector v1, const Vector v2,
               const Vector pond_vector,
               const VectorMetric metric, RValue * result)
{
  UsLgIntValue cur_component;   /* current vector component */
  RValue cur_v1, cur_v2;        /* current component values */
  RValue cur_pond;              /* current ponderation coefficient value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vectors were actually passed */
  if (v1 == NULL || v2 == NULL)
    {
      fprintf (stderr, "vector_metric: missing vector\n");
      return EXIT_FAILURE;
    }

  /* Checks the compatibility of the vector dimensions */
  if (v1->dimension != v2->dimension)
    {
      fprintf (stderr,
               "vector_metric: incompatible vector dimensions: dim(v1) = %ld and dim(v2) = %ld\n",
               v1->dimension, v2->dimension);
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility of the ponderation vector */
  if (pond_vector != NULL)
    if (pond_vector->dimension != v1->dimension)
      {
        fprintf (stderr,
                 "vector_metric: incompatible dimension for ponderation vector: dim(v) = %ld; dim(pond) = %ld\n",
                 v1->dimension, pond_vector->dimension);
      }

  /* Initialize the statistics */
  istt_clear_stat ();

  /* Calculates the distance */
  for (cur_component = 1; cur_component <= v1->dimension; cur_component++)
    {
      /* Get the component values */
      exit_status = vector_get_value (v1, cur_component, &cur_v1);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_metric: error getting value of vector 1 for component %ld\n",
                   cur_component);
          return EXIT_FAILURE;
        }

      exit_status = vector_get_value (v2, cur_component, &cur_v2);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vector_metric: error getting value of vector 2 for component %ld\n",
                   cur_component);
          return EXIT_FAILURE;
        }

      /* Optionally gets the ponderation coefficient */
      if (pond_vector != NULL)
        {
          exit_status =
            vector_get_value (pond_vector, cur_component, &cur_pond);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "vector_metric: error getting value of ponderation vector for component %ld\n",
                       cur_component);
              return EXIT_FAILURE;
            }

          /* Adds the values to the statistics */
          istt_add_stat (cur_pond * cur_v1, cur_pond * cur_v2);
        }
      else
        {
          /* Adds the values to the statistics */
          istt_add_stat (cur_v1, cur_v2);
        }
    }

  /* Calculates the requested metric */
  switch (metric)
    {
    case VECTOR_METR_EUCLIDEAN:
      *result = sqrt (istt_sum_sqr_diff_xy ());
      break;

    case VECTOR_METR_INNER_PRODUCT:
      *result = istt_sum_xy ();
      break;

    default:
      fprintf (stderr, "vector_metric: invalid metric\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * vector_metr_euclidean
 *
 * Computes the euclidean distance between v1 and v2
 */
int
vector_metr_euclidean (const Vector v1, const Vector v2, RValue * result)
{
  return vector_metric (v1, v2, NULL, VECTOR_METR_EUCLIDEAN, result);
}



/*
 * vector_metr_inner_product
 *
 * Computes the inner product v1 and v2
 */
int
vector_metr_inner_product (const Vector v1, const Vector v2, RValue * result)
{
  return vector_metric (v1, v2, NULL, VECTOR_METR_INNER_PRODUCT, result);
}
