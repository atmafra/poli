#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "nnet_types.h"
#include "nnet_sets.h"
#include "../common/types.h"
#include "../errorh/errorh.h"
#include "../vector/vector.h"
#include "../vector/vectorstat.h"
#include "../strutils/strutils.h"
#include "../incstat/incstat.h"

/******************************************************************************
 *                                                                            *
 *                             PRIVATE OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_tset_element_correct_indexes
 *
 * Corrects the element indexes after insertion or removal of an element
 * in other position than the last
 */
static void
nnet_tset_element_correct_indexes (TElement first_element,
                                   const ElementIndex first_element_index)
{
  TElement cur_element;         /* pointer to the current element */
  ElementIndex cur_index;       /* current element counter */


  cur_element = first_element;
  cur_index = first_element_index;

  while (cur_element != NULL)
    {
      cur_element->element_index = cur_index;
      cur_element = cur_element->next;
      ++cur_index;
    }

  return;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_tset_create
 *
 * Creates a new training set
 */
TSet
nnet_tset_create (const Name name,
                  const UnitIndex input_dimension,
                  const UnitIndex output_dimension)
{
  TSet new_set;                 /* new training set */
  int exit_status;              /* auxiliary function return status */


  /* Validates the number of inputs and outputs */
  if (input_dimension < 1)
    {
      fprintf (stderr, "nnet_tset_create: invalid number of inputs: %ld\n",
               input_dimension);
      return NULL;
    }

  if (output_dimension < 0)
    {
      fprintf (stderr, "nnet_tset_create: invalid number of outputs: %ld\n",
               output_dimension);
      return NULL;
    }

  /* Allocates the new training set */
  new_set = (TSet) malloc (sizeof (nnet_training_set_type));

  if (new_set == NULL)
    {
      fprintf (stderr, "nnet_tset_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Initializes the new set */
  if (name != NULL)
    strcpy (new_set->name, name);
  new_set->nu_elements = 0;
  new_set->input_dimension = input_dimension;
  new_set->output_dimension = output_dimension;
  new_set->first_element = NULL;
  new_set->last_element = NULL;

  /* Creates the statistics vectors */
  if (input_dimension > 0)
    {
      new_set->input_vector_stats = vcst_stats_create (input_dimension);

      if (new_set->input_vector_stats == NULL)
        {
          fprintf (stderr,
                   "nnet_tset_create: error creating input statistic vectors set\n");
          free (new_set);
          return NULL;
        }
    }
  else
    new_set->input_vector_stats = NULL;

  if (output_dimension > 0)
    {
      new_set->output_vector_stats = vcst_stats_create (output_dimension);

      if (new_set->output_vector_stats == NULL)
        {
          fprintf (stderr,
                   "nnet_tset_create: error creating output statistic vectors set\n");

          exit_status = vcst_stats_destroy (&(new_set->input_vector_stats));

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_create: error destroying input statistic vectors set\n");
              return NULL;
            }

          free (new_set);
          return NULL;
        }
    }
  else
    new_set->output_vector_stats = NULL;

  return new_set;
}



/*
 * nnet_tset_destroy
 *
 * Destroys a previously created training set
 */
int
nnet_tset_destroy (TSet * set, const BoolValue cascade_elements)
{
  TSet aux_set = *set;          /* auxiliary pointer to the actual set */
  TElement cur_element = NULL;  /* auxiliary current element */
  TElement next_element = NULL; /* auxiliary next element */
  Vector aux_vector = NULL;     /* auxiliary vector */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the training set was actually passed */
  if (set == NULL || aux_set == NULL)
    {
      fprintf (stderr, "nnet_tset_destroy: no training set to destroy\n");
      return EXIT_FAILURE;
    }

  /* Deletes the elements of the set */
  if (aux_set->nu_elements > 0)
    {
      if (cascade_elements == TRUE)
        {
          cur_element = aux_set->first_element;

          while (cur_element != NULL)
            {
              /* Destroys input and output vectors */
              if (cur_element->input != NULL)
                {
                  aux_vector = cur_element->input;

                  exit_status = vector_destroy (&aux_vector);
                  if (exit_status != EXIT_SUCCESS)
                    {
                      fprintf (stderr,
                               "nnet_tset_element_destroy: error destroying input vector\n");
                      return EXIT_FAILURE;
                    }
                }

              if (cur_element->output != NULL)
                {
                  aux_vector = cur_element->output;

                  exit_status = vector_destroy (&aux_vector);
                  if (exit_status != EXIT_SUCCESS)
                    {
                      fprintf (stderr,
                               "nnet_tset_element_destroy: error destroying output vector\n");
                      return EXIT_FAILURE;
                    }
                }

              /* If the element is attached to a set, dettaches it */
              aux_set->nu_elements--;

              /* Saves next element */
              next_element = cur_element->next;

              /* Destroys the current element */
              free (cur_element);

              /* On to the next element */
              cur_element = next_element;
            }
        }
      else
        {
          fprintf (stderr,
                   "nnet_tset_destroy: set still has %ld elements\n",
                   aux_set->nu_elements);
          return EXIT_FAILURE;
        }
    }

  /* Destroys the statistic vectors sets */
  if (aux_set->input_vector_stats != NULL)
    {
      exit_status = vcst_stats_destroy (&(aux_set->input_vector_stats));
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_destroy: error destroying input statistic vectors set\n");
          return EXIT_FAILURE;
        }
    }

  if (aux_set->output_vector_stats != NULL)
    {
      exit_status = vcst_stats_destroy (&(aux_set->output_vector_stats));
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_destroy: error destroying output statistic vectors set\n");
          return EXIT_FAILURE;
        }
    }

  /* Releases the memory of the training set */
  free (*set);
  *set = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_merge
 *
 * Transfers all elements from origin set to destination set.
 * Both sets must have been previously created.
 * Origin set will be empty after merging.
 */
int
nnet_tset_merge (TSet orig_set, TSet dest_set)
{
  TElement cur_element = NULL;  /* current origin element being transferred */
  TElement next_element = NULL; /* element next to current */


  /* Checks if both sets were passed */
  if (error_if_null (orig_set, "nnet_tset_merge", "no origin set\n"))
    return EXIT_FAILURE;

  if (error_if_null (orig_set, "nnet_tset_merge", "no destination set\n"))
    return EXIT_FAILURE;

  /* Trivial case */
  if (orig_set->nu_elements == 0)
    return EXIT_SUCCESS;

  /* Transfers all elements from origin set to destination set */
  cur_element = orig_set->first_element;

  while (cur_element != NULL)
    {
      next_element = cur_element->next;

      if (error_if_failure
          (nnet_tset_element_transfer_to_set
           (cur_element, dest_set, (ElementIndex *) NULL, FALSE),
           "nnet_tset_merge",
           "error transferring element %ld\n", cur_element->element_index))
        return EXIT_FAILURE;

      cur_element = next_element;
    }

  /* Corrects the element indexes on the destination set */
  nnet_tset_element_correct_indexes (dest_set->first_element, 1);

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_divide
 *
 * Creates a new training set by the division of an original training set,
 * transferring 'nu_elements' to the new set.
 * The elements can be picked from the beggining of the original set, from
 * the end of the original set or at random.
 */
int
nnet_tset_divide (TSet orig_set,
                  TSet * dest_set,
                  const Name new_set_name,
                  const ElementIndex nu_elements,
                  const TSetDivisionCriterion division_criterion)
{
  TSet new_set;                 /* new training set */
  TElement cur_element;         /* element being currently transferred */
  TElement next_element;        /* element next of current */
  ElementIndex cur_index;       /* current element index */
  ElementIndex first_element;   /* index of the first element to transfer */
  ElementIndex elements_done;   /* transferred elements counter */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the parameters were actually passed */
  if (orig_set == NULL)
    {
      fprintf (stderr, "nnet_tset_divide: no origin set\n");
      return EXIT_FAILURE;
    }

  /* Checks the number of elements to transfer */
  if (nu_elements < 0 || nu_elements > orig_set->nu_elements)
    {
      fprintf (stderr,
               "nnet_tset_divide: invalid number of elements to transfer: %ld\n",
               nu_elements);
      return EXIT_FAILURE;
    }

  /* Creates the new training set */
  new_set = nnet_tset_create
    (new_set_name, orig_set->input_dimension, orig_set->output_dimension);

  if (new_set == NULL)
    {
      fprintf (stderr, "nnet_tset_divide: error creating new set\n");
      return EXIT_FAILURE;
    }

  /* Trivial case: no elements to transfer */
  if (nu_elements == 0)
    return EXIT_SUCCESS;

  /* Defines the first element to transfer: beggining and end cases */
  if (division_criterion == PICK_FROM_BEGGINING)
    first_element = 1;
  else if (division_criterion == PICK_FROM_END)
    first_element = orig_set->nu_elements - nu_elements;
  else
    first_element = 0;

  /* Goes to the first element: beggining and end cases */
  if (division_criterion == PICK_FROM_BEGGINING ||
      division_criterion == PICK_FROM_END)
    {
      cur_element = nnet_tset_goto_element (orig_set, first_element);

      if (cur_element == NULL)
        {
          fprintf (stderr,
                   "nnet_tset_divide: error moving to first element to transfer\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      /* Picks an index at random */
      cur_index = istt_uniform_random (1, orig_set->nu_elements);

      /* Goes to the element at this index */
      cur_element = nnet_tset_goto_element (orig_set, cur_index);
      if (cur_element == NULL)
        {
          fprintf (stderr,
                   "nnet_tset_divide: error moving to element %ld\n",
                   cur_index);
          return EXIT_FAILURE;
        }
    }

  /* Transfers the elements according to the division criterion */
  for (elements_done = 0; elements_done < nu_elements; elements_done++)
    {
      /* Stores the next element */
      next_element = cur_element->next;

      /* Transfers the current element */
      exit_status =
        nnet_tset_element_transfer_to_set (cur_element, new_set, NULL, FALSE);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_divide: error transferring element %ld/%ld to the new set\n",
                   elements_done + 1, nu_elements);
          return EXIT_FAILURE;
        }

      if (orig_set->nu_elements > 0)
        {
          switch (division_criterion)
            {
            case PICK_FROM_BEGGINING:
            case PICK_FROM_END:

              /* Goes to the next element */
              cur_element = next_element;
              break;


            case PICK_AT_RANDOM:

              /* Picks an index at random */
              cur_index = istt_uniform_random (1, orig_set->nu_elements);

              /* Goes to the element at this index */
              cur_element = nnet_tset_goto_element (orig_set, cur_index);

              if (cur_element == NULL)
                {
                  fprintf (stderr,
                           "nnet_tset_divide: error moving to element %ld\n",
                           cur_index);
                  return EXIT_FAILURE;
                }

              break;
            }
        }
    }

  /* Corrects the element indexes */
  nnet_tset_element_correct_indexes (orig_set->first_element, 1);
  nnet_tset_element_correct_indexes (new_set->first_element, 1);

  /* Defines the new set */
  *dest_set = new_set;

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_randomize
 *
 * Shuffles the elements in the given set
 */
int
nnet_tset_randomize (TSet set)
{
  TSet aux_set = NULL;          /* auxiliary training set */
  int exit_status;              /* auxiliary function return status */


  /* checks if the training set was actually passed */
  if (set == NULL)
    {
      fprintf (stderr, "nnet_tset_randomize: no training set passed\n");
      return EXIT_FAILURE;
    }

  /* creates the auxiliary training set */
  aux_set =
    nnet_tset_create (NULL, set->input_dimension, set->output_dimension);
  if (aux_set == NULL)
    {
      fprintf (stderr,
               "nnet_tset_randomize: error creating auxiliary training set\n");
      return EXIT_FAILURE;
    }

  /* randomly transfers all elements to the auxiliary set */
  exit_status =
    nnet_tset_divide (set, &aux_set, NULL, set->nu_elements, PICK_AT_RANDOM);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_tset_randomize: error transferring elements to auxiliary training set\n");
      return EXIT_FAILURE;
    }

  /* transfers the elements back into the original set */
  exit_status = nnet_tset_merge (aux_set, set);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_tset_randomize: error transferring elements back to the original set\n");
      return EXIT_FAILURE;
    }

  /* destroys the auxiliary training set */
  exit_status = nnet_tset_destroy (&aux_set, TRUE);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_tset_randomize: error destroying auxiliary training set\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_element_create
 *
 * Creates a new training element
 */
TElement
nnet_tset_element_create (TSet set, ElementIndex * index,
                          const Vector input,
                          const Vector output,
                          const BoolValue normalize_input,
                          const BoolValue normalize_output,
                          const BoolValue correct_indexes)
{
  TElement new_elmt;            /* new element that will be returned */
  Vector norm1, norm2;          /* auxiliary vectors for normalization */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the input was passed */
  if (input == NULL)
    {
      fprintf (stderr, "nnet_tset_element_create: no input vector passed\n");
      return NULL;
    }

  /* Allocates the new training element */
  new_elmt = (TElement) malloc (sizeof (nnet_training_element_type));

  if (new_elmt == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Initializes the new training element */
  new_elmt->set = NULL;
  new_elmt->element_index = 0;
  new_elmt->input = input;
  new_elmt->output = output;
  new_elmt->next = NULL;

  /* Normalization */
  if (normalize_input == TRUE || normalize_output == TRUE)
    {
      if (normalize_input == TRUE)
        norm1 = new_elmt->input;
      else
        norm1 = NULL;

      if (normalize_output == TRUE)
        norm2 = new_elmt->output;
      else
        norm2 = NULL;

      exit_status = vector_normalize (norm1, norm2);

      if (exit_status != EXIT_SUCCESS)
        {
          free (new_elmt);
          fprintf (stderr,
                   "nnet_tset_element_create: error normalizing element\n");
          return NULL;
        }
    }

  /* If a training set was passed, attach the element to it */
  if (set != NULL)
    {
      exit_status =
        nnet_tset_element_attach_to_set (new_elmt, set, index,
                                         correct_indexes);

      if (exit_status != EXIT_SUCCESS)
        {
          free (new_elmt);
          fprintf (stderr,
                   "nnet_tset_element_create: error attaching new training element to training set\n");
          return NULL;
        }
    }

  return new_elmt;
}



/*
 * nnet_tset_element_destroy
 *
 * Destroys a previously created traning element
 * Cascades input and output vectors
 */
int
nnet_tset_element_destroy (TElement * element,
                           const BoolValue correct_indexes)
{
  Vector aux_vector;            /* auxiliary vector */
  int exit_status;              /* auxiliary function return status */


  /* Checks if an element was actually passed */
  if (element == NULL || *element == NULL)
    {
      fprintf (stderr, "nnet_tset_element_destroy: no element to destroy\n");
      return EXIT_FAILURE;
    }

  /* Destroys input and output vectors */
  if ((*element)->input != NULL)
    {
      aux_vector = (*element)->input;
      exit_status = vector_destroy (&aux_vector);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_element_destroy: error destroying input vector\n");
          return EXIT_FAILURE;
        }
    }

  if ((*element)->output != NULL)
    {
      aux_vector = (*element)->output;
      exit_status = vector_destroy (&aux_vector);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_element_destroy: error destroying output vector\n");
          return EXIT_FAILURE;
        }
    }

  /* If the element is attached to a set, dettaches it */
  if ((*element)->set != NULL)
    {
      exit_status =
        nnet_tset_element_remove_from_set (*element, correct_indexes);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_element_destroy: error removing element from set\n");
          return EXIT_FAILURE;
        }
    }

  /* Destroys the element */
  free (*element);

  /* Makes it point to NULL */
  *element = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_element_attach_to_set
 *
 * Attaches a training element to a training set
 */
int
nnet_tset_element_attach_to_set (TElement element,
                                 TSet set, ElementIndex * index,
                                 BoolValue correct_indexes)
{
  ElementIndex new_index;       /* index to attach the element */
  ElementIndex cur_elmt;        /* element position counter */
  TElement aux_elmt;            /* pointer to auxiliary element */
  int exit_status;              /* auxiliary function return status */


  /* Checks if an element was actually passed */
  if (element == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_attach_to_set: no element to attach\n");
      return EXIT_FAILURE;
    }

  /* Checks if a training set was actually passed */
  if (set == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_attach_to_set: no destination set\n");
      return EXIT_FAILURE;
    }

  /* Checks if the element has an input vector */
  if (element->input == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_attach_to_set: element has no input vector\n");
      return EXIT_FAILURE;
    }

  /* Checks the compatibility between the element and the set dimensions */
  if (element->input->dimension != set->input_dimension)
    {
      fprintf (stderr,
               "nnet_tset_element_attach_to_set: incompatible dimension of inputs\n");
      return EXIT_FAILURE;
    }

  if (element->output != NULL)
    if (element->output->dimension != set->output_dimension)
      {
        fprintf (stderr,
                 "nnet_tset_element_attach_to_set: incompatible dimension of outputs\n");
        return EXIT_FAILURE;
      }

  /* If the index was passed NULL, attach the element to the end of the set */
  if (index == NULL)
    new_index = set->nu_elements + 1;
  else
    new_index = *index;

  /* Validates the new index: 1's logic */
  if (new_index < 1 || new_index > set->nu_elements + 1)
    {
      fprintf (stderr,
               "nnet_tset_element_attach_to_set: invalid position requested: %ld\n",
               new_index);
      return EXIT_FAILURE;
    }

  /* If the element was attached to another set, dettaches from it */
  if (element->set != NULL)
    {
      if (element->set != set || element->element_index != new_index)
        {
          /* Removes from the old set */
          exit_status =
            nnet_tset_element_remove_from_set (element, correct_indexes);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_element_attach_to_set: error removing from old set\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Attaching to the same set: nothing to do */
          return EXIT_SUCCESS;
        }
    }

  /* Attaches the element */
  if (new_index == 1)
    {
      /* Inserts at the head */
      element->next = set->first_element;
      set->first_element = element;
    }
  else
    {
      /* Goes to the element before the requested position */
      if (new_index == set->nu_elements + 1)
        aux_elmt = set->last_element;
      else
        {
          aux_elmt = set->first_element;

          for (cur_elmt = 1; cur_elmt < new_index - 1; cur_elmt++)
            aux_elmt = aux_elmt->next;
        }

      /* Inserts the new element */
      element->next = aux_elmt->next;
      aux_elmt->next = element;
    }

  /* Increments the number of elements in the set */
  ++set->nu_elements;

  /* Keeps track of the last element */
  if (new_index == set->nu_elements)
    set->last_element = element;

  /* If not inserted at the tail, corrects the other elements' indexes */
  if (new_index != set->nu_elements && correct_indexes == TRUE)
    nnet_tset_element_correct_indexes (element, new_index);

  /* Sets the element's set attributes */
  element->set = set;
  element->element_index = new_index;

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_element_remove_from_set
 *
 * Removes a training element from its training set
 */
int
nnet_tset_element_remove_from_set (TElement element,
                                   const BoolValue correct_indexes)
{
  TSet set;                     /* auxiliary set */
  TElement aux_elmt;            /* auxiliary pointer to element */


  /* Check if an element was passed */
  if (element == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_remove_from_set: no element to remove\n");
      return EXIT_FAILURE;
    }

  /* Verifies if the element is attached to a set */
  if (element->set == NULL)
    {
      /* The element is not attached to a set: nothing to do */
      return EXIT_SUCCESS;
    }
  else
    set = element->set;

  /* Goes to the element before */
  if (element == set->first_element)
    {
      set->first_element = element->next;
      aux_elmt = NULL;
    }
  else
    {
      aux_elmt = set->first_element;

      while (aux_elmt->next != element && aux_elmt != NULL)
        aux_elmt = aux_elmt->next;

      /* Checks if the element is actually in the set */
      if (aux_elmt == NULL)
        {
          fprintf (stderr,
                   "nnet_tset_element_remove_from_set: element not found in set\n");
          return EXIT_FAILURE;
        }

      /* Removes the element */
      aux_elmt->next = element->next;
    }

  /* Keeps track of the last element in the set */
  if (element == set->last_element)
    set->last_element = aux_elmt;

  /* Corrects the indexes of the other elements */
  if (element->element_index != set->nu_elements && correct_indexes == TRUE)
    nnet_tset_element_correct_indexes (element->next, element->element_index);

  /* Decrements the number of elements in the set */
  set->nu_elements--;

  /* Unsets the element's set attributes */
  element->set = NULL;
  element->element_index = 0;
  element->next = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_element_transfer_to_set
 *
 * Transfers a training element from its current set to the given set
 */
int
nnet_tset_element_transfer_to_set (TElement element,
                                   TSet new_set, ElementIndex * index,
                                   const BoolValue correct_indexes)
{
  TSet old_set = NULL;          /* element's old set */
  ElementIndex old_index;       /* element's old set index */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the parameters were actually passed */
  if (element == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_transfer_to_set: no element passed\n");
      return EXIT_FAILURE;
    }

  if (new_set == NULL)
    {
      fprintf (stderr,
               "nnet_tset_element_transfer_to_set: no destination set\n");
      return EXIT_FAILURE;
    }

  /* If the element is already attached to a set, removes from it */
  if (element->set != NULL)
    {
      /* Stores the element's old set and index */
      old_set = element->set;
      old_index = element->element_index;

      exit_status =
        nnet_tset_element_remove_from_set (element, correct_indexes);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_element_transfer_to_set: error dettaching element from its original set\n");
          return EXIT_FAILURE;
        }
    }

  /* Attaches the element to the new set */
  exit_status =
    nnet_tset_element_attach_to_set (element, new_set, index,
                                     correct_indexes);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_tset_element_transfer_to_set: error attaching element to its destination set\n");

      /* Tries to undo the dettachment */
      if (old_set != NULL)
        {
          exit_status =
            nnet_tset_element_attach_to_set (element, old_set, &old_index,
                                             correct_indexes);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_element_transfer_to_set: error trying to re-attach element to its old set\n");
              return EXIT_FAILURE;
            }
        }

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_update_vector_stats
 *
 * Updates the input and output vector statistics
 */
int
nnet_tset_update_vector_stats (const TSet set)
{
  TElement cur_element;         /* current training element */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the training set was actually passed */
  if (set == NULL)
    {
      fprintf (stderr, "nnet_tset_update_vector_stats: no set passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the set has elements */
  if (set->nu_elements < 1)
    {
      fprintf (stderr, "nnet_tset_update_vector_stats: empty set\n");
      return EXIT_FAILURE;
    }

  /* Calculates the input statistics */
  if (set->input_dimension > 0)
    {
      vcst_init_stat (set->input_dimension);
      cur_element = set->first_element;

      while (cur_element != NULL)
        {
          /* Adds the input vector of the current element */
          exit_status = vcst_add_stat (cur_element->input);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_update_vector_stats: error adding input vector to statistics\n");
              vcst_init_stat (0);
              return EXIT_FAILURE;
            }

          /* Goes to the next element */
          cur_element = cur_element->next;
        }

      /* Gets the resulting statistics vector set */
      exit_status = vcst_stats_update (set->input_vector_stats);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_update_vector_stats: error getting input statistic vectors set\n");
          return EXIT_FAILURE;
        }
    }

  /* Calculates the output statistics */
  if (set->output_dimension > 0)
    {
      vcst_init_stat (set->output_dimension);
      cur_element = set->first_element;

      while (cur_element != NULL)
        {
          /* Adds the output vector of the current element */
          exit_status = vcst_add_stat (cur_element->output);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_update_vector_stats: error adding output vector to statistics\n");
              vcst_init_stat (0);
              return EXIT_FAILURE;
            }

          /* Goes to the next element */
          cur_element = cur_element->next;
        }

      /* Gets the resulting statistics vector set */
      exit_status = vcst_stats_update (set->output_vector_stats);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_update_vector_stats: error getting output statistic vectors set\n");
          return EXIT_FAILURE;
        }
    }

  /* Cleans up the statistics */
  vcst_init_stat (0);

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_regularize
 *
 * Subtracts the averages and regularizes the training set input and output
 * vectors according to the inverse standard deviations vectors
 */
int
nnet_tset_regularize (TSet set,
                      const BoolValue regularize_inputs,
                      const BoolValue regularize_outputs)
{
  TElement cur_element = NULL;  /* current training set element */
  Vector input_avg_vector = NULL;       /* input average vector */
  Vector output_avg_vector = NULL;      /* output average vector */
  Vector input_pond_vector = NULL;      /* input ponderation vector */
  Vector output_pond_vector = NULL;     /* output ponderation vector */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the training set was actually passed */
  if (set == NULL)
    {
      fprintf (stderr, "nnet_tset_regularize: no training set passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the set is not empty */
  if (set->nu_elements == 0)
    {
      fprintf (stderr, "nnet_tset_regularize: empty set passed\n");
      return EXIT_FAILURE;
    }

  /* Trivial case: nothing to do */
  if (regularize_inputs == FALSE && regularize_outputs == FALSE)
    return EXIT_SUCCESS;

  /* Initialization of the ponderation vectors */
  if (set->input_vector_stats != NULL)
    {
      input_avg_vector = set->input_vector_stats->average;
      input_pond_vector = set->input_vector_stats->invstddev;
    }

  if (set->output_vector_stats != NULL)
    {
      output_avg_vector = set->output_vector_stats->average;
      output_pond_vector = set->output_vector_stats->invstddev;
    }

  /* Checks if the statistic vectors are valid */
  if (regularize_inputs == TRUE && set->input_dimension > 0 &&
      (input_avg_vector == NULL || input_pond_vector == NULL))
    {
      fprintf (stderr,
               "nnet_tset_regularize: input statistics are out of date\n");
      return EXIT_FAILURE;
    }

  if (regularize_outputs == TRUE && set->output_dimension > 0 &&
      (output_avg_vector == NULL || output_pond_vector == NULL))
    {
      fprintf (stderr,
               "nnet_tset_regularize: output statistics are out of date\n");
      return EXIT_FAILURE;
    }

  /* Regularizes the training set */
  cur_element = set->first_element;

  while (cur_element != NULL)
    {
      /* Input */
      if (regularize_inputs == TRUE && set->input_dimension > 0)
        {
          /* Subtracts the average */
          exit_status = vector_subtract
            (cur_element->input, input_avg_vector, cur_element->input);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_regularize: error subtracting input average vector\n");
              return EXIT_FAILURE;
            }

          /* Multiplies by the inverse standard deviation */
          exit_status = vector_multiply
            (cur_element->input, input_pond_vector, cur_element->input);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_regularize: error ponderating input element\n");
              return EXIT_FAILURE;
            }
        }

      /* Output */
      if (regularize_outputs == TRUE && set->output_dimension > 0)
        {
          /* Subtracts the average */
          exit_status = vector_subtract
            (cur_element->output, output_avg_vector, cur_element->output);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_regularize: error subtracting output average vector\n");
              return EXIT_FAILURE;
            }

          /* Multiplies by the inverse standard deviation */
          exit_status = vector_multiply
            (cur_element->output, output_pond_vector, cur_element->output);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_regularize: error ponderating output element\n");
              return EXIT_FAILURE;
            }
        }

      /* Goes to the next element */
      cur_element = cur_element->next;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_read_from_file
 *
 * Reads a training set from a file
 */
int
nnet_tset_read_from_file (TSet set,
                          const char *file_name,
                          const BoolValue normalize_input,
                          const BoolValue normalize_output,
                          const BoolValue update_vector_stats,
                          const BoolValue regularize_inputs,
                          const BoolValue regularize_outputs)
{
  FILE *fp;                     /* pointer to file descriptor */
  char buf[NNET_BUF_SIZE];      /* read buffer */
  char *word;                   /* auxiliar character */
  const char delim[] = " ";     /* number delim in a line */

  TElement new_element = NULL;  /* new training element */
  Vector new_input = NULL;      /* new training element's input vector */
  Vector new_output = NULL;     /* new training element's output vector */
  Vector old_input = NULL;      /* old training element's input vector */
  Vector old_output = NULL;     /* old training element's output vector */
  Vector aux_vector = NULL;     /* auxiliary vector */

  ElementIndex new_index;       /* new element index */
  ElementIndex old_index;       /* old element index */
  UnitIndex new_pos;            /* new value position */
  RValue new_value;             /* new value */
  UnitIndex dim_cnt;            /* vector dimension counter */

  int exit_status;              /* auxiliary function return status */


  /* Checks if the training set was actually passed */
  if (set == NULL)
    {
      fprintf (stderr, "nnet_tset_read_from_file: no training set passed \n");
      return EXIT_FAILURE;
    }

  /* Checks if a file name was passed */
  if (file_name == NULL)
    {
      fprintf (stderr, "nnet_tset_read_from_file: no file name\n");
      return EXIT_FAILURE;
    }

  /* Checks flag consistency */
  if (update_vector_stats == FALSE &&
      (regularize_inputs == TRUE || regularize_outputs == TRUE))
    {
      fprintf (stderr,
               "nnet_tset_read_from_file: unable to regularize set without updating vector statistics\n");
      return EXIT_FAILURE;
    }

  /* Opens the file for reading */
  errno = 0;
  fp = fopen (file_name, "r");

  if (fp == NULL)
    {
      fprintf (stderr, "nnet_tset_read_from_file: %s\n", strerror (errno));
      return EXIT_FAILURE;
    }

  /* Initializes the auxiliary element indexes */
  new_index = 1;
  old_index = 1;
  new_pos = 1;
  new_value = 0.0;
  dim_cnt = 0;

  /* Reads the file */
  while (!feof (fp))
    {
      /* Reads one line */
      exit_status = read_valid_file_line (fp, NNET_BUF_SIZE, '#', 1, buf);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "nnet_tset_read_from_file: error reading file\n");
          return EXIT_FAILURE;
        }

      if (!feof (fp))
        {
          /* The first word is the element index */
          if ((word = (char *) strtok (buf, delim)) != NULL)
            {
              new_index = ((ElementIndex) strtod (word, NULL)) + 1;
            }
          else
            {
              fprintf (stderr, "nnet_tset_read_from_file: '%s'\n", buf);
              fprintf (stderr,
                       "nnet_tset_read_from_file: error reading element index\n");
              return EXIT_FAILURE;
            }

          /* The second word is the vector component */
          if ((word = (char *) strtok (NULL, delim)) != NULL)
            {
              new_pos = ((UnitIndex) strtod (word, NULL)) + 1;
            }
          else
            {
              fprintf (stderr, "nnet_tset_read_from_file: '%s'\n", buf);
              fprintf (stderr,
                       "nnet_tset_read_from_file: error reading vector component\n");
              return EXIT_FAILURE;
            }

          /* The third word is the value */
          if ((word = (char *) strtok (NULL, delim)) != NULL)
            {
              new_value = (RValue) strtod (word, NULL);
            }
          else
            {
              fprintf (stderr, "nnet_tset_read_from_file: '%s'\n", buf);
              fprintf (stderr,
                       "nnet_tset_read_from_file: error reading component value\n");
              return EXIT_FAILURE;
            }

          /* Increments the dimension counter */
          ++dim_cnt;

          /* Creates or not a new vector */
          if (dim_cnt == 1 && !feof (fp))
            {
              /* Switches between input and output */
              if (aux_vector == new_output || set->output_dimension == 0)
                {
                  /* Creates a new input element */
                  new_input = vector_create (set->input_dimension);
                  if (new_input == NULL)
                    {
                      fclose (fp);
                      fprintf (stderr,
                               "nnet_tset_read_from_file: error creating input vector\n");
                      return EXIT_FAILURE;
                    }

                  /* Switches to input */
                  aux_vector = new_input;

                  if (old_input == NULL)
                    old_input = new_input;
                }
              else
                {
                  if (set->output_dimension > 0)
                    {
                      /* Creates a new output element */
                      new_output = vector_create (set->output_dimension);
                      if (new_output == NULL)
                        {
                          fclose (fp);
                          fprintf (stderr,
                                   "nnet_tset_read_from_file: error creating output vector\n");
                          return EXIT_FAILURE;
                        }
                      aux_vector = new_output;

                      if (old_output == NULL)
                        old_output = new_output;
                    }
                }
            }

          /* Creation of a new element */
          if (new_index != old_index || feof (fp))
            {
              /* Creates a new element with the old input and output  */
              new_element = nnet_tset_element_create
                (set, NULL, old_input, old_output,
                 normalize_input, normalize_output, FALSE);

              if (new_element == NULL)
                {
                  fclose (fp);
                  fprintf (stderr,
                           "nnet_tset_read_from_file: error creating new training element\n");
                  return EXIT_FAILURE;
                }

              /* Refreshes the old index */
              old_index = new_index;
              old_input = new_input;
              old_output = new_output;
            }

          /* Sets the value of the dimension */
          exit_status = vector_set_value (aux_vector, new_pos, new_value);

          if (exit_status != EXIT_SUCCESS)
            {
              fclose (fp);
              fprintf (stderr,
                       "nnet_tset_read_from_file: error setting value\n");
              return EXIT_FAILURE;
            }

          /* Resets the dimension counter */
          if (dim_cnt == aux_vector->dimension)
            dim_cnt = 0;
        }
    }

  /* Closes the file */
  errno = 0;
  if (fclose (fp) == EOF)
    {
      fprintf (stderr, "nnet_tset_read_from_file: %s\n", strerror (errno));
      return EXIT_FAILURE;
    }

  /* Optionally update vector statistics */
  if (update_vector_stats == TRUE)
    {
      exit_status = nnet_tset_update_vector_stats (set);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_read_from_file: error updating vector statistics\n");
          return EXIT_FAILURE;
        }

      /* Optionally regularizes the set elements */
      if (regularize_inputs == TRUE || regularize_outputs == TRUE)
        {
          exit_status =
            nnet_tset_regularize (set, regularize_inputs, regularize_outputs);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_tset_read_from_file: error ponderating training set\n");
              return EXIT_FAILURE;
            }
        }

      /* Updates the vector statistics again */
      exit_status = nnet_tset_update_vector_stats (set);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_tset_read_from_file: error updating vector statistics after ponderation\n");
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_tset_create_from_file
 *
 * Creates a new training set and reads its contents from a file
 */
