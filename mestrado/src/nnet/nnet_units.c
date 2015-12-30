#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "errorh.h"
#include "vector.h"

#include "nnet_layers.h"
#include "nnet_units.h"
#include "nnet_conns.h"
#include "nnet_actv.h"

/******************************************************************************
 *                                                                            *
 *                       PRIVATE FUNCTIONS AND DATATYPES                      *
 *                                                                            *
 ******************************************************************************/

/*
 * unit_vector_type
 *
 * Possible input vectors to extract from an unit
 */
typedef enum
{
  VECTOR_INPUTS = 0,
  VECTOR_WEIGHTS = 1
}
unit_input_vector_type;



/*
 * nnet_unit_set_activation_function
 *
 * Sets the activation function of the given unit according to the parameters
 */
static int
nnet_unit_set_activation_function (Unit unit, Layer layer,
                                   ActivationClass actv_class,
                                   const RFunctionParameters parameters,
                                   const BoolValue
                                   use_layer_default_activation_class,
                                   const BoolValue
                                   use_activation_class_default_parameters)
{
  ActivationClass new_actv_class;       /* activation class to be used */
  ActivationFunction aux_function;      /* pointer to activation function */


  /* Activation function selection */
  switch (use_layer_default_activation_class)
    {
    case TRUE:

      /* Checks if the layer has a default activation class */
      if (layer == NULL)
        return error_failure ("nnet_unit_set_activation_function",
                              "a standalone unit can't inherit activation function from its layer\n");

      new_actv_class = layer->layer_class->default_activation_class;

      if (new_actv_class == NULL)
        return error_failure ("nnet_unit_set_activation_function",
                              "layer has no default activation class\n");
      break;

    case FALSE:
      new_actv_class = actv_class;

      if (actv_class == NULL)
        return error_failure ("nnet_unit_set_activation_function",
                              "cannot create unit without activation function\n");
      break;

    default:
      return error_failure ("nnet_unit_set_activation_function",
                            "invalid value for 'use_layer_default_activation_class\n");
    }

  /* If the unit already had an activation function, destroy it */
  if (unit->activation_function != NULL)
    {
      aux_function = unit->activation_function;
      nnet_actv_destroy_function (&aux_function);
    }

  /* Creates a new activation function */
  unit->activation_function = nnet_actv_create_function (new_actv_class);

  if (unit->activation_function == NULL)
    return error_failure ("nnet_unit_set_activation_function",
                          "error creating activation function\n");

  /* Activation function parameters selection */
  switch (use_activation_class_default_parameters)
    {
    case TRUE:
      /* Nothing to do: functions are created with default parameters */
      break;

    case FALSE:
      nnet_actv_set_parameters (unit->activation_function, parameters);
      break;

    default:
      return error_failure ("nnet_unit_set_activation_function",
                            "invalid value for 'use_activation_class_default_parameters\n");
    }

  /* Updating the new unit output */
  if (nnet_unit_update_output (unit) != EXIT_SUCCESS)
    return error_failure ("nnet_unit_set_activation_function",
                          "error updating new unit output\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_correct_indexes
 *
 * Corrects the unit indexes after creation or removal of an unit
 * in other position than the last
 */
static void nnet_unit_correct_indexes
  (Unit first_unit, const UnitIndex first_unit_index)
{
  Unit cur_unit;                /* auxiliary pointer to the current unit */
  UnitIndex cur_index;          /* current unit counter */


  cur_unit = first_unit;
  cur_index = first_unit_index;

  while (cur_unit != NULL)
    {
      cur_unit->unit_index = cur_index;
      cur_unit = cur_unit->next;
      ++cur_index;
    }
}



/*
 * nnet_unit_input_vectors
 *
 * Returns attributes of an unit input connections as a vector
 */
static int nnet_unit_input_vectors
  (const Unit unit, Vector vector, const unit_input_vector_type vector_type)
{
  RValue cur_value;             /* current input value */
  Connection cur_connection;    /* pointer to the current input connection */
  UsLgIntValue cur_comp;        /* current vector component */


  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_input_vectors", "no unit passed\n");

  /* Checks if the vector was actually passed */
  if (vector == NULL)
    return error_failure ("nnet_unit_input_vectors",
                          "no destination vector\n");

  /* Checks if the unit has input connections */
  if (unit->nu_inputs == 0)
    {
      /* The unit doesn't have input connections: nothing to do */
      return EXIT_SUCCESS;
    }

  /* Checks dimension compatibility */
  if (unit->nu_inputs != vector->dimension)
    return error_failure ("nnet_unit_input_vectors",
                          "incompatible dimensions: unit has %ld input connections while vector has dimension %ld\n",
                          unit->nu_inputs, vector->dimension);

  /* Fills the vector */
  cur_connection = unit->first_orig;
  cur_comp = 1;

  while (cur_connection != NULL)
    {
      /*
       * Chooses the value according to the input type
       * Sets the vector's component
       * On to the next connection
       */
      switch (vector_type)
        {
        case VECTOR_INPUTS:
          cur_value = cur_connection->orig->output;
          break;

        case VECTOR_WEIGHTS:
          cur_value = cur_connection->weight;
          break;

        default:
          return error_failure ("nnet_unit_input_vectors",
                                "invalid input vector type\n");
        }

      if (vector_set_value (vector, cur_comp, cur_value) != EXIT_SUCCESS)
        return error_failure ("nnet_unit_input_vectors",
                              "error setting vector component %ld\n",
                              cur_comp);

      cur_connection = cur_connection->next_orig;
      ++cur_comp;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC FUNCTIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_unit_create
 *
 * Creates one unit at the bottom of layer 'layer_index'
 * for the given layer array. The current unit of the layer is positioned
 * to the newly created unit.
 */
Unit
nnet_unit_create (Layer layer, UnitIndex * index, ActivationClass actv_class,
                  const ActivationParameters parameters,
                  const BoolValue use_layer_default_activation_class,
                  const BoolValue use_activation_class_default_parameters,
                  const Vector coordinates,
                  const RValue average, const RValue stddev)
{
  Unit new_unit;                /* new unit that will be created */


  /* Allocate memory for the new unit */
  new_unit = (Unit) malloc (sizeof (nnet_unit_type));
  if (new_unit == NULL)
    return null_failure ("nnet_unit_create", "virtual memory exhausted\n");

  /* Initializes the new unit */
  new_unit->layer = NULL;
  new_unit->unit_index = 0;
  new_unit->activation = 0.0;
  new_unit->output = 0.0;
  new_unit->activation_function = NULL;
  new_unit->nu_inputs = 0;
  new_unit->nu_outputs = 0;
  new_unit->first_orig = NULL;
  new_unit->first_dest = NULL;
  new_unit->next = NULL;
  new_unit->coord = coordinates;
  new_unit->average = average;
  new_unit->stddev = stddev;

  /* If a layer was passed, attach the unit to it */
  if (layer != NULL)
    {
      /* A unit connected to the given layer will be created */
      if (nnet_unit_attach_to_layer (new_unit, layer, index) != EXIT_SUCCESS)
        {
          free (new_unit);
          return null_failure ("nnet_unit_create",
                               "error connecting new unit to the requested layer\n");
        }
    }

  /* Activation function determination */
  if (nnet_unit_set_activation_function
      (new_unit, layer, actv_class, parameters,
       use_layer_default_activation_class,
       use_activation_class_default_parameters) != EXIT_SUCCESS)
    {
      free (new_unit);
      return null_failure ("nnet_unit_create",
                           "error setting activation function\n");
    }

  return new_unit;
}



/*
 * nnet_unit_destroy
 *
 * Destroys a previously created unit
 */
int
nnet_unit_destroy (Unit * unit, const BoolValue cascade_connections)
{
  Unit aux_unit = *unit;        /* auxiliary pointer to the real unit */
  ActivationFunction aux_func;  /* auxiliary pointer to activation function */
  Vector aux_coord;             /* auxiliary pointer to coordinates */

  /* Checks if the unit was actually passed */
  if (unit == NULL || aux_unit == NULL)
    return error_failure ("nnet_unit_destroy", "no unit to destroy\n");

  /* Checks if the unit is connected */
  if (aux_unit->nu_inputs > 0 || aux_unit->nu_outputs > 0)
    {
      switch (cascade_connections)
        {
        case TRUE:

          /* Destroys all connections from and to the unit */
          if (nnet_conn_disconnect_unit (aux_unit) != EXIT_SUCCESS)
            return error_failure ("nnet_unit_destroy",
                                  "error disconnecting unit\n");
          break;

        case FALSE:
          return error_failure ("nnet_unit_destroy",
                                "unit still has %ld input connections and %ld output connections\n",
                                aux_unit->nu_inputs, aux_unit->nu_outputs);

        default:
          return error_failure ("nnet_unit_destroy",
                                "invalid value for 'cascade_connections'\n");
        }
    }

  /* Destroys the activation function */
  if (aux_unit->activation_function != NULL)
    {
      aux_func = aux_unit->activation_function;
      nnet_actv_destroy_function (&aux_func);
    }

  /* Destroys the coordinates vector */
  if (aux_unit->coord != NULL)
    {
      aux_coord = aux_unit->coord;

      if (vector_destroy (&aux_coord) != EXIT_SUCCESS)
        return error_failure ("nnet_unit_destroy",
                              "error destroying coordinates vector\n");
    }

  /* If the unit is attached to a layer, dettaches from it */
  if (aux_unit->layer != NULL)
    if (nnet_unit_remove_from_layer (aux_unit) != EXIT_SUCCESS)
      return error_failure ("nnet_unit_destroy",
                            "error removing unit from its layer\n");

  /* Destroys the unit */
  free (*unit);

  /* Points the unit to NULL */
  *unit = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_attach_to_layer
 *
 * Attaches the given unit to the given neural network
 */
int
nnet_unit_attach_to_layer (Unit unit, Layer layer, UnitIndex * unit_index)
{
  Unit aux_unit;                /* auxiliary pointer to unit */
  UnitIndex cur_unit;           /* current unit counter */
  UnitIndex new_index;          /* auxiliary unit index */

  /* Check if the unit was passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_attach_to_layer", "no unit to attach\n");

  /* Check if the destination layer was passed */
  if (layer == NULL)
    return error_failure ("nnet_unit_attach_to_layer",
                          "no destination layer\n");

  /* If the unit index was passed NULL, attaches to the end of the layer */
  if (unit_index == NULL)
    new_index = layer->nu_units + 1;
  else
    new_index = *unit_index;

  /* Validates the domain of the new index */
  if (new_index < 1 || new_index > layer->nu_units + 1)
    return error_failure ("nnet_unit_attach_to_layer",
                          "invalid position requested: %ld\n", new_index);

  /* Checks if the unit's coordinates are dimensionally compatible */
  if (layer->nu_units > 0 && unit->coord != NULL)
    {
      if (layer->first_unit->coord != NULL)
        {
          if (unit->coord->dimension != layer->first_unit->coord->dimension)
            return error_failure ("nnet_unit_attach_to_layer",
                                  "new unit's coordinates has dimension %ld while layer coordinates have dimension %ld\n",
                                  unit->coord->dimension,
                                  layer->first_unit->coord->dimension);
        }
      else
        {
          return error_failure ("nnet_unit_attach_to_layer",
                                "no coordinate system defined for the units in this layer\n");
        }
    }

  /* If the unit was attached to another layer, dettaches from it */
  if (unit->layer != NULL)
    {
      if (unit->layer != layer || unit->unit_index != new_index)
        {
          /* Removes from the old layer */
          if (nnet_unit_remove_from_layer (unit) != EXIT_SUCCESS)
            return error_failure ("nnet_unit_attach_to_layer",
                                  "error removing from old layer\n");
        }
      else
        {
          /* Attaching to the same layer: nothing to do */
          return EXIT_SUCCESS;
        }
    }

  if (new_index == 1)
    {
      /* Inserts at the head */
      unit->next = layer->first_unit;
      layer->first_unit = unit;
    }
  else
    {
      /* Goes to the unit before the requested position */
      if (new_index == layer->nu_units + 1)
        aux_unit = layer->last_unit;
      else
        {
          aux_unit = layer->first_unit;

          for (cur_unit = 1; cur_unit < new_index - 1; cur_unit++)
            aux_unit = aux_unit->next;
        }

      /* Inserts the unit */
      unit->next = aux_unit->next;
      aux_unit->next = unit;
    }

  /* Increments the number of units in the layer */
  layer->nu_units++;

  /* Increments the number of units in the layer's neural network */
  if (layer->nnetwork != NULL)
    layer->nnetwork->nu_units++;

  /* Keeps track of the last unit */
  if (new_index == layer->nu_units)
    layer->last_unit = unit;

  /* If not inserted at the tail, correct the other units' indexes */
  if (new_index != layer->nu_units)
    nnet_unit_correct_indexes (unit, new_index);

  /* Sets the unit's layer attributes */
  unit->layer = layer;
  unit->unit_index = new_index;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_remove_from_layer
 *
 * Removes the given unit from its layer
 */
int
nnet_unit_remove_from_layer (Unit unit)
{
  Layer layer;                  /* auxiliary layer */
  Unit aux_unit;                /* auxiliary pointer to unit */

  /* Check if the unit was passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_remove_from_layer",
                          "no unit to remove\n");

  /* Verifies if the unit was attached to a layer */
  if (unit->layer == NULL)
    {
      /* The unit is not attached to a layer: nothing to do */
      return EXIT_SUCCESS;
    }
  else
    layer = unit->layer;

  /* Goes to the unit before */
  if (unit == layer->first_unit)
    {
      layer->first_unit = unit->next;
      aux_unit = NULL;
    }
  else
    {
      aux_unit = layer->first_unit;

      while (aux_unit->next != unit && aux_unit != NULL)
        aux_unit = aux_unit->next;

      /* Checks if the unit is actually in the layer */
      if (aux_unit == NULL)
        return error_failure ("nnet_unit_remove_from_layer",
                              "unit is not in the layer\n");

      /* Removes the unit */
      aux_unit->next = unit->next;
    }

  /* Updates the last unit of the layer */
  if (layer->last_unit == unit)
    layer->last_unit = aux_unit;

  /* Corrects the indexes of the other units */
  if (unit->unit_index != layer->nu_units)
    nnet_unit_correct_indexes (unit->next, unit->unit_index);

  /* Decrements the number of units in the layer */
  layer->nu_units--;

  /* Decrements the number of units in the network */
  if (layer->nnetwork != NULL)
    layer->nnetwork->nu_units++;

  /* Unsets the unit layer attributes */
  unit->layer = NULL;
  unit->unit_index = 0;
  unit->next = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_create_multiple
 *
 * Create multiple units on a given layer
 */
int
nnet_unit_create_multiple (const UnitIndex nu_units,
                           Layer layer,
                           ActivationClass actv_class,
                           const ActivationParameters parameters,
                           const BoolValue use_layer_default_activation_class,
                           const BoolValue
                           use_activation_class_default_parameters,
                           const Vector coord_distribution,
                           const Vector coord_increment,
                           const RValue average, const RValue stddev)
{
  Unit aux_unit;                /* auxiliary pointer to the new units */
  UnitIndex cur_unit;           /* auxiliary unit counter */
  UsLgIntValue cur_dim;         /* auxiliary dimension counter */
  RValue cur_comp;              /* auxiliary component value */
  UnitIndex dist_units;         /* number of units from distribution */
  Vector cur_coord;             /* current unit's coordinates */
  double aux_double;            /* auxiliary double */


  /* Checks if the distribution vector is according to the number of units */
  if (coord_distribution != NULL)
    {
      dist_units = 1;

      for (cur_dim = 1; cur_dim <= coord_distribution->dimension; cur_dim++)
        {
          /* Gets the number of units in the current dimension */
          if (vector_get_value (coord_distribution, cur_dim, &cur_comp)
              != EXIT_SUCCESS)
            return error_failure ("nnet_unit_create_multiple",
                                  "error getting coordinate distribution component %ld\n",
                                  cur_dim);

          /* Checks if it is an integer value */
          if (fabs (modf (cur_comp, &aux_double)) > DBL_EPSILON)
            return error_failure ("nnet_unit_create_multiple",
                                  "invalid number of units in distribution vector, dimension %ld\n",
                                  cur_dim);

          dist_units *= (UnitIndex) cur_comp;
        }

      if (dist_units != nu_units)
        return error_failure ("nnet_unit_create_multiple",
                              "distribution vector demands %ld units while %ld units were requested\n",
                              dist_units, nu_units);
    }

  for (cur_unit = 0; cur_unit < nu_units; cur_unit++)
    {
      /* Creates one unit */
      aux_unit = nnet_unit_create (layer, NULL, actv_class, parameters,
                                   use_layer_default_activation_class,
                                   use_activation_class_default_parameters,
                                   NULL, average, stddev);
      if (aux_unit == NULL)
        return error_failure ("nnet_unit_create_multiple",
                              "error creating %ldth unit\n", cur_unit);

      /* Sets its coordinates based on its index */
      if (coord_distribution != NULL)
        {
          cur_coord = nnet_unit_index_to_coordinates
            (aux_unit->unit_index, coord_distribution, coord_increment);

          if (cur_coord == NULL)
            return error_failure ("nnet_unit_create_multiple",
                                  "error getting new unit's coordinates\n");

          aux_unit->coord = cur_coord;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_by_index
 *
 * Returns an unit given its neural network and its layer index
 */
Unit
nnet_unit_by_index (NNetwork nnet,
                    LayerIndex layer_index, UnitIndex unit_index)
{
  static NNetwork last_nnet = NULL;     /* last neural network passed */
  static LayerIndex last_lindex = 0;    /* last layer index searched */
  static Layer layer = NULL;    /* unit's layer */
  static Unit last_unit = NULL; /* last unit returned */
  Unit cur_unit = NULL;         /* current unit */
  BoolValue unit_found = FALSE; /* unit found flag */

  /* Fetches the layer */
  if (nnet != last_nnet || layer_index != last_lindex)
    {
      layer = nnet_layer_by_index (nnet, layer_index);

      if (layer == NULL)
        return null_failure ("nnet_unit_by_index", "layer not found\n");

      last_nnet = nnet;
      last_lindex = layer_index;
      last_unit = NULL;
    }

  /* A good guess */
  if (last_unit != NULL)
    {
      if (last_unit->next->unit_index == unit_index)
        {
          last_unit = last_unit->next;
          return last_unit;
        }
    }

  /* Searches for the unit */
  cur_unit = layer->first_unit;

  while (cur_unit != NULL && unit_found == FALSE)
    {
      if (cur_unit->unit_index == unit_index)
        unit_found = TRUE;
      else
        cur_unit = cur_unit->next;
    }

  if (unit_found == FALSE)
    return null_failure ("nnet_unit_by_index", "unit not found\n");

  return cur_unit;
}



/*
 * nnet_unit_set_activation
 *
 * Sets the unit's activation to the given value
 */
int
nnet_unit_set_activation (Unit unit, const RValue activation)
{
  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_set_activation",
                          "no unit to set activation\n");

  /* Sets the activation */
  unit->activation = activation;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_get_input_vector
 *
 * Gathers the outputs of all units connected to the given unit as inputs
 * into the given vector
 */
int
nnet_unit_get_input_vector (const Unit unit, Vector input_vector)
{
  /* Dispatches the execution */
  if (nnet_unit_input_vectors (unit, input_vector, VECTOR_INPUTS)
      != EXIT_SUCCESS)

    return error_failure ("nnet_unit_input_vector",
                          "error setting input vector\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_get_weight_vector
 *
 * Gathers the weights of all input connections of the given unit
 * into the given vector
 */
int
nnet_unit_get_weight_vector (const Unit unit, Vector weight_vector)
{
  /* Dispatches the execution */
  if (nnet_unit_input_vectors (unit, weight_vector, VECTOR_WEIGHTS)
      != EXIT_SUCCESS)

    return error_failure ("nnet_unit_input_vector",
                          "error setting input vector\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_set_weight_vector
 *
 * Updates the unit's input weights according to the given weight vector
 */
int
nnet_unit_set_weight_vector (const Unit unit, const Vector weight_vector)
{
  Connection cur_conn;          /* current input connection */
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current vector component value */

  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_set_weight_vector", "no unit passed\n");

  /* Checks if the weight vector was actually passed */
  if (weight_vector == NULL)
    return error_failure ("nnet_unit_set_weight_vector",
                          "no weight vector passed\n");

  /* Checks dimensional compatibility */
  if (unit->nu_inputs != weight_vector->dimension)
    return error_failure ("nnet_unit_set_weight_vector",
                          "incompatible dimensions: unit has %ld inputs while vector has dimension %ld\n",
                          unit->nu_inputs, weight_vector->dimension);

  /* Sets the connection values to the components */
  cur_conn = unit->first_orig;
  cur_comp = 1;

  while (cur_conn != NULL)
    {
      /*
       * Gets the current component value
       * Sets the connection weight
       * Goes to the next connection
       */

      if (vector_get_value (weight_vector, cur_comp, &cur_value)
          != EXIT_SUCCESS)
        return error_failure ("nnet_unit_set_weight_vector",
                              "error getting weight vector component %ld\n",
                              cur_comp);

      if (nnet_conn_set_weight (cur_conn, cur_value) != EXIT_SUCCESS)
        return error_failure ("nnet_unit_set_weight_vector",
                              "error setting connection value for component %ld\n",
                              cur_comp);

      cur_conn = cur_conn->next_orig;
      cur_comp++;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_collect_activation
 *
 * Updates the unit's activation, performing the sum of the outputs of the
 * units connected to it, weighted by the connections weights.
 */
int
nnet_unit_collect_activation (Unit unit)
{
  RValue new_activation;        /* new activation */
  Connection cur_connection;    /* pointer to the current input connection */

  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_collect_activation", "no unit passed\n");

  /* Checks if the unit has input connections */
  if (unit->nu_inputs == 0)
    {
      /* The unit doesn't have input connections: nothing to do */
      return EXIT_SUCCESS;
    }

  /* Computes the weighted sum (inner product) */
  new_activation = 0.0;
  cur_connection = unit->first_orig;

  while (cur_connection != NULL)
    {
      new_activation += cur_connection->orig->output * cur_connection->weight;
      cur_connection = cur_connection->next_orig;
    }

  /* Updates the unit's activation */
  unit->activation = new_activation;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_vector_activation
 *
 * Updates the unit's activation, according to the requested metric
 * between the input and weight vectors
 */
int
nnet_unit_vector_activation (Unit unit, const VectorMetric metric)
{
  Vector input_vector = NULL;   /* unit's input vector */
  Vector weight_vector = NULL;  /* unit's input weights vector */
  RValue new_activation;        /* unit's new activation */

  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_vector_activation",
                          "no unit to activate\n");

  /* Checks if the unit has inputs */
  if (unit->nu_inputs == 0)
    return error_failure ("nnet_unit_vector_activation",
                          "unit has no inputs\n");

  /* Creates the input and weight vectors */
  input_vector = vector_create (unit->nu_inputs);

  if (input_vector == NULL)
    return error_failure ("nnet_unit_vector_activation",
                          "error creating input vector\n");

  weight_vector = vector_create (unit->nu_inputs);

  if (weight_vector == NULL)
    {
      free (input_vector);
      return error_failure ("nnet_unit_vector_activation",
                            "error creating weight vector\n");
    }

  /* Populates the input vector */
  if (nnet_unit_get_input_vector (unit, input_vector) != EXIT_SUCCESS)
    {
      free (input_vector);
      free (weight_vector);
      return error_failure ("nnet_unit_vector_activation",
                            "error populating input vector\n");
    }

  /* Populates the weight vector */
  if (nnet_unit_get_weight_vector (unit, weight_vector) != EXIT_SUCCESS)
    {
      free (input_vector);
      free (weight_vector);
      return error_failure ("nnet_unit_vector_activation",
                            "error populating weight vector\n");
    }

  /* Calculates the new activation according to the metric */
  if (vector_metric (input_vector, weight_vector, NULL,
                     metric, &new_activation) != EXIT_SUCCESS)
    {
      free (input_vector);
      free (weight_vector);
      return error_failure ("nnet_unit_vector_activation",
                            "error calculating metric\n");
    }

  /* Updates the unit's activation */
  if (nnet_unit_set_activation (unit, new_activation) != EXIT_SUCCESS)
    {
      free (input_vector);
      free (weight_vector);
      return error_failure ("nnet_unit_vector_activation",
                            "error setting unit's activation\n");
    }

  /* Frees the auxiliary vectors */
  if (vector_destroy (&input_vector) != EXIT_SUCCESS)
    return error_failure ("nnet_unit_vector_activation",
                          "error destroying input vector\n");

  if (vector_destroy (&weight_vector) != EXIT_SUCCESS)
    return error_failure ("nnet_unit_vector_activation",
                          "error destroying weight vector\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_update_output
 *
 * Updates the output of the given unit according to the current
 * activation function and activation
 */
int
nnet_unit_update_output (Unit unit)
{
  /* Checks if a unit was passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_update_output", "no unit given\n");

  /* Updates the output */
  unit->output = nnet_actv_value (unit->activation_function,
                                  unit->activation);

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_activate
 *
 * Performs activation collection followed by output update, according to
 * the new activation.
 * If a vector metric is passed, calculates the new activation according
 * to this metric between the input and weight vectors.
 */
int
nnet_unit_activate (Unit unit, const VectorMetric metric)
{
  int exit_status;              /* auxiliary function return status */

  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_unit_activate", "no unit passed\n");

  /* If the unit has inputs, collects the activation */
  if (unit->nu_inputs > 0)
    {
      /*if (metric == (int) NULL) */
      if (metric == (VectorMetric) NULL)
        exit_status = nnet_unit_collect_activation (unit);
      else
        exit_status = nnet_unit_vector_activation (unit, metric);

      if (exit_status != EXIT_SUCCESS)
        return error_failure ("nnet_unit_activate",
                              "error collecting activation\n");
    }

  /* Updates the output */
  if (nnet_unit_update_output (unit) != EXIT_SUCCESS)
    return error_failure ("nnet_unit_activate", "error updating output\n");;

  return EXIT_SUCCESS;
}



/*
 * nnet_unit_index_to_coordinates
 *
 * Converts one unit index to its coordinates vector, according to
 * the given unit distribution and distance vectors
 */
Vector nnet_unit_index_to_coordinates
  (const UnitIndex index, const Vector coord_distribution,
   const Vector coord_increment)
{
  Vector coord;                 /* unit coordinates */
  UsLgIntValue dim;             /* vectors dimension */
  UsLgIntValue cur_dim;         /* current dimension */
  RValue cur_val;               /* current component value */
  RValue cur_incr;              /* current dimension increment */
  RValue aux_value;             /* auxiliary real value */
  UnitIndex nu_units;           /* number of units in dimension */
  UsLgIntValue comp_product;    /* accumulated component product */

  /* Checks if the distribution vector was actually passed */
  if (coord_distribution == NULL)
    return null_failure ("nnet_unit_index_to_coordinates",
                         "no distribution vector passed\n");

  /* Initialization */
  dim = coord_distribution->dimension;
  comp_product = 1;

  /* Checks dimensional compatibility */
  if (coord_increment != NULL)
    if (coord_increment->dimension != dim)
      return null_failure ("nnet_unit_index_to_coordinates",
                           "distribution vector has dimension %ld while increment vector has dimension %ld\n",
                           dim, coord_increment->dimension);

  /* Creates the coordinates vector */
  coord = vector_create (dim);

  if (coord == NULL)
    return null_failure ("nnet_unit_index_to_coordinates",
                         "error creating coordinates vector\n");

  /* Conversion loop */
  for (cur_dim = 1; cur_dim <= dim; cur_dim++)
    {
      /*
       * Gets the number of units in the current dimension
       * Gets the coordinates increment at the current dimension
       * Calculates the current dimension value
       * Sets the coordinates vector component
       * Updates the accumulated product of the number of units
       */
      if (vector_get_value (coord_distribution, cur_dim, &aux_value)
          != EXIT_SUCCESS)
        return null_failure ("nnet_unit_index_to_coordinates",
                             "error getting %ld-th coordinate from distribution vector\n",
                             cur_dim);

      nu_units = (UnitIndex) aux_value;

      if (nu_units == 0)
        return null_failure ("nnet_unit_index_to_coordinates",
                             "dimension %ld has zero units\n", cur_dim);

      if (coord_increment != NULL)
        {
          if (vector_get_value (coord_increment, cur_dim, &cur_incr)
              != EXIT_SUCCESS)
            return null_failure ("nnet_unit_index_to_coordinates",
                                 "error getting %ld-th coordinate from increment vector\n",
                                 cur_dim);
        }
      else
        {
          cur_incr = 1.0;
        }

      cur_val = (RValue) (((index - 1) / comp_product) % nu_units) * cur_incr;

      if (vector_set_value (coord, cur_dim, cur_val) != EXIT_SUCCESS)
        return null_failure ("nnet_unit_index_to_coordinates",
                             "error setting %ld-th component of the coordinates vector\n",
                             cur_dim);

      comp_product *= nu_units;
    }

  return coord;
}



/*
 * nnet_unit_info
 *
 * Outputs unit information
 */
void nnet_unit_info
  (const Unit unit,
   const BoolValue include_in_connections,
   const BoolValue include_out_connections, FILE * output_fd)
{
  Connection cur_conn;          /* auxiliary pointer to current connection */
  UnitIndex cur_index;          /* auxiliary connection counter */

  if (unit == NULL || output_fd == NULL)
    return;

  fprintf (output_fd, "Unit       : %ld\n", unit->unit_index);
  fprintf (output_fd, "Activation : %f\n", unit->activation);
  fprintf (output_fd, "Output     : %f\n", unit->output);
  fprintf (output_fd, "Function   : %s\n",
           unit->activation_function->function_class->name);
  fprintf (output_fd, "Fan-in     : %ld\n", unit->nu_inputs);
  fprintf (output_fd, "Fan-out    : %ld\n", unit->nu_outputs);
  fprintf (output_fd, "Average    : %f\n", unit->average);
  fprintf (output_fd, "Std. Dev.  : %f\n", unit->stddev);

  /* Coordinates */
  if (unit->coord != NULL)
    {
      fprintf (output_fd, "Coordinates: ");
      vector_info (unit->coord, output_fd);
    }

  /* Input connections */
  if (unit->nu_inputs > 0 && include_in_connections == TRUE)
    {
      cur_conn = unit->first_orig;
      cur_index = 1;
      while (cur_conn != NULL)
        {
          fprintf (output_fd, "\n");
          fprintf (output_fd, "Connection: %ld\n", cur_index);
          nnet_conn_info (cur_conn, TRUE, FALSE, output_fd);
          cur_conn = cur_conn->next_orig;
          ++cur_index;
        }
    }

  /* Output connections */
  if (unit->nu_outputs > 0 && include_out_connections == TRUE)
    {
      cur_conn = unit->first_dest;
      cur_index = 1;

      while (cur_conn != NULL)
        {
          fprintf (output_fd, "\n");
          fprintf (output_fd, "Connection: %ld\n", cur_index);
          nnet_conn_info (cur_conn, FALSE, TRUE, output_fd);
          cur_conn = cur_conn->next_dest;
          ++cur_index;
        }
    }
}
