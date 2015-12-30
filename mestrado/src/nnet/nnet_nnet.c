#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nnet_nnet.h"
#include "nnet_layers.h"
#include "nnet_actv.h"
#include "nnet_weights.h"
#include "nnet_train.h"
#include "som/nnet_som.h"
#include "../vector/vector.h"


#define __NNET_NU_EXTENSIONS_ 4

/* Names of the neural network extensions */
const char nnet_extension_name[][10] = {
  "Generic",
  "SOM",
  "LVQ",
  "MLP"
};



/*
 * nnet_extension_by_name
 *
 * Returns the neural network extension index according to the given name
 */
NExtensionIndex
nnet_extension_by_name (Name name)
{
  BoolValue index_found = FALSE;        /* index found flag */
  int cur_index = 0;            /* current layer class index */


  /* Searches the extension names */
  while (index_found == FALSE && cur_index < __NNET_NU_EXTENSIONS_)
    {
      if (strncmp
          (name, nnet_extension_name[cur_index],
           strlen (nnet_extension_name[cur_index])) == 0)
        index_found = TRUE;
      else
        ++cur_index;
    }

  if (index_found == FALSE)
    {
      fprintf (stderr,
               "nnet_extension_by_name: unknown neural network extension: %s\n",
               name);
      return NNEXT_GEN;
    }

  return cur_index;
}



/*
 * nnet_nnetwork_create
 *
 * Creates a new Neural Network
 */
NNetwork
nnet_nnetwork_create (const Name name, const NExtension extension)
{
  NNetwork new_nnet;            /* new neural network to be returned */
  int exit_status;              /* auxiliary function return status */

  /* Allocates the new nerual network */
  new_nnet = (NNetwork) malloc (sizeof (nnet_nnetwork_type));
  if (new_nnet == NULL)
    {
      fprintf (stderr, "nnet_create_nnetwork: virtual memory exhausted\n");
      return NULL;
    }

  /* Initializes its attributes */
  strcpy (new_nnet->name, name);
  new_nnet->nu_layers = 0;
  new_nnet->nu_units = 0;
  new_nnet->first_layer = NULL;
  new_nnet->last_layer = NULL;
  new_nnet->extension = extension;

  /* Extends the basic network */
  if (extension != NULL)
    {
      switch (extension->index)
        {
        case NNEXT_GEN:
          break;

        case NNEXT_SOM:
          exit_status = nnet_som_extend_nnetwork (new_nnet, extension);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_create_nnetwork: error extending neural network to SOM\n");
              return NULL;
            }
          break;

        case NNEXT_LVQ:
          fprintf (stderr,
                   "nnet_create_nnetwork: LVQ extensions not implemented yet\n");
          return NULL;
          break;

        case NNEXT_MLP:
          fprintf (stderr,
                   "nnet_create_nnetwork: MLP extensions not implemented yet\n");
          return NULL;
          break;

        default:
          fprintf (stderr, "nnet_create_nnetwork: unknown extension index\n");
          return NULL;
          break;
        }
    }

  return new_nnet;
}



/*
 * nnet_nnetwork_destroy
 *
 * Destroys a previously created Neural Network
 */