TSet
nnet_tset_create_from_file (const Name name,
                            const UnitIndex input_dimension,
                            const UnitIndex output_dimension,
                            const BoolValue normalize_input,
                            const BoolValue normalize_output,
                            const BoolValue update_vector_stats,
                            const BoolValue regularize_inputs,
                            const BoolValue regularize_outputs,
                            const char *file_name)
{
  TSet new_set;                 /* new training set */
  int exit_status;              /* auxiliary function return status */

  /* Creates the new training set */
  new_set = nnet_tset_create (name, input_dimension, output_dimension);

  if (new_set == NULL)
    {
      fprintf (stderr,
               "nnet_tset_create_from_file: error creating new training set\n");
      return NULL;
    }

  /* Reads its contents from the given file */
  exit_status = nnet_tset_read_from_file (new_set,
                                          file_name,
                                          normalize_input,
                                          normalize_output,
                                          update_vector_stats,
                                          regularize_inputs,
                                          regularize_outputs);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_tset_create_from_file: error reading training set file\n");

      exit_status = nnet_tset_destroy (&new_set, TRUE);

      if (exit_status != EXIT_SUCCESS)
        fprintf (stderr,
                 "nnet_tset_create_from_file: error destroying set\n");

      return NULL;
    }

  return new_set;
}



