#include <stdio.h>
#include <stdlib.h>
#include "nnet_files.h"
#include "nnet_files_nnet.h"
#include "nnet_nnet.h"
#include "nnet_layers.h"
#include "nnet_units.h"
#include "nnet_conns.h"
#include "nnet_weights.h"
#include "nnet_train.h"
#include "nnet_actv.h"
#include "som/nnet_som.h"

#ifdef __STDIN_PROMPT_
#undef __STDIN_PROMPT_
#endif
#define __STDIN_PROMPT_ "(Pinga) "

/******************************************************************************
 *                                                                            *
 *                             PRIVATE VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

static NNetwork nnet = NULL;    /* new neural network */
static NExtensionIndex ext_index = NNEXT_GEN;   /* extension index */
static NExtension extension = NULL;     /* neural network extension */
static Layer layer = NULL;      /* last layer created */
static SomNNetwork som_nnet = NULL;     /* SOM extension */
static char stdin_prompt[60] = __STDIN_PROMPT_; /* standard input prompt */

/*
 * NNetFile
 *
 * Set of sections and attributes that compound a configuration file
 */
static NFile nnetfile = {

  /* Neural Network Section */
  {__NNSEC_NNET_,
   __NNSEC_NNET_START_,
   __NNSEC_NNET_END_,
   2,
   {{__NNATT_NNET_NAME_, STRING, __NNATT_NNET_NAME_TAG_}
    ,
    {__NNATT_NNET_EXT_, STRING, __NNATT_NNET_EXT_TAG_}
    }
   }
  ,

  /* SOM Neural Network Extension Section */
  {__NNSEC_XSOM_,
   __NNSEC_XSOM_START_,
   __NNSEC_XSOM_END_,
   4,
   {{__NNATT_XSOM_NGB_CLASS_, STRING, __NNATT_XSOM_NGB_CLASS_TAG_}
    ,
    {__NNATT_XSOM_NGB_PARM_, REAL_VECTOR, __NNATT_XSOM_NGB_PARM_TAG_}
    ,
    {__NNATT_XSOM_LRATE_CLASS_, STRING, __NNATT_XSOM_LRATE_CLASS_TAG_}
    ,
    {__NNATT_XSOM_LRATE_PARM_, REAL_VECTOR, __NNATT_XSOM_LRATE_PARM_TAG_}
    }
   }
  ,

  /* Layer Section */
  {__NNSEC_LAYR_,
   __NNSEC_LAYR_START_,
   __NNSEC_LAYR_END_,
   8,
   {{__NNATT_LAYR_INDEX_, UNSIGNED_INT, __NNATT_LAYR_INDEX_TAG_}
    ,
    {__NNATT_LAYR_NAME_, STRING, __NNATT_LAYR_NAME_TAG_}
    ,
    {__NNATT_LAYR_CLASS_, STRING, __NNATT_LAYR_CLASS_TAG_}
    ,
    {__NNATT_LAYR_NU_UNITS_, UNSIGNED_LONG_INT, __NNATT_LAYR_NU_UNITS_TAG_}
    ,
    {__NNATT_LAYR_ACTV_CLASS_, STRING, __NNATT_LAYR_ACTV_CLASS_TAG_}
    ,
    {__NNATT_LAYR_ACTV_PARM_, REAL_VECTOR, __NNATT_LAYR_ACTV_PARM_TAG_}
    ,
    {__NNATT_LAYR_DIST_VECTOR_, REAL_VECTOR, __NNATT_LAYR_DIST_VECTOR_TAG_}
    ,
    {__NNATT_LAYR_INCR_VECTOR_, REAL_VECTOR, __NNATT_LAYR_INCR_VECTOR_TAG_}
    }
   }
  ,

  /* Unit Section */
  {__NNSEC_UNIT_,
   __NNSEC_UNIT_START_,
   __NNSEC_UNIT_END_,
   5,
   {{__NNATT_UNIT_LINDEX_, UNSIGNED_INT, __NNATT_UNIT_LINDEX_TAG_}
    ,
    {__NNATT_UNIT_INDEX_, UNSIGNED_LONG_INT, __NNATT_UNIT_INDEX_TAG_}
    ,
    {__NNATT_UNIT_ACTV_CLASS_, STRING, __NNATT_UNIT_ACTV_CLASS_TAG_}
    ,
    {__NNATT_UNIT_ACTV_PARM_, REAL_VECTOR, __NNATT_UNIT_ACTV_PARM_TAG_}
    ,
    {__NNATT_UNIT_COORD_, REAL_VECTOR, __NNATT_UNIT_COORD_TAG_}
    }
   }
  ,

  /* Connection Section */
  {__NNSEC_CONN_,
   __NNSEC_CONN_START_,
   __NNSEC_CONN_END_,
   7,
   {{__NNATT_CONN_OLAYER_, UNSIGNED_INT, __NNATT_CONN_OLAYER_TAG_}
    ,
    {__NNATT_CONN_OUNIT_, UNSIGNED_LONG_INT, __NNATT_CONN_OUNIT_TAG_}
    ,
    {__NNATT_CONN_DLAYER_, UNSIGNED_INT, __NNATT_CONN_DLAYER_TAG_}
    ,
    {__NNATT_CONN_DUNIT_, UNSIGNED_LONG_INT, __NNATT_CONN_DUNIT_TAG_}
    ,
    {__NNATT_CONN_WEIGHT_, REAL, __NNATT_CONN_WEIGHT_TAG_}
    ,
    {__NNATT_CONN_WEIGHT_CLASS_, STRING, __NNATT_CONN_WEIGHT_CLASS_TAG_}
    ,
    {__NNATT_CONN_WEIGHT_PARM_, REAL_VECTOR, __NNATT_CONN_WEIGHT_PARM_TAG_}
    }
   }
  ,

  /* Layer Connection Section */
  {__NNSEC_LCNN_,
   __NNSEC_LCNN_START_,
   __NNSEC_LCNN_END_,
   5,
   {{__NNATT_LCNN_ORIGIN_, UNSIGNED_INT, __NNATT_LCNN_ORIGIN_TAG_}
    ,
    {__NNATT_LCNN_DESTINATION_, UNSIGNED_INT, __NNATT_LCNN_DESTINATION_TAG_}
    ,
    {__NNATT_LCNN_INITIAL_WEIGHT_, REAL, __NNATT_LCNN_INITIAL_WEIGHT_TAG_}
    ,
    {__NNATT_LCNN_WEIGHT_CLASS_, STRING, __NNATT_LCNN_WEIGHT_CLASS_TAG_}
    ,
    {__NNATT_LCNN_WEIGHT_PARM_, REAL_VECTOR, __NNATT_LCNN_WEIGHT_PARM_TAG_}
    }
   }

};