int
nnet_nnetwork_destroy (NNetwork * nnet,
                       const BoolValue cascade_extension,
                       const BoolValue cascade_layers,
                       const BoolValue cascade_units,
                       const BoolValue cascade_connections)
{
  NNetwork aux_nnet = *nnet;    /* pointed neural network */
  NExtension aux_ext = NULL;    /* auxiliary neural network extension */
  Layer aux_layer;              /* pointer to layer */
  int exit_status;              /* auxiliary function return status */


  /* Checks if a valid network was actually passed */
  if (nnet == NULL || aux_nnet == NULL)
    {
      fprintf (stderr, "nnet_nnetwork_destroy: no network to destroy\n");
      return EXIT_FAILURE;
    }

  /* Checks if the units can be destroyed */
  if (aux_nnet->nu_units > 0)
    {
      switch (cascade_units)
        {
        case TRUE:
          /* The layers will see if units can be destroyed */
          break;

        case FALSE:
          /* Don't destroy units */
          fprintf (stderr,
                   "nnet_nnetwork_destroy: neural network still has units\n");
          return EXIT_FAILURE;

        default:
          fprintf (stderr,
                   "nnet_nnetwork_destroy: invalid value for 'cascade_units'\n");
          return EXIT_FAILURE;
        }
    }

  /* Checks if the layers can be destroyed */
  if (aux_nnet->nu_layers > 0)
    {
      switch (cascade_layers)
        {
        case TRUE:
          while (aux_nnet->first_layer != NULL)
            {
              aux_layer = aux_nnet->last_layer;

              exit_status = nnet_layer_destroy
                (&aux_layer, cascade_units, cascade_connections);

              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "nnet_nnetwork_destroy: error destroying layer\n");
                  return EXIT_FAILURE;
                }
            }
          break;

        case FALSE:
          fprintf (stderr,
                   "nnet_nnetwork_destroy: neural network still has layers\n");
          return EXIT_FAILURE;

        default:
          fprintf (stderr,
                   "nnet_nnetwork_destroy: invalid value for 'cascade_layers'\n");
          return EXIT_FAILURE;
        }
    }

  /* Destroys the extension */
  if (aux_nnet->extension != NULL && cascade_extension == TRUE)
    {
      switch (aux_nnet->extension->index)
        {
        case NNEXT_GEN:
          /* Nothing to do */
          break;

        case NNEXT_SOM:
          aux_ext = aux_nnet->extension;
          exit_status =
            nnet_som_destroy (&aux_ext, FALSE, FALSE, FALSE, FALSE);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_nnetwork_destroy: error destroying SOM extension\n");
              return EXIT_FAILURE;
            }
          break;

        case NNEXT_LVQ:
          break;

        case NNEXT_MLP:
          break;

        default:
          fprintf (stderr, "nnet_nnetwork_destroy: invalid extension\n");
          return EXIT_FAILURE;
        }
    }


  /* Frees up the neural network */
  free (aux_nnet);

  /* Makes it point to NULL */
  *nnet = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_nnetwork_activate
 *
 * Activates all the layers in the neural network
 */
int
nnet_nnetwork_activate (NNetwork nnet, const VectorMetric vector_metric)
{
  Layer cur_layer = NULL;       /* current layer being activated */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the neural network was actually passed */
  if (nnet == NULL)
    {
      fprintf (stderr, "nnet_nnetwork_activate: no neural network passed\n");
      return EXIT_FAILURE;
    }

  /* Trivial case: no layers */
  if (nnet->nu_layers == 0)
    return EXIT_SUCCESS;

  /* Activates the layers */
  cur_layer = nnet->first_layer;

  while (cur_layer != NULL)
    {
      /* Activates the current layer */
      exit_status = nnet_layer_activate (cur_layer, vector_metric);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_nnetwork_activate: error activating layer %d\n",
                   cur_layer->layer_index);
          return EXIT_FAILURE;
        }

      /* Goes to the next layer */
      cur_layer = cur_layer->next;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_nnetwork_info
 *
 * Outputs information about the neural network
 */
void nnet_nnetwork_info
  (const NNetwork nnet,
   const BoolValue include_layers_info,
   const BoolValue include_units_info,
   const BoolValue include_in_connections,
   const BoolValue include_out_connections, FILE * output_fd)
{
  Layer cur_layer = NULL;       /* auxiliary layer pointer */


  if (output_fd == NULL)
    return;

  if (nnet == NULL)
    return;

  fprintf (output_fd, "Name      : %s\n", nnet->name);
  fprintf (output_fd, "Extension : %s\n",
           nnet_extension_name[nnet->extension->index]);
  fprintf (output_fd, "Layers    : %d\n", nnet->nu_layers);
  fprintf (output_fd, "Units     : %ld\n", nnet->nu_units);

  /* Optionally displays layers info */
  if (include_layers_info == TRUE && nnet->nu_layers > 0)
    {
      cur_layer = nnet->first_layer;

      while (cur_layer != NULL)
        {
          fprintf (output_fd, "\n");
          nnet_layer_info
            (cur_layer, include_units_info,
             include_in_connections, include_out_connections, output_fd);
          cur_layer = cur_layer->next;
        }
    }

  return;
}
