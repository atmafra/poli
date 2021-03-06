#include <stdio.h>
#include <stdlib.h>

#include "nnet_weights.h"
#include "nnet_conns.h"
#include "../incstat/incstat.h"

/******************************************************************************
 *                                                                            *
 *                             PRIVATE OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

#define __ORIG_ 0
#define __DEST_ 1

typedef enum
{
  ORIG = __ORIG_,
  DEST = __DEST_
}
nnet_conn_node;


/*
 * nnet_conn_add_connection
 *
 * Links a connection to an unit.
 * Can link origin connections or destination connections.
 */
static int nnet_conn_add_connection
  (Connection conn, Unit unit, const nnet_conn_node node)
{
  /* Checks if the elements were passed */
  if (conn == NULL)
    {
      fprintf (stderr, "nnet_conn_add_connection: no connection given\n");
      return EXIT_FAILURE;
    }

  if (unit == NULL)
    {
      fprintf (stderr, "nnet_conn_add_connection: no unit to connect\n");
      return EXIT_FAILURE;
    }

  /* Determines the list to use: origin or destination */
  switch (node)
    {
    case ORIG:

      /* Checks if the given unit is the destination of the connection */
      if (conn->orig != unit)
        {
          fprintf (stderr,
                   "nnet_conn_add_connection: wrong destination unit\n");
          return EXIT_FAILURE;
        }

      /* Appends to the list of destinations of the origin unit */
      if (unit->first_dest == NULL)
        unit->first_dest = conn;        /* empty list */
      else
        unit->last_dest->next_dest = conn;      /* non-empty list */

      unit->last_dest = conn;

      break;

    case DEST:

      /* Checks if the given unit is the origin of the connection */
      if (conn->dest != unit)
        {
          fprintf (stderr, "nnet_conn_add_connection: wrong origin unit\n");
          return EXIT_FAILURE;
        }

      /* Appends to the list of origins of the destination unit */
      if (unit->first_orig == NULL)
        unit->first_orig = conn;
      else
        unit->last_orig->next_orig = conn;

      unit->last_orig = conn;

      break;

    default:
      fprintf (stderr,
               "nnet_conn_add_connection: invalid value for 'node'\n");
      return EXIT_FAILURE;
    }

  /* Increments the number of connections */
  switch (node)
    {
    case ORIG:
      ++unit->nu_outputs;
      break;

    case DEST:
      ++unit->nu_inputs;
      break;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_remove_connection
 *
 * Unlinks a connection from/to an unit.
 * Can unlink origin connections or destination connections.
 */
static int nnet_conn_remove_connection
  (Connection conn, const nnet_conn_node node)
{
  Connection aux_conn;          /* auxiliary pointer to connections */


  /* Checks if a connection was actually passed */
  if (conn == NULL)
    {
      fprintf (stderr,
               "nnet_conn_remove_connection: no connection to remove\n");
      return EXIT_FAILURE;
    }

  /* Removes the connection */
  switch (node)
    {
    case ORIG:

      if (conn == conn->orig->first_dest)
        {
          /* Removing first connection */
          conn->orig->first_dest = conn->next_dest;
          aux_conn = NULL;      /* for tail tracking */
        }
      else
        {
          /* Goes to the connection before */
          aux_conn = conn->orig->first_dest;

          while (aux_conn->next_dest != conn && aux_conn->next_dest != NULL)
            aux_conn = aux_conn->next_dest;

          if (aux_conn->next_dest == NULL)
            {
              /* Connection not found */
              fprintf (stderr,
                       "nnet_conn_remove_connection: connection already removed from origin layer destinations list\n");
              return EXIT_FAILURE;
            }

          /* Unlinks the connection */
          aux_conn->next_dest = conn->next_dest;
        }

      /* Keep track of the last connection */
      if (conn == conn->orig->last_dest)
        conn->orig->last_dest = aux_conn;

      /* Update the number of outputs */
      --conn->orig->nu_outputs;

      break;


    case DEST:

      if (conn == conn->dest->first_orig)
        {
          /* Removing first connection */
          conn->dest->first_orig = conn->next_orig;
          aux_conn = NULL;      /* for tail tracking */
        }
      else
        {
          /* Goes to the connection before */
          aux_conn = conn->dest->first_orig;

          while (aux_conn->next_orig != conn && aux_conn->next_orig != NULL)
            aux_conn = aux_conn->next_orig;

          if (aux_conn->next_orig == NULL)
            {
              /* Connection not found */
              fprintf (stderr,
                       "nnet_conn_remove_connection: connection already removed from destination layer origins list\n");
              return EXIT_FAILURE;
            }

          /* Unlinks the connection */
          aux_conn->next_orig = conn->next_orig;
        }

      /* Keep track of the last connection */
      if (conn == conn->dest->last_orig)
        conn->dest->last_orig = aux_conn;

      /* Updates the number of input connections */
      --conn->dest->nu_inputs;

      break;


    default:
      fprintf (stderr,
               "nnet_conn_remove_connection: invalid value for 'node'\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_conn_create
 *
 * Creates a connection between two units
 */
Connection
nnet_conn_create (Unit orig, Unit dest,
                  const RValue weight,
                  const WeightInitClass weight_init_class,
                  const WeightInitParameters weight_init_parameters)
{
  Connection new_conn;          /* new connection to be created */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the origin and destination units were passed */
  if (orig == NULL || dest == NULL)
    {
      fprintf (stderr,
               "nnet_conn_create: origin and destination units must be informed\n");
      return NULL;
    }

  /* Allocates the new connection */
  new_conn = (Connection) malloc (sizeof (nnet_conn_type));
  if (new_conn == NULL)
    {
      fprintf (stderr, "nnet_conn_create: vitual memory exhausted\n");
      return NULL;
    }

  /* Initial attributes */
  new_conn->orig = orig;
  new_conn->dest = dest;
  new_conn->weight = weight;
  new_conn->next_orig = NULL;
  new_conn->next_dest = NULL;

  /* Adds the connection to the origin unit */
  exit_status = nnet_conn_add_connection (new_conn, orig, ORIG);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "nnet_conn_create: error linking origin unit\n");
      free (new_conn);
      return NULL;
    }

  /* Adds the connection to the destination unit */
  exit_status = nnet_conn_add_connection (new_conn, dest, DEST);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "nnet_conn_create: error linking destination unit\n");
      free (new_conn);
      return NULL;
    }

  /* Instantiates a weight initialization function */
  new_conn->wght_function = nnet_wght_create_function (weight_init_class);

  if (new_conn->wght_function == NULL)
    {
      fprintf (stderr,
               "nnet_conn_create: error creating weight initialization function\n");
      return NULL;
    }

  /* Sets the parameters for the weight initialization function */
  if (weight_init_parameters != NULL)
    {
      exit_status =
        function_set_parameter_vector (new_conn->wght_function,
                                       weight_init_parameters);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_conn_create: error setting parameter vector for weight initialization function\n");
          return NULL;
        }
    }

  return new_conn;
}



