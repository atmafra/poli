#include <stdio.h>
#include <stdlib.h>
#include "nnet_types.h"
#include "nnet_metrics.h"


/*
 * nnet_metr_layer_winner
 *
 * Returns the "winner" unit of the given layer.
 * The winner is the unit with highest or lowest output, depending on
 * the metrics used for comparison.
 */
Unit
nnet_metr_layer_winner (const Layer layer, const VectorMetric metrics)
{
  Unit cur_unit;                /* current unit */
  Unit winner = NULL;           /* current winner */


  /* Checks if a layer was actually passed */
  if (layer == NULL)
    {
      fprintf (stderr,
               "nnet_metr_layer_winner: no layer to determine winner\n");
      return NULL;
    }

  /* Checks if the layer has units */
  if (layer->nu_units == 0)
    {
      fprintf (stderr, "nnet_metr_layer_winner: layer has no units\n");
      return NULL;
    }

  /* Search for the winner in the layer */
  cur_unit = layer->first_unit;
  winner = cur_unit;

  while (cur_unit != NULL)
    {
      /* Compares current unit with current winner */
      switch (metrics)
        {
        case VECTOR_METR_EUCLIDEAN:

          /* smallest euclidean distance */
          if (cur_unit->output < winner->output)
            winner = cur_unit;
          break;

        case VECTOR_METR_INNER_PRODUCT:

          /* largest inner product */
          if (cur_unit->output > winner->output)
            winner = cur_unit;
          break;

        default:

          /* unknown metrics */
          fprintf (stderr,
                   "nnet_metr_layer_winner: unknown vector metrics\n");
          return NULL;
        }

      /* Moves to the next */
      cur_unit = cur_unit->next;
    }

  return winner;
}



/*
 * nnet_metr_layer_2_winners
 *
 * Returns the "winner" and the "2nd place" units of the given layer.
 * The winner is the unit with highest or lowest output, depending on
 * the metrics used for comparison.
 */
int
nnet_metr_layer_2_winners (const Layer layer, const VectorMetric metrics,
                           Unit * winner1, Unit * winner2)
{
  Unit cur_unit;                /* current unit */
  Unit w1 = NULL;               /* current winner */
  Unit w2 = NULL;               /* current 2nd place */


  /* Checks if a layer was actually passed */
  if (layer == NULL)
    {
      fprintf (stderr,
               "nnet_metr_layer_winner: no layer to determine winner\n");
      return EXIT_FAILURE;
    }

  /* Checks if the layer has units */
  if (layer->nu_units == 0)
    {
      fprintf (stderr, "nnet_metr_layer_winner: layer has no units\n");
      return EXIT_FAILURE;
    }

  /* Initialization */
  cur_unit = layer->first_unit;
  w1 = cur_unit;

  /* Trivial case */
  if (layer->nu_units == 1)
    {
      *winner1 = w1;
      *winner2 = NULL;
      return EXIT_SUCCESS;
    }

  /* Search for the winner in the layer */

  while (cur_unit != NULL)
    {
      /* Compares current unit with current winner */
      switch (metrics)
        {
        case VECTOR_METR_EUCLIDEAN:

          /* smallest euclidean distance */
          if (cur_unit->output < w1->output)
            {
              w1 = cur_unit;
            }
          else if (w2 == NULL)
            {
              if (w1 != cur_unit)
                w2 = cur_unit;
            }
          else
            {
              if (cur_unit->output < w2->output &&
                  cur_unit->output > w1->output)
                w2 = cur_unit;
            }
          break;

        case VECTOR_METR_INNER_PRODUCT:

          /* largest inner product */
          if (cur_unit->output > w1->output)
            {
              w1 = cur_unit;
            }
          else if (w2 == NULL)
            {
              if (w1 != cur_unit)
                w2 = cur_unit;
            }
          else
            {
              if (cur_unit->output > w2->output &&
                  cur_unit->output < w1->output)
                w2 = cur_unit;
            }
          break;

        default:

          /* unknown metrics */
          fprintf (stderr,
                   "nnet_metr_layer_winner: unknown vector metrics\n");
          return EXIT_FAILURE;
        }

      /* Moves to the next */
      cur_unit = cur_unit->next;
    }

  /* Sets the return units */
  *winner1 = w1;
  *winner2 = w2;

  return EXIT_SUCCESS;
}