/******************************************************************************
 *                                                                            *
 *                             PRIVATE OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_file_process_nnetwork
 *
 * Creates the neural network according to the section attributes
 */
static int
nnet_file_process_nnetwork ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  Name nnet_name;               /* neural network name */
  Name ext_name;                /* extension name */

  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_nnetwork: error reading neural network attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_NNET_NAME_:
              value.stringvalue = nnet_name;
              nnet_file_parse_attribute (nnetfile, &value);
              break;

            case __NNATT_NNET_EXT_:
              value.stringvalue = ext_name;
              nnet_file_parse_attribute (nnetfile, &value);
              ext_index = nnet_extension_by_name (ext_name);
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_nnetwork: invalid neural network attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Creates the neural network */
          nnet = nnet_nnetwork_create (nnet_name, extension);
          if (nnet == NULL)
            {
              fprintf (stderr,
                       "nnet_file_process_nnetwork: error creating neural network\n");
              return EXIT_FAILURE;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_som
 *
 * Creates a new SOM extension according to the section attributes
 */
static int
nnet_file_process_som ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  Name ngb_name;                /* neighborhood class name */
  NgbFunctionClass ngb_class = NULL;    /* neighborhood funcion class */
  RValue *ngb_parm = NULL;      /* neighborhood function parameters */

  Name lrate_name;              /* learning rate class name */
  LRateFunctionClass lrate_class = NULL;        /* learning rate function class */
  RValue *lrate_parm = NULL;    /* learning rate function parameters */



  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_som: error reading SOM extension attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_XSOM_NGB_CLASS_:
              value.stringvalue = ngb_name;
              nnet_file_parse_attribute (nnetfile, &value);
              ngb_class = nnet_som_ngb_class_by_name (ngb_name);
              break;

            case __NNATT_XSOM_NGB_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              ngb_parm = value.rvectorvalue->value;
              break;

            case __NNATT_XSOM_LRATE_CLASS_:
              value.stringvalue = lrate_name;
              nnet_file_parse_attribute (nnetfile, &value);
              lrate_class = nnet_train_lrate_class_by_name (lrate_name);
              break;

            case __NNATT_XSOM_LRATE_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              lrate_parm = value.rvectorvalue->value;
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_som: invalid SOM extension attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Creates the SOM extension */
          som_nnet = nnet_som_create
            (nnet, ngb_class, ngb_parm, lrate_class, lrate_parm);

          if (som_nnet == NULL)
            {
              fprintf (stderr,
                       "nnet_file_process_som: error creating SOM extension\n");

              return EXIT_FAILURE;
            }

          /* Sets the global extension */
          extension = som_nnet;
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_layer
 *
 * Creates a new layer according to the section attributes
 */
static int
nnet_file_process_layer ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  LayerIndex index;             /* layer index */
  Name name;                    /* layer name */
  Name class_name;              /* layer class name */
  LayerClass layer_class = NULL;        /* layer class */
  UnitIndex nu_units = 0;       /* number of units */
  Name actv_name;               /* activation function class name */
  ActivationClass actv_class = NULL;    /* activation function class */
  RValue *actv_parm = NULL;     /* activation function parameters */
  Vector dist_vector = NULL;    /* distribution vector */
  Vector incr_vector = NULL;    /* increment vector */
  BoolValue dft_actv_class = TRUE;      /* use layer default activation class */
  BoolValue dft_actv_parm = TRUE;       /* use layer default activ. params */


  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_layer: error reading layer attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_LAYR_INDEX_:
              nnet_file_parse_attribute (nnetfile, &value);
              index = value.usintvalue;
              break;

            case __NNATT_LAYR_NAME_:
              value.stringvalue = name;
              nnet_file_parse_attribute (nnetfile, &value);
              break;

            case __NNATT_LAYR_CLASS_:
              value.stringvalue = class_name;
              nnet_file_parse_attribute (nnetfile, &value);
              layer_class = nnet_layer_class_by_name (class_name);
              break;

            case __NNATT_LAYR_NU_UNITS_:
              nnet_file_parse_attribute (nnetfile, &value);
              nu_units = value.uslgintvalue;
              break;

            case __NNATT_LAYR_ACTV_CLASS_:
              value.stringvalue = actv_name;
              nnet_file_parse_attribute (nnetfile, &value);
              actv_class = nnet_actv_class_by_name (actv_name);
              dft_actv_class = FALSE;
              break;

            case __NNATT_LAYR_ACTV_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              actv_parm = value.rvectorvalue->value;
              dft_actv_parm = FALSE;
              break;

            case __NNATT_LAYR_DIST_VECTOR_:
              nnet_file_parse_attribute (nnetfile, &value);
              dist_vector = value.rvectorvalue;
              break;

            case __NNATT_LAYR_INCR_VECTOR_:
              nnet_file_parse_attribute (nnetfile, &value);
              incr_vector = value.rvectorvalue;
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_layer: invalid layer attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Creates the layer */
          layer = nnet_layer_create (nnet, &index, layer_class, name);

          if (layer == NULL)
            {
              fprintf (stderr,
                       "nnet_file_process_layer: error creating layer\n");
              return EXIT_FAILURE;
            }

          /* Creates the units */
          if (nu_units > 0)
            {
              exit_status = nnet_unit_create_multiple
                (nu_units, layer, actv_class, actv_parm, dft_actv_class,
                 dft_actv_parm, dist_vector, incr_vector);

              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "nnet_file_process_layer: error creating %ld units\n",
                           nu_units);
                  return EXIT_FAILURE;
                }
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_unit
 *
 * Creates a new layer according to the section attributes
 */
