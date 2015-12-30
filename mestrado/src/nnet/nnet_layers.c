#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nnet_layers.h"
#include "nnet_units.h"
#include "nnet_actv.h"
#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                            LAYER IMPLEMENTATIONS                           *
 *                                                                            *
 ******************************************************************************/

#define __NNET_LAYER_NU_CLASSES_ 4

/*
 * nnet_layer_class_name
 *
 * Description of the layer classes
 */
const char nnet_layer_class_name[__NNET_LAYER_NU_CLASSES_][10] = {
  "Undefined",
  "Input",
  "Hidden",
  "Output"
};



/*
 * Layer Classes
 */

/* Input layer class */
nnet_layer_class_type nnet_layer_class_input = {
  NNET_LAYER_INPUT,
  NULL
};

/* Hidden layer class */
nnet_layer_class_type nnet_layer_class_hidden = {
  NNET_LAYER_HIDDEN,
  NULL
};

/* Output layer class */
nnet_layer_class_type nnet_layer_class_output = {
  NNET_LAYER_OUTPUT,
  NULL
};



/******************************************************************************
 *                                                                            *
 *                              PRIVATE FUNCTIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * layer_vector_type
 *
 * Possible vectors to extract from a layer
 */
typedef enum
{
  VECTOR_ACTIVATION = 0,
  VECTOR_OUTPUT = 1
}
layer_vector_type;



/*
 * nnet_layer_correct_indexes
 *
 * Corrects the layer indexes after creation or removal of a layer
 * in other position than the last
 */
static void nnet_layer_correct_indexes
  (Layer first_layer, const LayerIndex first_layer_index)
{
  Layer cur_layer;              /* auxiliary pointer to the current layer */
  LayerIndex cur_index;         /* current layer counter */

  cur_layer = first_layer;
  cur_index = first_layer_index;

  while (cur_layer != NULL)
    {
      cur_layer->layer_index = cur_index;
      cur_layer = cur_layer->next;
      ++cur_index;
    }

  return;
}



/*
 * nnet_layer_vectors
 *
 * Returns attributes of the layer's units as a vector
 */
