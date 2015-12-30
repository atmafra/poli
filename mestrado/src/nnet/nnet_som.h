#ifndef __NNET_SOM_H_
#define __NNET_SOM_H_ 1

#include "nnet_types.h"
#include "nnet_som_ngb.h"

#define __RATE_THRESHOLD_ 0.001


/******************************************************************************
 *                                                                            *
 *                        PUBLIC DATATYPES AND VARIABLES                      *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_som_type
 *
 * Extends the NNetwork type to include SOM-specific attributes:
 * - neighborhood function
 * - learning rate function
 */
typedef struct
{
  NgbFunction ngb_function;
  LRateFunction lrate_function;
}
nnet_som_attr_type;


/* Symbolic type */
typedef nnet_som_attr_type *SomAttributes;
typedef NExtension SomNNetwork;



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_som_create
 *
 * Creates a new SOM neural network architecture extension
 */
extern SomNNetwork
nnet_som_create (const NNetwork nnet,
                 const NgbFunctionClass ngb_class,
                 const NgbRFunctionParameters ngb_parameters,
                 const LRateFunctionClass lrate_class,
                 const LRateFunctionParameters lrate_parameters);



/*
 * nnet_som_extend_nnetwork
 *
 * Associates the given SOM extension to the given neural network
 */
extern int nnet_som_extend_nnetwork (NNetwork nnet, SomNNetwork som_nnet);



/*
 * nnet_som_create_nnetwork
 *
 * Creates a new generic neural network accorging to the SOM architecture
 * and then attaches it to the given SOM extension
 */
extern int
nnet_som_create_nnetwork (SomNNetwork som_nnet,
                          const Name nnet_name,
                          const Name input_layer_name,
                          const Name output_layer_name,
                          const UnitIndex nu_inputs,
                          const UnitIndex nu_outputs,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters,
                          const Vector coord_distribution,
                          const Vector coord_increment);



/*
 * nnet_som_destroy
 *
 * Destroys a previously created SOM neural network
 */
extern int
nnet_som_destroy (SomNNetwork * som_nnet,
                  const BoolValue cascade_nnetwork,
                  const BoolValue cascade_layers,
                  const BoolValue cascade_units,
                  const BoolValue cascade_connections);



/*
 * nnet_som_ngb_vector
 *
 * Returns the neighborhood vector for the layer of the given winner unit
 */
extern int
nnet_som_ngb_vector (const SomNNetwork som_nnet,
                     const Unit winner_unit, Vector ngb_vector);



/*
 * nnet_som_propagate_element
 *
 * Loads and propagates an element
 */
extern int
nnet_som_propagate_element (const SomNNetwork som_nnet,
                            const TElement element, Unit * winner);



/*
 * nnet_som_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
extern Vector
nnet_som_propagate_set (const SomNNetwork som_nnet, const TSet set,
                        const BoolValue calculate_mse_error,
                        RValue * mse_error);



/*
 * nnet_som_train_element
 *
 * Executes training for one element
 */
extern int
nnet_som_train_element (SomNNetwork som_nnet,
                        const TElement element, const RValue learning_rate,
                        const BoolValue calculate_mse_error,
                        RValue * mse_error);



/*
 * nnet_som_train_set
 *
 * Executes one training pass through all the elements in the given
 * training set
 */
extern int
nnet_som_train_set (SomNNetwork som_nnet,
                    const TSet training_set,
                    const DTime first_epoch,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character,
                    const BoolValue calculate_mse_error, RValue * mse_error);



/*
 * nnet_som_info
 *
 * Outputs information about the SOM neural network
 */
extern void
nnet_som_info (const SomNNetwork som_nnet,
               const BoolValue include_ngb_function_info,
               const BoolValue include_layers_info,
               const BoolValue include_units_info,
               const BoolValue include_in_connections,
               const BoolValue include_out_connections, FILE * output_fd);



#endif /* __NNET_SOM_H_ */
