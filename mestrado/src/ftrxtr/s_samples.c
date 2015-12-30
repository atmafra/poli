#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "errorh.h"
#include "incstat.h"

#include "s_complex.h"
#include "s_smptypes.h"
#include "s_samples.h"

/*
 * get_new_index_id
 *
 * Returns a new sequential value to be used as an index identifier
 */
static smp_index_pos
get_new_index_id (void)
{
  static smp_index_pos index_seq = 0;

  /* Returns the current value of index_seq */
  ++index_seq;

  return (smp_index_pos) index_seq - 1;
}



/*
 * create_index
 *
 * Creates and initializes a new index
 */
int
create_index (index_list_type * index, const index_list_ptr parent_index)
{
  /* Initialize head, tail and current */
  index->head = NULL;
  index->current = NULL;
  index->tail = NULL;

  /* Initializes the number of entries on the index */
  index->num_entries = 0;

  /* Set the parent index */
  index->parent_index = parent_index;

  /* Gets the new index identifier */
  index->index_id = get_new_index_id ();

  return EXIT_SUCCESS;
}



/*
 * destroy_index
 *
 * Frees all the memory associated to the given index structure.
 *
 * Parameters:
 * - index: index to be released
 */
int
destroy_index (index_list_type * index)
{
  index_entry_type current_entry;       /* current entry */
  index_entry_type previous_entry;      /* saved previous entry */

  /* Destroys all the index entries, from tail to head */
  current_entry = index->tail;

  while (current_entry != NULL)
    {
      previous_entry = current_entry->previous;

      /*
       * destroys the last entry
       * Moves to the previous entry
       */
      if (destroy_index_entry (index) != EXIT_SUCCESS)
        return error_failure ("destroy_index",
                              "error releasing last index entry %ld\n",
                              index->num_entries);

      current_entry = previous_entry;
    }

  /* Destroys the index itself */
  index->head = NULL;
  index->tail = NULL;
  index->current = NULL;
  index = NULL;

  return EXIT_SUCCESS;
}




/*
 * create_index_entry
 *
 * Allocates memory for a new index entry
 */
static int
create_index_entry (index_entry_type * entry)
{
  *entry = (index_entry_type) malloc (sizeof (index_entry));

  if (*entry == NULL)
    return error_failure ("create_index_entry",
                          "virtual memory exhausted.\n");

  /* The list pointers of the index are initialized to NULL */
  (*entry)->next = NULL;
  (*entry)->previous = NULL;
  (*entry)->list = NULL;

  return EXIT_SUCCESS;
}



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
int
add_index_entry (index_list_type * index,
                 const char *list_name,
                 const char *file_name,
                 const index_entry_type parent,
                 const smp_entries_type data_type,
                 const cmp_real ini_time,
                 const cmp_real inc_time,
                 const smp_num_samples ini_norm_time,
                 smp_index_pos * pos, const smp_set_current_type set_current)
{
  index_entry_type new_entry;   /* pointer for the new entry */


  /* Creates a new index entry */
  if (create_index_entry (&new_entry) != EXIT_SUCCESS)
    return error_failure ("add_index_entry",
                          "error creating new index entry\n");

  /* If the list is empty, makes the new entry the head of the list */
  if (index->num_entries == 0)
    {
      /* Make this single entry the head, tail and current of the list */
      index->head = new_entry;
      index->current = new_entry;
    }
  else
    {
      /*
       * Appends new entry to the tail of the list
       * Makes the new entry the next one for the tail
       */
      new_entry->previous = index->tail;
      index->tail->next = new_entry;
    }

  /*
   * Advances the tail of the list to the new entry
   * Increments the number of index entries of the list
   * Initializes the list name and the associated file name
   * Links to the parent list
   * Sets the list position at the index
   * Optionally sets the new entry as the current entry
   */
  index->tail = new_entry;
  index->num_entries++;
  strncpy (new_entry->name, list_name, NAME_SIZE);
  strncpy (new_entry->file, file_name, FILE_NAME_SIZE);
  new_entry->parent = parent;
  new_entry->position = index->num_entries;

  if (set_current == SMP_SET_CURRENT)
    index->current = new_entry;

  /* Creates the samples list */
  if (create_list (&new_entry->list, data_type, ini_time, inc_time,
                   ini_norm_time) != EXIT_SUCCESS)

    return error_failure ("add_index_entry",
                          "error allocating memory for the samples list\n");

  /* The new list is always appended to the tail of the index */
  *pos = index->num_entries;

  return EXIT_SUCCESS;
}



