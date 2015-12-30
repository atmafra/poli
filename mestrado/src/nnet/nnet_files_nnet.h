#ifndef __NNET_FILES_NNET_H_
#define __NNET_FILES_NNET_H_ 1

#include <stdio.h>
#include "nnet_types.h"


/* Ignore lines that start with this char */
#define __NNIGNORE_ '#'

/* Number of Sections defined */
#define __NNSECTIONS_ 6


/*
 * Section Tags
 */

/* Neural Network Section */
#define __NNSEC_NNET_ 0
#define __NNSEC_NNET_START_ "<neural network>"
#define __NNSEC_NNET_END_ "</neural network>"


/* SOM Neural Network Extension Section */
#define __NNSEC_XSOM_ 1
#define __NNSEC_XSOM_START_ "<som extension>"
#define __NNSEC_XSOM_END_ "</som extension>"


/* Layer Section */
#define __NNSEC_LAYR_ 2
#define __NNSEC_LAYR_START_ "<layer>"
#define __NNSEC_LAYR_END_ "</layer>"


/* Unit Section */
#define __NNSEC_UNIT_ 3
#define __NNSEC_UNIT_START_ "<unit>"
#define __NNSEC_UNIT_END_ "</unit>"


/* Connection Section */
#define __NNSEC_CONN_ 4
#define __NNSEC_CONN_START_ "<connection>"
#define __NNSEC_CONN_END_ "</connection>"


/* Layer Connection Section */
#define __NNSEC_LCNN_ 5
#define __NNSEC_LCNN_START_ "<layer connection>"
#define __NNSEC_LCNN_END_ "</layer connection>"



/*
 * Attribute Tags
 */

/* Neural Network Attributes */
#define __NNATT_NNET_NAME_ 0
#define __NNATT_NNET_NAME_TAG_ "neural network name:"

#define __NNATT_NNET_EXT_ 1
#define __NNATT_NNET_EXT_TAG_ "neural network extension:"


/* SOM Neural Network Extension Section */
#define __NNATT_XSOM_NGB_CLASS_ 0
#define __NNATT_XSOM_NGB_CLASS_TAG_ "neighborhood function:"

#define __NNATT_XSOM_NGB_PARM_ 1
#define __NNATT_XSOM_NGB_PARM_TAG_ "neighborhood function parameters:"

#define __NNATT_XSOM_LRATE_CLASS_ 2
#define __NNATT_XSOM_LRATE_CLASS_TAG_ "learning rate function:"

#define __NNATT_XSOM_LRATE_PARM_ 3
#define __NNATT_XSOM_LRATE_PARM_TAG_ "learning rate function parameters:"


/* Layer Attributes */
#define __NNATT_LAYR_INDEX_ 0
#define __NNATT_LAYR_INDEX_TAG_ "layer index:"

#define __NNATT_LAYR_NAME_ 1
#define __NNATT_LAYR_NAME_TAG_ "layer name:"

#define __NNATT_LAYR_CLASS_ 2
#define __NNATT_LAYR_CLASS_TAG_ "layer class:"

#define __NNATT_LAYR_NU_UNITS_ 3
#define __NNATT_LAYR_NU_UNITS_TAG_ "layer default units:"

#define __NNATT_LAYR_ACTV_CLASS_ 4
#define __NNATT_LAYR_ACTV_CLASS_TAG_ "activation class:"

#define __NNATT_LAYR_ACTV_PARM_ 5
#define __NNATT_LAYR_ACTV_PARM_TAG_ "activation parameters:"

#define __NNATT_LAYR_DIST_VECTOR_ 6
#define __NNATT_LAYR_DIST_VECTOR_TAG_ "distribution vector:"

#define __NNATT_LAYR_INCR_VECTOR_ 7
#define __NNATT_LAYR_INCR_VECTOR_TAG_ "increment vector:"


/* Unit Attributes */
#define __NNATT_UNIT_LINDEX_ 0
#define __NNATT_UNIT_LINDEX_TAG_ "layer index:"

#define __NNATT_UNIT_INDEX_ 1
#define __NNATT_UNIT_INDEX_TAG_ "unit index:"

#define __NNATT_UNIT_ACTV_CLASS_ 2
#define __NNATT_UNIT_ACTV_CLASS_TAG_ "activation class:"

#define __NNATT_UNIT_ACTV_PARM_ 3
#define __NNATT_UNIT_ACTV_PARM_TAG_ "activation parameters:"

#define __NNATT_UNIT_COORD_ 4
#define __NNATT_UNIT_COORD_TAG_ "coordinates:"


/* Connection Attributes */
#define __NNATT_CONN_OLAYER_ 0
#define __NNATT_CONN_OLAYER_TAG_ "origin layer index:"

#define __NNATT_CONN_OUNIT_ 1
#define __NNATT_CONN_OUNIT_TAG_ "origin unit index:"

#define __NNATT_CONN_DLAYER_ 2
#define __NNATT_CONN_DLAYER_TAG_ "destination layer index:"

#define __NNATT_CONN_DUNIT_ 3
#define __NNATT_CONN_DUNIT_TAG_ "destination unit index:"

#define __NNATT_CONN_WEIGHT_ 4
#define __NNATT_CONN_WEIGHT_TAG_ "weight:"

#define __NNATT_CONN_WEIGHT_CLASS_ 5
#define __NNATT_CONN_WEIGHT_CLASS_TAG_ "initialization method:"

#define __NNATT_CONN_WEIGHT_PARM_ 6
#define __NNATT_CONN_WEIGHT_PARM_TAG_ "initialization parameters:"


/* Layer Connection Attributes */
#define __NNATT_LCNN_ORIGIN_ 0
#define __NNATT_LCNN_ORIGIN_TAG_ "origin layer:"

#define __NNATT_LCNN_DESTINATION_ 1
#define __NNATT_LCNN_DESTINATION_TAG_ "destination layer:"

#define __NNATT_LCNN_INITIAL_WEIGHT_ 2
#define __NNATT_LCNN_INITIAL_WEIGHT_TAG_ "initial weight:"

#define __NNATT_LCNN_WEIGHT_CLASS_ 3
#define __NNATT_LCNN_WEIGHT_CLASS_TAG_ "initialization method:"

#define __NNATT_LCNN_WEIGHT_PARM_ 4
#define __NNATT_LCNN_WEIGHT_PARM_TAG_ "initialization parameters:"


/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_file_create_nnetwork
 *
 * Creates a new neural network according to the given configuration file
 */
extern NNetwork nnet_file_create_nnetwork (FILE * in_fd);



/*
 * nnet_file_write_nnetwork
 *
 * Writes the given neural network to the given stream
 */
extern void
nnet_file_write_nnetwork (const NNetwork nnet,
                          const BoolValue write_extension,
                          const BoolValue write_nnetwork,
                          const BoolValue write_layers,
                          const BoolValue write_units,
                          const BoolValue write_connections,
                          FILE * output_fd);



#endif /* __NNET_FILES_NNET_H_ */
