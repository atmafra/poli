#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "errorh.h"
#include "strutils.h"
#include "trmap.h"

#include "nnet_lvq.h"
#include "nnet_lvq_window.h"
#include "nnet_train.h"
#include "nnet_nnet.h"
#include "nnet_layers.h"
#include "nnet_units.h"
#include "nnet_actv.h"
#include "nnet_conns.h"
#include "nnet_metrics.h"

/******************************************************************************
 *                                                                            *
 *                             PRIVATE OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_right_classes
 *
 * Checks if the classes of the winner and second place units are correct
 */
static void
nnet_lvq_right_classes (const Unit winner1, const Unit winner2,
                        const UsLgIntValue units_per_class,
                        const UsLgIntValue desired_class,
                        UsLgIntValue * winner1_class,
                        UsLgIntValue * winner2_class,
                        BoolValue * right1, BoolValue * right2)
{
  ldiv_t aux_div;               /* auxiliary integer division */

  aux_div = ldiv (winner1->unit_index - 1, units_per_class);
  *winner1_class = aux_div.quot + 1;

  if (*winner1_class == desired_class)
    *right1 = TRUE;
  else
    *right1 = FALSE;

  if (winner2 != NULL)
    {
      aux_div = ldiv (winner2->unit_index - 1, units_per_class);
      *winner2_class = aux_div.quot + 1;

      if (*winner2_class == desired_class)
        *right2 = TRUE;
      else
        *right2 = FALSE;
    }
}



/*
 * nnet_lvq_update
 *
 * Updates one of the winners vectors
 */
