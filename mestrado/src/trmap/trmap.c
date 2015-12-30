#include <stdlib.h>
#include "../common/types.h"
#include "../errorh/errorh.h"
#include "../vector/vector.h"
#include "../matrix/matrix.h"
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
      fprintf (stderr,
               "trmap_create: error creating %ld-dimension transition map\n",
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
  int exit_status;              /* auxiliary function return status */

  /* Destroys the matrix */
  exit_status = matrix_destroy (map);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "trmap_destroy: error destroying transition map matrix\n");
      return EXIT_FAILURE;
    }

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
  int exit_status;              /* auxiliary function return status */


  /* Checks if the map was actually passed */
  if (map == NULL)
    {
      fprintf (stderr, "trmap_transition: no transition map passed\n");
      return EXIT_FAILURE;
    }

  /* Sums one to the matrix element corresponding to the transition */
  exit_status = matrix_sum_value (map, s2, s1, 1.0);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "trmap_transition: error registering transition from state %ld to state %ld\n",
               s1, s2);
      return EXIT_FAILURE;
    }

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
  int exit_status;              /* auxiliary function return status */

  /* Re-initializes the transition map matrix with the given value */
  exit_status = matrix_init (map, value);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "trmap_transition: error re-initializing transition map\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * trmap_create_from_vector
 *
 * Creates a new transition map, based on an input vector
 */
TransitionMap
trmap_create_from_vector (const TMState dimension, const Vector input)
{
  TransitionMap map = NULL;     /* new transition map */
  TMState s1 = 0, s2 = 0;       /* last indexes */
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current component state */


  /* checks if the vector was actually passed */
  if (input == NULL)
    {
      error_failure ("trmap_create_from_vector", "no input vector passed\n");
      return NULL;
    }

  /* creates the new transition map, based on the vector dimension */
  if (error_if_null
      (map =
       trmap_create (dimension, 0.0), "trmap_create_from_vector",
       "error creating new transition map\n"))
    return NULL;

  /* trivial case */
  if (input->dimension == 0)
    return map;

  /* registers all transitions in the vector */
  if (error_if_failure
      (vector_get_value (input, 1, &cur_value), "trmap_create_from_vector",
       "error getting value of first component\n"))
    return NULL;

  s1 = (TMState) cur_value;

  for (cur_comp = 2; cur_comp <= input->dimension; cur_comp++)
    {
      /* gets the vector component as state */
      if (error_if_failure
          (vector_get_value (input, cur_comp, &cur_value),
           "trmap_create_from_vector",
           "error getting value of component %ld\n", cur_comp))
        return NULL;

      s2 = (TMState) cur_value;

      /* registers transition */
      if (error_if_failure
          (trmap_transition (map, s1, s2), "trmap_create_from_vector",
           "error setting transition from state %ld to state %ld\n", s1, s2))
        return NULL;

      s1 = s2;
    }

  return map;
}
