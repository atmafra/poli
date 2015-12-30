#ifndef __TRMAP_H_
#define __TRMAP_H_ 1

#include "../common/types.h"
#include "../vector/vector.h"
#include "../matrix/matrix.h"


/******************************************************************************
 *                                                                            *
 *                           TRANSITION MAPS DATATYPES                        *
 *                                                                            *
 ******************************************************************************/

/*
 * TransitionMap
 *
 * The transition map is itself a bidimesional square matrix, counting the
 * transitions from the states given by the columns to the states given by the
 * rows. Thus, the states are matrix indexes.
 */
typedef Matrix TransitionMap;
typedef MIndex TMState;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * trmap_create
 *
 * Creates a new transition map
 */
extern TransitionMap
trmap_create (const TMState dimension, const RValue initial_value);



/*
 * trmap_destroy
 *
 * Destroys a previously created transition map
 */
extern int trmap_destroy (TransitionMap * map);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * trmap_transition
 *
 * Registers a new transition from state s1 to state s2 in the given map
 */
extern int
trmap_transition (TransitionMap map, const MIndex s1, const MIndex s2);



/*
 * trmap_reset
 *
 * Resets the given transition map to the given value
 */
extern int trmap_reset (TransitionMap map, const RValue value);



/*
 * trmap_create_from_vector
 *
 * Creates a new transition map, based on an input vector
 */
extern TransitionMap
trmap_create_from_vector (const TMState dimension, const Vector input);



#endif /* __TRMAP_H_ */