/*
 * nnet_tset_create_from_list
 *
 * Creates a new training set and reads its contents from a file
 * containint a list of files
 */
TSet
nnet_tset_create_from_list (const Name name,
                            const UnitIndex input_dimension,
                            const UnitIndex output_dimension,
                            const BoolValue normalize_input,
                            const BoolValue normalize_output,
                            const BoolValue update_vector_stats,
                            const BoolValue regularize_inputs,
                            const BoolValue regularize_outputs,
                            const char *list_file_name)
{
  TSet t_set = NULL;            /* new training set */
  TSet aux_set = NULL;          /* auxiliary input training set */
  FILE *inlist_fd;              /* input list file descriptor */
  FileName buf = "";            /* input buffer */
  char *set_file = NULL;        /* current set file name */


  /* checks if the list file name was actually passed */
  if (list_file_name == NULL)
    {
      fprintf (stderr,
               "nnet_tset_create_from_list: no input list file name passed\n");
      return NULL;
    }

  /* creates the new training set */
  if (error_if_null
      (t_set = nnet_tset_create (name, input_dimension, output_dimension),
       "nnet_tset_create_from_list", "error creating training set\n"))
    return NULL;

  /* creates an auxiliary training set */
  if (error_if_null
      (aux_set = nnet_tset_create (NULL, input_dimension, output_dimension),
       "nnet_tset_create_from_list",
       "error creating temporary training set\n"))
    return NULL;

  /* processes all files in list */
  if (error_if_null (inlist_fd = fopen (list_file_name, "r"),
                     "nnet_tset_create_from_list", strerror (errno)))
    {
      puts ("");
      return NULL;
    }


  while (!feof (inlist_fd))
    {
      /* Reads the input file name */
      if (error_if_failure (read_valid_file_line
                            (inlist_fd, FILE_NAME_SIZE, IGNORE_TOKEN, 1, buf),
                            "nnet_tset_create_from_list",
                            "error reading input file name from list '%s'\n",
                            list_file_name))
        return NULL;

      /* creates the auxiliary training set with the current file */
      set_file = buf;

      if (!feof (inlist_fd))
        {
          if (error_if_failure (nnet_tset_read_from_file
                                (aux_set, set_file, normalize_input,
                                 normalize_output, update_vector_stats,
                                 regularize_inputs, regularize_outputs),
                                "nnet_tset_create_from_list",
                                "error reading training set from file '%s'\n",
                                set_file))
            return NULL;

          /* merges the auxiliary set with the real training set */
          if (error_if_failure (nnet_tset_merge (aux_set, t_set),
                                "nnet_tset_create_from_list",
                                "error merging training set with file '%s'\n",
                                set_file))
            return NULL;
        }
    }

  fclose (inlist_fd);

  /* destroys auxiliary training set */
  if (error_if_failure (nnet_tset_destroy (&aux_set, TRUE),
                        "nnet_tset_create_from_list",
                        "error destroying auxiliary training set\n"))
    return NULL;

  return t_set;
}



