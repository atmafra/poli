#ifndef __NNET_NNET_H_
#define __NNET_NNET_H_ 1

#include <stdio.h>
#include "nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/* Names of the neural network extensions */
extern const char nnet_extension_name[][10];


/*
 * nnet_extension_by_name
 *
 * Returns the neural network extension index according to the given name
 */
extern NExtensionIndex nnet_extension_by_name (Name name);



/*
 * nnet_nnetwork_create
 *
 * Creates a new Neural Network
 */
extern NNetwork
nnet_nnetwork_create (const Name name, const NExtension extension);



/*
 * nnet_nnetwork_destroy
 *
 * Destroys a previously created Neural Network
 */
extern int
nnet_nnetwork_destroy (NNetwork * nnet,
                       const BoolValue cascade_extension,
                       const BoolValue cascade_layers,
                       const BoolValue cascade_units,
                       const BoolValue cascade_connections);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_nnetwork_activate
 *
 * Activates all the layers in the neural network
 */
extern int
nnet_nnetwork_activate (NNetwork nnet, const VectorMetric vector_metric);



/*
 * nnet_nnetwork_info
 *
 * Outputs information about the neural network
 */
extern void
nnet_nnetwork_info (const NNetwork nnet,
                    const BoolValue include_layers_info,
                    const BoolValue include_units_info,
                    const BoolValue include_in_connections,
                    const BoolValue include_out_connections,
                    FILE * output_fd);


#endif /* __NNET_NNET_H_ */