static int
nnet_file_process_unit ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  Unit unit = NULL;             /* new unit */
  LayerIndex lindex = 0;        /* layer index */
  Layer int_layer = NULL;       /* unit's layer: default last layer */
  UnitIndex index = 0;          /* number of units */
  Name actv_name;               /* activation function class name */
  ActivationClass actv_class = NULL;    /* activation function class */
  RValue *actv_parm = NULL;     /* activation function parameters */
  Vector coord = NULL;          /* unit's coordinates */
  BoolValue dft_actv_class = TRUE;      /* use layer default activation class */
  BoolValue dft_actv_parm = TRUE;       /* use layer default activ. params */


  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_unit: error reading unit attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_UNIT_LINDEX_:
              nnet_file_parse_attribute (nnetfile, &value);
              lindex = value.usintvalue;
              int_layer = nnet_layer_by_index (nnet, lindex);
              if (int_layer == NULL)
                int_layer = layer;
              break;

            case __NNATT_UNIT_INDEX_:
              nnet_file_parse_attribute (nnetfile, &value);
              index = value.uslgintvalue;
              break;

            case __NNATT_UNIT_ACTV_CLASS_:
              value.stringvalue = actv_name;
              nnet_file_parse_attribute (nnetfile, &value);
              actv_class = nnet_actv_class_by_name (actv_name);
              dft_actv_class = FALSE;
              break;

            case __NNATT_UNIT_ACTV_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              actv_parm = value.rvectorvalue->value;
              dft_actv_parm = FALSE;
              break;

            case __NNATT_UNIT_COORD_:
              nnet_file_parse_attribute (nnetfile, &value);
              coord = value.rvectorvalue;
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_unit: invalid unit attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Creates the unit */
          unit = nnet_unit_create (int_layer, &index, actv_class, actv_parm,
                                   dft_actv_class, dft_actv_parm, coord);
          if (unit == NULL)
            {
              fprintf (stderr,
                       "nnet_file_process_unit: error creating unit\n");
              return EXIT_FAILURE;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_connection
 *
 * Connects two units according to the section attributes
 */
static int
nnet_file_process_connection ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  Connection new_conn = NULL;   /* new connection */
  LayerIndex orig_layer_index = -1;     /* origin layer index */
  LayerIndex dest_layer_index = -1;     /* destination layer index */
  UnitIndex orig_unit_index = -1;       /* origin unit index */
  UnitIndex dest_unit_index = -1;       /* destination unit index */
  Unit orig_unit = NULL;        /* origin unit */
  Unit dest_unit = NULL;        /* destination unit */
  RValue weight = 0.0;          /* initial weight */
  Name wght_name;               /* weight initialization class name */
  WeightInitClass wght_class = NULL;    /* weight initialization class */
  WeightInitParameters wght_parm = NULL;        /* activation function parameters */


  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_connection: error reading connection attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_CONN_OLAYER_:
              nnet_file_parse_attribute (nnetfile, &value);
              orig_layer_index = value.usintvalue;
              if (orig_unit_index != -1)
                {
                  orig_unit = nnet_unit_by_index
                    (nnet, orig_layer_index, orig_unit_index);
                }
              break;

            case __NNATT_CONN_DLAYER_:
              nnet_file_parse_attribute (nnetfile, &value);
              dest_layer_index = value.usintvalue;
              if (dest_unit_index != -1)
                {
                  dest_unit = nnet_unit_by_index
                    (nnet, dest_layer_index, dest_unit_index);
                }
              break;

            case __NNATT_CONN_OUNIT_:
              nnet_file_parse_attribute (nnetfile, &value);
              orig_unit_index = value.uslgintvalue;
              if (orig_layer_index != -1)
                {
                  orig_unit = nnet_unit_by_index
                    (nnet, orig_layer_index, orig_unit_index);
                }
              break;

            case __NNATT_CONN_DUNIT_:
              nnet_file_parse_attribute (nnetfile, &value);
              dest_unit_index = value.uslgintvalue;
              if (dest_layer_index != -1)
                {
                  dest_unit = nnet_unit_by_index
                    (nnet, dest_layer_index, dest_unit_index);
                }
              break;

            case __NNATT_CONN_WEIGHT_:
              nnet_file_parse_attribute (nnetfile, &value);
              weight = value.realvalue;
              break;

            case __NNATT_CONN_WEIGHT_CLASS_:
              value.stringvalue = wght_name;
              nnet_file_parse_attribute (nnetfile, &value);
              wght_class = nnet_wght_class_by_name (wght_name);
              break;

            case __NNATT_CONN_WEIGHT_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              wght_parm = value.rvectorvalue->value;
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_connection: invalid connection attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Connects the layers */
          new_conn = nnet_conn_create
            (orig_unit, dest_unit, weight, wght_class, wght_parm);

          if (new_conn == NULL)
            {
              fprintf (stderr,
                       "nnet_file_process_connection: error connecting units\n");
              return EXIT_FAILURE;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_layer_connection
 *
 * Connects layers according to section attributes
 */
static int
nnet_file_process_layer_connection ()
{
  BoolValue end_of_section = FALSE;     /* end of section found */
  CompositeUnion value;         /* current attribute value */
  int exit_status;              /* auxiliary function return status */

  LayerIndex orig_layer_index = -1;     /* origin layer index */
  LayerIndex dest_layer_index = -1;     /* destination layer index */
  Layer orig_layer = NULL;      /* origin layer */
  Layer dest_layer = NULL;      /* destination layer */
  RValue initial_weight = 0.0;  /* initial weights */
  Name wginit_name;             /* weight initialization function name */
  WeightInitClass wginit_class = NULL;  /* function */
  RValue *wginit_parm = NULL;   /* activation function parameters */


  while (end_of_section == FALSE)
    {
      /* Reads a new attribute */
      exit_status =
        nnet_file_read_attribute (nnetfile, &cur_attr, &end_of_section,
                                  __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_layer_connection: error reading layer connection attribute\n");
          return EXIT_FAILURE;
        }

      if (end_of_section == FALSE)
        {
          /* Sets the attributes */
          switch (cur_attr)
            {
            case __NNATT_LCNN_ORIGIN_:
              nnet_file_parse_attribute (nnetfile, &value);
              orig_layer_index = value.usintvalue;
              orig_layer = nnet_layer_by_index (nnet, orig_layer_index);
              break;

            case __NNATT_LCNN_DESTINATION_:
              nnet_file_parse_attribute (nnetfile, &value);
              dest_layer_index = value.usintvalue;
              dest_layer = nnet_layer_by_index (nnet, dest_layer_index);
              break;

            case __NNATT_LCNN_INITIAL_WEIGHT_:
              nnet_file_parse_attribute (nnetfile, &value);
              initial_weight = value.realvalue;
              break;

            case __NNATT_LCNN_WEIGHT_CLASS_:
              value.stringvalue = wginit_name;
              nnet_file_parse_attribute (nnetfile, &value);
              wginit_class = nnet_wght_class_by_name (wginit_name);
              break;

            case __NNATT_LCNN_WEIGHT_PARM_:
              nnet_file_parse_attribute (nnetfile, &value);
              wginit_parm = value.rvectorvalue->value;
              break;

            default:
              fprintf (stderr,
                       "nnet_file_process_layer_connection: invalid layer connection attribute\n");
              return EXIT_FAILURE;
            }
        }
      else
        {
          /* Connects the layers */
          exit_status = nnet_conn_connect_layers
            (orig_layer, dest_layer, initial_weight, wginit_class,
             wginit_parm);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_file_process_layer_connection: error connecting layers %d and %d\n",
                       orig_layer_index, dest_layer_index);
              return EXIT_FAILURE;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_process_section
 *
 * Processes one section according to its ID
 */
static int
nnet_file_process_section (int section_id)
{
  int exit_status;              /* auxiliary function return status */


  /*
   * Creates one neural network element according to the
   * associated session ID
   */
  switch (section_id)
    {
    case __NNSEC_NNET_:
      exit_status = nnet_file_process_nnetwork ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing neural network section\n");
          return EXIT_FAILURE;
        }
      break;


    case __NNSEC_XSOM_:
      exit_status = nnet_file_process_som ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing SOM extension section\n");
          return EXIT_FAILURE;
        }
      break;


    case __NNSEC_LAYR_:
      exit_status = nnet_file_process_layer ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing layer section\n");
          return EXIT_FAILURE;
        }
      break;


    case __NNSEC_UNIT_:
      exit_status = nnet_file_process_unit ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing unit section\n");
          return EXIT_FAILURE;
        }
      break;


    case __NNSEC_CONN_:
      exit_status = nnet_file_process_connection ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing connection section\n");
          return EXIT_FAILURE;
        }
      break;


    case __NNSEC_LCNN_:
      exit_status = nnet_file_process_layer_connection ();

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_process_section: error processing layer connection section\n");
          return EXIT_FAILURE;
        }
      break;

    default:
      fprintf (stderr, "nnet_file_process_section: invalid section ID\n");
      return EXIT_FAILURE;
    }

  cur_section = -1;
  cur_attr = 0;

  return EXIT_SUCCESS;
}



