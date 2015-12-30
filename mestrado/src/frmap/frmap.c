#include <stdlib.h>
#include <float.h>

#include "types.h"
#include "errorh.h"
#include "vector.h"

#include "frmap.h"


/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * frmap_create
 *
 * Creates a new frequency map
 */
FrequencyMap
frmap_create (const FMState dimension, const RValue initial_value)
{
  FrequencyMap new_map;         /* new frequency map */

  /* Creates the new frequency map vector */
  new_map = vector_create (dimension);
  if (new_map == NULL)
    return null_failure ("frmap_create",
                         "error creating %ld-dimension frequency map\n",
                         dimension);

  /* initializes */
  if (frmap_reset (new_map, initial_value) != EXIT_SUCCESS)
    return null_failure ("frmap_create",
                         "error initializing frequency map\n");

  return new_map;
}



/*
 * frmap_destroy
 *
 * Destroys a previously created frequency map
 */
int
frmap_destroy (FrequencyMap * map)
{
  /* Destroys the vector */
  if (vector_destroy (map) != EXIT_SUCCESS)
    return error_failure ("frmap_destroy",
                          "error destroying frequency map vector\n");

  *map = NULL;

  return EXIT_SUCCESS;
}



/*
 * frmap_occurrence
 *
 * Registers a new occurrence of state 's'
 */
int
frmap_occurrence (FrequencyMap map, const UsLgIntValue s)
{
  /* Checks if the map was actually passed */
  if (map == NULL)
    return error_failure ("frmap_occurrence", "no frequency map passed\n");

  /* Sums one to the vector element corresponding to the frequency */
  if (vector_sum_value (map, s, 1.0) != EXIT_SUCCESS)
    return error_failure ("frmap_occurrence",
                          "error registering occurrence of state %ld\n", s);

  return EXIT_SUCCESS;
}



/*
 * frmap_reset
 *
 * Resets the given frequency map to the given value
 */
int
frmap_reset (FrequencyMap map, const RValue value)
{
  UsLgIntValue cur_dim;

  if (map == NULL)
    return error_failure ("frmap_reset", "no frequency map passed\n");

  for (cur_dim = 1; cur_dim <= map->dimension; cur_dim++)
    if (vector_set_value (map, cur_dim, value) != EXIT_SUCCESS)
      return error_failure ("frmap_reset",
                            "error initializing frequency map\n");

  return EXIT_SUCCESS;
}



/*
 * frmap_create_from_vector
 *
 * Creates a new frequency map, based on an input vector
 */
FrequencyMap
frmap_create_from_vector (const FMState dimension, const Vector input,
                          const BoolValue divide_by_length)
{
  FrequencyMap map = NULL;      /* new frequency map */
  FMState s = 0;                /* last indexes */
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current component state */


  /* checks if the vector was actually passed */
  if (input == NULL)
    return null_failure ("frmap_create_from_vector",
                         "no input vector passed\n");

  /* creates the new frequency map, based on the vector dimension */
  map = frmap_create (dimension, 0.0);
  if (map == NULL)
    return null_failure ("frmap_create_from_vector",
                         "error creating new frequency map\n");

  /* trivial case */
  if (input->dimension == 0)
    return map;

  /* registers all occurrences in the vector */
  if (vector_get_value (input, 1, &cur_value) != EXIT_SUCCESS)
    return null_failure ("frmap_create_from_vector",
                         "error getting value of first component\n");

  for (cur_comp = 1; cur_comp <= input->dimension; cur_comp++)
    {
      /* gets the vector component as state */
      if (vector_get_value (input, cur_comp, &cur_value) != EXIT_SUCCESS)
        return null_failure ("frmap_create_from_vector",
                             "error getting value of component %ld\n",
                             cur_comp);

      s = (FMState) cur_value;

      /* registers occurrence */
      if (frmap_occurrence (map, s) != EXIT_SUCCESS)
        return null_failure ("frmap_create_from_vector",
                             "error setting occurence of state %ld\n", s);
    }

  /* divide by winner vector length? */
  if (divide_by_length == TRUE)
    if (vector_scalar_multiply (map, 1.0 / (RValue) input->dimension, map)
        != EXIT_SUCCESS)
      return null_failure ("frmap_create_from_vector",
                           "error dividing map by input vector length\n");

  return map;
}



/*
 * frmap_most_frequent_state
 *
 * Returns the most frequent state in frequency map
 */
FMState
frmap_most_frequent_state (const FrequencyMap map)
{
  RValue max = 0.0;             /* maximum value */
  RValue cur = 0.0;             /* current value */
  FMState cur_state = 1;        /* current state */
  FMState max_state = 0;        /* state with maximum value */

  /* trivial case */
  if (map == NULL)
    return 0;

  /* finds maximum value state */
  for (cur_state = 1; cur_state <= map->dimension; cur_state++)
    {
      if (vector_get_value (map, cur_state, &cur) != EXIT_SUCCESS)
        return 0;

      if (cur - max > DBL_EPSILON)
        {
          max = cur;
          max_state = cur_state;
        }
    }

  return max_state;
}
