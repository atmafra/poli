#include <stdio.h>
#include <stdlib.h>

#include "incstat.h"
#include "errorh.h"

#include "nnet_weights.h"
#include "nnet_conns.h"

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
    return error_failure ("nnet_conn_add_connection",
                          "no connection given\n");

  if (unit == NULL)
    return error_failure ("nnet_conn_add_connection", "no unit to connect\n");

  /* Determines the list to use: origin or destination */
  switch (node)
    {
    case ORIG:

      /* Checks if the given unit is the destination of the connection */
      if (conn->orig != unit)
        return error_failure ("nnet_conn_add_connection",
                              "wrong destination unit\n");

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
        return error_failure ("nnet_conn_add_connection",
                              "wrong origin unit\n");

      /* Appends to the list of origins of the destination unit */
      if (unit->first_orig == NULL)
        unit->first_orig = conn;
      else
        unit->last_orig->next_orig = conn;

      unit->last_orig = conn;

      break;

    default:
      return error_failure ("nnet_conn_add_connection",
                            "invalid value for 'node'\n");
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
    return error_failure ("nnet_conn_remove_connection",
                          "no connection to remove\n");

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
            return error_failure ("nnet_conn_remove_connection",
                                  "connection already removed from origin layer destinations list\n");

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
            return error_failure ("nnet_conn_remove_connection",
                                  "connection already removed from destination layer origins list\n");

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
      return error_failure ("nnet_conn_remove_connection",
                            "invalid value for 'node'\n");
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


  /* Checks if the origin and destination units were passed */
  if (orig == NULL || dest == NULL)
    return null_failure ("nnet_conn_create",
                         "origin and destination units must be informed\n");

  /* Allocates the new connection */
  new_conn = (Connection) malloc (sizeof (nnet_conn_type));

  if (new_conn == NULL)
    return null_failure ("nnet_conn_create", "vitual memory exhausted\n");

  /* Initial attributes */
  new_conn->orig = orig;
  new_conn->dest = dest;
  new_conn->weight = weight;
  new_conn->next_orig = NULL;
  new_conn->next_dest = NULL;

  /* Adds the connection to the origin unit */
  if (nnet_conn_add_connection (new_conn, orig, ORIG) != EXIT_SUCCESS)
    {
      free (new_conn);
      return null_failure ("nnet_conn_create", "error linking origin unit\n");
    }

  /* Adds the connection to the destination unit */
  if (nnet_conn_add_connection (new_conn, dest, DEST) != EXIT_SUCCESS)
    {
      free (new_conn);
      return null_failure ("nnet_conn_create",
                           "error linking destination unit\n");
    }

  /* Instantiates a weight initialization function */
  new_conn->wght_function = nnet_wght_create_function (weight_init_class);

  if (new_conn->wght_function == NULL)
    return null_failure ("nnet_conn_create",
                         "error creating weight initialization function\n");

  /* Sets the parameters for the weight initialization function */
  if (weight_init_parameters != NULL)

    if (function_set_parameter_vector (new_conn->wght_function,
                                       weight_init_parameters) !=
        EXIT_SUCCESS)

      return null_failure ("nnet_conn_create",
                           "error setting parameter vector for weight initialization function\n");

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


  /* Creates the connection with this weight */
  new_conn = nnet_conn_create (orig, dest, 0.0, weight_init_class,
                               weight_init_parameters);

  if (new_conn == NULL)
    return null_failure ("nnet_conn_create_init",
                         "error creating connection\n");

  /* Initializes the connection's weight */
  if (nnet_conn_init_weight (new_conn) != EXIT_SUCCESS)
    {
      error_failure ("nnet_conn_create_init",
                     "error initializing new connection\n");

      if (nnet_conn_destroy (&new_conn) != EXIT_SUCCESS)
        error_failure ("nnet_conn_create_init",
                       "error destroying new connection\n");

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
    return error_failure ("nnet_conn_connect_layers", "no origin layer\n");

  if (dest == NULL)
    return error_failure ("nnet_conn_connect_layers",
                          "no destination layer\n");

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
          aux_conn = nnet_conn_create_init (cur_orig, cur_dest,
                                            weight_init_class,
                                            weight_init_parms);

          if (aux_conn == NULL)
            return error_failure ("nnet_conn_connect_layers",
                                  "error connecting units\n");

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
  /* Check if a connection was actually passed */
  if (connection == NULL || *connection == NULL)
    return error_failure ("nnet_conn_destroy", "no connection to destroy\n");

  /* Removes the connection from the origin unit */
  if (nnet_conn_remove_connection (*connection, ORIG) != EXIT_SUCCESS)
    return error_failure ("nnet_conn_destroy",
                          "error removing connection from origin unit destinations list\n");

  /* Removes the connection from the destination unit */
  if (nnet_conn_remove_connection (*connection, DEST) != EXIT_SUCCESS)
    return error_failure ("nnet_conn_destroy",
                          "error removing connection from destination unit origins list\n");

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

  if (unit->nu_inputs > 0)

    while (unit->first_orig != NULL)
      {
        aux_conn = unit->first_orig;

        if (nnet_conn_destroy (&aux_conn) != EXIT_SUCCESS)
          return error_failure ("nnet_conn_disconnect_unit_inputs",
                                "error destroying input connection\n");
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

  if (unit->nu_outputs > 0)

    while (unit->first_dest != NULL)
      {
        aux_conn = unit->first_dest;

        if (nnet_conn_destroy (&aux_conn) != EXIT_SUCCESS)
          return error_failure ("nnet_conn_disconnect_unit_outputs",
                                "error destroying output connection\n");
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
  /* Checks if the unit was actually passed */
  if (unit == NULL)
    return error_failure ("nnet_conn_disconnect_unit",
                          "no unit to disconnect\n");

  /* Disconnects the outputs */
  if (nnet_conn_disconnect_unit_outputs (unit) != EXIT_SUCCESS)

    return error_failure ("nnet_conn_disconnect_unit",
                          "error disconnecting outputs\n");

  /* Disconnects the inputs */
  if (nnet_conn_disconnect_unit_inputs (unit) != EXIT_SUCCESS)

    return error_failure ("nnet_conn_disconnect_unit",
                          "error disconnecting inputs\n");

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
    return error_failure ("nnet_conn_set_weight",
                          "no connection to set weight\n");

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


  /* Checks if the connection was passed */
  if (conn == NULL)

    return error_failure ("nnet_conn_init_weight", "no connection passed\n");

  /* Checks if the connection has a weight initialization function */
  if (conn->wght_function == NULL)

    return error_failure ("nnet_conn_init_weight",
                          "connection has no weight initialization function\n");

  /* Gets the weight initialization function value */
  initial_weight = nnet_wght_value (conn->wght_function);

  /* Sets the initial weight of the connection */
  if (nnet_conn_set_weight (conn, initial_weight) != EXIT_SUCCESS)

    return error_failure ("nnet_conn_init_weight",
                          "error setting initial weight value for connection\n");

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
    return error_failure ("nnet_conn_increment_weight",
                          "no connection to increment weight\n");

  /* Updates the weight to the given value */
  conn->weight += delta_weight;

  return EXIT_SUCCESS;
}



/*
 * nnet_conn_info
 *
 * Outputs information about the connection
 */
void
nnet_conn_info (const Connection conn,
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
}