/*
 * destroy_index_entry
 *
 * Remove the last entry of the given index
 */
int
destroy_index_entry (index_list_type * index)
{
  index_entry_type aux_entry = NULL;    /* auxiliary entry */


  /* The index must have at least one entry to be removed */
  if (index->num_entries == 0)
    return error_failure ("destroy_index_entry", "index has no entries\n");

  /* Releases the memory for the last entry */
  if (index->tail->list->samples > 0)
    {
      /* Releases data */
      switch (index->tail->list->data_type)
        {
        case SMP_REAL:
          free (index->tail->list->r);
          break;

        case SMP_COMPLEX:
          free (index->tail->list->z);
          break;

        default:
          return error_failure ("destroy_index_entry",
                                "unknown list data type\n");
        }

      /* Releases the list */
      free (index->tail->list);
      index->tail->list = NULL;
    }

  /*
   * Releases the index entry attributes
   * Deals with the current entry
   * Stores the tail address
   * Moves the tail back one entry
   * Makes the tail point to NULL
   * Deallocates tail entry memory
   * Decrements the number of index entries of the list
   */
  if (index->current == index->tail)
    index->current = index->tail->previous;

  aux_entry = index->tail;
  index->tail = index->tail->previous;

  if (index->tail != NULL)
    index->tail->next = NULL;

  free (aux_entry);
  --index->num_entries;

  /* If removing the last entry, make header point to NULL */
  if (index->num_entries == 0)
    index->head = NULL;

  return EXIT_SUCCESS;
}



/*
 * move_to_entry
 *
 * Returns the pointer 'entry' to the entry given by 'pos'
 */
int
move_to_entry (index_list_type * index, const smp_index_pos pos,
               index_entry_type * entry)
{
  index_entry_type cur_entry;   /* current entry */
  smp_index_pos entry_cnt;      /* current position counter */


  /* Validates the current position */
  if (pos <= 0)
    return error_failure ("move_to_entry",
                          "requested entry is non-positive\n");

  if (pos > index->num_entries)
    return error_failure ("move_to_entry",
                          "requested entry is beyond the end of the index\n");

  /* Goes to the requested position */
  cur_entry = index->head;

  for (entry_cnt = 1; entry_cnt < pos; entry_cnt++)
    cur_entry = cur_entry->next;

  /* Sets the return variable */
  *entry = cur_entry;

  return EXIT_SUCCESS;
}



/*
 * set_current_entry_pos
 *
 * Sets the current entry of the index to the one at position given by pos
 */
int
set_current_entry_pos (index_list_type * index, const smp_index_pos pos)
{
  index_entry_type cur_entry;   /* auxiliary current entry pointer */
  int exit_status;              /* auxiliary function return status */


  /* Moves to the requested entry */
  exit_status = move_to_entry (index, pos, &cur_entry);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "set_current_entry: error moving to requested entry\n");
      return EXIT_FAILURE;
    }

  /* Sets the current position */
  index->current = cur_entry;

  return EXIT_SUCCESS;
}


/*
 * set_current_entry
 *
 * Sets the current entry of the index to the given index entry
 */
int
set_current_entry (index_list_type * index, const index_entry_type entry)
{
  /* Sets the current entry */
  index->current = entry;

  return EXIT_SUCCESS;
}


/*
 * switch_entries
 *
 * Switch the entries given by 'pos1' and 'pos2' of the index
 */