/*
 * nnet_conn_create_init
 *
 * Creates a connection between two units and initializes its weight
 * according to the given weight initialization function
 */
Connection
nnet_conn_create_init (Unit orig, Unit dest,
                       const WeightInitClass weight_init_class,
                       const WeightInitParameters weight_init_parameters)
{
  Connection new_conn;          /* pointer to the new connection */
  int exit_status;              /* auxiliary function return status */


  /* Creates the connection with this weight */
  new_conn =
    nnet_conn_create (orig, dest, 0.0, weight_init_class,
                      weight_init_parameters);

  if (new_conn == NULL)
    {
      fprintf (stderr, "nnet_conn_create_init: error creating connection\n");
      return NULL;
    }

  /* Initializes the connection's weight */
  exit_status = nnet_conn_init_weight (new_conn);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_create_init: error initializing new connection\n");
      exit_status = nnet_conn_destroy (&new_conn);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_conn_create_init: error destroying new connection\n");
        }
      return NULL;
    }

  return new_conn;
}



/*
 * nnet_conn_connect_layers
 *
 * Connects all units from the origin to the destination layer
 */
int
nnet_conn_connect_layers (Layer orig, Layer dest,
                          const RValue weight,
                          const WeightInitClass weight_init_class,
                          const WeightInitParameters weight_init_parms)
{
  /* pointers to the current origin and destination units */
  Unit cur_orig = NULL, cur_dest = NULL;

  Connection aux_conn;          /* auxiliary connection */


  /* Checks if the layers were passed */
  if (orig == NULL)
    {
      fprintf (stderr, "nnet_conn_connect_layers: no origin layer\n");
      return EXIT_FAILURE;
    }

  if (dest == NULL)
    {
      fprintf (stderr, "nnet_conn_connect_layers: no destination layer\n");
      return EXIT_FAILURE;
    }

  /* Gets a new random seed */
  istt_gaussian_randomize ();

  /* Connects the units */
  cur_orig = orig->first_unit;

  while (cur_orig != NULL)
    {
      cur_dest = dest->first_unit;

      while (cur_dest != NULL)
        {
          /* Creates and initializes the connection */
          aux_conn = nnet_conn_create_init
            (cur_orig, cur_dest, weight_init_class, weight_init_parms);

          if (aux_conn == NULL)
            {
              fprintf (stderr,
                       "nnet_conn_connect_layers: error connecting units\n");
              return EXIT_FAILURE;
            }

          /* Goes to the next destination unit */
          cur_dest = cur_dest->next;
        }

      /* Goes to the next origin unit */
      cur_orig = cur_orig->next;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_destroy
 *
 * Destroys a previously created connection between two units
 */
int
nnet_conn_destroy (Connection * connection)
{
  int exit_status;              /* auxiliary function return status */


  /* Check if a connection was actually passed */
  if (connection == NULL || *connection == NULL)
    {
      fprintf (stderr, "nnet_conn_destroy: no connection to destroy\n");
      return EXIT_FAILURE;
    }

  /* Removes the connection from the origin unit */
  exit_status = nnet_conn_remove_connection (*connection, ORIG);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_destroy: error removing connection from origin unit destinations list\n");
      return EXIT_FAILURE;
    }

  /* Removes the connection from the destination unit */
  exit_status = nnet_conn_remove_connection (*connection, DEST);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_destroy: error removing connection from destination unit origins list\n");
      return EXIT_FAILURE;
    }

  /* Destroys the weight initialization function */
  function_destroy (&((*connection)->wght_function));

  /* Frees up connection memory */
  free (*connection);

  /* Makes it point to NULL */
  *connection = NULL;

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_disconnect_unit_inputs
 *
 * Destroys all connections (if any) TO the given unit
 */
int
nnet_conn_disconnect_unit_inputs (Unit unit)
{
  Connection aux_conn;          /* pointer to the connection to destroy */
  int exit_status;              /* auxiliary function return status */

  if (unit->nu_inputs > 0)
    while (unit->first_orig != NULL)
      {
        aux_conn = unit->first_orig;

        exit_status = nnet_conn_destroy (&aux_conn);

        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr,
                     "nnet_conn_disconnect_unit_inputs: error destroying input connection\n");
            return EXIT_FAILURE;
          }
      }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_disconnect_unit_outputs
 *
 * Destroys all connections (if any) FROM the given unit
 */