/*
 * nnet_file_write_nnetwork_section
 *
 * Writes the Neural Network Section to the given stream
 */
static void
nnet_file_write_nnetwork_section (NNetwork nnet, FILE * output_fd)
{
  fprintf (output_fd, "%s\n", __NNSEC_NNET_START_);

  fprintf (output_fd, "  %s %s\n", __NNATT_NNET_NAME_TAG_, nnet->name);

  if (nnet->extension != NULL)
    {
      fprintf (output_fd, "  %s %s\n", __NNATT_NNET_EXT_TAG_,
               nnet_extension_name[nnet->extension->index]);
    }

  fprintf (output_fd, "%s\n", __NNSEC_NNET_END_);

  return;
}



/*
 * nnet_file_write_som_section
 *
 * Writes the SOM Extension Section to the given stream
 */
static void
nnet_file_write_som_section (SomNNetwork som_nnet, FILE * output_fd)
{
  SomAttributes som_attr;       /* SOM attributes */


  som_attr = (SomAttributes) som_nnet->attr;

  fprintf (output_fd, "%s\n", __NNSEC_XSOM_START_);

  fprintf (output_fd, "  %s %s\n", __NNATT_XSOM_NGB_CLASS_TAG_,
           som_attr->ngb_function->function_class->ngb_class->name);


  if (som_attr->ngb_function->function_class->ngb_class->nu_parameters > 0)
    {
      fprintf (output_fd, "  %s ", __NNATT_XSOM_NGB_PARM_TAG_);
      nnet_file_write_vector
        (som_attr->ngb_function->function_class->ngb_class->nu_parameters,
         som_attr->ngb_function->function->parameters, output_fd);
    }

  fprintf (output_fd, "  %s %s\n", __NNATT_XSOM_LRATE_CLASS_TAG_,
           som_attr->lrate_function->function_class->name);

  if (som_attr->lrate_function->function_class->nu_parameters > 0)
    {
      fprintf (output_fd, "  %s ", __NNATT_XSOM_LRATE_PARM_TAG_);
      nnet_file_write_vector
        (som_attr->lrate_function->function_class->nu_parameters,
         som_attr->lrate_function->parameters, output_fd);
    }

  fprintf (output_fd, "%s\n", __NNSEC_XSOM_END_);

  return;
}



