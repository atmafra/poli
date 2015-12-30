#ifndef __NNET_LVQ_H_
#define __NNET_LVQ_H_ 1

#include "table.h"
#include "trmap.h"

#include "nnet_types.h"
#include "nnet_lvq_window.h"

#define __LVQ_1_STRING_ "LVQ-1"
#define __LVQ_2_1_STRING_ "LVQ-2.1"
#define __LVQ_3_STRING_ "LVQ-3"
#define __OLVQ_1_STRING_ "OLVQ-1"


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
 * - number of output classes
 * - number of output units per class
 * - a table of class definitions
 */
typedef struct
{
  LRateFunction lrate_function;
  LvqAlgorithmType lvq_algorithm;
  VectorMetric activation_metric;
  UnitIndex output_classes;
  UnitIndex units_per_class;
  RValue window_width;
  RValue epsilon;
  Table class_table;
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
 * nnet_lvq_str_to_algorithm
 *
 * Converts a string into its corresponding LVQ learning algorithm
 */
extern int
nnet_lvq_str_to_algorithm (const char *str, LvqAlgorithmType * algorithm);



/*
 * nnet_lvq_algorithm_to_str
 *
 * Returns the string associated to the given algorithm
 */
extern char *nnet_lvq_algorithm_to_str (const LvqAlgorithmType algorithm);



/*
 * nnet_lvq_create
 *
 * Creates a new LVQ neural network architecture extension
 */
extern LvqNNetwork
nnet_lvq_create (const NNetwork nnet,
                 const LvqAlgorithmType lvq_algorithm,
                 const LRateFunctionClass lrate_class,
                 const LRateFunctionParameters lrate_parameters,
                 const VectorMetric activation_metric,
                 const UnitIndex output_classes,
                 const UnitIndex units_per_class,
                 const RValue window_width, const RValue epsilon);



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
                          const UnitIndex output_classes,
                          const UnitIndex units_per_class,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parameters);



/*
 * nnet_lvq_attach_class_table
 *
 * Attaches an output classes table to the given LVQ
 */
extern int
nnet_lvq_attach_class_table (LvqNNetwork lvq_nnet, const Table class_table);



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
 * nnet_lvq_init_weights
 *
 * Initializes LVQ's weights according to the training set
 */
extern int nnet_lvq_init_weights (const LvqNNetwork lvq_nnet, const TSet set);



/*
 * nnet_lvq_propagate_element
 *
 * Loads and propagates an element
 */
extern int
nnet_lvq_propagate_element (const LvqNNetwork lvq_nnet,
                            const TElement element,
                            Unit * winner, Unit * winner2,
                            RValue * mse_error);



/*
 * nnet_lvq_propagate_set
 *
 * Loads and propagates all elements in the given set, returning a list
 * of winner indexes
 */
extern int
nnet_lvq_propagate_set (const LvqNNetwork lvq_nnet, const TSet set,
                        RValue * error_rate, RValue * mse_error,
                        TransitionMap classification_map);



/*
 * nnet_lvq_train_element
 *
 * Executes training for one element
 */
extern int
nnet_lvq_train_element (LvqNNetwork lvq_nnet,
                        const TElement element,
                        const UsLgIntValue desired_class,
                        const LvqAlgorithmType algorithm,
                        const RValue learning_rate,
                        UsLgIntValue * winner_class,
                        BoolValue * match, RValue * mse_error);



/*
 * nnet_lvq_train_set
 *
 * Executes one training pass through all the elements in the given
 * training set
 */
extern int
nnet_lvq_train_set (LvqNNetwork lvq_nnet,
                    const TSet training_set,
                    const DTime first_epoch,
                    const DTime max_epochs,
                    const BoolValue reset_time,
                    const BoolValue output_progress,
                    const size_t progress_width,
                    const char progress_character,
                    RValue * error_rate, RValue * mse_error,
                    TransitionMap classification_map);



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