int
switch_entries (index_list_type * index, const smp_index_pos pos1,
                const smp_index_pos pos2)
{
  index_entry_type entry1, entry2, aux_entry;   /* auxiliary entry pointers */
  int exit_status;              /* auxiliary function return status */


  /* Retrieves the pointers to the requested entries */
  exit_status = move_to_entry (index, pos1, &entry1);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "switch_entries: error moving to the first entry\n");
      return EXIT_FAILURE;
    }

  exit_status = move_to_entry (index, pos2, &entry2);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "switch_entries: error moving to the second entry\n");
      return EXIT_FAILURE;
    }

  /* Allocate memory space for the auxiliary pointer */
  exit_status = create_index_entry (&aux_entry);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "switch_entries: error allocating memory space\n");
      return EXIT_FAILURE;
    }

  /* Backs up the old first entry */
  aux_entry->previous = entry1->previous;
  aux_entry->next = entry1->next;
  aux_entry->position = entry1->position;

  /* Loads the first entry with the second entry content */
  entry1->previous = entry2->previous;
  entry1->next = entry2->next;
  entry1->position = entry2->position;

  if (entry2->previous != NULL)
    entry2->previous->next = entry1;

  if (entry1->next != NULL)
    entry2->next->previous = entry1;

  /* Loads the second entry with the backed up first entry content */
  entry2->previous = aux_entry->previous;
  entry2->next = aux_entry->next;
  entry2->position = aux_entry->position;

  if (aux_entry->previous != NULL)
    aux_entry->previous->next = entry2;

  if (aux_entry->next != NULL)
    aux_entry->next->previous = entry2;

  /* Deals with head and tail */
  if (entry1 == index->head)
    index->head = entry2;
  else if (entry2 == index->head)
    index->head = entry1;

  if (entry1 == index->tail)
    index->tail = entry2;
  else if (entry2 == index->tail)
    index->tail = entry1;

  /* Frees up auxiliary pointer memory space */
  free (aux_entry);
  aux_entry = NULL;

  return EXIT_SUCCESS;
}



/*
 * add_current_list_value
 *
 * Adds a new value to the current list of the index
 */
