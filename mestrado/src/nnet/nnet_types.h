#ifndef __NNET_TYPES_H_
#define __NNET_TYPES_H_ 1

#include <stddef.h>

#include "types.h"
#include "function.h"
#include "vector.h"
#include "vectorstat.h"

#ifndef NNET_NAME_SIZE
#define NNET_NAME_SIZE 256
#endif

#ifndef NNET_BUF_SIZE
#define NNET_BUF_SIZE 1024
#endif


/******************************************************************************
 *                                                                            *
 *                              GENERAL DATATYPES                             *
 *                                                                            *
 ******************************************************************************/

/* Index of a layer */
typedef UsIntValue LayerIndex;


/* Index of a processing unit */
typedef UsLgIntValue UnitIndex;


/* Index of a training element */
typedef UsLgIntValue ElementIndex;


/* Discrete time index */
typedef UsLgIntValue DTime;


/* Training Epoch */
typedef UsIntValue Epoch;


/******************************************************************************
 *                                                                            *
 *                              BASIC POINTERS                                *
 *                                                                            *
 ******************************************************************************/

/* Pointer to Layer */
typedef struct nnet_layer_struct *LayerPtr;


/* Pointer to Processing Unit */
typedef struct nnet_unit_struct *UnitPtr;


/* Pointer to Unit Connection */
typedef struct nnet_conn_struct *ConnectionPtr;


/* Pointer to Neural Network Extension */
typedef struct nnet_extension_struct *NExtensionPtr;


/* Pointer to Traning Element */
typedef struct nnet_training_element_struct *TElementPtr;



/******************************************************************************
 *                                                                            *
 *                     DATATYPES FOR ACTIVATION FUNCTIONS                     *
 *                                                                            *
 ******************************************************************************/

/*
 * ActivationType
 *
 * Activation function class types.
 * Must map to exactly one activation function class.
 */
typedef enum
{
  NNET_ACTV_PASSTHROUGH = 0,
  NNET_ACTV_THRESHOLD = 1,
  NNET_ACTV_LINEAR = 2,
  NNET_ACTV_PCWLINEAR = 3,
  NNET_ACTV_SIGMOIDAL = 4,
  NNET_ACTV_TANH = 5
}
ActivationType;



/*
 * nnet_actv_type
 *
 * Activation function instances, with a function and its parameters
 */
typedef RFunction ActivationFunction;



/* Symbolic Types */
typedef RFunctionClass ActivationClass;
typedef RFunctionParameters ActivationParameters;



/******************************************************************************
 *                                                                            *
 *                        DATATYPES FOR NEURAL NETWORKS                       *
 *                                                                            *
 ******************************************************************************/

/* Neural Network datatype */
typedef struct
{
  Name name;
  NExtensionPtr extension;
  LayerIndex nu_layers;
  UnitIndex nu_units;
  LayerPtr first_layer;
  LayerPtr last_layer;
}
nnet_nnetwork_type;


/* Symbolic Type */
typedef nnet_nnetwork_type *NNetwork;



/******************************************************************************
 *                                                                            *
 *                           DATATYPES FOR LAYERS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * Layer class types
 *
 * Defined according to the position of the layer in the network
 */
typedef enum
{
  NNET_LAYER_UNDEFINED = 0,
  NNET_LAYER_INPUT = 1,
  NNET_LAYER_HIDDEN = 2,
  NNET_LAYER_OUTPUT = 3
}
LayerPosition;


/* Layer Class datatype */
typedef struct
{
  LayerPosition position;
  ActivationClass default_activation_class;
}
nnet_layer_class_type;


/* Symbolic Types */
typedef nnet_layer_class_type *LayerClass;


/* Layer datatype */
typedef struct nnet_layer_struct
{
  NNetwork nnetwork;
  LayerIndex layer_index;
  LayerClass layer_class;
  Name name;
  UnitIndex nu_units;
  UnitPtr first_unit;
  UnitPtr last_unit;
  LayerPtr next;
}
nnet_layer_type;


/* Symbolic Type */
typedef nnet_layer_type *Layer;



/******************************************************************************
 *                                                                            *
 *                       DATATYPES FOR PROCESSING UNITS                       *
 *                                                                            *
 ******************************************************************************/

