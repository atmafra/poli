#ifndef __NNET_METRICS_H_
#define __NNET_METRICS_H_ 1

#include "vector.h"

#include "nnet_types.h"

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
 * nnet_metr_mse_error
 *
 * Calculates MSE error between winner and input
 */
extern int
nnet_metr_mse_error (const Vector input, const Unit unit, RValue * mse_error);



#endif /* __NNET_METRICS_H_ */
