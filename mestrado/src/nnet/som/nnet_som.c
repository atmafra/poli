#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "nnet_som.h"
#include "../../errorh/errorh.h"
#include "../../strutils/strutils.h"
#include "../../vector/vector.h"
#include "../nnet_types.h"
#include "../nnet_nnet.h"
#include "../nnet_layers.h"
#include "../nnet_units.h"
#include "../nnet_actv.h"
#include "../nnet_weights.h"
#include "../nnet_conns.h"
#include "../nnet_metrics.h"
#include "../nnet_train.h"



/*
 * nnet_som_create
 *
 * Creates a new neural network, according to the SOM architecture
 */
SomNNetwork
nnet_som_create (const NNetwork nnet,
                 const NgbFunctionClass ngb_class,
                 const NgbRFunctionParameters ngb_parameters,
                 const LRateFunctionClass lrate_class,
                 const LRateFunctionParameters lrate_parameters)
{
  SomNNetwork new_som = NULL;   /* new SOM neural network */
  SomAttributes som_attr = NULL;        /* new SOM attributes */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the neighborhood function class was actually passed */
  if (ngb_class == NULL)
    {
      fprintf (stderr,
               "nnet_som_create: no neighborhood function class passed\n");
      return NULL;
    }

  /* Checks if the learning rate function class was passed */
  if (lrate_class == NULL)
    {
      fprintf (stderr,
               "nnet_som_create: no learning rate function class passed\n");
      return NULL;
    }

  /* Creates the new SOM attribute set */
  som_attr = (SomAttributes) malloc (sizeof (nnet_som_attr_type));

  if (som_attr == NULL)
    {
      fprintf (stderr, "nnet_som_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Creates the SOM neighborhood function */
  som_attr->ngb_function = (NgbFunction)
    nnet_som_ngb_create_function (ngb_class, ngb_parameters);

  if (som_attr->ngb_function == NULL)
    {
      fprintf (stderr,
               "nnet_som_create: error creating real neighborhood function\n");
      free (new_som);
      return NULL;
    }

  /* Creates the SOM learning rate function */
  som_attr->lrate_function =
    nnet_train_lrate_create (lrate_class, lrate_parameters);

  if (som_attr->lrate_function == NULL)
    {
      fprintf (stderr,
               "nnet_som_create: error creating learning rate function\n");

      exit_status = nnet_som_ngb_destroy_function (&(som_attr->ngb_function));

      free (new_som);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_create: error destroying neighborhood function\n");
          return NULL;
        }

      return NULL;
    }

  /* Creates the SOM extension */
  new_som = (SomNNetwork) malloc (sizeof (nnet_extension_type));

  if (new_som == NULL)
    {
      fprintf (stderr, "nnet_som_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Attaches the extension */
  new_som->index = NNEXT_SOM;
  new_som->attr = som_attr;

  if (nnet != NULL)
    nnet_som_extend_nnetwork (nnet, new_som);

  return new_som;
}



/*
 * nnet_som_extend_nnetwork
 *
 * Associates the given SOM extension to the given neural network
 */
int
nnet_som_extend_nnetwork (NNetwork nnet, const SomNNetwork som_nnet)
{
  /* Checks if the SOM extension was passed */
  if (som_nnet == NULL)
    {
      fprintf (stderr, "nnet_som_extend_nnetwork: no SOM extension passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the neural network was passed */
  if (nnet == NULL)
    {
      fprintf (stderr,
               "nnet_som_extend_nnetwork: no neural network passed\n");
      return EXIT_FAILURE;
    }

  /* Associates the SOM extension to the neural network */
  nnet->extension = som_nnet;
  nnet->extension->nnet = nnet;

  return EXIT_SUCCESS;
}



/*
 * nnet_som_create_nnetwork
 *
 * Creates a new generic neural network accorging to the SOM architecture
 * and then attaches it to the given SOM extension
 */
int
nnet_som_create_nnetwork (SomNNetwork som_nnet,
                          const Name nnet_name,
                          const Name input_layer_name,
                          const Name output_layer_name,
                          const UnitIndex nu_inputs,
                          const UnitIndex nu_outputs,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters,
                          const Vector coord_distribution,
                          const Vector coord_increment)
{
  NNetwork new_nnet = NULL;     /* new associated generic neural network */
  Layer input_layer = NULL;     /* SOM input layer */
  Layer output_layer = NULL;    /* SOM output layer */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the SOM extension was actually passed */
  if (som_nnet == NULL)
    {
      fprintf (stderr, "nnet_som_create_nnetwork: no SOM extension passed\n");
      return EXIT_FAILURE;
    }

  /* Creates the new generic neural network */
  new_nnet = nnet_nnetwork_create (nnet_name, som_nnet);

  if (new_nnet == NULL)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error creating new SOM neural network\n");
      return EXIT_FAILURE;
    }

  /* Creates the input layer */
  input_layer = nnet_layer_create
    (new_nnet, NULL, &nnet_layer_class_input, input_layer_name);

  if (input_layer == NULL)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error creating input layer\n");
      return EXIT_FAILURE;
    }

  /* Creates the output layer */
  output_layer = nnet_layer_create
    (new_nnet, NULL, &nnet_layer_class_output, output_layer_name);

  if (output_layer == NULL)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error creating output layer\n");
      return EXIT_FAILURE;
    }

  /* Creates the input layer's units */
  exit_status =
    nnet_unit_create_multiple (nu_inputs, input_layer,
                               nnet_actv_class_passthrough, NULL, FALSE, TRUE,
                               NULL, NULL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error creating input units\n");
      return EXIT_FAILURE;
    }

  /* Creates the output layer's units */
  exit_status = nnet_unit_create_multiple
    (nu_outputs, output_layer, nnet_actv_class_linear, NULL, FALSE, TRUE,
     coord_distribution, coord_increment);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error creating output units\n");
      return EXIT_FAILURE;
    }

  /* Fully connects the input layer to the output layer */
  exit_status = nnet_conn_connect_layers
    (input_layer, output_layer, 0.0, weight_init_class,
     weight_init_parameters);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error connecting input layer to output layer\n");
      return EXIT_FAILURE;
    }

  /* Attaches the new neural network to the given SOM extension */
  exit_status = nnet_som_extend_nnetwork (new_nnet, som_nnet);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_create_nnetwork: error attaching generic neural network\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_destroy
 *
 * Destroys a previously created SOM neural network
 */
int
nnet_som_destroy (SomNNetwork * som_nnet,
                  const BoolValue cascade_nnetwork,
                  const BoolValue cascade_layers,
                  const BoolValue cascade_units,
                  const BoolValue cascade_connections)
{
  SomAttributes som_attr;       /* SOM attributes */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the SOM was actually passed */
  if (som_nnet == NULL || *som_nnet == NULL)
    {
      fprintf (stderr,
               "nnet_som_destroy: no SOM neural network to destroy\n");
      return EXIT_FAILURE;
    }

  /* Destroys the associated generic neural network */
  if ((*som_nnet)->nnet != NULL && cascade_nnetwork == TRUE)
    {
      exit_status = nnet_nnetwork_destroy
        (&((*som_nnet)->nnet), TRUE, cascade_layers, cascade_units,
         cascade_connections);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_destroy: error destroying associated generic neural network\n");
          return EXIT_FAILURE;
        }
    }

  /* Auxiliary pointer to the SOM attributes */
  som_attr = (SomAttributes) (*som_nnet)->attr;

  /* Destroys the neighborhood function */
  exit_status = nnet_som_ngb_destroy_function (&(som_attr->ngb_function));

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_destroy: error destroying neighborhood function\n");
      return EXIT_FAILURE;
    }

  /* Destroys the learning rate function */
  if (som_attr->lrate_function != NULL)
    function_destroy (&(som_attr->lrate_function));

  /* Destroys the SOM network itself */
  free (*som_nnet);

  /* Makes it point to NULL */
  *som_nnet = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_som_ngb_vector
 *
 * Returns the neighborhood vector for the layer of the given winner unit
 */