/* Processing Unit (neuron) datatype */
typedef struct nnet_unit_struct
{
  Layer layer;
  UnitIndex unit_index;
  RValue activation;
  RValue output;
  ActivationFunction activation_function;
  UnitIndex nu_inputs;
  UnitIndex nu_outputs;
  RValue average;
  RValue stddev;
  ConnectionPtr first_orig;
  ConnectionPtr first_dest;
  ConnectionPtr last_orig;
  ConnectionPtr last_dest;
  Vector coord;
  UnitPtr next;
}
nnet_unit_type;


/* Symbolic Types */
typedef nnet_unit_type *Unit;



/******************************************************************************
 *                                                                            *
 *                DATATYPES FOR WEIGHT INITIALIZATION FUNCTIONS               *
 *                                                                            *
 ******************************************************************************/

/*
 * WeightType
 *
 * Weight initialization function class types.
 */
typedef enum
{
  NNET_WGHT_FIXED = 0,
  NNET_WGHT_UNIFORM = 1,
  NNET_WGHT_GAUSSIAN = 2
}
WeightType;


/*
 * nnet_wght_init_type
 *
 * Weight initialization function
 */
typedef RFunctionImplementation nnet_wght_init_type;


/* Symbolic Types */
typedef RFunctionClass WeightInitClass;
typedef RFunction WeightInitFunction;
typedef RFunctionParameters WeightInitParameters;



/******************************************************************************
 *                                                                            *
 *                       DATATYPES FOR UNIT CONNECTIONS                       *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_conn_type
 *
 * Unit Connection data type
 */
typedef struct nnet_conn_struct
{
  Unit orig;
  Unit dest;
  RValue weight;
  WeightInitFunction wght_function;
  ConnectionPtr next_orig;
  ConnectionPtr next_dest;
}
nnet_conn_type;


/* Symbolic Type */
typedef nnet_conn_type *Connection;



/******************************************************************************
 *                                                                            *
 *                   DATATYPES FOR NEURAL NETWORK EXTENSIONS                  *
 *                                                                            *
 ******************************************************************************/

/*
 * NExtensionIndex
 *
 * Neural network extension index
 */
typedef enum
{
  NNEXT_GEN = 0,
  NNEXT_SOM = 1,
  NNEXT_LVQ = 2,
  NNEXT_MLP = 3
}
NExtensionIndex;



/*
 * NExtension
 *
 * Neural Network Extension instance
 */
typedef struct nnet_extension_struct
{
  NExtensionIndex index;
  NNetwork nnet;
  void *attr;
}
nnet_extension_type;


/* Symbolic Type */
typedef nnet_extension_type *NExtension;



/******************************************************************************
 *                                                                            *
 *                   DATATYPES FOR LEARNING RATE FUNCTIONS                    *
 *                                                                            *
 ******************************************************************************/

/*
 * LRateType
 *
 * Learning rate function class types
 */
typedef enum
{
  NNET_LRATE_FIXED = 0,
  NNET_LRATE_PERCSTEP = 1,
  NNET_LRATE_EXPDECAY = 2
}
LRateType;


/* Symbolic Types */
typedef RFunctionClass LRateFunctionClass;
typedef RFunctionParameters LRateFunctionParameters;
typedef RFunction LRateFunction;



/******************************************************************************
 *                                                                            *
 *                 DATATYPES FOR TRAINING ELEMENTS AND SETS                   *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_training_set_type
 *
 * Training or Test sets
 */
typedef struct
{
  Name name;
  ElementIndex nu_elements;
  UnitIndex input_dimension;
  UnitIndex output_dimension;
  TElementPtr first_element;
  TElementPtr last_element;
  VectorStats input_vector_stats;
  VectorStats output_vector_stats;
}
nnet_training_set_type;


/* Symbolic Type */
typedef nnet_training_set_type *TSet;


/*
 * nnet_training_element_type
 *
 * One training/testing element
 */
typedef struct nnet_training_element_struct
{
  TSet set;
  ElementIndex element_index;
  Vector input;
  Vector output;
  TElementPtr next;
}
nnet_training_element_type;


/* Symbolic Type */
typedef nnet_training_element_type *TElement;



/******************************************************************************
 *                                                                            *
 *                       DATATYPES FOR TRAINING SESSIONS                      *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_training_session_type
 *
 * Training sessions
 */
typedef struct
{
  NNetwork nnet;
  LRateFunction etha;
  TSet training_set;
  TSet validation_set;
  TSet test_set;
  DTime time;
  Epoch epoch;
}
nnet_training_session_type;


/* Symbolic Type */
typedef nnet_training_session_type *TSession;


#endif /* nnet_types.h */
