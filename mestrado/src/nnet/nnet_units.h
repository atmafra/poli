#ifndef __NNET_UNITS_H_
#define __NNET_UNITS_H_ 1

#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_unit_create
 *
 * Creates one unit on layer 'layer_index' for the given layer array
 */
extern Unit
nnet_unit_create (Layer layer,
                  UnitIndex * index,
                  ActivationClass actv_class,
                  const ActivationParameters parameters,
                  const BoolValue use_layer_default_activation_class,
                  const BoolValue use_activation_class_default_parameters,
                  const Vector coordinates);



/*
 * nnet_unit_destroy
 *
 * Destroys a previously created unit
 */
extern int
nnet_unit_destroy (Unit * unit, const BoolValue cascade_connections);



/*
 * nnet_unit_attach_to_layer
 *
 * Attaches the given unit to the given neural network
 */
extern int
nnet_unit_attach_to_layer (Unit unit, Layer layer, UnitIndex * layer_index);



/*
 * nnet_unit_remove_from_layer
 *
 * Removes the given unit from its layer
 */
extern int nnet_unit_remove_from_layer (Unit unit);



/*
 * nnet_unit_create_multiple
 *
 * Create multiple units on a given layer
 */
extern int
nnet_unit_create_multiple (const UnitIndex nu_units,
                           Layer layer,
                           ActivationClass actv_class,
                           const ActivationParameters parameters,
                           const BoolValue
                           use_layer_default_activation_class,
                           const BoolValue
                           use_activation_class_default_parameters,
                           const Vector coord_distribution,
                           const Vector coord_increment);



/*
 * nnet_unit_by_index
 *
 * Returns an unit given its neural network and its layer index
 */
extern Unit
nnet_unit_by_index (NNetwork nnet, LayerIndex layer_index,
                    UnitIndex unit_index);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_unit_set_activation
 *
 * Sets the unit's activation to the given value
 */
extern int nnet_unit_set_activation (Unit unit, const RValue activation);



/*
 * nnet_unit_get_input_vector
 *
 * Gathers the outputs of all units connected to the given unit as inputs
 * into the given vector
 */
extern int nnet_unit_get_input_vector (const Unit unit, Vector input_vector);



/*
 * nnet_unit_get_weight_vector
 *
 * Gathers the weights of all input connections TO the given unit
 * into the given vector
 */
extern int
nnet_unit_get_weight_vector (const Unit unit, Vector weight_vector);



/*
 * nnet_unit_set_weight_vector
 *
 * Updates the unit's input weights according to the given weight vector
 */
extern int
nnet_unit_set_weight_vector (const Unit unit, const Vector weight_vector);



/*
 * nnet_unit_collect_activation
 *
 * Updates the unit's activation, performing the sum of the outputs of the
 * units connected to it, weighted by the connections weights.
 */
extern int nnet_unit_collect_activation (Unit unit);



/*
 * nnet_unit_vector_activation
 *
 * Updates the unit's activation, according to the requested metric
 * between the input and weight vectors
 */
extern int nnet_unit_vector_activation (Unit unit, const VectorMetric metric);



/*
 * nnet_unit_update_output
 *
 * Updates the output of the given unit according to the current
 * activation function and activation
 */
extern int nnet_unit_update_output (Unit unit);



/*
 * nnet_unit_activate
 *
 * Performs activation collection followed by output update, according to
 * the new activation.
 * If a vector metric is passed, calculates the new activation according
 * to this metric between the input and weight vectors.
 */
extern int nnet_unit_activate (Unit unit, const VectorMetric metric);



/*
 * nnet_unit_index_to_coordinates
 *
 * Converts one unit index to its coordinates vector, according to
 * the given unit distribution and distance vectors
 */
extern Vector
nnet_unit_index_to_coordinates (const UnitIndex index,
                                const Vector coord_distribution,
                                const Vector coord_increment);



/*
 * nnet_unit_info
 *
 * Outputs unit information
 */
extern void
nnet_unit_info (const Unit unit,
                const BoolValue include_in_connections,
                const BoolValue include_out_connections, FILE * output_fd);


#endif /* __NNET_UNITS_H_ */