int
nnet_som_ngb_vector (const SomNNetwork som_nnet,
                     const Unit winner_unit, Vector ngb_vector)
{
  SomAttributes som_attr;       /* SOM attributes */
  Layer input_layer = NULL;     /* SOM's input layer */
  Layer output_layer = NULL;    /* SOM's output layer */
  Unit cur_unit = NULL;         /* current output unit */
  RValue ngb_value;             /* neighborhood function value */
  UsLgIntValue cur_comp;        /* current ngb vector component */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the parameters were actually passed */
  if (som_nnet == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_vector: no SOM neural network passed\n");
      return EXIT_FAILURE;
    }

  if (winner_unit == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_vector: no winner unit passed\n");
      return EXIT_FAILURE;
    }

  if (ngb_vector == NULL)
    {
      fprintf (stderr, "nnet_som_ngb_vector: no vector passed\n");
      return EXIT_FAILURE;
    }

  /* Sets the auxiliary pointers */
  input_layer = som_nnet->nnet->first_layer;
  output_layer = som_nnet->nnet->last_layer;
  som_attr = (SomAttributes) som_nnet->attr;

  /* Initialization */
  cur_unit = output_layer->first_unit;
  cur_comp = 1;

  /* Constructs the neighborhood vector */
  while (cur_unit != NULL)
    {
      /* Gets the neighborhood function value */
      exit_status = nnet_som_ngb_value
        (som_attr->ngb_function, winner_unit->coord, cur_unit->coord,
         &ngb_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_ngb_vector: error calculating neighborhood function value\n");
          return EXIT_FAILURE;
        }

      /* Sets the corresponding component of the neighborhood vector */
      exit_status = vector_set_value (ngb_vector, cur_comp, ngb_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_ngb_vector: error setting neighborhood vector component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Goes to the next unit */
      cur_unit = cur_unit->next;
      ++cur_comp;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_propagate_element
 *
 * Loads and propagates an element
 */
int
nnet_som_propagate_element (const SomNNetwork som_nnet,
                            const TElement element, Unit * winner)
{
  Layer input_layer, output_layer;      /* auxiliary layer pointers */
  SomAttributes som_attr = NULL;        /* SOM attributes */
  VectorMetric metric;          /* vector metric for activation */
  int exit_status;              /* auxiliary funciton return status */


  /*************************************************************************
   *                             PRE-CONDITIONS                            *
   *************************************************************************/

  /* Checks if the network was actually passed */
  if (som_nnet == NULL)
    {
      fprintf (stderr,
               "nnet_som_propagate_element: no neural network passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the element was actually passed */
  if (element == NULL)
    {
      fprintf (stderr,
               "nnet_som_propagate_element: no training element passed\n");
      return EXIT_FAILURE;
    }


  /*************************************************************************
   *                             INITIALIZATION                            *
   *************************************************************************/

  /* Initializes the auxiliary pointers */
  input_layer = som_nnet->nnet->first_layer;
  output_layer = som_nnet->nnet->last_layer;
  som_attr = (SomAttributes) som_nnet->attr;
  metric = som_attr->ngb_function->function_class->vector_metric;


  /*************************************************************************
   *                               PROPAGATION                             *
   *************************************************************************/

  /* Loads the element into the input layer */
  exit_status = nnet_layer_load_vector (element->input, input_layer);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_propagate_element: error loading input vector\n");
      return EXIT_FAILURE;
    }

  /* Activates the neural network */
  exit_status = nnet_nnetwork_activate (som_nnet->nnet, metric);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_propagate_element: error activating neural network\n");
      return EXIT_FAILURE;
    }


  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/

  /* Determines the winner: largest output according to the metrics */
  *winner = nnet_metr_layer_winner (output_layer, metric);

  if (*winner == NULL)
    {
      fprintf (stderr,
               "nnet_som_propagate_element: error determining winner unit\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
Vector
nnet_som_propagate_set (const SomNNetwork som_nnet, const TSet set)
{
  TElement cur_element = NULL;  /* current training element */
  ElementIndex cur_index = 1;   /* current element counter */
  Unit winner = NULL;           /* current winner unit */
  Vector winners = NULL;        /* list of winners indexes */


  /* checks if the SOM was actually passed */
  if (som_nnet == NULL)
    {
      error_failure ("nnet_som_propagate_set",
                     "no SOM neural network passed\n");
      return NULL;
    }

  /* checks if the training set was actually passed */
  if (set == NULL)
    {
      error_failure ("nnet_som_propagate_set", "no training set passed\n");
      return NULL;
    }

  /* creates the unit index list */
  if (error_if_null
      (winners =
       vector_create (set->nu_elements), "nnet_som_propagate_set",
       "error creating list of winner indexes\n"))
    return NULL;

  /* propagates all elements in the set */
  cur_element = set->first_element;

  while (cur_element != NULL)
    {
      /* propagates current element */
      if (error_if_failure
          (nnet_som_propagate_element (som_nnet, cur_element, &winner),
           "nnet_som_propagate_set", "error propagating element %ld\n",
           cur_element->element_index))
        return NULL;

      /* sets the winner index in the unit index list */
      if (error_if_failure
          (vector_set_value (winners, cur_index, (RValue) winner->unit_index),
           "nnet_som_propagate_set",
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
 * nnet_som_train_element
 *
 * Executes training for one element
 */
int
nnet_som_train_element (SomNNetwork som_nnet,
                        const TElement element, const RValue learning_rate)
{
  Layer input_layer = NULL;     /* SOM input layer */
  Layer output_layer = NULL;    /* SOM output layer */
  Unit winner_unit = NULL;      /* winning unit */
  Unit cur_unit = NULL;         /* current output unit */
  UsLgIntValue cur_comp;        /* current neighborhood vector component */
  Vector ngb_vector = NULL;     /* neighborhood vector */
  Vector weight_vector = NULL;  /* auxiliary weight vector */
  Vector error_vector = NULL;   /* current unit error vector */
  RValue ngb_value;             /* neighborhood value */
  RValue rate;                  /* fraction of the error vector */
  int exit_status;              /* auxiliary function return status */


  /*************************************************************************
   *                             PRE-CONDITIONS                            *
   *************************************************************************/

  /* Checks if the network was actually passed */
  if (som_nnet == NULL)
    {
      fprintf (stderr, "nnet_som_train_element: no neural network passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the SOM extension is attached to a neural network */
  if (som_nnet->nnet == NULL)
    {
      fprintf (stderr,
               "nnet_som_train_element: SOM extension is not attached to a neural network\n");
      return EXIT_FAILURE;
    }

  /* Checks if the network has a plausible SOM architecture */
  if (som_nnet->nnet->nu_layers != 2)
    {
      fprintf (stderr,
               "nnet_som_train_element: a SOM must have two layers\n");
      return EXIT_FAILURE;
    }


  /*************************************************************************
   *                             INITIALIZATION                            *
   *************************************************************************/

  /* Sets the auxiliary layer pointers */
  input_layer = som_nnet->nnet->first_layer;
  output_layer = som_nnet->nnet->last_layer;

  /* Creates the neighborhood vector */
  ngb_vector = vector_create (output_layer->nu_units);

  if (ngb_vector == NULL)
    {
      fprintf (stderr,
               "nnet_som_train_element: error creating neighborhood vector\n");
      return EXIT_FAILURE;
    }

  /* Creates the auxiliary weight vector */
  weight_vector = vector_create (input_layer->nu_units);

  if (weight_vector == NULL)
    {
      fprintf (stderr,
               "nnet_som_train_element: error creating auxiliary weight vector\n");
      return EXIT_FAILURE;
    }

  /* Creates the auxiliary error vector */
  error_vector = vector_create (input_layer->nu_units);

  if (error_vector == NULL)
    {
      fprintf (stderr,
               "nnet_som_train_element: error creating auxiliary error vector\n");
      return EXIT_FAILURE;
    }


  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/

  /* Loads the training element, activates the network and find the winner */
  exit_status = nnet_som_propagate_element (som_nnet, element, &winner_unit);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_train_element: error performing competition\n");
      return EXIT_FAILURE;
    }

  /*
     printf ("winner: unit %ld\n", winner_unit->unit_index);
   */

  /*************************************************************************
   *                               COOPERATION                             *
   *************************************************************************/

  /* Gets the neighborhood vector for the winner unit */
  exit_status = nnet_som_ngb_vector (som_nnet, winner_unit, ngb_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_train_element: error getting neighborhood vector\n");
      return EXIT_FAILURE;
    }

  /*
     printf ("NGB:");
     vector_info (ngb_vector, stdout);
   */

  /*************************************************************************
   *                                ADAPTION                               *
   *************************************************************************/

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
                   "nnet_som_train_element: error getting current unit weight vector\n");
          return EXIT_FAILURE;
        }

      /* Calculates the error vector */
      exit_status =
        vector_subtract (element->input, weight_vector, error_vector);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_train_element: error calculating error vector\n");
          return EXIT_FAILURE;
        }

      /* Gets the neighborhood value */
      exit_status = vector_get_value (ngb_vector, cur_comp, &ngb_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_som_train_element: error getting neighborhood value from vector at component %ld\n",
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
                       "nnet_som_train_element: error calculating weight correction vector\n");
              return EXIT_FAILURE;
            }

          /* Calculates the new weights */
          exit_status =
            vector_sum (weight_vector, error_vector, weight_vector);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_som_train_element: error calculating new weight vector\n");
              return EXIT_FAILURE;
            }

          /* Updates the weights */
          exit_status = nnet_unit_set_weight_vector (cur_unit, weight_vector);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_som_train_element: error updating current unit's weights\n");
              return EXIT_FAILURE;
            }

          /*
             printf ("WVN %ld:", cur_unit->unit_index);
             vector_info (weight_vector, stdout);
           */
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
               "nnet_som_train_element: error destroying neighborhood vector\n");
      return EXIT_FAILURE;
    }

  /* Destroys the auxiliary weight vector */
  exit_status = vector_destroy (&weight_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_train_element: error destroying auxiliary weight vector\n");
      return EXIT_FAILURE;
    }

  /* Destroys the error vector */
  exit_status = vector_destroy (&error_vector);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_som_train_element: error destroying error vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_som_train_set
 *
 * Executes one training pass through all the elements in the given
 * training set
 */
int
nnet_som_train_set (SomNNetwork som_nnet,
                    const TSet training_set,
                    const DTime first_epoch,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character)
{
  static DTime t = 0;           /* current training time */
  TElement element = NULL;      /* current element */
  SomAttributes somatt = NULL;  /* SOM attributes */
  LRateFunction lfunc = NULL;   /* learning rate function */
  NgbFunction nfunc = NULL;     /* neighborhood function */
  RValue etha = 0.0;            /* current learning rate */


  /* Checks if the network was actually passed */
  if (som_nnet == NULL)
    {
      return error_failure ("nnet_som_train_set",
                            "no SOM neural network passed\n");
    }

  /* Checks if the training set was actually passed */
  if (training_set == NULL)
    {
      return error_failure ("nnet_som_train_set", "no training set passed\n");
    }

  /* Checks if internal time should be reset */
  if (reset_time == TRUE)
    t = first_epoch;

  /* Checks if the maximum epochs has been reached */
  if (t >= max_epochs)
    {
      return error_failure ("nnet_som_train_set",
                            "maximum epochs (%ld) reached\n", max_epochs);
    }

  /* Initialization */
  somatt = (SomAttributes) som_nnet->attr;
  lfunc = somatt->lrate_function;
  nfunc = somatt->ngb_function;

  /* Calculates the current learning rate */
  etha = nnet_train_lrate_value (lfunc, (RValue) t);

  /* Updates time in neighborhood function */
  if (error_if_failure (nnet_som_ngb_set_parameter (nfunc, 0, (RValue) t),
                        "nnet_som_train_set",
                        "error updating time in neighborhood function\n") ==
      EXIT_FAILURE)
    return EXIT_FAILURE;

  /* Updates progress bar */
  if (output_progress == TRUE)
    {
      if (error_if_failure
          (display_progress
           (first_epoch + 1, (long) max_epochs, (long) t + 1, progress_width,
            progress_character, 0), "nnet_som_train_set",
           "error updating progress bar to %ld\n", t))
        return EXIT_FAILURE;
    }

  /* Training elements loop */
  element = training_set->first_element;
  while (element != NULL)
    {
      /* Trains the current element */
      if (error_if_failure (nnet_som_train_element (som_nnet, element, etha),
                            "nnet_som_train_set",
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
 * nnet_som_info
 *
 * Outputs information about the SOM neural network
 */
void
nnet_som_info (const SomNNetwork som_nnet,
               const BoolValue include_ngb_function_info,
               const BoolValue include_layers_info,
               const BoolValue include_units_info,
               const BoolValue include_in_connections,
               const BoolValue include_out_connections, FILE * output_fd)
{
  SomAttributes som_attr;       /* SOM attributes */


  if (output_fd == NULL)
    return;

  if (som_nnet == NULL)
    return;


  /* Initialization */
  som_attr = (SomAttributes) som_nnet->attr;

  /* Outputs information about the neighborhood function */
  if (include_ngb_function_info == TRUE)
    {
      nnet_som_ngb_function_info (som_attr->ngb_function, output_fd);
      fprintf (output_fd, "\n");
    }

  /* Outputs information about the neural network */
  nnet_nnetwork_info (som_nnet->nnet, include_layers_info, include_units_info,
                      include_in_connections, include_out_connections,
                      output_fd);

  return;
}