/*
 * nnet_file_write_layer_section
 *
 * Writes a Layer Section to the given stream
 */
static void
nnet_file_write_layer_section (Layer layer,
                               BoolValue write_units, FILE * output_fd)
{
  fprintf (output_fd, "%s\n", __NNSEC_LAYR_START_);

  fprintf (output_fd, "  %s %s\n", __NNATT_LAYR_NAME_TAG_, layer->name);
  fprintf (output_fd, "  %s %d\n", __NNATT_LAYR_INDEX_TAG_,
           layer->layer_index);
  fprintf (output_fd, "  %s %s\n", __NNATT_LAYR_CLASS_TAG_,
           nnet_layer_class_name[layer->layer_class->position]);
  fprintf (output_fd, "  %s %ld\n", __NNATT_LAYR_NU_UNITS_TAG_,
           (write_units == TRUE ? 0 : layer->nu_units));
  fprintf (output_fd, "  %s %s\n", __NNATT_LAYR_ACTV_CLASS_TAG_,
           layer->layer_class->default_activation_class->name);

  fprintf (output_fd, "%s\n", __NNSEC_LAYR_END_);

  return;
}



/*
 * nnet_file_write_unit_section
 *
 * Writes a Unit Section to the given stream
 */
static void
nnet_file_write_unit_section (Unit unit, FILE * output_fd)
{
  fprintf (output_fd, "%s\n", __NNSEC_UNIT_START_);

  fprintf (output_fd, "  %s %d\n", __NNATT_UNIT_LINDEX_TAG_,
           unit->layer->layer_index);
  fprintf (output_fd, "  %s %ld\n", __NNATT_UNIT_INDEX_TAG_,
           unit->unit_index);
  fprintf (output_fd, "  %s %s\n", __NNATT_UNIT_ACTV_CLASS_TAG_,
           unit->activation_function->function_class->name);
  if (unit->activation_function->function_class->nu_parameters > 0)
    {
      fprintf (output_fd, "  %s ", __NNATT_UNIT_ACTV_PARM_TAG_);
      nnet_file_write_vector
        (unit->activation_function->function_class->nu_parameters,
         unit->activation_function->parameters, output_fd);
    }
  if (unit->coord != NULL)
    {
      fprintf (output_fd, "  %s ", __NNATT_UNIT_COORD_TAG_);
      nnet_file_write_vector
        (unit->coord->dimension, unit->coord->value, output_fd);
    }

  fprintf (output_fd, "%s\n", __NNSEC_UNIT_END_);

  return;
}



