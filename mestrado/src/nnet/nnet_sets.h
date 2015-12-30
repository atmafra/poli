#ifndef __NNET_SETS_H_
#define __NNET_SETS_H_ 1

#include <stdio.h>
#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                              PUBLIC DATATYPES                              *
 *                                                                            *
 ******************************************************************************/

typedef enum
{
  PICK_FROM_BEGGINING,
  PICK_FROM_END,
  PICK_AT_RANDOM
}
TSetDivisionCriterion;



/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_tset_create
 *
 * Creates a new training set
 */
extern TSet
nnet_tset_create (const Name name,
                  const UnitIndex input_dimension,
                  const UnitIndex output_dimension);



/*
 * nnet_tset_destroy
 *
 * Destroys a previously created training set
 */
extern int nnet_tset_destroy (TSet * set, const BoolValue cascade_elements);



/*
 * nnet_tset_merge
 *
 * Transfers all elements from origin set to destination set.
 * Both sets must have been previously created.
 * Origin set will be empty after merging.
 */
extern int nnet_tset_merge (TSet orig_set, TSet dest_set);



/*
 * nnet_tset_divide
 *
 * Creates a new training set by the division of an original training set,
 * transferring 'nu_elements' to the new set.
 * The elements can be picked from the beggining of the original set, from
 * the end of the original set or at random.
 */
extern int
nnet_tset_divide (TSet orig_set,
                  TSet * dest_set,
                  const Name new_set_name,
                  const ElementIndex nu_elements,
                  const TSetDivisionCriterion division_criterion);



/*
 * nnet_tset_randomize
 *
 * Shuffles the elements in the given set
 */
extern int nnet_tset_randomize (TSet set);



/*
 * nnet_tset_element_create
 *
 * Creates a new training element
 */
extern TElement
nnet_tset_element_create (TSet set, ElementIndex * index,
                          const Vector input,
                          const Vector output,
                          const BoolValue normalize_input,
                          const BoolValue normalize_output,
                          const BoolValue correct_indexes);



/*
 * nnet_tset_element_destroy
 *
 * Destroys a previously created traning element
 */
extern int nnet_tset_element_destroy (TElement * element,
                                      const BoolValue correct_indexes);



/*
 * nnet_tset_element_attach_to_set
 *
 * Attaches a training element to a training set.
 * If 'index' is passed NULL, the element will be transferred to the
 * end of the set.
 */
extern int
nnet_tset_element_attach_to_set (TElement element,
                                 TSet set, ElementIndex * index,
                                 BoolValue correct_indexes);



/*
 * nnet_tset_element_remove_from_set
 *
 * Removes a training element from its training set
 */
extern int
nnet_tset_element_remove_from_set (TElement element,
                                   const BoolValue correct_indexes);



/*
 * nnet_tset_element_transfer_to_set
 *
 * Transfers a training element from its current set to the given set.
 * If 'index' is passed NULL, the element will be transferred to the
 * end of the set.
 */
extern int
nnet_tset_element_transfer_to_set (TElement element,
                                   TSet new_set, ElementIndex * index,
                                   const BoolValue correct_indexes);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_tset_update_vector_stats
 *
 * Updates the input and output vector statistics
 */
extern int nnet_tset_update_vector_stats (const TSet set);



/*
 * nnet_tset_regularize
 *
 * Subtracts the averages and regularizes the training set input and output
 * vectors according to the inverse standard deviations vectors
 */
extern int
nnet_tset_regularize (TSet set,
                      const BoolValue regularize_inputs,
                      const BoolValue regularize_outputs);



/*
 * nnet_tset_read_from_file
 *
 * Reads a training set from a file
 */
extern int
nnet_tset_read_from_file (TSet set,
                          const char *file_name,
                          const BoolValue normalize_input,
                          const BoolValue normalize_output,
                          const BoolValue update_vector_stats,
                          const BoolValue regularize_inputs,
                          const BoolValue regularize_outputs);



/*
 * nnet_tset_create_from_file
 *
 * Creates a new training set and reads its contents from a file
 */
extern TSet
nnet_tset_create_from_file (const Name name,
                            const UnitIndex input_dimension,
                            const UnitIndex output_dimension,
                            const BoolValue normalize_input,
                            const BoolValue normalize_output,
                            const BoolValue update_vector_stats,
                            const BoolValue regularize_inputs,
                            const BoolValue regularize_outputs,
                            const char *file_name);



/*
 * nnet_tset_create_from_list
 *
 * Creates a new training set and reads its contents from a file
 * containint a list of files
 */
extern TSet
nnet_tset_create_from_list (const Name name,
                            const UnitIndex input_dimension,
                            const UnitIndex output_dimension,
                            const BoolValue normalize_input,
                            const BoolValue normalize_output,
                            const BoolValue update_vector_stats,
                            const BoolValue regularize_inputs,
                            const BoolValue regularize_outputs,
                            const char *list_file_name);



/*
 * nnet_tset_goto_element
 *
 * Returns the element at the given index of the set
 */
extern TElement
nnet_tset_goto_element (const TSet set, const ElementIndex index);



/*
 * nnet_tset_element_info
 *
 * Outputs element information
 */
extern void
nnet_tset_element_info (const TElement element,
                        const BoolValue include_input_array,
                        const BoolValue include_output_array,
                        FILE * output_fd);



/*
 * nnet_tset_info
 *
 * Outputs training set information
 */
extern void
nnet_tset_info (const TSet set,
                const BoolValue include_elements,
                const BoolValue include_input_array,
                const BoolValue include_output_array,
                const BoolValue include_input_vector_stats,
                const BoolValue include_oudput_vector_stats,
                FILE * output_fd);



#endif /* __NNET_SETS_H_ */