int
nnet_conn_disconnect_unit_outputs (Unit unit)
{
  Connection aux_conn;          /* pointer to the connection to destroy */
  int exit_status;              /* auxiliary function return status */

  if (unit->nu_outputs > 0)
    while (unit->first_dest != NULL)
      {
        aux_conn = unit->first_dest;

        exit_status = nnet_conn_destroy (&aux_conn);

        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr,
                     "nnet_conn_disconnect_unit_outputs: error destroying output connection\n");
            return EXIT_FAILURE;
          }
      }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_disconnect_unit
 *
 * Destroys all connections (if any) FROM and TO the given unit
 */
int
nnet_conn_disconnect_unit (Unit unit)
{
  int exit_status;              /* auxiliary function return status */

  /* Checks if the unit was actually passed */
  if (unit == NULL)
    {
      fprintf (stderr, "nnet_conn_disconnect_unit: no unit to disconnect\n");
      return EXIT_FAILURE;
    }

  /* Disconnects the outputs */
  exit_status = nnet_conn_disconnect_unit_outputs (unit);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_disconnect_unit: error disconnecting outputs\n");
      return EXIT_FAILURE;
    }

  /* Disconnects the inputs */
  exit_status = nnet_conn_disconnect_unit_inputs (unit);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_disconnect_unit: error disconnecting inputs\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_set_weight
 *
 * Sets the weight of a connection to the given value
 */
