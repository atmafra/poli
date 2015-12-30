#include <stdlib.h>

#include "types.h"
#include "errorh.h"
#include "vector.h"
#include "matrix.h"

#include "trmap.h"


/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * trmap_create
 *
 * Creates a new transition map
 */
TransitionMap
trmap_create (const TMState dimension, const RValue initial_value)
{
  TransitionMap new_map;        /* new transition map */

  /* Creates the new transition map matrix */
  new_map = matrix_create (dimension, dimension, TRUE, initial_value);
  if (new_map == NULL)
    {
      error_failure ("trmap_create",
                     "error creating %ld-dimension transition map\n",
                     dimension);
      return NULL;
    }

  return new_map;
}



/*
 * trmap_destroy
 *
 * Destroys a previously created transition map
 */
int
trmap_destroy (TransitionMap * map)
{
  /* Destroys the matrix */
  if (matrix_destroy (map) != EXIT_SUCCESS)
    return error_failure ("trmap_destroy",
                          "error destroying transition map matrix\n");

  return EXIT_SUCCESS;
}



/*
 * trmap_transition
 *
 * Registers a new transition from state s1 to state s2 in the given map
 */
int
trmap_transition (TransitionMap map, const MIndex s1, const MIndex s2)
{
  /* Checks if the map was actually passed */
  if (map == NULL)
    return error_failure ("trmap_transition", "no transition map passed\n");

  /* Sums one to the matrix element corresponding to the transition */
  if (matrix_sum_value (map, s2, s1, 1.0) != EXIT_SUCCESS)
    return error_failure ("trmap_transition",
                          "error registering transition from state %ld to state %ld\n",
                          s1, s2);

  return EXIT_SUCCESS;
}



/*
 * trmap_reset
 *
 * Resets the given transition map to the given value
 */
int
trmap_reset (TransitionMap map, const RValue value)
{
  /* Re-initializes the transition map matrix with the given value */
  if (matrix_init (map, value) != EXIT_SUCCESS)
    return error_failure ("trmap_transition",
                          "error re-initializing transition map\n");

  return EXIT_SUCCESS;
}



/*
 * trmap_create_from_vector
 *
 * Creates a new transition map, based on an input vector
 */
TransitionMap
trmap_create_from_vector (const TMState dimension, const Vector input,
                          const BoolValue divide_by_length,
                          const BoolValue apply_log)
{
  TransitionMap map = NULL;     /* new transition map */
  TMState s1 = 0, s2 = 0;       /* last indexes */
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current component state */


  /* checks if the vector was actually passed */
  if (input == NULL)
    return null_failure ("trmap_create_from_vector",
                         "no input vector passed\n");

  /* creates the new transition map, based on the vector dimension */
  map = trmap_create (dimension, 0.0);
  if (map == NULL)
    return null_failure ("trmap_create_from_vector",
                         "error creating new transition map\n");

  /* trivial case */
  if (input->dimension == 0)
    return map;

  /* registers all transitions in the vector */
  if (vector_get_value (input, 1, &cur_value) != EXIT_SUCCESS)
    return null_failure ("trmap_create_from_vector",
                         "error getting value of first component\n");

  s1 = (TMState) cur_value;

  for (cur_comp = 2; cur_comp <= input->dimension; cur_comp++)
    {
      /* gets the vector component as state */
      if (vector_get_value (input, cur_comp, &cur_value) != EXIT_SUCCESS)
        return null_failure ("trmap_create_from_vector",
                             "error getting value of component %ld\n",
                             cur_comp);

      s2 = (TMState) cur_value;

      /* registers transition */
      if (trmap_transition (map, s1, s2) != EXIT_SUCCESS)
        return null_failure ("trmap_create_from_vector",
                             "error setting transition from state %ld to state %ld\n",
                             s1, s2);
      s1 = s2;
    }

  /* apply log to elements? */
  if (apply_log == TRUE)
    {
      if (matrix_sum_all (map, 1.0) != EXIT_SUCCESS)
        return null_failure ("trmap_create_from_vector",
                             "error adding 1.0 to map's elements\n");

      if (matrix_log_all (map) != EXIT_SUCCESS)
        return null_failure ("trmap_create_from_vector",
                             "error applying log to map's elements\n");
    }

  /* divide by winner vector length? */
  if (divide_by_length == TRUE)
    if (matrix_mult_all (map, 1.0 / (RValue) input->dimension)
        != EXIT_SUCCESS)
      return null_failure ("trmap_create_from_vector",
                           "error dividing map by input vector length\n");

  return map;
}