static int
nnet_lvq_update (Unit unit, Vector err, Vector wgt, RValue etha,
                 BoolValue match)
{
  RValue rate = 0.0;            /* learning rate */

  if (match == TRUE)
    rate = etha;
  else
    rate = -etha;

  /* multiplies the error vector by the correction rate */
  if (vector_scalar_multiply (err, rate, err) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_update",
                          "error multiplying error vector in LVQ\n");

  /* calculates the new weights */
  if (vector_sum (wgt, err, wgt) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_update",
                          "error calculating new weight vector\n");

  /* normalizes the weight vector */
  /*
     if (vector_normalize (wgt, NULL) != EXIT_SUCCESS)
     return error_failure ("nnet_lvq_update",
     "error normalizing new weight vector\n");
   */

  /* updates the weights */
  if (nnet_unit_set_weight_vector (unit, wgt) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_update",
                          "error updating unit's weights\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_adapt
 *
 * Updates winning units weight vectors according to the LVQ algorithm
 */
static int
nnet_lvq_adapt (Unit winner1, Unit winner2, Vector input,
                LvqAlgorithmType algorithm, RValue etha,
                RValue window_width, RValue epsilon,
                BoolValue right1, BoolValue right2)
{
  static UsLgIntValue dim = 0;  /* dimension of the vectors */
  static Vector wv1 = NULL;     /* weight vector for winner 1 */
  static Vector wv2 = NULL;     /* weight vector for winner 2 */
  static Vector er1 = NULL;     /* error vector for winner 1 */
  static Vector er2 = NULL;     /* error vector for winner 2 */
  static LvqAlgorithmType ialg = LVQ_1; /* internal LVQ algorithm */
  static BoolValue uses_w2 = FALSE;     /* algorithm uses winner 2? */
  BoolValue update_winner2 = FALSE;     /* update winner 2? */
  RValue rate = 0.0;            /* learning rate */

  /* defines if the algorithm uses winner 2 */
  if (algorithm != ialg)
    {
      switch (algorithm)
        {
        case LVQ_1:
        case OLVQ_1:
          uses_w2 = FALSE;
          break;

        case LVQ_2_1:
        case LVQ_3:
          uses_w2 = TRUE;
          break;

        default:
          return error_failure ("nnet_lvq_adapt", "unknown LVQ algorithm\n");
        }

      ialg = algorithm;
    }

  /* destroys the vectors */
  if (input == NULL)
    {
      vector_destroy (&wv1);
      vector_destroy (&wv2);
      if (uses_w2 == TRUE)
        {
          vector_destroy (&er1);
          vector_destroy (&er2);
        }
      dim = 0;
      return EXIT_SUCCESS;
    }

  /* checks if everything was passed */
  if (winner1 == NULL)
    return error_failure ("nnet_lvq_adapt", "winner unit not passed\n");

  if (uses_w2 == TRUE && winner2 == NULL)
    return error_failure ("nnet_lvq_adapt",
                          "second winner unit not passed\n");


  /* creates the vectors */
  if (input->dimension != dim)
    {
      wv1 = vector_create (input->dimension);
      er1 = vector_create (input->dimension);

      if (wv1 == NULL || er1 == NULL)
        {
          error_failure ("nnet_lvq_adapt",
                         "error creating internal vectors\n");
          goto destroy_n_failure;
        }

      if (uses_w2 == TRUE)
        {
          wv2 = vector_create (input->dimension);
          er2 = vector_create (input->dimension);

          if (wv2 == NULL || er2 == NULL)
            {
              error_failure ("nnet_lvq_adapt",
                             "error creating internal vectors\n");
              goto destroy_n_failure;
            }
        }

      dim = input->dimension;
    }

  /* weight vectors */
  if (nnet_unit_get_weight_vector (winner1, wv1) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_adapt",
                          "error getting weight vector for first winner\n");
  if (uses_w2 == TRUE)
    if (nnet_unit_get_weight_vector (winner2, wv2) != EXIT_SUCCESS)
      return error_failure ("nnet_lvq_adapt",
                            "error getting weight vector for second winner\n");

  /* error vectors */
  if (vector_subtract (input, wv1, er1) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_adapt",
                          "error getting error vector for first winner\n");

  if (uses_w2 == TRUE)
    if (vector_subtract (input, wv2, er2) != EXIT_SUCCESS)
      return error_failure ("nnet_lvq_adapt",
                            "error getting error vector for second winner\n");

  /* weight adaption */
  switch (algorithm)
    {
    case LVQ_1:

      if (nnet_lvq_update (winner1, er1, wv1, etha, right1) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_adapt",
                              "error updating winner's weight vector\n");
      break;


    case LVQ_2_1:

      if (nnet_lvq_update (winner1, er1, wv1, etha, right1) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_adapt",
                              "error updating winner's weight vector\n");

      if ((right1 == TRUE && right2 == FALSE) ||
          (right1 == FALSE && right2 == TRUE))
        {
          if (nnet_lvq_window (input, winner1, winner2, wv1, wv2,
                               window_width, &update_winner2) != EXIT_SUCCESS)
            return error_failure ("nnet_lvq_adapt",
                                  "error checking second winner in window\n");

          if (update_winner2 == TRUE)
            if (nnet_lvq_update (winner2, er2, wv2, etha, right2) !=
                EXIT_SUCCESS)
              return error_failure ("nnet_lvq_adapt",
                                    "error updating second winner's weight vector\n");
        }
      break;


    case LVQ_3:

      if (right1 == TRUE && right2 == TRUE)
        {
          rate = etha * epsilon;

          if (nnet_lvq_update (winner2, er2, wv2, etha, right2) !=
              EXIT_SUCCESS)
            return error_failure ("nnet_lvq_adapt",
                                  "error updating second winner's weight vector\n");
        }
      else
        {
          rate = etha;
        }

      if (nnet_lvq_update (winner1, er1, wv1, rate, right1) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_adapt",
                              "error updating winner's weight vector\n");

      if ((right1 == TRUE && right2 == FALSE) ||
          (right1 == FALSE && right2 == TRUE))
        {
          if (nnet_lvq_window (input, winner1, winner2, wv1, wv2,
                               window_width, &update_winner2) != EXIT_SUCCESS)
            return error_failure ("nnet_lvq_adapt",
                                  "error checking second winner in window\n");

          if (update_winner2 == TRUE)
            if (nnet_lvq_update (winner2, er2, wv2, etha, right2) !=
                EXIT_SUCCESS)
              return error_failure ("nnet_lvq_adapt",
                                    "error updating second winner's weight vector\n");
        }
      break;

    case OLVQ_1:
      return error_failure ("nnet_lvq_adapt",
                            "algorithm OLVQ 1 not implemented yet\n");

    default:
      return error_failure ("nnet_lvq_adapt", "unknown LVQ algorithm\n");
    }

  return EXIT_SUCCESS;

destroy_n_failure:

  if (wv1 != NULL)
    vector_destroy (&wv1);

  if (wv2 != NULL)
    vector_destroy (&wv2);

  if (er1 != NULL)
    vector_destroy (&er1);

  if (er2 != NULL)
    vector_destroy (&er2);

  dim = 0;

  return EXIT_FAILURE;
}



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_str_to_algorithm
 *
 * Converts a string into its corresponding LVQ learning algorithm
 */
int
nnet_lvq_str_to_algorithm (const char *str, LvqAlgorithmType * algorithm)
{
  if (str == NULL)
    return error_failure ("nnet_lvq_str_to_algorithm", "no string passed\n");

  if (strcmp (str, __LVQ_1_STRING_) == 0)
    {
      *algorithm = LVQ_1;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __LVQ_2_1_STRING_) == 0)
    {
      *algorithm = LVQ_2_1;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __LVQ_3_STRING_) == 0)
    {
      *algorithm = LVQ_3;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __OLVQ_1_STRING_) == 0)
    {
      *algorithm = OLVQ_1;
      return EXIT_SUCCESS;
    }

  return error_failure ("nnet_lvq_str_to_algorithm",
                        "unknown LVQ algorithm: '%s'\n", str);
}