int
nnet_conn_set_weight (Connection conn, const RValue weight)
{
  /* Checks if a connection was actually passed */
  if (conn == NULL)
    {
      fprintf (stderr, "nnet_conn_set_weight: no connection to set weight\n");
      return EXIT_FAILURE;
    }

  /* Updates the weight to the given value */
  conn->weight = weight;

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_init_weight
 *
 * Initializes the weight of the connection based on the weight initialization
 * function
 */
int
nnet_conn_init_weight (Connection conn)
{
  RValue initial_weight;        /* initial weight value */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the connection was passed */
  if (conn == NULL)
    {
      fprintf (stderr, "nnet_conn_init_weight: no connection passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the connection has a weight initialization function */
  if (conn->wght_function == NULL)
    {
      fprintf (stderr,
               "nnet_conn_init_weight: connection has no weight initialization function\n");
      return EXIT_FAILURE;
    }

  /* Gets the weight initialization function value */
  initial_weight = nnet_wght_value (conn->wght_function);

  /* Sets the initial weight of the connection */
  exit_status = nnet_conn_set_weight (conn, initial_weight);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_conn_init_weight: error setting initial weight value for connection\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_increment_weight
 *
 * Increments the current weight of the connection by the given delta
 */
int
nnet_conn_increment_weight (Connection conn, const RValue delta_weight)
{
  /* Checks if a connection was actually passed */
  if (conn == NULL)
    {
      fprintf (stderr,
               "nnet_conn_increment_weight: no connection to increment weight\n");
      return EXIT_FAILURE;
    }

  /* Updates the weight to the given value */
  conn->weight += delta_weight;

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_info
 *
 * Outputs information about the connection
 */
void nnet_conn_info
  (const Connection conn,
   const BoolValue include_origin,
   const BoolValue include_destination, FILE * output_fd)
{
  if (conn == NULL || output_fd == NULL)
    return;

  if (include_origin == TRUE)
    {
      fprintf (output_fd, "From Layer: %d\n", conn->orig->layer->layer_index);
      fprintf (output_fd, "From Unit : %ld\n", conn->orig->unit_index);
      fprintf (output_fd, "Weight    : %f\n", conn->weight);
    }

  if (include_destination == TRUE)
    {
      fprintf (output_fd, "To Layer  : %d", conn->dest->layer->layer_index);

      if (conn->dest->layer == conn->orig->layer)
        fprintf (output_fd, " (self) \n");
      else
        fprintf (output_fd, "\n");

      fprintf (output_fd, "To Unit   : %ld", conn->dest->unit_index);

      if (conn->dest == conn->orig)
        fprintf (output_fd, " (self) \n");
      else
        fprintf (output_fd, "\n");

      fprintf (output_fd, "Weight    : %f\n", conn->weight);
    }

  return;
}