/*
 * nnet_file_write_connection_section
 *
 * Writes a Connection Section to the given stream
 */
static void
nnet_file_write_connection_section (Connection connection, FILE * output_fd)
{
  fprintf (output_fd, "%s\n", __NNSEC_CONN_START_);

  fprintf (output_fd, "  %s %d\n", __NNATT_CONN_OLAYER_TAG_,
           connection->orig->layer->layer_index);
  fprintf (output_fd, "  %s %ld\n", __NNATT_CONN_OUNIT_TAG_,
           connection->orig->unit_index);
  fprintf (output_fd, "  %s %d\n", __NNATT_CONN_DLAYER_TAG_,
           connection->dest->layer->layer_index);
  fprintf (output_fd, "  %s %ld\n", __NNATT_CONN_DUNIT_TAG_,
           connection->dest->unit_index);
  fprintf (output_fd, "  %s %f\n", __NNATT_CONN_WEIGHT_TAG_,
           connection->weight);
  fprintf (output_fd, "  %s %s\n", __NNATT_CONN_WEIGHT_CLASS_TAG_,
           connection->wght_function->function_class->name);

  if (connection->wght_function->function_class->nu_parameters > 0)
    {
      fprintf (output_fd, "  %s ", __NNATT_CONN_WEIGHT_PARM_TAG_);
      nnet_file_write_vector
        (connection->wght_function->function_class->nu_parameters,
         connection->wght_function->parameters, output_fd);
    }

  fprintf (output_fd, "%s\n", __NNSEC_CONN_END_);

  return;
}



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
NNetwork
nnet_file_create_nnetwork (FILE * in_fd)
{
  int exit_status;              /* auxiliary function return status */


  /* Checks if the file name was passed */
  if (in_fd == NULL)
    {
      fprintf (stderr, "nnet_file_create_nnetwork: no input stream passed\n");
      return NULL;
    }

  /* Sets the global input file descriptor */
  input_fd = in_fd;

  /* Reads and parses the file */
  while (!feof (input_fd))
    {
      /* Searches a section start tag */
      exit_status = nnet_file_read_section (nnetfile, __NNSECTIONS_,
                                            __NNIGNORE_, stdin_prompt);

      if (exit_status != EXIT_SUCCESS && !feof (input_fd))
        {
          fprintf (stderr,
                   "nnet_file_create_nnetwork: error searching section start tag\n");
          return NULL;
        }

      /* Processes the section */
      if (!feof (input_fd))
        {
          /* Sets the standard input prompt */
          if (input_fd == stdin)
            sprintf (stdin_prompt, "%s%s ",
                     __STDIN_PROMPT_, nnetfile[cur_section].begin);

          exit_status = nnet_file_process_section (cur_section);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_file_create_nnetwork: error processing section %s\n",
                       nnetfile[cur_section].begin);
              return NULL;
            }
          /* Resets the standard input prompt */
          if (input_fd == stdin)
            sprintf (stdin_prompt, "%s", __STDIN_PROMPT_);
        }
    }


  if (nnet == NULL)
    {
      fprintf (stderr,
               "nnet_file_create_nnetwork: invalid neural network configuration file\n");
      return NULL;
    }

  return nnet;
}