/*
 * nnet_lvq_algorithm_to_str
 *
 * Returns the string associated to the given algorithm
 */
char *
nnet_lvq_algorithm_to_str (const LvqAlgorithmType algorithm)
{
  switch (algorithm)
    {
    case LVQ_1:
      return __LVQ_1_STRING_;

    case LVQ_2_1:
      return __LVQ_2_1_STRING_;

    case LVQ_3:
      return __LVQ_3_STRING_;

    case OLVQ_1:
      return __OLVQ_1_STRING_;

    default:
      return null_failure ("nnet_lvq_algorithm_to_str",
                           "unknown LVQ algorithm\n");
    }
}



/*
 * nnet_lvq_create
 *
 * Creates a new LVQ neural network architecture extension
 */
LvqNNetwork
nnet_lvq_create (const NNetwork nnet,
                 const LvqAlgorithmType lvq_algorithm,
                 const LRateFunctionClass lrate_class,
                 const LRateFunctionParameters lrate_parameters,
                 const VectorMetric activation_metric,
                 const UnitIndex output_classes,
                 const UnitIndex units_per_class,
                 const RValue window_width, const RValue epsilon)
{
  LvqNNetwork new_lvq = NULL;   /* new LVQ neural network */
  LvqAttributes lvq_attr = NULL;        /* new LVQ attributes */


  /* Checks if the learning rate function class was passed */
  if (lrate_class == NULL)
    return null_failure ("nnet_lvq_create",
                         "no learning rate function class passed\n");

  /* validates units per class */
  if (units_per_class == 0)
    return null_failure ("nnet_lvq_create",
                         "LVQ must have at least one unit per class\n");

  /* window width */
  if ((lvq_algorithm == LVQ_2_1 || lvq_algorithm == LVQ_3) &&
      (window_width < DBL_EPSILON || window_width > 1.0 - DBL_EPSILON))
    return null_failure ("nnet_lvq_create",
                         "window width must be in range 0.0 .. 1.0\n");

  /* epsilon */
  if (lvq_algorithm == LVQ_3 &&
      (epsilon < DBL_EPSILON || epsilon > 1.0 - DBL_EPSILON))
    return null_failure ("nnet_lvq_create",
                         "epsilon must be in range 0.0 .. 1.0\n");

  /* Creates the new LVQ attribute set */
  lvq_attr = (LvqAttributes) malloc (sizeof (nnet_lvq_attr_type));
  if (lvq_attr == NULL)
    return null_failure ("nnet_lvq_create", "virtual memory exhausted\n");

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

  /* sets the LVQ activation metric */
  switch (activation_metric)
    {
    case VECTOR_METR_EUCLIDEAN:
    case VECTOR_METR_INNER_PRODUCT:
      lvq_attr->activation_metric = activation_metric;
      break;

    default:
      return null_failure ("nnet_lvq_create", "unknown activation metric\n");
    }

  /* sets the other attributes */
  lvq_attr->output_classes = output_classes;
  lvq_attr->units_per_class = units_per_class;
  lvq_attr->window_width = window_width;
  lvq_attr->epsilon = epsilon;

  /* Creates the LVQ extension */
  new_lvq = (LvqNNetwork) malloc (sizeof (nnet_extension_type));
  if (new_lvq == NULL)
    return null_failure ("nnet_lvq_create", "virtual memory exhausted\n");

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
    return error_failure ("nnet_lvq_extend_nnetwork",
                          "no LVQ extension passed\n");

  /* Checks if the neural network was passed */
  if (nnet == NULL)
    return error_failure ("nnet_lvq_extend_nnetwork",
                          "no neural network passed\n");

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
                          const UnitIndex output_classes,
                          const UnitIndex units_per_class,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters)
{
  NNetwork new_nnet = NULL;     /* new associated generic neural network */
  Layer input_layer = NULL;     /* LVQ input layer */
  Layer output_layer = NULL;    /* LVQ output layer */
  UnitIndex nu_outputs = 0;     /* number of output units */


  /* Checks if the LVQ extension was actually passed */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "no LVQ extension passed\n");

  /* checks the number of output units */
  if (output_classes == 0)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "LVQ must have at least one output class\n");

  if (units_per_class == 0)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "LVQ must have at least one output unit per class\n");

  nu_outputs = output_classes * units_per_class;

  /* Creates the new generic neural network */
  new_nnet = nnet_nnetwork_create (nnet_name, lvq_nnet);
  if (new_nnet == NULL)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error creating new LVQ neural network\n");

  /* Creates the input layer */
  input_layer = nnet_layer_create (new_nnet,
                                   NULL,
                                   &nnet_layer_class_input, input_layer_name);
  if (input_layer == NULL)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error creating input layer\n");

  /* Creates the output layer */
  output_layer = nnet_layer_create (new_nnet,
                                    NULL,
                                    &nnet_layer_class_output,
                                    output_layer_name);
  if (output_layer == NULL)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error creating output layer\n");

  /* Creates the input layer's units */
  if (nnet_unit_create_multiple (nu_inputs, input_layer,
                                 nnet_actv_class_passthrough,
                                 NULL, FALSE, TRUE, NULL, NULL, 0.0, 0.0)
      != EXIT_SUCCESS)

    return error_failure ("nnet_lvq_create_nnetwork",
                          "error creating input units\n");

  /* Creates the output layer's units */
  if (nnet_unit_create_multiple (nu_outputs, output_layer,
                                 nnet_actv_class_linear,
                                 NULL, FALSE, TRUE, NULL, NULL, 0.0, 0.0)
      != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error creating output units\n");

  /* Fully connects the input layer to the output layer */
  if (nnet_conn_connect_layers (input_layer, output_layer, 0.0,
                                weight_init_class, weight_init_parameters)
      != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error connecting input layer to output layer\n");

  /* Attaches the new neural network to the given LVQ extension */
  if (nnet_lvq_extend_nnetwork (new_nnet, lvq_nnet) != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_create_nnetwork",
                          "error attaching generic neural network\n");

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_attach_class_table
 *
 * Attaches an output classes table to the given LVQ
 */
int
nnet_lvq_attach_class_table (LvqNNetwork lvq_nnet, const Table class_table)
{
  LvqAttributes lvq_attr = NULL;        /* LVQ's attributes */
  UsLgIntValue output_classes = 0;      /* number of output classes */

  /* checks if the LVQ was passed */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_attach_class_table",
                          "no LVQ neural network passed\n");

  /* checks if the class table was passed */
  if (class_table == NULL)
    return error_failure ("nnet_lvq_attach_class_table",
                          "no class table passed\n");

  if (class_table->rows == 0)
    return error_failure ("nnet_lvq_create", "empty class table\n");

  /* initialization */
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  output_classes = lvq_attr->output_classes;

  /* validates output classes */
  if (output_classes != class_table->rows)
    return error_failure ("nnet_lvq_create",
                          "incompatible number of classes between network (%ld) and class table (%ld)\n",
                          output_classes, class_table->rows);

  /* attaches the class table */
  lvq_attr->class_table = class_table;

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


  /* checks if the LVQ extension was actually passed */
  if (lvq_nnet == NULL || *lvq_nnet == NULL)
    return error_failure ("nnet_lvq_destroy",
                          "no LVQ neural network to destroy\n");

  /* destroys the associated generic neural network */
  if ((*lvq_nnet)->nnet != NULL && cascade_nnetwork == TRUE)
    {
      if (nnet_nnetwork_destroy (&((*lvq_nnet)->nnet),
                                 TRUE, cascade_layers, cascade_units,
                                 cascade_connections) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_destroy",
                              "error destroying associated generic neural network\n");
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
 * nnet_lvq_init_weights
 *
 * Initializes LVQ's weights according to the training set
 */
int
nnet_lvq_init_weights (const LvqNNetwork lvq_nnet, const TSet set)
{
  NNetwork nnet = NULL;         /* LVQ attached neural network */
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  Layer out_layer = NULL;       /* network's output layer */
  Unit unit = NULL;             /* output layer current unit */
  UsLgIntValue units_per_class; /* units per class */
  BoolValue dummy_bool = FALSE; /* auxiliary boolean */
  UsLgIntValue cur_class;       /* current unit's associated class */
  RValue cur_output = 0.0;      /* current unit's desired output */
  RValue elm_output = 0.0;      /* current element's output */
  TElement element = NULL;      /* current training set element */
  TElement ini_element = NULL;  /* initialization element */
  BoolValue element_found;      /* valid element found flag */
  Vector aux_vector = NULL;     /* auxiliary initialization vector */

  /* NULL checks */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_init_weights", "no LVQ passed\n");

  if (lvq_nnet->nnet == NULL)
    return error_failure ("nnet_lvq_init_weights",
                          "LVQ not attached to neural network\n");

  if (set == NULL)
    return error_failure ("nnet_lvq_init_weights", "no set passed\n");

  /* initialization */
  nnet = lvq_nnet->nnet;
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  units_per_class = lvq_attr->units_per_class;
  out_layer = nnet->last_layer;
  unit = out_layer->first_unit;

  aux_vector = vector_create (unit->nu_inputs);
  if (aux_vector == NULL)
    return error_failure ("nnet_lvq_init_weights",
                          "error creating auxiliary vector\n");

  /* output units loop */
  while (unit != NULL)
    {
      /*
       * finds the class associated to the current unit
       * finds an element from the set with the desired output
       */
      nnet_lvq_right_classes (unit, NULL, units_per_class, 0, &cur_class,
                              NULL, &dummy_bool, NULL);

      cur_output = (RValue) cur_class;

      if ((unit->unit_index - 1) % units_per_class == 0)
        element = set->first_element;

      element_found = FALSE;

      while (element_found == FALSE && element != NULL)
        {
          if (vector_get_value (element->output, 1, &elm_output) !=
              EXIT_SUCCESS)
            return error_failure ("nnet_lvq_init_weights",
                                  "error getting output from current element\n");

          if (fabs (elm_output - cur_output) < DBL_EPSILON)
            {
              element_found = TRUE;

              if (vector_copy (element->input, aux_vector) != EXIT_SUCCESS)
                return error_failure ("nnet_lvq_init_weights",
                                      "error copying element input\n");

              ini_element = element;
            }

          element = element->next;
        }

      if (element_found == TRUE)
        {
          /* subtract average vector */
          if (vector_subtract (aux_vector,
                               set->input_vector_stats->average, aux_vector)
              != EXIT_SUCCESS)

            return error_failure ("nnet_lvq_init_weights",
                                  "error subtracting average\n");

          /* divides by variance */
          if (vector_multiply (aux_vector,
                               set->input_vector_stats->invstddev, aux_vector)
              != EXIT_SUCCESS)

            return error_failure ("nnet_lvq_init_weights",
                                  "error dividing by standard deviation\n");

          if (nnet_unit_set_weight_vector (unit, ini_element->input) !=
              EXIT_SUCCESS)
            return
              error_failure ("nnet_lvq_init_weights",
                             "error setting current unit's weight vector\n");
        }
      else
        {
          return error_failure ("nnet_lvq_init_weights",
                                "couldn't find adequate element for weight initialization\n");
        }

      unit = unit->next;
    }

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
                            Unit * winner1, Unit * winner2,
                            RValue * mse_error)
{
  Layer input_layer, output_layer;      /* auxiliary layer pointers */
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  VectorMetric metric;          /* vector metric for activation */
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
  if (nnet_layer_load_vector (element->input, input_layer) != EXIT_SUCCESS)
    return
      error_failure ("nnet_lvq_propagate_element",
                     "error loading input vector\n");
  /* Activates the neural network */
  if (nnet_nnetwork_activate (lvq_nnet->nnet, metric) != EXIT_SUCCESS)
    return
      error_failure ("nnet_lvq_propagate_element",
                     "error activating neural network\n");
  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/
  /* Determines the winner: largest output according to the metrics */
  if (nnet_metr_layer_2_winners
      (output_layer, metric, winner1, winner2) != EXIT_SUCCESS)
    return
      error_failure ("nnet_lvq_propagate_element",
                     "error executing LVQ competition\n");
  nnet_metr_mse_error (element->input, *winner1, mse_error);
  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
int
nnet_lvq_propagate_set (const LvqNNetwork lvq_nnet,
                        const TSet set, RValue * error_rate,
                        RValue * mse_error, TransitionMap classification_map)
{
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  TElement cur_element = NULL;  /* current training element */
  ElementIndex cur_index = 1;   /* current element counter */
  Unit winner1 = NULL;          /* current winner unit */
  Unit winner2 = NULL;          /* current 2nd place unit */
  RValue desired_output = 0.0;  /* desired output */
  UsLgIntValue desired_class;   /* desired class */
  UsLgIntValue winner1_class;   /* winner 1 class index */
  UsLgIntValue winner2_class;   /* winner 2 class index */
  BoolValue right1 = FALSE;     /* winner 1 correct classification flag */
  BoolValue right2 = FALSE;     /* winner 2 correct classification flag */
  UsLgIntValue mistakes = 0;    /* classification matches */
  RValue mse_element = 0.0;     /* current element MSE error */

  /* NULL checks */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_propagate_set",
                          "no LVQ neural network passed\n");

  if (set == NULL)
    return error_failure ("nnet_lvq_propagate_set",
                          "no training set passed\n");

  if (classification_map == NULL)
    return error_failure ("nnet_lvq_propagate_set",
                          "no classification map passed\n");

  /* initialization */
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  cur_element = set->first_element;

  if (trmap_reset (classification_map, 0.0) != EXIT_SUCCESS)

    return error_failure ("nnet_lvq_propagate_set",
                          "error re-initializing classification map\n");

  *mse_error = 0.0;

  while (cur_element != NULL)
    {
      /*
       * propagates current element
       * gets the desired output
       * sets the winner index in the unit index list
       * goes to the next element
       */
      if (nnet_lvq_propagate_element (lvq_nnet, cur_element,
                                      &winner1, &winner2,
                                      &mse_element) != EXIT_SUCCESS)

        return error_failure ("nnet_lvq_propagate_set",
                              "error propagating element %ld\n",
                              cur_element->element_index);

      if (vector_get_value (cur_element->output, 1, &desired_output)
          != EXIT_SUCCESS)

        return error_failure ("nnet_lvq_propagate_set",
                              "error getting desired output for training element\n");
      desired_class = (UsLgIntValue) desired_output;

      nnet_lvq_right_classes (winner1, winner2,
                              lvq_attr->units_per_class,
                              desired_class,
                              &winner1_class, &winner2_class,
                              &right1, &right2);

      if (trmap_transition (classification_map,
                            desired_class, winner1_class) != EXIT_SUCCESS)
        return
          error_failure ("nnet_lvq_propagate_set",
                         "error setting transition in classification map\n");

      if (right1 == FALSE)
        mistakes++;

      *mse_error += mse_element;

      ++cur_index;

      cur_element = cur_element->next;
    }

  *error_rate = (double) mistakes / (double) set->nu_elements;

  return EXIT_SUCCESS;
}



/*
 * nnet_lvq_train_element
 *
 * Executes training for one element
 */
int
nnet_lvq_train_element (LvqNNetwork lvq_nnet,
                        const TElement element,
                        const UsLgIntValue desired_class,
                        const LvqAlgorithmType algorithm,
                        const RValue learning_rate,
                        UsLgIntValue * winner_class,
                        BoolValue * match, RValue * mse_error)
{
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  /*Layer input_layer = NULL; *//* LVQ input layer */
  /*Layer output_layer = NULL; *//* LVQ output layer */
  Unit winner1 = NULL;          /* winner 1 unit */
  Unit winner2 = NULL;          /* winner 2 unit */
  UsLgIntValue winner1_class;   /* winner 1 class index */
  UsLgIntValue winner2_class;   /* winner 2 class index */
  BoolValue right1 = FALSE;     /* correct classification of winner 1 */
  BoolValue right2 = FALSE;     /* correct classification of winner 2 */
  RValue window_width = 0.0;    /* LVQ 2.1/3 window width */
  RValue epsilon = 0.0;         /* LVQ 3 epsilon */

  /* NULL checks */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "no neural network passed\n");
  if (lvq_nnet->nnet == NULL)
    return error_failure ("nnet_lvq_train_element",
                          "LVQ extension not attached to neural network\n");
  /* Checks if the network has a plausible LVQ architecture */
  if (lvq_nnet->nnet->nu_layers != 2)
    return error_failure ("nnet_lvq_train_element",
                          "a LVQ must have two layers\n");
  /* initialization */
  /*input_layer = lvq_nnet->nnet->first_layer; */
  /*output_layer = lvq_nnet->nnet->last_layer; */
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  window_width = lvq_attr->window_width;
  epsilon = lvq_attr->epsilon;

  /*************************************************************************
   *                               COMPETITION                             *
   *************************************************************************/
  /* Loads the training element, activates the network and find the winner */
  if (nnet_lvq_propagate_element (lvq_nnet, element,
                                  &winner1, &winner2, mse_error)
      != EXIT_SUCCESS)
    return error_failure ("nnet_lvq_train_element",
                          "error performing competition\n");
  /*************************************************************************
   *                                ADAPTION                               *
   *************************************************************************/
  /*
   * checks if the winners were correctly classificated
   * updates winners weight vectors
   */
  nnet_lvq_right_classes (winner1, winner2,
                          lvq_attr->units_per_class,
                          desired_class, &winner1_class,
                          &winner2_class, &right1, &right2);
  *match = right1;
  *winner_class = winner1_class;
  if (nnet_lvq_adapt
      (winner1, winner2, element->input, algorithm,
       learning_rate, window_width, epsilon, right1, right2) != EXIT_SUCCESS)
    return
      error_failure ("nnet_lvq_train_element", "error performing adaption\n");
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
                    const DTime first_epoch,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character,
                    RValue * error_rate, RValue * mse_error,
                    TransitionMap classification_map)
{
  static DTime t = 0;           /* current training time */
  TElement element = NULL;      /* current element */
  LvqAttributes lvqatt = NULL;  /* LVQ attributes */
  LRateFunction lfunc = NULL;   /* learning rate function */
  RValue etha = 0.0;            /* current learning rate */
  LvqAlgorithmType algorithm;   /* LVQ training algorithm */
  RValue desired_output;        /* current element's output */
  UsLgIntValue desired_class;   /* desired output for current element */
  UsLgIntValue winner_class;    /* current winner class */
  BoolValue match;              /* current element match */
  UsLgIntValue mistakes = 0;    /* classification matches */
  RValue mse_element = 0.0;     /* current element's MSE error */
  /* NULL checks */
  if (lvq_nnet == NULL)
    return error_failure ("nnet_lvq_train_set",
                          "no LVQ neural network passed\n");
  if (training_set == NULL)
    return error_failure ("nnet_lvq_train_set", "no training set passed\n");
  /* time checks */
  if (reset_time == TRUE)
    t = first_epoch;
  if (t >= max_epochs)
    return error_failure ("nnet_lvq_train_set",
                          "maximum epochs (%ld) reached\n", max_epochs);
  /* initialization */
  lvqatt = (LvqAttributes) lvq_nnet->attr;
  lfunc = lvqatt->lrate_function;
  etha = nnet_train_lrate_value (lfunc, (RValue) t);
  algorithm = lvqatt->lvq_algorithm;
  *mse_error = 0.0;
  if (classification_map != NULL)
    trmap_reset (classification_map, 0.0);
  /* updates progress bar */
  if (output_progress == TRUE)
    {
      if (display_progress
          (first_epoch + 1, (long) max_epochs, (long) t + 1,
           progress_width, progress_character, 0) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_train_set",
                              "error updating progress bar to %ld\n", t);
    }

  /* training elements loop */
  element = training_set->first_element;
  while (element != NULL)
    {
      if (vector_get_value (element->output, 1, &desired_output)
          != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_train_set",
                              "error getting desired output for training element\n");
      desired_class = (UsLgIntValue) desired_output;
      if (nnet_lvq_train_element (lvq_nnet, element, desired_class,
                                  algorithm, etha, &winner_class,
                                  &match, &mse_element) != EXIT_SUCCESS)
        return error_failure ("nnet_lvq_train_set",
                              "error training element\n");
      if (classification_map != NULL)
        if (trmap_transition (classification_map,
                              desired_class, winner_class) != EXIT_SUCCESS)
          return
            error_failure ("nnet_lvq_train_set",
                           "error setting transition in classification map\n");
      if (match == FALSE)
        mistakes++;
      *mse_error += mse_element;
      element = element->next;
    }

  /* increment epoch counter */
  t++;
  *error_rate = (double) mistakes / (double) training_set->nu_elements;
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
  /*LvqAttributes lvq_attr; *//* LVQ attributes */
  if (output_fd == NULL)
    return;

  if (lvq_nnet == NULL)
    return;

  /* Initialization */
  /*lvq_attr = (LvqAttributes) lvq_nnet->attr; */

  /* LVQ specific information */
  /*
     if (include_ngb_function_info == TRUE)
     {
     nnet_lvq_ngb_function_info (lvq_attr->ngb_function, output_fd);
     fprintf (output_fd, "\n");
     }
   */
  nnet_nnetwork_info (lvq_nnet->nnet, include_layers_info,
                      include_units_info, include_in_connections,
                      include_out_connections, output_fd);
  return;
}
