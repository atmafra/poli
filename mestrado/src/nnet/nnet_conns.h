#ifndef __NNET_CONNS_H_
#define __NNET_CONNS_H_ 1

#include <stdio.h>

#include "nnet_types.h"
#include "nnet_units.h"

/******************************************************************************
 *                                                                            *
 *                            STRUCTURAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_conn_create
 *
 * Creates a connection between two units
 */
extern Connection
nnet_conn_create (Unit orig, Unit dest,
                  const RValue weight,
                  const WeightInitClass weight_init_class,
                  const WeightInitParameters weight_init_parameters);



/*
 * nnet_conn_create_init
 *
 * Creates a connection between two units and initializes its weight
 * according to the given weight initialization function
 */
extern Connection
nnet_conn_create_init (Unit orig, Unit dest,
                       const WeightInitClass weight_init_class,
                       const WeightInitParameters weight_init_parameters);



/*
 * nnet_conn_connect_layers
 *
 * Connects all units from the origin to the destination layer
 */
extern int
nnet_conn_connect_layers (Layer orig, Layer dest,
                          const RValue weight,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parms);



/*
 * nnet_conn_destroy
 *
 * Destroys a previously created connection between two units
 */
extern int nnet_conn_destroy (Connection * connection);



/*
 * nnet_conn_disconnect_unit_inputs
 *
 * Destroys all connections (if any) TO the given unit
 */
extern int nnet_conn_disconnect_unit_inputs (Unit unit);



/*
 * nnet_conn_disconnect_unit_outputs
 *
 * Destroys all connections (if any) FROM the given unit
 */
extern int nnet_conn_disconnect_unit_outputs (Unit unit);



/*
 * nnet_conn_disconnect_unit
 *
 * Destroys all connections (if any) FROM and TO the given unit
 */
extern int nnet_conn_disconnect_unit (Unit unit);



/******************************************************************************
 *                                                                            *
 *                            FUNCTIONAL OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_conn_set_weight
 *
 * Sets the weight of a connection to the given value
 */
extern int nnet_conn_set_weight (Connection conn, const RValue weight);



/*
 * nnet_conn_init_weight
 *
 * Initializes the weight of the connection based on the weight initialization
 * function
 */
extern int nnet_conn_init_weight (Connection conn);



/*
 * nnet_conn_increment_weight
 *
 * Increments the current weight of the connection by the given delta
 */
extern int
nnet_conn_increment_weight (Connection conn, const RValue delta_weight);



/*
 * nnet_conn_info
 *
 * Outputs information about the connection
 */
extern void
nnet_conn_info (const Connection conn,
                const BoolValue include_origin,
                const BoolValue include_destination, FILE * output_fd);



#endif /* __NNET_CONNS_H_ */