/*
 * nnet_file_write_nnetwork
 *
 * Writes the given neural network to the given stream
 */
void
nnet_file_write_nnetwork (const NNetwork nnet,
                          const BoolValue write_extension,
                          const BoolValue write_nnetwork,
                          const BoolValue write_layers,
                          const BoolValue write_units,
                          const BoolValue write_connections, FILE * output_fd)
{
  Layer cur_layer;              /* current layer */
  Unit cur_unit;                /* current unit */
  Connection cur_connection;    /* current connection */


  /* Check if nothing to do */
  if (output_fd == NULL)
    return;

  if (nnet == NULL)
    return;

  /* Writes the Neural Network Section */
  if (write_nnetwork == TRUE)
    nnet_file_write_nnetwork_section (nnet, output_fd);

  /* Writes the Neural Network Extension Section */
  if (write_extension == TRUE && nnet->extension != NULL)
    {
      switch (nnet->extension->index)
        {
        case NNEXT_GEN:
          /* nothing to do */
          break;

        case NNEXT_SOM:
          fprintf (output_fd, "\n");
          nnet_file_write_som_section (som_nnet, output_fd);
          break;

        case NNEXT_LVQ:
          fprintf (output_fd, "\n");
          fprintf (output_fd, "LVQ EXTENSION NOT IMPLEMENTED YET\n");
          break;

        case NNEXT_MLP:
          fprintf (output_fd, "\n");
          fprintf (output_fd, "MLP EXTENSION NOT IMPLEMENTED YET\n");
          break;
        }
    }

  /* Writes the Layer Sections */
  if (write_layers == TRUE)
    {
      cur_layer = nnet->first_layer;
      while (cur_layer != NULL)
        {
          fprintf (output_fd, "\n");
          nnet_file_write_layer_section (cur_layer, write_units, output_fd);
          cur_layer = cur_layer->next;
        }
    }

  /* Writes the Unit Sections */
  if (write_units == TRUE)
    {
      cur_layer = nnet->first_layer;
      while (cur_layer != NULL)
        {
          cur_unit = cur_layer->first_unit;
          while (cur_unit != NULL)
            {
              fprintf (output_fd, "\n");
              nnet_file_write_unit_section (cur_unit, output_fd);
              cur_unit = cur_unit->next;
            }
          cur_layer = cur_layer->next;
        }
    }

  /* Writes the Connection Sections */
  if (write_connections == TRUE)
    {
      cur_layer = nnet->first_layer;
      while (cur_layer != NULL)
        {
          cur_unit = cur_layer->first_unit;
          while (cur_unit != NULL)
            {
              cur_connection = cur_unit->first_orig;
              while (cur_connection != NULL)
                {
                  fprintf (output_fd, "\n");
                  nnet_file_write_connection_section
                    (cur_connection, output_fd);
                  cur_connection = cur_connection->next_orig;
                }
              cur_unit = cur_unit->next;
            }
          cur_layer = cur_layer->next;
        }
    }

  return;
}
