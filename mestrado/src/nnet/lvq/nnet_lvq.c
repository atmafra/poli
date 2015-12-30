#include <stdio.h>
#include <stdlib.h>
#include "nnet_lvq.h"
#include "nnet_lvq_window.h"
#include "../../errorh/errorh.h"
#include "../nnet_train.h"
#include "../nnet_nnet.h"
#include "../nnet_layers.h"
#include "../nnet_units.h"
#include "../nnet_actv.h"
#include "../nnet_conns.h"
#include "../nnet_metrics.h"

/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_create
 *
 * Creates a new LVQ neural network architecture extension
 */
LvqNNetwork
nnet_lvq_create (const NNetwork nnet,
                 const LvqAlgorithmType lvq_algorithm,
                 const LRateFunctionClass lrate_class,
                 const VectorMetric activation_metric,
                 const LRateFunctionParameters lrate_parameters)
{
  LvqNNetwork new_lvq = NULL;   /* new LVQ neural network */
  LvqAttributes lvq_attr = NULL;        /* new LVQ attributes */


  /* Checks if the learning rate function class was passed */
  if (error_if_null
      (lrate_class, "nnet_lvq_create",
       "no learning rate function class passed\n"))
    return NULL;

  /* Creates the new LVQ attribute set */
  lvq_attr = (LvqAttributes) malloc (sizeof (nnet_lvq_attr_type));
  if (lvq_attr == NULL)
    {
      fprintf (stderr, "nnet_lvq_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Creates the LVQ learning rate function */
  lvq_attr->lrate_function = (LRateFunction)
    nnet_train_lrate_create (lrate_class, lrate_parameters);
  if (lvq_attr->lrate_function == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_create: error creating learning rate function\n");
      free (new_lvq);
      return NULL;
    }

  /* Sets the LVQ algorithm */
  switch (lvq_algorithm)
    {
    case LVQ_1:
    case LVQ_2_1:
    case LVQ_3:
    case OLVQ_1:
      lvq_attr->lvq_algorithm = lvq_algorithm;
      break;

    default:
      fprintf (stderr, "nnet_lvq_create: unknown LVQ algorithm\n");
      return NULL;
    }

  /* Sets the LVQ activation metric */
  switch (activation_metric)
    {
    case VECTOR_METR_EUCLIDEAN:
    case VECTOR_METR_INNER_PRODUCT:
      lvq_attr->activation_metric = activation_metric;
      break;

    default:
      fprintf (stderr, "nnet_lvq_create: unknown activation metric\n");
      return NULL;
    }

  /* Creates the LVQ extension */
  new_lvq = (LvqNNetwork) malloc (sizeof (nnet_extension_type));
  if (new_lvq == NULL)
    {
      fprintf (stderr, "nnet_lvq_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Attaches the extension */
  new_lvq->index = NNEXT_LVQ;
  new_lvq->attr = lvq_attr;

  /* Extends the passed network to LVQ */
  if (nnet != NULL)
    nnet_lvq_extend_nnetwork (nnet, new_lvq);

  return new_lvq;
}



/*
 * nnet_lvq_extend_nnetwork
 *
 * Associates the given LVQ extension to the given neural network
 */
int
nnet_lvq_extend_nnetwork (NNetwork nnet, LvqNNetwork lvq_nnet)
{
  /* Checks if the LVQ extension was passed */
  if (lvq_nnet == NULL)
    {
      fprintf (stderr, "nnet_lvq_extend_nnetwork: no LVQ extension passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the neural network was passed */
  if (nnet == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_extend_nnetwork: no neural network passed\n");
      return EXIT_FAILURE;
    }

  /* Associates the LVQ extension to the neural network */
  nnet->extension = lvq_nnet;
  nnet->extension->nnet = nnet;

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_create_nnetwork
 *
 * Creates a new generic neural network accorging to the LVQ architecture
 * and then attaches it to the given LVQ extension
 */
int
nnet_lvq_create_nnetwork (LvqNNetwork lvq_nnet,
                          const Name nnet_name,
                          const Name input_layer_name,
                          const Name output_layer_name,
                          const UnitIndex nu_inputs,
                          const UnitIndex nu_outputs,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters)
{
  NNetwork new_nnet = NULL;     /* new associated generic neural network */
  Layer input_layer = NULL;     /* LVQ input layer */
  Layer output_layer = NULL;    /* LVQ output layer */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the LVQ extension was actually passed */
  if (lvq_nnet == NULL)
    {
      fprintf (stderr, "nnet_lvq_create_nnetwork: no LVQ extension passed\n");
      return EXIT_FAILURE;
    }

  /* Creates the new generic neural network */
  new_nnet = nnet_nnetwork_create (nnet_name, lvq_nnet);

  if (new_nnet == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error creating new LVQ neural network\n");
      return EXIT_FAILURE;
    }

  /* Creates the input layer */
  input_layer = nnet_layer_create
    (new_nnet, NULL, &nnet_layer_class_input, input_layer_name);

  if (input_layer == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error creating input layer\n");
      return EXIT_FAILURE;
    }

  /* Creates the output layer */
  output_layer = nnet_layer_create
    (new_nnet, NULL, &nnet_layer_class_output, output_layer_name);

  if (output_layer == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error creating output layer\n");
      return EXIT_FAILURE;
    }

  /* Creates the input layer's units */
  exit_status =
    nnet_unit_create_multiple (nu_inputs, input_layer,
                               nnet_actv_class_passthrough,
                               NULL, FALSE, TRUE, NULL, NULL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error creating input units\n");
      return EXIT_FAILURE;
    }

  /* Creates the output layer's units */
  exit_status =
    nnet_unit_create_multiple (nu_outputs, output_layer,
                               nnet_actv_class_linear,
                               NULL, FALSE, TRUE, NULL, NULL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error creating output units\n");
      return EXIT_FAILURE;
    }

  /* Fully connects the input layer to the output layer */
  exit_status =
    nnet_conn_connect_layers (input_layer, output_layer, 0.0,
                              weight_init_class, weight_init_parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error connecting input layer to output layer\n");
      return EXIT_FAILURE;
    }

  /* Attaches the new neural network to the given LVQ extension */
  exit_status = nnet_lvq_extend_nnetwork (new_nnet, lvq_nnet);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_create_nnetwork: error attaching generic neural network\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_destroy
 *
 * Destroys a previously created LVQ neural network
 */
int
nnet_lvq_destroy (LvqNNetwork * lvq_nnet,
                  const BoolValue cascade_nnetwork,
                  const BoolValue cascade_layers,
                  const BoolValue cascade_units,
                  const BoolValue cascade_connections)
{
  LvqAttributes lvq_attr;       /* LVQ attributes */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the lvq was actually passed */
  if (lvq_nnet == NULL || *lvq_nnet == NULL)
    {
      fprintf (stderr,
               "nnet_lvq_destroy: no LVQ neural network to destroy\n");
      return EXIT_FAILURE;
    }

  /* Destroys the associated generic neural network */
  if ((*lvq_nnet)->nnet != NULL && cascade_nnetwork == TRUE)
    {
      exit_status =
        nnet_nnetwork_destroy (&((*lvq_nnet)->nnet), TRUE, cascade_layers,
                               cascade_units, cascade_connections);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_lvq_destroy: error destroying associated generic neural network\n");
          return EXIT_FAILURE;
        }
    }

  /* Auxiliary pointer to the lvq attributes */
  lvq_attr = (LvqAttributes) (*lvq_nnet)->attr;

  /* Destroys the learning rate function */
  if (lvq_attr->lrate_function != NULL)
    function_destroy (&(lvq_attr->lrate_function));

  /* Destroys the LVQ network itself */
  free (*lvq_nnet);

  /* Makes it point to NULL */
  *lvq_nnet = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_propagate_element
 *
 * Loads and propagates an element
 */
int
nnet_lvq_propagate_element (const LvqNNetwork lvq_nnet,
                            const TElement element,
                            Unit * winner1, Unit * winner2)
{
  Layer input_layer, output_layer;      /* auxiliary layer pointers */
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  VectorMetric metric;          /* vector metric for activation */
  int exit_status;              /* auxiliary funciton return status */


  /*************************************************************************
   *                             PRE-CONDITIONS                            *
   *************************************************************************/

  /* Checks if the network was actually passed */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_propagate_element",
                          "no neural network passed\n");

  /* Checks if the extension has its attributes set */
  if (lvq_nnet->attr == NULL)
    return error_failure ("nnet_lvq_propagate_element",
                          "LVQ extension doesn't have its attributes set\n");

  /* Checks if the element was actually passed */
  if (element == NULL)
    return error_failure ("nnet_lvq_propagate_element",
                          "no training element passed\n");


  /*************************************************************************
   *                             INITIALIZATION                            *
   *************************************************************************/

  /* Initializes the auxiliary pointers */
  input_layer = lvq_nnet->nnet->first_layer;
  output_layer = lvq_nnet->nnet->last_layer;
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  metric = lvq_attr->activation_metric;


  /*************************************************************************
   *                               PROPAGATION                             *
   *************************************************************************/

  /* Loads the element into the input layer */
  exit_status = nnet_layer_load_vector (element->input, input_layer);
  if (exit_status != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_propagate_element",
                          "error loading input vector\n");

  /* Activates the neural network */
  exit_status = nnet_nnetwork_activate (lvq_nnet->nnet, metric);
  if (exit_status != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_propagate_element",
                          "error activating neural network\n");


  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/

  /* Determines the winner: largest output according to the metrics */
  if (error_if_failure
      (nnet_metr_layer_2_winners (output_layer, metric, winner1, winner2),
       "nnet_lvq_propagate_element", "error executing LVQ competition\n"))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
Vector
nnet_lvq_propagate_set (const LvqNNetwork lvq_nnet, const TSet set)
{
  TElement cur_element = NULL;  /* current training element */
  ElementIndex cur_index = 1;   /* current element counter */
  Unit winner1 = NULL;          /* current winner unit */
  Unit winner2 = NULL;          /* current 2nd place unit */
  Vector winners = NULL;        /* list of winners indexes */


  /* checks if the LVQ was actually passed */
  if (lvq_nnet == NULL)
    {
      error_failure ("nnet_lvq_propagate_set",
                     "no LVQ neural network passed\n");
      return NULL;
    }

  /* checks if the training set was actually passed */
  if (set == NULL)
    {
      error_failure ("nnet_lvq_propagate_set", "no training set passed\n");
      return NULL;
    }

  /* creates the unit index list */
  if (error_if_null
      (winners = vector_create (set->nu_elements), "nnet_lvq_propagate_set",
       "error creating list of winner indexes\n"))
    return NULL;

  /* propagates all elements in the set */
  cur_element = set->first_element;

  while (cur_element != NULL)
    {
      /* propagates current element */
      if (error_if_failure
          (nnet_lvq_propagate_element
           (lvq_nnet, cur_element, &winner1, &winner2),
           "nnet_lvq_propagate_set", "error propagating element %ld\n",
           cur_element->element_index))
        return NULL;

      /* sets the winner index in the unit index list */
      if (error_if_failure
          (vector_set_value
           (winners, cur_index, (RValue) winner1->unit_index),
           "nnet_lvq_propagate_set",
           "error setting winner index for element %ld\n",
           cur_element->element_index))
        return NULL;

      /* goes to the next element */
      ++cur_index;
      cur_element = cur_element->next;
    }

  return winners;
}



/*
 * nnet_lvq_train_element
 *
 * Executes training for one element
 */
int
nnet_lvq_train_element (LvqNNetwork lvq_nnet,
                        const TElement element, const RValue learning_rate);
{
  Layer input_layer = NULL;     /* LVQ input layer */
  Layer output_layer = NULL;    /* LVQ output layer */
  Unit winner1 = NULL;          /* winning unit */
  Unit winner2 = NULL;          /* 2nd place unit */
  Unit cur_unit = NULL;         /* current output unit */
  UsLgIntValue cur_comp;        /* current neighborhood vector component */
  Vector weight_vector = NULL;  /* auxiliary weight vector */
  Vector error_vector = NULL;   /* current unit error vector */
  RValue rate;                  /* fraction of the error vector */
  int exit_status;              /* auxiliary function return status */


  /*************************************************************************
   *                             PRE-CONDITIONS                            *
   *************************************************************************/

  /* Checks if the network was actually passed */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "no neural network passed\n");

  /* Checks if the LVQ extension is attached to a neural network */
  if (lvq_nnet->nnet == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "LVQ extension not attached to neural network\n");

  /* Checks if the network has a plausible LVQ architecture */
  if (lvq_nnet->nnet->nu_layers != 2)
    return error_failure ("nnet_lvq_train_element",
                          "a LVQ must have two layers\n");


  /*************************************************************************
   *                             INITIALIZATION                            *
   *************************************************************************/

  /* Sets the auxiliary layer pointers */
  input_layer = lvq_nnet->nnet->first_layer;
  output_layer = lvq_nnet->nnet->last_layer;

  /* Creates the auxiliary weight vector */
  weight_vector = vector_create (input_layer->nu_units);

  if (weight_vector == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "error creating auxiliary weight vector\n");

  /* Creates the auxiliary error vector */
  error_vector = vector_create (input_layer->nu_units);
  if (error_vector == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "error creating auxiliary error vector\n");


  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/

  /* Loads the training element, activates the network and find the winner */
  exit_status =
    nnet_lvq_propagate_element (lvq_nnet, element, &winner1, &winner2);

  if (exit_status != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_train_element",
                          "error performing competition\n");


  /*************************************************************************
   *                                ADAPTION                               *
   *************************************************************************/

  /*
   * 1. atualizar os pesos das vencedoras
   * 2. switch do algoritmo LVQ
   * 2.1 atualizar os pesos da 2.a colocada
   * 2.2 atualizar a funcao de aprendizado da vencedora
   */

  /* Updates the weights of the output units */
  cur_unit = output_layer->first_unit;
  cur_comp = 1;

  while (cur_unit != NULL)
    {
      /* Gets the weight vector for the current output unit */
      exit_status = nnet_unit_get_weight_vector (cur_unit, weight_vector);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_lvq_train_element: error getting current unit weight vector\n");
          return EXIT_FAILURE;
        }

      /* Calculates the error vector */
      exit_status =
        vector_subtract (element->input, weight_vector, error_vector);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_lvq_train_element: error calculating error vector\n");
          return EXIT_FAILURE;
        }

      /* Gets the neighborhood value */
      exit_status = vector_get_value (ngb_vector, cur_comp, &ngb_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_lvq_train_element: error getting neighborhood value from vector at component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Calculates the correction rate */
      rate = ngb_value * learning_rate;

      /* Selects the units to update */
      if (rate > DBL_EPSILON)
        {
          /* Multiplies the error vector by the correction rate */
          exit_status =
            vector_scalar_multiply (error_vector, rate, error_vector);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_lvq_train_element: error calculating weight correction vector\n");
              return EXIT_FAILURE;
            }

          /* Calculates the new weights */
          exit_status =
            vector_sum (weight_vector, error_vector, weight_vector);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_lvq_train_element: error calculating new weight vector\n");
              return EXIT_FAILURE;
            }

          /* Updates the weights */
          exit_status = nnet_unit_set_weight_vector (cur_unit, weight_vector);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_lvq_train_element: error updating current unit's weights\n");
              return EXIT_FAILURE;
            }
        }

      /* Goes to the next output unit */
      cur_unit = cur_unit->next;
      ++cur_comp;
    }


  /*************************************************************************
   *                              FINALIZATION                             *
   *************************************************************************/

  /* Destroys the neighborhood vector */
  exit_status = vector_destroy (&ngb_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_train_element: error destroying neighborhood vector\n");
      return EXIT_FAILURE;
    }

  /* Destroys the auxiliary weight vector */
  exit_status = vector_destroy (&weight_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_train_element: error destroying auxiliary weight vector\n");
      return EXIT_FAILURE;
    }

  /* Destroys the error vector */
  exit_status = vector_destroy (&error_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_lvq_train_element: error destroying error vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_train_set
 *
 * Executes one training pass through all the elements in the given
 * training set
 */
int
nnet_lvq_train_set (LvqNNetwork lvq_nnet,
                    const TSet training_set,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character);
{
  static DTime t = 0;           /* current training time */
  TElement element = NULL;      /* current element */
  LvqAttributes lvqatt = NULL;  /* LVQ attributes */
  LRateFunction lfunc = NULL;   /* learning rate function */
  NgbFunction nfunc = NULL;     /* neighborhood function */
  RValue etha = 0.0;            /* current learning rate */


  /* Checks if the network was actually passed */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_train_set",
                          "no LVQ neural network passed\n");

  /* Checks if the training set was actually passed */
  if (training_set == NULL)
    return error_failure ("nnet_lvq_train_set", "no training set passed\n");

  /* Checks if internal time should be reset */
  if (reset_time == TRUE)
    t = first_epoch;

  /* Checks if the maximum epochs has been reached */
  if (t >= max_epochs)
    return error_failure ("nnet_lvq_train_set",
                          "maximum epochs (%ld) reached\n", max_epochs);

  /* Initialization */
  lvqatt = (LvqAttributes) lvq_nnet->attr;
  lfunc = lvqatt->lrate_function;
  nfunc = lvqatt->ngb_function;

  /* Calculates the current learning rate */
  etha = nnet_train_lrate_value (lfunc, (RValue) t);

  /* Updates progress bar */
  if (output_progress == TRUE)
    {
      if (error_if_failure
          (display_progress
           (first_epoch + 1, (long) max_epochs, (long) t + 1, progress_width,
            progress_character, 0), "nnet_lvq_train_set",
           "error updating progress bar to %ld\n", t))
        return EXIT_FAILURE;
    }

  /* Training elements loop */
  element = training_set->first_element;
  while (element != NULL)
    {
      /* Trains the current element */
      if (error_if_failure (nnet_lvq_train_element (lvq_nnet, element, etha),
                            "nnet_lvq_train_set",
                            "error training element\n") == EXIT_FAILURE)
        return EXIT_FAILURE;

      /* On to the next element */
      element = element->next;
    }

  /* Increment time and epoch counters */
  t++;

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_info
 *
 * Outputs information about the LVQ neural network
 */
void
nnet_lvq_info (const LvqNNetwork lvq_nnet,
               const BoolValue include_layers_info,
               const BoolValue include_units_info,
               const BoolValue include_in_connections,
               const BoolValue include_out_connections, FILE * output_fd)
{
  LvqAttributes lvq_attr;       /* LVQ attributes */


  if (output_fd == NULL)
    return;

  if (lvq_nnet == NULL)
    return;


  /* Initialization */
  lvq_attr = (LvqAttributes) lvq_nnet->attr;

  /* LVQ specific information */
  /*
     if (include_ngb_function_info == TRUE)
     {
     nnet_lvq_ngb_function_info (lvq_attr->ngb_function, output_fd);
     fprintf (output_fd, "\n");
     }
   */

  /* Outputs information about the neural network */
  nnet_nnetwork_info (lvq_nnet->nnet, include_layers_info, include_units_info,
                      include_in_connections, include_out_connections,
                      output_fd);

  return;
}
