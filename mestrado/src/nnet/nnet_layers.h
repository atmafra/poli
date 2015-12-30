#ifndef __NNET_LAYERS_H_
#define __NNET_LAYERS_H_ 1

#include "nnet_types.h"

/*
 * Layer Classes
 */

/* Class names */
extern const char nnet_layer_class_name[][10];

/* Input layer class */
extern nnet_layer_class_type nnet_layer_class_input;

/* Hidden layer class */
extern nnet_layer_class_type nnet_layer_class_hidden;

/* Output layer class */
extern nnet_layer_class_type nnet_layer_class_output;



/******************************************************************************
 *                                                                            *
 *                           STRUCTURAL OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_layer_init_all_classes
 *
 * Initializes all the internal layer classes
 */
extern int nnet_layer_init_all_classes ();



/*
 * nnet_layer_class_by_name
 *
 * Returns the layer class variable associated to the given name
 */
extern LayerClass nnet_layer_class_by_name (Name name);



/*
 * nnet_layer_create
 *
 * Creates a layer, given the parameters.
 * If the layer already exists, prints the corresponding message and returns.
 */
extern Layer
nnet_layer_create (NNetwork nnet, LayerIndex * nnet_index,
                   LayerClass layer_class, const Name name);



/*
 * nnet_layer_destroy
 *
 * Destroys a previously created layer
 */
extern int
nnet_layer_destroy (Layer * layer,
                    const BoolValue cascade_units,
                    const BoolValue cascade_connections);



/*
 * nnet_layer_attach_to_nnetwork
 *
 * attachs the given layer to the given neural network
 */
extern int
nnet_layer_attach_to_nnetwork (Layer layer,
                               NNetwork nnet, LayerIndex * nnet_index);



/*
 * nnet_layer_remove_from_network
 *
 * Removes the given layer from the neural network it is attached to
 */
extern int nnet_layer_remove_from_nnetwork (Layer layer);



/*
 * nnet_layer_by_index
 *
 * Returns a layer given its neural network and its layer index
 */
extern Layer nnet_layer_by_index (NNetwork nnet, LayerIndex index);



/******************************************************************************
 *                                                                            *
 *                           FUNCTIONAL OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_layer_load_vector
 *
 * Applies the given vector as activations for the given layer
 */
extern int nnet_layer_load_vector (const Vector vector, Layer layer);



/*
 * nnet_layer_activation_vector
 *
 * Returns the activations of the layer's units as a vector
 */
extern int
nnet_layer_activation_vector (const Layer layer, Vector activation_vector);



/*
 * nnet_layer_output_vector
 *
 * Returns the outputs of the layer's units as a vector
 */
extern int nnet_layer_output_vector (const Layer layer, Vector output_vector);



/*
 * nnet_layer_activate
 *
 * Activates all units of the given layer
 */
extern int nnet_layer_activate (Layer layer, const VectorMetric metric);



/*
 * nnet_layer_class_info
 *
 * Outputs information about the layer class
 */
extern void
nnet_layer_class_info (const LayerClass layer_class, FILE * output_fd);



/*
 * nnet_layer_info
 *
 * Outputs information about the layer
 */
extern void
nnet_layer_info (const Layer layer,
                 const BoolValue include_units_info,
                 const BoolValue include_in_connections,
                 const BoolValue include_out_connections, FILE * output_fd);


#endif /* __NNET_LAYERS_H_ */