static int nnet_layer_vectors
  (const Layer layer, Vector vector, const layer_vector_type vector_type)
{
  Unit cur_unit;                /* layer's current unit */
  UsLgIntValue cur_comp;        /* vector's current component */
  RValue value;                 /* current unit's activation */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the layer and the vector were actually passed */
  if (layer == NULL)
    {
      fprintf (stderr, "nnet_layer_vectors: no layer passed\n");
      return EXIT_FAILURE;
    }

  if (vector == NULL)
    {
      fprintf (stderr, "nnet_layer_vectors: no vector for output\n");
      return EXIT_FAILURE;
    }

  /* Checks dimension compatibility */
  if (layer->nu_units != vector->dimension)
    {
      fprintf (stderr,
               "nnet_layer_vectors: incompatible dimensions: layer has %ld units while vector has dimension %ld\n",
               layer->nu_units, vector->dimension);
      return EXIT_FAILURE;
    }

  /* Vector filling */
  cur_unit = layer->first_unit;
  cur_comp = 1;

  while (cur_unit != NULL)
    {
      /* Chooses the value to get from the layer */
      switch (vector_type)
        {
        case VECTOR_ACTIVATION:
          value = cur_unit->activation;
          break;

        case VECTOR_OUTPUT:
          value = cur_unit->output;
          break;

        default:
          fprintf (stderr, "nnet_layer_vectors: unknown vector to extract\n");
          return EXIT_FAILURE;
        }

      /* Sets the vector component as the unit's activation */
      exit_status = vector_set_value (vector, cur_comp, value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_layer_vectors: error setting vector component %ld value\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Goes to the next unit and to the next component */
      cur_comp++;
      cur_unit = cur_unit->next;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                               PUBLIC FUNCTIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_layer_class_by_name
 *
 * Returns the layer class variable associated to the given name
 */
LayerClass
nnet_layer_class_by_name (Name name)
{
  BoolValue index_found = FALSE;        /* index found flag */
  int cur_index = 0;            /* current layer class index */

  /* Searches the class names */
  while (index_found == FALSE && cur_index < __NNET_LAYER_NU_CLASSES_)
    {
      if (strncmp
          (name, nnet_layer_class_name[cur_index],
           strlen (nnet_layer_class_name[cur_index])) == 0)
        index_found = TRUE;
      else
        ++cur_index;
    }

  if (index_found == FALSE)
    {
      fprintf (stderr, "nnet_layer_class_by_name: unknown layer class\n");
      return NULL;
    }

  /* Selects the appropriate layer class */
  switch (cur_index)
    {
    case NNET_LAYER_UNDEFINED:
      return NULL;

    case NNET_LAYER_INPUT:
      return &nnet_layer_class_input;

    case NNET_LAYER_HIDDEN:
      return &nnet_layer_class_hidden;

    case NNET_LAYER_OUTPUT:
      return &nnet_layer_class_output;

    default:
      fprintf (stderr, "nnet_layer_class_by_name: invalid layer class\n");
      return NULL;
    }
};



/*
 * nnet_layer_create
 *
 * Creates a layer, given the parameters.
 * If the layer already exists, prints the corresponding message and returns.
 */
Layer nnet_layer_create
  (NNetwork nnet,
   LayerIndex * layer_index, LayerClass layer_class, const Name name)
{
  Layer new_layer;              /* returned layer */
  int exit_status;              /* auxiliary function return status */


  /* Allocates the new layer */
  new_layer = (Layer) malloc (sizeof (nnet_layer_type));
  if (new_layer == NULL)
    {
      fprintf (stderr, "nnet_layer_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Associates the default activation classes */
  nnet_layer_class_input.default_activation_class =
    nnet_actv_class_passthrough;
  nnet_layer_class_hidden.default_activation_class =
    nnet_actv_class_sigmoidal;
  nnet_layer_class_output.default_activation_class = nnet_actv_class_linear;

  /* Initializes the new layer */
  new_layer->nnetwork = NULL;
  new_layer->layer_index = 0;
  new_layer->layer_class = layer_class;
  new_layer->nu_units = 0;
  strcpy (new_layer->name, name);
  new_layer->first_unit = NULL;
  new_layer->last_unit = NULL;
  new_layer->next = NULL;

  /* If a neural network was passed, attach the layer to it */
  if (nnet != NULL || layer_index != NULL)
    {
      exit_status =
        nnet_layer_attach_to_nnetwork (new_layer, nnet, layer_index);
      if (exit_status != EXIT_SUCCESS)
        {
          free (new_layer);
          fprintf (stderr,
                   "nnet_layer_create: error adding layer to the array\n");
          return NULL;
        }
    }

  return new_layer;
}



/*
 * nnet_layer_destroy
 *
 * Destroy a previously created layer
 */
int nnet_layer_destroy
  (Layer * layer,
   const BoolValue cascade_units, const BoolValue cascade_connections)
{
  Layer aux_layer = *layer;     /* auxiliary pointer to the real layer */
  Unit aux_unit;                /* auxiliary pointer to unit */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the layer was actually passed */
  if (layer == NULL || aux_layer == NULL)
    {
      fprintf (stderr, "nnet_layer_destroy: no layer to destroy\n");
      return EXIT_FAILURE;
    }

  /* Cascade layer units */
  if (aux_layer->nu_units > 0)
    {
      switch (cascade_units)
        {
        case TRUE:

          while (aux_layer->first_unit != NULL)
            {
              /* Destroys the first unit */
              aux_unit = aux_layer->first_unit;
              exit_status =
                nnet_unit_destroy (&aux_unit, cascade_connections);

              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "nnet_layer_destroy: error destroying unit\n");
                  return EXIT_FAILURE;
                }
            }

          break;


        case FALSE:
          fprintf (stderr, "nnet_layer_destroy: layer still has units\n");
          return EXIT_FAILURE;

        default:
          fprintf (stderr,
                   "nnet_layer_destroy: invalid value for 'cascade_units'\n");
          return EXIT_FAILURE;
        }
    }

  /* Removes the layer is attached to a neural network, dettaches from it */
  if (aux_layer->nnetwork != NULL)
    {
      exit_status = nnet_layer_remove_from_nnetwork (aux_layer);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_layer_destroy: error removing layer from index\n");
          return EXIT_FAILURE;
        }
    }

  /* Frees up the layer */
  free (*layer);

  /* Makes it point to NULL */
  *layer = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_attach_to_nnetwork
 *
 * Adds a new layer to the layer array
 */
int nnet_layer_attach_to_nnetwork
  (Layer layer, NNetwork nnet, LayerIndex * layer_index)
{
  Layer aux_layer;              /* auxiliary pointer to layer */
  LayerIndex cur_layer;         /* current layer counter */
  LayerIndex new_index;         /* auxiliary layer index */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the layer was passed */
  if (layer == NULL)
    {
      fprintf (stderr, "nnet_layer_attach_to_nnetwork: no layer to attach\n");
      return EXIT_FAILURE;
    }

  /* Checks if the destination network was passed */
  if (nnet == NULL)
    {
      fprintf (stderr,
               "nnet_layer_attach_to_nnetwork: no destination network\n");
      return EXIT_FAILURE;
    }

  /* If the layer index was passed NULL, attaches to the end of the network */
  if (layer_index == NULL)
    new_index = nnet->nu_layers + 1;
  else
    new_index = *layer_index;

  /* Checks the domain of the index: 1's logic */
  if (new_index < 1 || new_index > nnet->nu_layers + 1)
    {
      fprintf (stderr, "nnet_layer_attach_to_nnetwork: invalid position\n");
      return EXIT_FAILURE;
    }

  /* If the layer was attached to another neural network, dettaches it */
  if (layer->nnetwork != NULL)
    {
      /* Checks if the network or position is different */
      if (layer->nnetwork != nnet || layer->layer_index != new_index)
        {
          /* Removes from the neural network */
          exit_status = nnet_layer_remove_from_nnetwork (layer);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_layer_attach_to_network: error removing from old network\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Attaching to the same network: nothing to do */
          return EXIT_SUCCESS;
        }
    }

  if (new_index == 1)
    {
      /* Inserts at the head */
      layer->next = nnet->first_layer;
      nnet->first_layer = layer;
    }
  else
    {
      /* Goes to one layer before the requested position */
      if (new_index == nnet->nu_layers + 1)
        aux_layer = nnet->last_layer;
      else
        {
          aux_layer = nnet->first_layer;

          for (cur_layer = 1; cur_layer < new_index - 1; cur_layer++)
            aux_layer = aux_layer->next;
        }

      /* Inserts the layer */
      layer->next = aux_layer->next;
      aux_layer->next = layer;
    }

  /* Increments the number of layers and units in the network */
  ++nnet->nu_layers;
  nnet->nu_units += layer->nu_units;

  /* Updates the last layer of the neural network */
  if (new_index == nnet->nu_layers)
    nnet->last_layer = layer;

  /* If necessary, corrects the indexes of the other layers */
  if (new_index != nnet->nu_layers)
    nnet_layer_correct_indexes (layer, new_index);

  /* Sets the new layer index: 1's logic - first layer is number one */
  layer->nnetwork = nnet;
  layer->layer_index = new_index;

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_remove_from_nnetwork
 *
 * Removes a layer from the layer array
 */
int
nnet_layer_remove_from_nnetwork (Layer layer)
{
  NNetwork nnet;                /* auxiliary layer array */
  Layer aux_layer;              /* auxiliary pointer to layer */


  /* Checks if the layer was passed */
  if (layer == NULL)
    {
      fprintf (stderr,
               "nnet_layer_remove_from_network: no layer to remove\n");
      return EXIT_FAILURE;
    }

  /* Verifies if the layer is attached to a network */
  if (layer->nnetwork == NULL)
    {
      /* The layer is not attached to a network: nothing to do */
      return EXIT_SUCCESS;
    }
  else
    nnet = layer->nnetwork;

  /* Goes to the layer before */
  if (layer == nnet->first_layer)
    {
      nnet->first_layer = layer->next;
      aux_layer = NULL;
    }
  else
    {
      aux_layer = nnet->first_layer;

      while (aux_layer->next != layer && aux_layer != NULL)
        aux_layer = aux_layer->next;

      /* Checks if the layer is actually in the neural network */
      if (aux_layer == NULL)
        {
          fprintf (stderr,
                   "nnet_layer_remove_from_nnetwork: layer not found in the neural network\n");
          return EXIT_FAILURE;
        }

      /* Removes the layer */
      aux_layer->next = layer->next;
    }

  /* Updates the last layer of the neural network */
  if (layer == nnet->last_layer)
    nnet->last_layer = aux_layer;

  /* Corrects the indexes of the other layers */
  if (layer->layer_index != nnet->nu_layers)
    nnet_layer_correct_indexes (layer->next, layer->layer_index);

  /* Decrements the number of layers and units in the network */
  --nnet->nu_layers;
  nnet->nu_units -= layer->nu_units;

  /* Unsets the layer network attributes */
  layer->nnetwork = NULL;
  layer->layer_index = 0;
  layer->next = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_by_index
 *
 * Returns a layer given its neural network and its layer index
 */
Layer
nnet_layer_by_index (NNetwork nnet, LayerIndex index)
{
  Layer cur_layer = NULL;       /* current layer */
  BoolValue layer_found = FALSE;        /* layer found flag */


  /* Checks if the neural network was actually passed */
  if (nnet == NULL)
    {
      fprintf (stderr, "nnet_layer_by_index: no neural network passed\n");
      return NULL;
    }

  /* Trivial case: network has no layers */
  if (nnet->nu_layers == 0)
    {
      fprintf (stderr, "nnet_layer_by_index: neural network has no layers\n");
      return NULL;
    }

  /* Searches for the layer */
  cur_layer = nnet->first_layer;

  while (layer_found == FALSE && cur_layer != NULL)
    {
      if (cur_layer->layer_index == index)
        layer_found = TRUE;
      else
        cur_layer = cur_layer->next;
    }

  if (layer_found == FALSE)
    {
      fprintf (stderr, "nnet_layer_by_index: layer not found with index %d\n",
               index);
      return NULL;
    }

  return cur_layer;
}



/*
 * nnet_layer_load_vector
 *
 * Applies the given vector as activations for the given layer
 */
int
nnet_layer_load_vector (const Vector vector, Layer layer)
{
  Unit cur_unit;                /* current unit of the layer */
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* value at the current component */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vector was actually passed */
  if (vector == NULL)
    {
      fprintf (stderr, "nnet_layer_load_vector: no vector to apply\n");
      return EXIT_FAILURE;
    }

  /* Checks if the layer was actually passed */
  if (layer == NULL)
    {
      fprintf (stderr, "nnet_layer_load_vector: no layer to load\n");
      return EXIT_FAILURE;
    }

  /* Checks the compatibility of the dimension of the vector and
   * the number of units in the layer */
  if (vector->dimension != layer->nu_units)
    {
      fprintf (stderr,
               "nnet_layer_load_vector: vector has dimension %ld while layer has %ld units\n",
               vector->dimension, layer->nu_units);
      return EXIT_FAILURE;
    }

  /* Applies the values to the activations of the units */
  cur_unit = layer->first_unit;
  cur_comp = 1;

  while (cur_unit != NULL)
    {
      /* Gets the value from the vector */
      exit_status = vector_get_value (vector, cur_comp, &cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_layer_load_vector: error getting value of component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Sets the current unit's activation */
      exit_status = nnet_unit_set_activation (cur_unit, cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_layer_load_vector: error updating unit's activation\n");
          return EXIT_FAILURE;
        }

      /* Moves on to the next unit */
      cur_unit = cur_unit->next;
      ++cur_comp;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_activation_vector
 *
 * Returns the activations of the layer's units as a vector
 */
int
nnet_layer_activation_vector (const Layer layer, Vector activation_vector)
{
  int exit_status;              /* auxiliary function return status */

  exit_status =
    nnet_layer_vectors (layer, activation_vector, VECTOR_ACTIVATION);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_layer_activation_vector: error extracting activation vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_output_vector
 *
 * Returns the outputs of the layer's units as a vector
 */
int
nnet_layer_output_vector (const Layer layer, Vector output_vector)
{
  int exit_status;              /* auxiliary function return status */

  exit_status = nnet_layer_vectors (layer, output_vector, VECTOR_OUTPUT);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_layer_output_vector: error extracting output vector\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_activate
 *
 * Activates all units of the given layer with the given vector metric
 */
int
nnet_layer_activate (Layer layer, const VectorMetric metric)
{
  Unit cur_unit;                /* pointer to the current unit */
  int exit_status;              /* auxiliary funciton return status */

  /* Checks if the layer was actually passed */
  if (layer == NULL)
    {
      fprintf (stderr, "nnet_layer_activate: no layer to activate\n");
      return EXIT_FAILURE;
    }

  /* Activates the units */
  cur_unit = layer->first_unit;

  while (cur_unit != NULL)
    {
      exit_status = nnet_unit_activate (cur_unit, metric);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "nnet_layer_activate: error activating unit\n");
          return EXIT_FAILURE;
        }

      cur_unit = cur_unit->next;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_layer_class_info
 *
 * Outputs layer class information
 */
void
nnet_layer_class_info (const LayerClass layer_class, FILE * output_fd)
{
  if (output_fd == NULL)
    return;

  fprintf (output_fd, "Layer Class: %s\n",
           nnet_layer_class_name[layer_class->position]);
  fprintf (output_fd, "Default Activation: %s\n",
           layer_class->default_activation_class->name);

  return;
}



/*
 * nnet_layer_info
 *
 * Outputs information about the layer
 */
void nnet_layer_info
  (const Layer layer,
   const BoolValue include_units_info,
   const BoolValue include_in_connections,
   const BoolValue include_out_connections, FILE * output_fd)
{
  Unit cur_unit;                /* auxiliary unit pointer */


  if (output_fd == NULL || layer == NULL)
    return;

  fprintf (output_fd, "Layer     : %d\n", layer->layer_index);
  fprintf (output_fd, "Name      : %s\n", layer->name);
  fprintf (output_fd, "Class     : %s\n",
           nnet_layer_class_name[layer->layer_class->position]);
  fprintf (output_fd, "Units     : %ld\n", layer->nu_units);

  /* Optionally displays units info */
  if (include_units_info == TRUE && layer->nu_units > 0)
    {
      cur_unit = layer->first_unit;

      while (cur_unit != NULL)
        {
          fprintf (output_fd, "\n");
          nnet_unit_info (cur_unit, include_in_connections,
                          include_out_connections, output_fd);
          cur_unit = cur_unit->next;
        }
    }

  return;
}