/*
 * nnet_tset_goto_element
 *
 * Returns the element at the given index of the set
 */
TElement
nnet_tset_goto_element (const TSet set, const ElementIndex index)
{
  TElement cur_element;         /* auxiliary training element */
  ElementIndex cur_index;       /* auxiliary element index */

  /* Check if the set was passed */
  if (set == NULL)
    {
      fprintf (stderr, "nnet_tset_goto_element: no set passed\n");
      return NULL;
    }

  /* Checks the index */
  if (index < 1 || index > set->nu_elements)
    {
      fprintf (stderr,
               "nnet_tset_goto_element: invalid index: %ld (%ld elements)\n",
               index, set->nu_elements);
      return NULL;
    }

  /* Trivial cases */
  if (index == 1)
    return set->first_element;

  if (index == set->nu_elements)
    return set->last_element;

  /* Searches the set */
  cur_element = set->first_element;

  for (cur_index = 1; cur_index <= index; cur_index++)
    cur_element = cur_element->next;

  return cur_element;
}



/*
 * nnet_tset_element_info
 *
 * Outputs element information
 */
void
nnet_tset_element_info (const TElement element,
                        const BoolValue include_input_vector,
                        const BoolValue include_output_vector,
                        FILE * output_fd)
{
  if (element == NULL || output_fd == NULL)
    return;

  fprintf (output_fd, "Element   : %ld\n", element->element_index);

  if (include_input_vector == TRUE)
    {
      fprintf (output_fd, "Input     :");
      vector_info (element->input, output_fd);
    }

  if (include_output_vector == TRUE && element->output != NULL)
    {
      fprintf (output_fd, "Output    :");
      vector_info (element->output, output_fd);
    }

  return;
}



