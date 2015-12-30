#ifndef __NNET_METRICS_H_
#define __NNET_METRICS_H_ 1

#include "nnet_types.h"
#include "../vector/vector.h"


/******************************************************************************
 *                                                                            *
 *                               PUBLIC OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_metr_layer_winner
 *
 * Returns the "winner" unit of the given layer.
 * The winner is the unit with highest or lowest output, depending on
 * the metrics used for comparison.
 */
extern Unit
nnet_metr_layer_winner (const Layer layer, const VectorMetric metrics);



/*
 * nnet_metr_layer_2_winners
 *
 * Returns the "winner" and the "2nd place" units of the given layer.
 * The winner is the unit with highest or lowest output, depending on
 * the metrics used for comparison.
 */
extern int
nnet_metr_layer_2_winners (const Layer layer, const VectorMetric metrics,
                           Unit * winner1, Unit * winner2);



/*
 * nnet_metr_layer_error_vector
 *
 * Returns the error vector for the given layer
 */
extern RValue nnet_metr_layer_error_vector (const Layer layer);


#endif /* __NNET_METRICS_H_ */
