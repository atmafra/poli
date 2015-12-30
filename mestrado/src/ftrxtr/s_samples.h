#ifndef __SMP_SAMPLES_
#define __SMP_SAMPLES_ 1

#include "s_smptypes.h"

/******************************************************************************
 *                                                                            *
 *                              INDEX OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * create_index
 *
 * Creates and initializes a new list index
 * - index: returns the created index
 * - parent_index: pointer to the parent index
 */
extern int
create_index (index_list_type * index, const index_list_ptr parent_index);



/*
 * destroy_index
 *
 * Frees all the memory associated to the given index structure.
 *
 * Parameters:
 * - index: index to be released
 */
extern int destroy_index (index_list_type * index);



/*
 * add_index_entry
 *
 * Adds a new list to the list index
 * - list_name: string with the name of the list
 * - file_name: string with the file the list is stored in
 * - parent: pointer to the parent list, if it exists
 * - data_type: data type of the list elements (SMP_REAL/SMP_COMPLEX)
 * - ini_time: time associated to the first sample
 * - inc_time: time interval (increment) between consequent samples
 * - ini_norm_time: normalized time associated to the first sample
 * - pos: returns the list position at the list index
 * - set_current: the new index entry should be set as the current entry
 */
extern int
add_index_entry (index_list_type * index,
                 const char *list_name,
                 const char *file_name,
                 const index_entry_type parent,
                 const smp_entries_type data_type,
                 const cmp_real ini_time,
                 const cmp_real inc_time,
                 const smp_num_samples ini_norm_time,
                 smp_index_pos * pos, const smp_set_current_type set_current);



/*
 * destroy_index_entry
 *
 * Remove the last entry of the index list
 */
extern int destroy_index_entry (index_list_type * index);



/*
 * move_to_entry
 *
 * Returns the pointer 'entry' to the entry given by 'pos'
 */
extern int
move_to_entry (index_list_type * index, const smp_index_pos pos,
               index_entry_type * entry);



/*
 * set_current_entry_pos
 *
 * Sets the current entry of the index to the one at position given by pos
 */
extern int
set_current_entry_pos (index_list_type * index, const smp_index_pos pos);



/*
 * set_current_entry
 *
 * Sets the current entry of the index to the given index entry
 */
extern int
set_current_entry (index_list_type * index, const index_entry_type entry);



/*
 * switch_entries
 *
 * Switch the entries given by 'pos1' and 'pos2' of the index
 */
extern int
switch_entries (index_list_type * index,
                const smp_index_pos pos1, const smp_index_pos pos2);



/*
 * add_current_list_value
 *
 * Adds a new value to the current list of the index
 */
extern int
add_current_list_value (index_list_type * index, const cmp_complex value);



/*
 * apply_index_function
 *
 * Applies a complex function to the elements of all the lists in the index
 *
 * Parameters:
 * - index: is the input index
 * - function: the complex function to apply
 * - parc: parameter counter. The number of parameters to the function.
 * - ...: list of parameters
 */
extern int
apply_index_function (index_list_type * index, cmp_function function,
                      const cmp_fcn_mode_type mode, const size_t parc, ...);



/*
 * resize_index
 *
 * Resizes all lists in index to the given number of samples
 */
extern int
resize_index (index_list_type * index, const smp_num_samples samples);



/******************************************************************************
 *                                                                            *
 *                               List Operations                              *
 *                                                                            *
 ******************************************************************************/

/*
 * create_list
 *
 * Creates the effective list of samples
 */
extern int
create_list (sample_list_type ** smp_list,
             const smp_entries_type data_type,
             const cmp_real ini_time,
             const cmp_real inc_time, const smp_num_samples ini_norm_time);



/*
 * destroy_list
 *
 * Frees the list of samples
 */
extern int destroy_list (sample_list_type ** smp_list);



/*
 * resize_list
 *
 * Resizes the list of samples to the given number of samples
 */
extern int
resize_list (sample_list_type ** smp_list, const smp_num_samples samples);



/*
 * get_list_value
 *
 * Returns the value at position pos of the list.
 */
extern int
get_list_value (const sample_list_type smp_list,
                const smp_num_samples pos, cmp_complex * value);



/*
 * set_list_value
 *
 * Set the value at position pos of the list.
 */
extern int
set_list_value (sample_list_type * smp_list, const smp_num_samples pos,
                const cmp_complex value);


/*
 * add_list_value
 *
 * Adds a new value to the list
 */
extern int
add_list_value (sample_list_type ** smp_list, const cmp_complex value);



/*
 * update_list_statistics
 *
 * Update the list's statistics
 */
extern void update_list_statistics (sample_list_type ** smp_list);



/*
 * calculate_list_statistics
 *
 * Calculates and updates the list's statistics.
 * Sets the 'valid_stats' flag to yes.
 */
extern int calculate_list_statistics (sample_list_type ** smp_list);



/*
 * apply_list_function
 *
 * Applies a complex function to the elements of a list
 *
 * Parameters:
 * - smp_list: list of samples
 * - function: complex function to apply
 * - mode: 
 * - parc: parameter counter. The number of parameters in the list.
 * - ...: list of parameters
 */
extern int
apply_list_function (sample_list_type ** smp_list,
                     cmp_function function,
                     const cmp_fcn_mode_type mode, const size_t parc, ...);



/*
 * convert_complex_list_to_real
 *
 * Converts a complex valued list into a real valued one by discarding its
 * imaginary part
 *
 * Parameters:
 * - smp_list: complex valued input list
 */
extern int convert_complex_list_to_real (sample_list_type * smp_list);



#endif /* !__SMP_SAMPLES */