/*
 * nnet_tset_info
 *
 * Outputs training set information
 */
void
nnet_tset_info (const TSet set,
                const BoolValue include_elements,
                const BoolValue include_input_vector,
                const BoolValue include_output_vector,
                const BoolValue include_input_vector_stats,
                const BoolValue include_output_vector_stats, FILE * output_fd)
{
  TElement cur_elmt;            /* pointer to the current training element */


  if (set == NULL || output_fd == NULL)
    return;

  /* Training Set information */
  fprintf (output_fd, "Set       : %s\n", set->name);
  fprintf (output_fd, "Elements  : %ld\n", set->nu_elements);
  fprintf (output_fd, "In Dim    : %ld\n", set->input_dimension);
  fprintf (output_fd, "Out Dim   : %ld\n", set->output_dimension);

  /* Vector Statistics */
  if (set->nu_elements > 0)
    {
      if (include_input_vector_stats == TRUE && set->input_dimension > 0)
        {
          fprintf (output_fd, "\nInput Vector Statistics:\n");
          vcst_stats_info (set->input_vector_stats, output_fd);
        }

      if (include_output_vector_stats == TRUE && set->output_dimension > 0)
        {
          fprintf (output_fd, "\nOutput Vector Statistics:\n");
          vcst_stats_info (set->output_vector_stats, output_fd);
        }
    }

  /* Elements information */
  if (include_elements == TRUE && set->nu_elements > 0)
    {
      cur_elmt = set->first_element;

      while (cur_elmt != NULL)
        {
          fprintf (output_fd, "\n");

          nnet_tset_element_info
            (cur_elmt, include_input_vector, include_output_vector,
             output_fd);

          cur_elmt = cur_elmt->next;
        }
    }

  return;
}
