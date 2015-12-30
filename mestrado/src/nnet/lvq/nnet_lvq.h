#ifndef __NNET_LVQ_H_
#define __NNET_LVQ_H_ 1

#include "../nnet_types.h"
#include "nnet_lvq_window.h"

/******************************************************************************
 *                                                                            *
 *                        PUBLIC DATATYPES AND VARIABLES                      *
 *                                                                            *
 ******************************************************************************/

/*
 * LvqAlgorithmType
 *
 * Lvq Training Algorithm Class
 */
typedef enum
{
  LVQ_1 = 0,
  LVQ_2_1 = 1,
  LVQ_3 = 2,
  OLVQ_1 = 3
}
LvqAlgorithmType;



/*
 * nnet_lvq_type
 *
 * Extends the NNetwork type to include LVQ-specific attributes:
 * - learning rate function
 * - LVQ training algorithm
 * - unit activation vector metric
 */
typedef struct
{
  LRateFunction lrate_function;
  LvqAlgorithmType lvq_algorithm;
  VectorMetric activation_metric;
}
nnet_lvq_attr_type;


/* Symbolic type */
typedef nnet_lvq_attr_type *LvqAttributes;
typedef NExtension LvqNNetwork;



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_lvq_create
 *
 * Creates a new LVQ neural network architecture extension
 */
extern LvqNNetwork
nnet_lvq_create (const NNetwork nnet,
                 const LvqAlgorithmType lvq_algorithm,
                 const LRateFunctionClass lrate_class,
                 const VectorMetric activation_metric,
                 const LRateFunctionParameters lrate_parameters);



/*
 * nnet_lvq_extend_nnetwork
 *
 * Associates the given LVQ extension to the given neural network
 */
extern int nnet_lvq_extend_nnetwork (NNetwork nnet, LvqNNetwork lvq_nnet);



/*
 * nnet_lvq_create_nnetwork
 *
 * Creates a new generic neural network accorging to the LVQ architecture
 * and then attaches it to the given LVQ extension
 */
extern int
nnet_lvq_create_nnetwork (LvqNNetwork lvq_nnet,
                          const Name nnet_name,
                          const Name input_layer_name,
                          const Name output_layer_name,
                          const UnitIndex nu_inputs,
                          const UnitIndex nu_outputs,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters);



/*
 * nnet_lvq_destroy
 *
 * Destroys a previously created LVQ neural network
 */
extern int
nnet_lvq_destroy (LvqNNetwork * lvq_nnet,
                  const BoolValue cascade_nnetwork,
                  const BoolValue cascade_layers,
                  const BoolValue cascade_units,
                  const BoolValue cascade_connections);



/*
 * nnet_lvq_propagate_element
 *
 * Loads and propagates an element
 */
extern int
nnet_lvq_propagate_element (const LvqNNetwork lvq_nnet,
                            const TElement element,
                            Unit * winner, Unit * winner2);



/*
 * nnet_lvq_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
extern Vector
nnet_lvq_propagate_set (const LvqNNetwork lvq_nnet, const TSet set);



/*
 * nnet_lvq_train_element
 *
 * Executes training for one element
 */
extern int
nnet_lvq_train_element (LvqNNetwork lvq_nnet,
                        const TElement element, const RValue learning_rate);



/*
 * nnet_lvq_train_set
 *
 * Executes one training pass through all the elements in the given
 * training set
 */
extern int
nnet_lvq_train_set (LvqNNetwork lvq_nnet,
                    const TSet training_set,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character);



/*
 * nnet_lvq_info
 *
 * Outputs information about the LVQ neural network
 */
extern void
nnet_lvq_info (const LvqNNetwork lvq_nnet,
               const BoolValue include_layers_info,
               const BoolValue include_units_info,
               const BoolValue include_in_connections,
               const BoolValue include_out_connections, FILE * output_fd);



#endif /* __NNET_LVQ_H_ */
