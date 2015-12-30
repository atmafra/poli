#ifndef __FRMAP_H_
#define __FRMAP_H_ 1

#include "types.h"
#include "vector.h"


/******************************************************************************
 *                                                                            *
 *                           TRANSITION MAPS DATATYPES                        *
 *                                                                            *
 ******************************************************************************/

/*
 * FrequencyMap
 *
 * The frequency map is itself a vector, counting the occurrences of states
 */
typedef Vector FrequencyMap;
typedef UsLgIntValue FMState;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * frmap_create
 *
 * Creates a new frequency map
 */
extern FrequencyMap
frmap_create (const FMState dimension, const RValue initial_value);



/*
 * frmap_destroy
 *
 * Destroys a previously created frequency map
 */
extern int frmap_destroy (FrequencyMap * map);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * frmap_occurrence
 *
 * Registers a new occurrence of state 's'
 */
extern int frmap_occurrence (FrequencyMap map, const UsLgIntValue s);



/*
 * frmap_reset
 *
 * Resets the given frequency map to the given value
 */
extern int frmap_reset (FrequencyMap map, const RValue value);



/*
 * frmap_create_from_vector
 *
 * Creates a new frequency map, based on an input vector
 */
extern FrequencyMap
frmap_create_from_vector (const FMState dimension, const Vector input,
                          const BoolValue divide_by_length);



/*
 * frmap_most_frequent_state
 *
 * Returns the most frequent state in frequency map
 */
extern FMState frmap_most_frequent_state (const FrequencyMap map);



#endif /* __FRMAP_H_ */