int
add_current_list_value (index_list_type * index, const cmp_complex value)
{
  int exit_status;              /* auxiliary function return status */


  /* Dispatch to the direct list operation */
  exit_status = add_list_value (&(index->current->list), value);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "add_current_list_value: error including new value to the current list\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}




/*
 * apply_index_function
 *
 * Applies a complex function to the elements of all the lists in the index
 *
 * Parameters:
 * - index: input index
 * - function: complex function to apply
 * - mode: selects if Real or Complex calculations should be done
 * - parc: parameters count. The number of parameters to the function.
 * - ...: list of the function parameters
 */
int
apply_index_function (index_list_type * index, cmp_function function,
                      const cmp_fcn_mode_type mode, const size_t parc, ...)
{
  va_list parlist;              /* list of input parameters */
  cmp_complex listval;          /* list element value */
  cmp_complex retval;           /* function return value */
  smp_num_samples cur_element;  /* list element counter */
  index_entry_type cur;         /* current index entry */


  cur = index->head;

  /* List loop */
  while (cur != NULL)
    {
      /* Applies the function to all the elements of the list */
      for (cur_element = 1; cur_element <= cur->list->samples; cur_element++)
        {
          /* Get the value of the element at the current position */
          if (get_list_value (*(cur->list), cur_element, &listval)
              != EXIT_SUCCESS)

            return error_failure ("apply_index_function",
                                  "error getting list value at position %ld\n",
                                  cur_element);

          /* Initializes the parameter list */
          va_start (parlist, parc);

          /* Applies the function to the element value */
          if (cmp_fcn_exec (function, &retval, mode, parc, &parlist, 1,
                            listval) != EXIT_SUCCESS)

            return error_failure ("apply_index_function",
                                  "error in call to complex function for element %ld\n",
                                  cur_element);

          /* Finalizes the parameter list */
          va_end (parlist);

          /* Sets the value of the current element */
          if (set_list_value (cur->list, cur_element, retval) != EXIT_SUCCESS)

            return error_failure ("apply_index_function",
                                  "error setting element %ld value\n",
                                  cur_element);
        }

      /* Move to the next list */
      cur = cur->next;
    }

  return EXIT_SUCCESS;
}



/*
 * resize_index
 *
 * Resizes all lists in index to the given number of samples
 */
int
resize_index (index_list_type * index, const smp_num_samples samples)
{
  index_entry_type cur;         /* current index entry */

  cur = index->head;

  while (cur != NULL)
    {
      if (resize_list (&(cur->list), samples) != EXIT_SUCCESS)
        return error_failure ("resize_index", "error resizing list %ld\n",
                              cur->position);

      cur = cur->next;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                            VALUES LISTS OPERATIONS                         *
 *                                                                            *
 ******************************************************************************/

/*
 * create_list
 *
 * Creates the effective list of samples
 */
int
create_list (sample_list_type ** smp_list, const smp_entries_type data_type,
             const cmp_real ini_time, const cmp_real inc_time,
             const smp_num_samples ini_norm_time)
{
  /* Allocates memory space for the new list */
  (*smp_list) = (sample_list_type *) malloc (sizeof (sample_list_type));
  if ((*smp_list) == NULL)
    {
      fprintf (stderr, "create_list: virtual memory exhausted\n");
      return EXIT_FAILURE;
    }

  /* Initialize the number of samples in the list */
  (*smp_list)->samples = 0;

  /* Sets the list elements data type */
  if (data_type != SMP_REAL && data_type != SMP_COMPLEX)
    {
      fprintf (stderr,
               "create_list: invalid elements data type for list creation\n");
      return EXIT_FAILURE;
    }
  else
    (*smp_list)->data_type = data_type;

  /* Associated time scale */
  (*smp_list)->ini_time = ini_time;
  (*smp_list)->inc_time = inc_time;
  (*smp_list)->ini_norm_time = ini_norm_time;

  /* Initialize the statistics */
  istt_clear_stat ();

  /* Update the statistics */
  update_list_statistics (smp_list);

  /* Marks the list's statistics status as valid */
  (*smp_list)->valid_stats = SMP_YES;

  /* Initialize the list */
  (*smp_list)->r = NULL;
  (*smp_list)->z = NULL;

  return EXIT_SUCCESS;
}




/*
 * destroy_list
 *
 * Frees the list of samples
 */
int
destroy_list (sample_list_type ** smp_list)
{
  /* List elements data type */
  smp_entries_type data_type;


  /* Frees the memory space of the list */
  if ((*smp_list) != NULL)
    {
      /* Initializes the elements data type */
      data_type = (*smp_list)->data_type;

      /* Frees the list memory */
      switch (data_type)
        {
        case SMP_REAL:
          free ((*smp_list)->r);
          break;

        case SMP_COMPLEX:
          free ((*smp_list)->z);
          break;

        default:
          fprintf (stderr, "destroy_list: invalid elements data type\n");
          return EXIT_FAILURE;
        }
      free ((*smp_list));
      *smp_list = NULL;
    }
  return EXIT_SUCCESS;
}




/*
 * resize_list
 *
 * Resizes the list of samples to the given number of samples
 */
int
resize_list (sample_list_type ** smp_list, const smp_num_samples samples)
{
  if (samples > __SMP_MAX_SAMPLES_)
    return error_failure ("resize_list",
                          "exceeded the maximum of samples supported\n");

  /* If zero samples requested, dispatch to 'destroy_list' */
  if (samples == 0)
    {
      destroy_list (smp_list);
      return EXIT_SUCCESS;
    }

  /* Calls 'realloc' for resizing */
  /* Extends the list for the correct data type */
  switch ((*smp_list)->data_type)
    {
    case SMP_REAL:

      (*smp_list)->r = (cmp_real *) realloc ((*smp_list)->r,
                                             (samples + 1) *
                                             sizeof (cmp_real));
      if ((*smp_list)->r == NULL)
        return error_failure ("resize_list", "virtual memory exhausted\n");

      break;


    case SMP_COMPLEX:
      (*smp_list)->z = (cmp_complex *) realloc ((*smp_list)->z,
                                                (samples + 1) *
                                                sizeof (cmp_complex));
      if ((*smp_list)->z == NULL)
        return error_failure ("resize_list", "virtual memory exhausted\n");

      break;


    default:
      return error_failure ("resize_list", "invalid elements data type\n");
    }

  /* Sets the number of samples */
  (*smp_list)->samples = samples;

  /* Invalidates statistics */
  (*smp_list)->valid_stats = SMP_NO;

  return EXIT_SUCCESS;
}




/*
 * get_list_value
 *
 * Returns the value at position pos of the list.
 */
int
get_list_value (const sample_list_type smp_list, const smp_num_samples pos,
                cmp_complex * value)
{
  /* Validates the interval for pos */
  if (pos < 1 || pos > smp_list.samples)
    return error_failure ("get_list_value",
                          "requested position %ld is beyond the end of the list %ld\n",
                          pos, smp_list.samples);

  /* Sets the return value */
  switch (smp_list.data_type)
    {
    case SMP_REAL:
      value->re = *(smp_list.r + pos);
      value->im = 0.0;
      break;

    case SMP_COMPLEX:
      value->re = (smp_list.z + pos)->re;
      value->im = (smp_list.z + pos)->im;
      break;

    default:
      return error_failure ("get_list_value", "invalid elements data type\n");
    }

  return EXIT_SUCCESS;
}



/*
 * set_list_value
 *
 * Set the value at position pos of the list.
 */
int
set_list_value (sample_list_type * smp_list, const smp_num_samples pos,
                const cmp_complex value)
{
  /* Validates the interval for pos */
  if (pos < 1 || pos > smp_list->samples)
    return error_failure ("set_list_value",
                          "requested value (%ld) is out of range (1..%ld)\n",
                          pos, smp_list->samples);

  /* Sets the list value */
  switch (smp_list->data_type)
    {
    case SMP_REAL:
      *(smp_list->r + pos) = value.re;
      break;

    case SMP_COMPLEX:
      (smp_list->z + pos)->re = value.re;
      (smp_list->z + pos)->im = value.im;
      break;

    default:
      return error_failure ("set_list_value", "invalid elements data type\n");
    }

  /* Invalidates the current statistics status */
  smp_list->valid_stats = SMP_NO;

  return EXIT_SUCCESS;
}



/*
 * add_list_value
 *
 * Adds a new value to the list
 */
int
add_list_value (sample_list_type ** smp_list, const cmp_complex value)
{
  smp_num_samples samples = (*smp_list)->samples;       /* number of samples */

  /*
   * Dispatches the resizing to 'resize_list'
   * Sets the new value
   */

  if (resize_list (smp_list, samples + 1) != EXIT_SUCCESS)
    return error_failure ("add_list_value", "error resizing samples list\n");

  if (set_list_value ((*smp_list), samples + 1, value) != EXIT_SUCCESS)
    return error_failure ("add_list_value", "error setting new value\n");

  return EXIT_SUCCESS;
}



/*
 * update_list_statistics
 *
 * Update the list's statistics
 */
void
update_list_statistics (sample_list_type ** smp_list)
{
  /* Update the list's statistics */
  (*smp_list)->sum.re = (cmp_real) istt_sum_x ();
  (*smp_list)->sum.im = (cmp_real) istt_sum_y ();
  (*smp_list)->avg.re = (cmp_real) istt_average_x ();
  (*smp_list)->avg.im = (cmp_real) istt_average_y ();
  (*smp_list)->var.re = (cmp_real) istt_variance_x ();
  (*smp_list)->var.im = (cmp_real) istt_variance_y ();
  (*smp_list)->std.re = (cmp_real) istt_stddev_x ();
  (*smp_list)->std.im = (cmp_real) istt_stddev_y ();
  (*smp_list)->max.re = (cmp_real) istt_max_x ();
  (*smp_list)->max.im = (cmp_real) istt_max_y ();
  (*smp_list)->min.re = (cmp_real) istt_min_x ();
  (*smp_list)->min.im = (cmp_real) istt_min_y ();
}



/*
 * calculate_list_statistics
 *
 * Calculates and updates the list's statistics.
 * Sets the 'valid_stats' flag to yes.
 */
int
calculate_list_statistics (sample_list_type ** smp_list)
{
  smp_num_samples cur_sample, samples;  /* sample counters */
  cmp_complex aux_z;            /* sample value */


  /* If the statistics are already valid, nothing to do */
  if ((*smp_list)->valid_stats != SMP_YES)
    {
      /*
       * Clear the statistics
       * Sets the number of samples
       * Recalculates the statistics
       * Update the list's statistics
       * Set the 'valid_stats' flag to yes
       * Clear the statistics again for future use
       */

      istt_clear_stat ();
      samples = (*smp_list)->samples;

      for (cur_sample = 1; cur_sample <= samples; cur_sample++)
        {
          /*
           * Get the list value
           * Increments the statistics
           */
          if (get_list_value (**smp_list, cur_sample, &aux_z) != EXIT_SUCCESS)

            return error_failure ("calculate_list_statistics",
                                  "error getting %ld sample value\n",
                                  cur_sample);

          istt_add_stat (aux_z.re, aux_z.im);
        }

      update_list_statistics (smp_list);
      (*smp_list)->valid_stats = SMP_YES;
      istt_clear_stat ();
    }

  return EXIT_SUCCESS;
}




/*
 * apply_list_function
 *
 * Applies a complex function to the elements of a list, one by one
 *
 * Parameters:
 * - smp_list: is the list of samples
 * - function: the complex function to apply
 * - parc: parameters count. The number of parameters in the list.
 * - ...: list of the function parameters
 */
int
apply_list_function (sample_list_type ** smp_list, cmp_function function,
                     const cmp_fcn_mode_type mode, const size_t parc, ...)
{
  va_list parlist;              /* List of input parameters */
  cmp_complex listval;          /* list element value */
  cmp_complex retval;           /* function returned value */
  smp_num_samples cur_element;  /* List element counter */


  /* Applies the function to all the elements of the list */
  for (cur_element = 1; cur_element <= (*smp_list)->samples; cur_element++)
    {
      /*
       * Get the value of the element at the current position
       * Initializes the parameter list
       * Applies the function to the element value
       * Finalizes the parameter list
       * Sets the value of the current element
       */

      if (get_list_value (**smp_list, cur_element, &listval) != EXIT_SUCCESS)

        return error_failure ("apply_list_function",
                              "error getting list value at position %ld\n",
                              cur_element);

      va_start (parlist, parc);

      if (cmp_fcn_exec (function, &retval, mode, parc, &parlist, 1, listval)
          != EXIT_SUCCESS)

        return error_failure ("apply_list_function",
                              "error in call to complex function\n");

      va_end (parlist);

      if (set_list_value (*smp_list, cur_element, retval) != EXIT_SUCCESS)

        return error_failure ("apply_list_function",
                              "error setting list value at position %ld\n",
                              cur_element);
    }

  return EXIT_SUCCESS;
}




/*
 * convert_complex_list_to_real
 *
 * Converts a complex valued list into a real valued one by discarding its
 * imaginary part
 *
 * Parameters:
 * - smp_list: complex valued input list
 */
int
convert_complex_list_to_real (sample_list_type * smp_list)
{
  smp_num_samples cur_sample;   /* Current sample counter */
  cmp_complex aux_z;            /* Auxiliary complex value */


  /* Checks the input list's entries data type */
  if (smp_list->data_type != SMP_COMPLEX)
    return error_failure ("convert_complex_list_to_real",
                          "input list must be a complex valued list\n");

  /* Changes the data type attribute of the input list */
  smp_list->data_type = SMP_REAL;

  /* Trivial case */
  if (smp_list->samples == 0)
    return EXIT_SUCCESS;

  /* Allocates memory space for the real list */
  if (resize_list (&smp_list, smp_list->samples) != EXIT_SUCCESS)
    return error_failure ("convert_complex_list_to_real",
                          "error allocating memory space for the real list\n");

  /* Copies the real part of the complex list to the real list */
  for (cur_sample = 1; cur_sample <= smp_list->samples; cur_sample++)
    {
      /*
       * Gets the complex list value
       * Sets the real list value
       */

      smp_list->data_type = SMP_COMPLEX;

      if (get_list_value (*smp_list, cur_sample, &aux_z) != EXIT_SUCCESS)
        return error_failure ("convert_complex_list_to_real",
                              "error getting complex value at position %ld\n",
                              cur_sample);

      smp_list->data_type = SMP_REAL;

      if (set_list_value (smp_list, cur_sample, aux_z) != EXIT_SUCCESS)
        return error_failure ("convert_complex_list_to_real",
                              "error setting real value at position %ld\n",
                              cur_sample);
    }

  /* Frees the complex list */
  free (smp_list->z);

  return EXIT_SUCCESS;
}
