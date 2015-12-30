#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "errorh.h"
#include "strutils.h"
#include "inparse.h"
#include "vector.h"
#include "vectorstat.h"
#include "table.h"
#include "frmap.h"

#include "nnet_som.h"
#include "nnet_lvq.h"
#include "nnet_types.h"
#include "nnet_nnet.h"
#include "nnet_layers.h"
#include "nnet_actv.h"
#include "nnet_weights.h"
#include "nnet_sets.h"
#include "nnet_train.h"
#include "nnet_files.h"
#include "nnet_files_nnet.h"

#ifdef __PROG_NAME_
#undef __PROG_NAME_
#endif
#define __PROG_NAME_ "som_rec"

#define __PROGRESS_BAR_WIDTH_ 50
#define __PROGRESS_BAR_CHAR_ '='


/******************************************************************************
 *                                                                            *
 *                              GLOBAL VARIABLES                              *
 *                                                                            *
 ******************************************************************************/

static FILE *log_fd;            /* log file descriptor */



/******************************************************************************
 *                                                                            *
 *                              PRIVATE FUNCTIONS                             *
 *                                                                            *
 ******************************************************************************/

static void
usage (void)
{
  puts ("");
  puts ("Usage: som_vq [-in | --input-network <file>]");
  puts ("              [-ct | --class-table <control file>]");
  puts ("              [-nt | --network-table <control file>]");
  puts ("              [-tt | --test-table <control file>]");
  puts ("              [-td | --train-dir <directory>]");
  puts ("              [-nd | --network-dir <directory>]");
  puts ("              [-t  | --train]");
  puts ("              [-r  | --randomize-set]");
  puts ("              [-e  | --max-epochs <number>]");
  puts ("              [-ie | --initial-epoch <number>]");
  puts ("              [-se | --save-epochs <number>]");
  puts ("              [-mr | --min-mse-error <number>]");
  puts ("              [-ss | --segment-size <number>]");
  puts ("              [-fs | --first-segment <number>]");
  puts ("              [-ls | --last-segment <number>]");
  puts ("              [-ef | --mse-error-file <file>]");
  puts ("              [-lf | --log-file <file>]");
  puts ("              [-h  | --help]");
  puts ("\nOptions are:\n");
  puts ("  -in | --input-network   input neural network file name");
  puts ("  -ct | --class-table     table of target classes");
  puts ("  -nt | --network-table   table of neural networks");
  puts ("  -tt | --test-table      table of test pattern files");
  puts ("  -td | --train-dir       train files directory");
  puts ("  -nd | --network-dir     network configuration files directory");
  puts ("  -t  | --train           execute neural network training");
  puts ("  -r  | --randomize-set   randomize training set before train");
  puts ("  -e  | --max-epochs      maximum training epochs");
  puts ("  -ie | --initial-epoch   initial epoch for resume training");
  puts ("  -se | --save-epochs     save network status each n epochs");
  puts ("  -mr | --min-mse-error   stop training when this MSE error");
  puts ("  -ss | --segment-size    segment size for segmental testing");
  puts ("  -fs | --first-segment   first segment to consider");
  puts ("  -ls | --last-segment    last segment to consider");
  puts ("  -ef | --mse-error-file  MSE error log file");
  puts ("  -lf | --log-file        output log file");
  puts ("  -h  | --help            outputs this help message and exit\n");
}



/*
 * get_class_name
 *
 * Returns the class name associated to the class index
 */
static char *
get_class_name (Table class_table, UsLgIntValue class_index)
{
  Row class_row = NULL;         /* row that corresponds to the class */
  Column class_col = NULL;      /* class index column */
  Column descr_col = NULL;      /* class name column */
  CompositeUnion value;         /* value to search for */
  BoolValue null = FALSE;       /* flag: description is NULL */

  if (class_table == NULL)
    return null_failure ("get_class_name", "class table not informed\n");

  if (class_table->columns < 2)
    return null_failure ("get_class_name",
                         "class table must have at least 2 rows: class index and class description\n");

  class_col = class_table->first_column;
  descr_col = class_col->next_column;

  if (class_col->type != UNSIGNED_LONG_INT)
    return null_failure ("get_class_name",
                         "class table's first column must be the class index (unsigned long int)\n");


  if (descr_col->type != STRING)
    return null_failure ("get_class_name",
                         "second column of class table must be class description (string)\n");

  value.uslgintvalue = class_index;

  class_row = table_select (class_table, class_col, value);

  if (class_row == NULL)
    return null_failure ("get_class_name",
                         "class %ld not found in class table\n", class_index);

  if (table_get_cell (class_row, descr_col, &value, &null) != EXIT_SUCCESS)
    return null_failure ("get_class_name",
                         "error getting class description\n");

  if (null == TRUE)
    return __NULL_STR_;
  else
    return value.stringvalue;
}



/*
 * create_network_from_row
 *
 * Creates the network according to the given row from the
 * networks table
 */
static int
create_network_from_row (Table net_table, Row net_row,
                         Table class_table, FILE * net_fd,
                         UnitIndex * input_dim)
{
  NNetwork network = NULL;      /* current row's network */
  Column class_col = NULL;      /* network's class index column */
  Column net_col = NULL;        /* auxiliary network handler column */
  CompositeUnion value;         /* auxiliary cell value */
  BoolValue null = TRUE;        /* current value is null? */
  UsLgIntValue class_index;     /* network's associated class index */
  char *class_name = NULL;      /* current network's associated class name */
  char *net_name = NULL;        /* new network name */

  /* NULL checks */
  if (net_table == NULL)
    return error_failure ("create_network_from_row",
                          "network table not informed\n");
  if (net_row == NULL)
    return error_failure ("create_network_from_row",
                          "network column not informed\n");

  /* initialization */
  class_col = net_table->first_column;
  net_col = net_table->last_column;

  /* gets network handler */
  if (table_get_cell (net_row, net_col, &value, &null) != EXIT_SUCCESS)
    return error_failure ("create_network_from_row",
                          "error getting current row's network handler\n");

  /* creates the network */
  network = nnet_file_create_nnetwork (net_fd);

  if (network == NULL)
    return error_failure ("create_network_from_row",
                          "error creating network from file\n");

  /* sets the network handler */
  value.handlervalue = (Handler) network;

  if (table_set_cell (net_row, net_col, value, FALSE) != EXIT_SUCCESS)
    return error_failure ("create_network_from_row",
                          "error setting network handler\n");

  /* gets network associated class */
  if (table_get_cell (net_row, class_col, &value, &null) != EXIT_SUCCESS)
    return error_failure ("create_network_from_row",
                          "error getting network's associated class\n");

  if (null == TRUE)
    return error_failure ("create_network_from_row",
                          "class associated to network is undefined\n");

  class_index = value.uslgintvalue;
  class_name = get_class_name (class_table, class_index);
  if (class_name == NULL)
    return error_failure ("create_network_from_row",
                          "error getting class name associated to network\n");

  /* combines network name with class name */
  net_name =
    (char *) malloc (strlen (network->name) + strlen (class_name) + 4);
  if (net_name == NULL)
    return error_failure ("create_network_from_row",
                          "can't get memory for new network name\n");

  sprintf (net_name, "%s - %s", network->name, class_name);
  strncpy (network->name, net_name, NAME_SIZE - 1);
  free (net_name);

  *input_dim = network->first_layer->nu_units;

  return EXIT_SUCCESS;
}



/*
 * create_networks_from_table
 *
 * Creates multiple networks, one for each row
 */
static Table
create_networks_from_table (char *net_tab_file, char *net_file, char *net_dir,
                            BoolValue trn_flag, Table class_table,
                            UnitIndex * input_dim)
{
  Table net_table = NULL;       /* table of neural networks */
  Column class_col = NULL;      /* class column in networks table */
  Column tfile_col = NULL;      /* trained network column */
  Row net_row = NULL;           /* current network row */
  char *used_net_file = NULL;   /* real net file to be used */
  FILE *net_fd = NULL;          /* input network configuration file */
  BoolValue last_row = FALSE;   /* flag: last row from network table */
  CompositeUnion value;         /* auxiliary net row cell value */
  BoolValue null = TRUE;        /* auxiliary NULL cell flag */

  /*
   * opens the networks table control file
   * creates the table by its control file
   * checks if it has a valid network table structure
   * adds a network handler as table's last column
   * populates the table
   */
  net_table = table_create_from_control_file (net_tab_file);

  if (net_table == NULL)
    return null_failure ("create_networks_from_table",
                         "error creating table of networks\n");

  if (net_table->columns < 2)
    return null_failure ("create_networks_from_table",
                         "table must have at leas two rows: class ID and output network file\n");

  class_col = net_table->first_column;
  tfile_col = class_col->next_column;

  if (class_col->type != UNSIGNED_LONG_INT)
    return null_failure ("create_networks_from_table",
                         "first column must be class ID (unsigned long)\n");

  if (tfile_col->type != STRING)
    return null_failure ("create_networks_from_table",
                         "second column must be output network file name (string)\n");

  if (table_add_column (net_table, "NET_HANDLER", HANDLER, FALSE) !=
      EXIT_SUCCESS)
    return null_failure ("create_networks_from_table",
                         "error adding auxiliary network handler column\n");

  if (table_read_data (net_table) != EXIT_SUCCESS)
    return null_failure ("create_networks_from_table",
                         "error reading network table data\n");

  /*
   * opens input network configuration file
   * creates one network for each row: all initially identical
   * closes input network configuration file
   */
  while (last_row == FALSE)
    {
      table_fetch (net_table, &last_row, &net_row);

      if (net_row == NULL)
        return null_failure ("create_networks_from_table",
                             "NULL row found in networks table\n");

      if (trn_flag == TRUE)
        {
          used_net_file = get_file_name (net_dir, net_file, NULL);
        }
      else
        {
          if (table_get_cell (net_row, tfile_col, &value, &null)
              != EXIT_SUCCESS)
            return null_failure ("create_networks_from_table",
                                 "error getting trained network file name\n");

          if (null == TRUE)
            return null_failure ("create_networks_from_table",
                                 "undefined trained network file name\n");

          used_net_file = get_file_name (net_dir, value.stringvalue, NULL);
        }

      fprintf (log_fd, "Using file '%s' as neural network configuration... ",
               used_net_file);

      net_fd = fopen (used_net_file, "r");

      if (net_fd == NULL)
        return null_failure ("create_networks_from_table",
                             "error opening input network configuration file '%s': %s\n",
                             used_net_file, strerror (errno));

      if (create_network_from_row (net_table, net_row, class_table,
                                   net_fd, input_dim) != EXIT_SUCCESS)
        {
          fputs ("FAILED\n", log_fd);

          return null_failure ("create_networks_from_table",
                               "error creating network table from row %ld\n",
                               net_row->rowid);
        }

      fputs ("OK\n", log_fd);
      fclose (net_fd);
      free (used_net_file);
    }

  return net_table;
}



/*
 * train_network
 *
 * Trains one network from the table of networks
 */
static int
train_network (NNetwork nnet, char *mse_file,
               char *trn_tab_file, char *tr_net_file,
               char *net_dir, char *dat_dir,
               DTime first_epoch, DTime max_epochs, DTime save_epochs,
               BoolValue rnd_flag, BoolValue mse_flag,
               BoolValue mse_stop, RValue stop_mse_error)
{
  SomNNetwork som_nnet = NULL;  /* current SOM extension */
  /*SomAttributes som_attr = NULL; *//* SOM attributes */
  /*LRateFunction lrate_function = NULL; *//* learning rate function */
  UnitIndex input_dim = 0;      /* input layer dimension */
  /*UnitIndex output_dim = 0; *//* output layer dimension */

  char *net_fext = ".net";      /* network file extension */

  char *ctl_dir = NULL;         /* control file directory */
  char *ctl_base = NULL;        /* control file base name */
  char *ctl_fext = NULL;        /* control file extension */

  char *trn_tab_dir = NULL;     /* training table directory */
  char *trn_tab_base = NULL;    /* training table base name */
  char *trn_tab_fext = NULL;    /* training table extension */

  char *tr_net_dir = NULL;      /* trained network output directory */
  char *tr_net_base = NULL;     /* trained network base name */

  char *sv_net_file = NULL;     /* savepoint neural network file name */
  char *sv_fext = NULL;         /* savepoint network file extension */

  FILE *sv_net_fd = NULL;       /* savepoint network file descriptor */
  FILE *tr_net_fd = NULL;       /* trained network file descriptor */
  FILE *mse_fd = NULL;          /* MSE errors file descriptor */

  Table trn_table = NULL;       /* current training files table */
  TSet trn_set = NULL;          /* training set */
  DTime epoch = 0;              /* current epoch */
  BoolValue rst_flag = TRUE;    /* flag: reset initial epoch */
  RValue trn_mse_error = 0.0;   /* training set MSE error */
  /*DTime max_epochs_order; *//* number of digits of max_epochs */
  time_t t_start, t_stop;       /* training start and stop times */
  double training_duration;     /* training duration */


  /* NULL checks */
  if (nnet == NULL)
    return error_failure (__PROG_NAME_, "no input network passed\n");

  if (trn_tab_file == NULL)
    return error_failure (__PROG_NAME_,
                          "no training set control file passed\n");

  /* Initialization */
  som_nnet = (SomNNetwork) nnet->extension;
  /*som_attr = (SomAttributes) som_nnet->attr; */
  /*lrate_function = som_attr->lrate_function; */
  input_dim = nnet->first_layer->nu_units;
  /*output_dim = nnet->last_layer->nu_units; */

  split_file_name (trn_tab_file, &trn_tab_dir, &trn_tab_base, &trn_tab_fext);

  /* savepoint network extension names */
  /*max_epochs_order = (DTime) log10 (max_epochs); */

  sv_fext = (char *) malloc (max_epochs + strlen (net_fext) + 3);
  if (sv_fext == NULL)
    return error_failure (__PROG_NAME_,
                          "error creating save file extension: %s\n",
                          strerror (errno));

  /* training table control file name */
  trn_tab_file = get_file_name (dat_dir, trn_tab_file, NULL);

  /* loads training table */
  trn_table = table_create_from_control_file (trn_tab_file);
  if (trn_table == NULL)
    return error_failure (__PROG_NAME_,
                          "error creating table of input files\n");

  if (trn_table->columns < 2)
    return error_failure (__PROG_NAME_,
                          "file table must have at least two columns: file name and associated class\n");

  if (trn_table->first_column->type != STRING)
    return error_failure (__PROG_NAME_,
                          "first column in file table must be file name string\n");

  if (trn_table->first_column->next_column->type != UNSIGNED_LONG_INT)
    return error_failure (__PROG_NAME_,
                          "second column in file table must be desired class, UNSIGNED_LONG_INT\n");

  if (table_read_data (trn_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_,
                          "error loading data into file table\n");

  split_file_name (trn_table->ctlfile, &ctl_dir, &ctl_base, &ctl_fext);

  /* creates the target training set */
  fprintf (log_fd, "\nTraining %s\n", nnet->name);

  fprintf (log_fd, "Using file '%s' as training files table... ",
           trn_tab_file);

  fflush (log_fd);

  /* creates and normalizes training set */
  trn_set = nnet_tset_create_from_table (trn_table, input_dim, 0,
                                         FALSE, FALSE, TRUE, TRUE, FALSE);
  if (trn_set == NULL)
    {
      fputs ("FAILED\n", log_fd);
      return error_failure (__PROG_NAME_,
                            "error creating training set from table %s\n",
                            trn_table->name);
    }
  else
    {
      fputs ("OK\n", log_fd);
    }

  fprintf (log_fd, "Training set has %ld elements\n", trn_set->nu_elements);

  fflush (log_fd);

  /* randomizes training set elements */
  if (rnd_flag == TRUE)
    {
      fprintf (log_fd, "Randomizing training set... ");
      fflush (log_fd);

      if (nnet_tset_randomize (trn_set) != EXIT_SUCCESS)
        {
          fputs ("FAILED\n", log_fd);
          return error_failure (__PROG_NAME_,
                                "error randomizing training set\n");
        }
      else
        {
          fputs ("OK\n", log_fd);
        }
    }

  /* Performs network training stage */
  t_start = time (NULL);
  fprintf (log_fd, "SOM training started at %s", ctime (&t_start));

  if (mse_flag == TRUE)
    fprintf (log_fd, "Using file '%s' as MSE error log\n", mse_file);

  fflush (log_fd);

  /* training loop */
  epoch = first_epoch;

  do
    {
      /* time reset */
      if (epoch == first_epoch)
        rst_flag = TRUE;
      else
        rst_flag = FALSE;

      /* executes one pass through the entire training set */
      if (nnet_som_train_set (som_nnet, trn_set, first_epoch, max_epochs,
                              rst_flag, TRUE, __PROGRESS_BAR_WIDTH_,
                              __PROGRESS_BAR_CHAR_, mse_flag, &trn_mse_error)
          != EXIT_SUCCESS)

        return error_failure (__PROG_NAME_,
                              "error executing training epoch %ld\n", epoch);

      trn_mse_error /= (RValue) trn_set->nu_elements;

      /* writes MSE error */
      if (mse_flag == TRUE)
        {
          mse_fd = fopen (mse_file, "a");
          if (mse_fd == NULL)
            return error_failure (__PROG_NAME_,
                                  "error opening MSE error file '%s': %s\n",
                                  mse_file, strerror (errno));

          fprintf (mse_fd, "%f\n", trn_mse_error);

          if (fclose (mse_fd) != 0)
            return error_failure (__PROG_NAME_, "error writing MSE file\n");
        }

      /* checks if savepoint is reached */
      if (save_epochs > 0)
        if (epoch % save_epochs == 0 && epoch > first_epoch)
          {
            /*
             * savepoint network configuration name
             * creates savepoint file
             * writes the current neural network to savepoint file
             */

            sprintf (sv_fext, ".t%ld%s", epoch, net_fext);

            sv_net_file = get_file_name (tr_net_dir, tr_net_base, sv_fext);

            sv_net_fd = fopen (sv_net_file, "w");
            if (sv_net_fd == NULL)
              return error_failure (__PROG_NAME_,
                                    "error creating savepoint network file\n");

            nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE,
                                      TRUE, sv_net_fd);

            fclose (sv_net_fd);
          }

      epoch++;
    }
  while (epoch <= max_epochs &&
         ((mse_stop == TRUE && trn_mse_error - stop_mse_error > DBL_EPSILON)
          || (mse_stop == FALSE)));

  t_stop = time (NULL);

  if (epoch < max_epochs)
    printf ("|\n");

  fprintf (log_fd, "Neural network training finished at %s", ctime (&t_stop));

  /* Calculates training stage elapsed time */
  training_duration = difftime (t_stop, t_start);

  fprintf (log_fd, "Training stage lasted %ld seconds\n",
           (long) training_duration);

  fprintf (log_fd, "Final training MSE error is %f\n", trn_mse_error);

  /* Destroys training set */
  fprintf (log_fd, "Destroying training set... ");
  fflush (log_fd);

  if (nnet_tset_destroy (&trn_set, TRUE) != EXIT_SUCCESS)
    {
      fputs ("FAILED\n", log_fd);
      return error_failure (__PROG_NAME_, "error destroying training set\n");
    }
  else
    {
      fputs ("OK\n", log_fd);
    }

  /* Destroys the file table */
  fprintf (log_fd, "Destroying file table... ");
  fflush (log_fd);

  if (table_drop (&trn_table) != EXIT_SUCCESS)
    {
      fputs ("FAILED\n", log_fd);
      return error_failure (__PROG_NAME_, "error dropping file table\n");
    }
  else
    {
      fputs ("OK\n", log_fd);
    }

  /* Save trained neural network configuration */
  if (tr_net_file != NULL)
    {
      tr_net_file = get_file_name (net_dir, tr_net_file, NULL);
      tr_net_fd = fopen (tr_net_file, "w");

      if (tr_net_fd == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating trained network file '%s': %s\n",
                              tr_net_file, strerror (errno));

      fprintf (log_fd,
               "Using file '%s' to save trained network configuration... ",
               tr_net_file);

      fflush (log_fd);

      nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE, TRUE,
                                tr_net_fd);

      fputs ("OK\n", log_fd);
      fclose (tr_net_fd);
    }

  free (sv_fext);
  free (trn_tab_file);

  return EXIT_SUCCESS;
}



/*
 * train_table_of_networks
 *
 * Trains all networks in the given table
 */
static int
train_table_of_networks (Table net_table, char *mse_file,
                         char *net_dir, char *dat_dir,
                         DTime first_epoch, DTime max_epochs,
                         DTime save_epochs, BoolValue rnd_flag,
                         BoolValue mse_flag, BoolValue mse_stop,
                         RValue stop_mse_error)
{
  Row net_row = NULL;           /* current network row */
  Column net_col = NULL;        /* network handler column */
  Column trn_col = NULL;        /* training table control file column */
  Column tnt_col = NULL;        /* trained network file column */
  BoolValue last_row = FALSE;   /* flag: last row from network table */
  NNetwork nnet = NULL;         /* current neural network */
  CompositeUnion aux_value;     /* auxiliary composite */
  BoolValue null = FALSE;       /* auxiliary null boolean */
  char *trn_tab_file = NULL;    /* training table control file */
  char *tr_net_file = NULL;     /* trained neural network file name */


  /* initialization */
  net_col = net_table->last_column;
  trn_col = net_col->prev_column;
  tnt_col = trn_col->prev_column;

  while (last_row == FALSE)
    {
      /* fetches one network from table */
      table_fetch (net_table, &last_row, &net_row);

      /* gets network handler */
      if (table_get_cell (net_row, net_col, &aux_value, &null)
          != EXIT_SUCCESS)
        return error_failure ("train_table_of_networs",
                              "error getting network handler\n");
      if (null == TRUE)
        return error_failure ("train_table_of_networks",
                              "network handler not defined yet\n");

      nnet = (NNetwork) aux_value.handlervalue;

      /* gets training table control file name */
      if (table_get_cell (net_row, trn_col, &aux_value, &null)
          != EXIT_SUCCESS)
        return error_failure ("train_table_of_networks",
                              "error getting training set control file name\n");

      if (null == TRUE)
        return error_failure ("train_table_of_networks",
                              "training set control file name is undefined\n");

      trn_tab_file = aux_value.stringvalue;

      /* gets trained network configuration file name */
      if (table_get_cell (net_row, tnt_col, &aux_value, &null)
          != EXIT_SUCCESS)

        return error_failure ("train_table_of_networks",
                              "error getting trained network configuration file name\n");

      if (null == TRUE)
        tr_net_file = NULL;
      else
        tr_net_file = aux_value.stringvalue;

      /* trains the current network */
      if (train_network (nnet, mse_file, trn_tab_file, tr_net_file,
                         net_dir, dat_dir,
                         first_epoch, max_epochs, save_epochs,
                         rnd_flag, mse_flag, mse_stop, stop_mse_error)
          != EXIT_SUCCESS)

        return error_failure ("train_table_of_networks",
                              "error training SOM network\n");
    }

  return EXIT_SUCCESS;
}



/*
 * test_table_of_networks
 *
 * Executes the test for all files in test table
 */
static int
test_table_of_networks (Table net_table, Table class_table,
                        char *tst_tab_file, char *dat_dir,
                        UnitIndex input_dim, UsLgIntValue segment_size,
                        RValue * correct_rate)
{
  Table tst_table = NULL;       /* current test files table */
  Column net_col = NULL;        /* network handler column in network table */
  Column file_col = NULL;       /* file name column in test table */
  Column des_class_col = NULL;  /* desired class column in test table */
  Column net_class_col = NULL;  /* network associated class column */
  Row net_row = NULL;           /* current network row */
  Row tst_row = NULL;           /* current test pattern row */
  BoolValue last_tst = FALSE;   /* flag: last element in test table */
  BoolValue last_net = FALSE;   /* flag: last network in net table */
  CompositeUnion value;         /* neural network handler cell value */
  BoolValue null = FALSE;       /* flag: current handler undefined */

  NNetwork cur_nnet = NULL;     /* current network being tested */
  SomNNetwork som_nnet = NULL;  /* current network's SOM extension */
  TSet tst_set = NULL;          /* test set */
  char *tst_file = NULL;        /* current test pattern file name */

  RValue tst_mse_error = 0.0;   /* test set MSE error */
  RValue min_mse_error = 0.0;   /* minimum network MSE error */
  BoolValue reset_win = TRUE;   /* flag: reset winner */

  UsLgIntValue desired_class;   /* desired class for current test pattern */
  UsLgIntValue net_class = 0;   /* network associated class */
  UsLgIntValue win_class = 0;   /* winner network associated class */
  Vector winners = NULL;        /* winners vector */

  TSet sgm_set = NULL;          /* current segment training set */
  FrequencyMap cls_freq = NULL; /* classification frequency vector */
  UsLgIntValue matches = 0;     /* number of correct classifications */
  ElementIndex transf_elements; /* elements to transfer to segment set */

  /* creates test table */
  tst_table = table_create_from_control_file (tst_tab_file);
  if (tst_table == NULL)
    return error_failure (__PROG_NAME_,
                          "error creating table of test files\n");

  if (tst_table->columns < 2)
    return error_failure (__PROG_NAME_,
                          "file table must have at least two columns: file name and associated class\n");

  if (tst_table->first_column->type != STRING)
    return error_failure (__PROG_NAME_,
                          "first column in file table must be file name string\n");

  if (tst_table->first_column->next_column->type != UNSIGNED_LONG_INT)
    return error_failure (__PROG_NAME_,
                          "second column in file table must be desired class, UNSIGNED_LONG_INT\n");

  if (table_read_data (tst_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_,
                          "error loading data into file table\n");

  /* creates frequency map */
  cls_freq = frmap_create (net_table->rows, 0.0);

  if (cls_freq == NULL)
    return error_failure (__PROG_NAME_, "error creting frequency map\n");

  /* initialization */
  file_col = tst_table->first_column;
  des_class_col = file_col->next_column;
  net_col = net_table->last_column;
  net_class_col = net_table->first_column;

  /* test loop */
  fprintf (log_fd, "Using table %s as test table... ", tst_table->name);
  fflush (log_fd);

  while (last_tst == FALSE)
    {
      /* test patterns loop */
      table_fetch (tst_table, &last_tst, &tst_row);

      if (tst_row == NULL)
        return error_failure (__PROG_NAME_, "NULL row found in test table\n");

      if (table_get_cell (tst_row, file_col, &value, &null) != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error getting test pattern file name\n");

      if (null == TRUE)
        return error_failure (__PROG_NAME_,
                              "undefined test pattern file name\n");

      tst_file = get_file_name (dat_dir, value.stringvalue, NULL);

      if (table_get_cell (tst_row, des_class_col, &value, &null)
          != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error getting test pattern desired class\n");

      if (null == TRUE)
        return error_failure (__PROG_NAME_, "undefined test pattern class\n");

      desired_class = value.uslgintvalue;

      fprintf (log_fd, "\nTarget class: %s\n",
               get_class_name (class_table, desired_class));

      /* creates and normalizes test set */
      tst_set = nnet_tset_create_from_file (tst_file, input_dim, 0,
                                            FALSE, FALSE, TRUE, TRUE, FALSE,
                                            tst_file);

      if (tst_set == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating test set for file '%s'\n",
                              tst_file);


      /* defines actual segment size */
      if (segment_size > tst_set->nu_elements || segment_size == 0)
        transf_elements = tst_set->nu_elements;
      else
        transf_elements = segment_size;

      while (tst_set->nu_elements > segment_size)
        {
          /* transfers segment to segment set */
          if (nnet_tset_divide (tst_set, &sgm_set, "SEGMENT SET",
                                transf_elements, PICK_FROM_BEGGINING)
              != EXIT_SUCCESS)

            return error_failure (__PROG_NAME_,
                                  "error segmenting test set\n");

          while (last_net == FALSE)
            {
              /* networks loop */
              table_fetch (net_table, &last_net, &net_row);

              if (net_row == NULL)
                return error_failure (__PROG_NAME_,
                                      "NULL row found in networks table\n");

              if (table_get_cell (net_row, net_col, &value, &null) !=
                  EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error getting network handler\n");

              if (null == TRUE)
                return error_failure (__PROG_NAME_,
                                      "undefined network handler\n");

              cur_nnet = (NNetwork) value.handlervalue;

              if (cur_nnet->extension->index != NNEXT_SOM)
                return error_failure (__PROG_NAME_,
                                      "current neural network doesn't have SOM architecture\n");

              som_nnet = (SomNNetwork) cur_nnet->extension;

              if (table_get_cell (net_row, net_class_col, &value, &null)
                  != EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error getting current network associated class\n");

              if (null == TRUE)
                return error_failure (__PROG_NAME_,
                                      "undefined associated class for current network\n");

              net_class = value.uslgintvalue;

              /* segment set propagation */
              winners = nnet_som_propagate_set (som_nnet, sgm_set,
                                                TRUE, &tst_mse_error);

              if (winners == NULL)
                return error_failure (__PROG_NAME_,
                                      "error propagating test pattern\n");

              if (vector_destroy (&winners) != EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error destroying winners vector\n");

              if (tst_mse_error - min_mse_error < DBL_EPSILON
                  || reset_win == TRUE)
                {
                  if (reset_win == TRUE)
                    reset_win = FALSE;

                  win_class = net_class;
                  min_mse_error = tst_mse_error;
                }
            }

          /* points the winner */
          if (frmap_occurrence (cls_freq, win_class) != EXIT_SUCCESS)
            return error_failure (__PROG_NAME_,
                                  "error registering winner in histogram\n");

          /* destroys segment set */
          if (nnet_tset_destroy (&sgm_set, TRUE) != EXIT_SUCCESS)
            return error_failure (__PROG_NAME_,
                                  "error destroying segment set\n");

          /* resets variables */
          net_row = NULL;
          last_net = FALSE;
          win_class = 0;
          min_mse_error = 0.0;
          reset_win = TRUE;
        }

      /* destroys test set */
      if (nnet_tset_destroy (&tst_set, TRUE) != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_, "error destroying test set\n");

      free (tst_file);

      /* defines most frequent winner */
      win_class = frmap_most_frequent_state (cls_freq);

      if (frmap_reset (cls_freq, 0.0) != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_, "error resetting histogram\n");

      fprintf (log_fd, "Winner class: %s\n",
               get_class_name (class_table, win_class));

      fflush (log_fd);

      if (win_class == desired_class)
        matches++;

      net_row = NULL;
      last_net = FALSE;
      win_class = 0;
      min_mse_error = 0.0;
      reset_win = TRUE;
    }

  *correct_rate = (RValue) matches / (RValue) tst_table->rows;

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                                    MAIN                                    *
 *                                                                            *
 ******************************************************************************/

int
main (int argc, char **argv)
{
  char *net_file = NULL;        /* neural network file name */
  char *cls_tab_file = NULL;    /* class table control file */
  char *net_tab_file = NULL;    /* neural network table control file */
  char *tst_tab_file = NULL;    /* test table control file */
  char *mse_file = NULL;        /* MSE errors file */
  char *log_file = NULL;        /* output log file */

  char *dat_dir = "./";         /* training table directory */
  char *net_dir = "./";         /* input network directory */

  Table cls_table = NULL;       /* target classes table */
  Table net_table = NULL;       /* neural networks table */

  BoolValue trn_flag = FALSE;   /* flag: execute training */
  BoolValue tst_flag = FALSE;   /* flag: execute testing */
  BoolValue rnd_flag = FALSE;   /* flag: randomize training set */

  DTime max_epochs;             /* maximum training epochs */
  DTime first_epoch = 1;        /* start with this epoch */
  DTime save_epochs = 0;        /* save network status each n epochs */

  BoolValue mse_flag = FALSE;   /* calculate MSE error? */
  UnitIndex input_dim = 0;      /* input layer dimension */
  RValue correct_rate = 0.0;    /* correct patterns in test table */

  RValue stop_mse_error = 0.0;  /* stop training when this MSE error */
  BoolValue mse_stop = FALSE;   /* flag: stop when acceptable MSE error */

  UsLgIntValue seg_size = 0;    /* segment size for segmented testing */
  /*UsLgIntValue seg_first = 0; *//* first pattern segment to consider */
  /*UsLgIntValue seg_last = 0; *//* last pattern segment to consider */

  /* command line parameters */
  InputParameterSet pset = {
    {"-h", "--help", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-in", "--input-network", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-ct", "--class-table", STRING, FALSE, TRUE,
     {.stringvalue = (char *) NULL}},
    {"-nt", "--network-table", STRING, FALSE, TRUE,
     {.stringvalue = (char *) NULL}},
    {"-tt", "--test-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-t", "--train", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-r", "--randomize-set", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-e", "--max-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1000}},
    {"-se", "--save-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-ie", "--initial-epoch", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1}},
    {"-ef", "--mse-error-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-lf", "--log-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-td", "--train-dir", STRING, FALSE, FALSE,
     {.stringvalue = "./"}},
    {"-nd", "--network-dir", STRING, FALSE, FALSE,
     {.stringvalue = "./"}},
    {"-mf", "--min-mse-error", REAL, FALSE, FALSE,
     {.realvalue = 0.0}},
    {"-ss", "--segment-size", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-fs", "--first-segment", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-ls", "--last-segment", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
  };

  InputParameterList plist = { 18, pset };



/******************************************************************************
 *                                                                            *
 *                                 PARAMETERS                                 *
 *                                                                            *
 ******************************************************************************/

  /* Parses the command line */
  if (inpr_parse (argc, argv, plist) != EXIT_SUCCESS)
    {
      usage ();
      return error_failure (__PROG_NAME_, "error parsing command line\n");
    }

  /* Help request */
  if (plist.parameter[0].passed == TRUE)
    {
      usage ();
      return EXIT_SUCCESS;
    }

  /* network configuration file name */
  if (plist.parameter[1].passed == TRUE)
    net_file = plist.parameter[1].value.stringvalue;

  /* class table control file name */
  if (plist.parameter[2].passed == TRUE)
    cls_tab_file = plist.parameter[2].value.stringvalue;

  /* neural network table control file */
  if (plist.parameter[3].passed == TRUE)
    net_tab_file = plist.parameter[3].value.stringvalue;

  /* test pattern files control file */
  if (plist.parameter[4].passed == TRUE)
    {
      tst_tab_file = plist.parameter[4].value.stringvalue;
      tst_flag = TRUE;
    }

  /* execute training? */
  if (plist.parameter[5].passed == TRUE)
    trn_flag = TRUE;

  /* randomize training set? */
  if (plist.parameter[6].passed == TRUE && trn_flag == TRUE)
    rnd_flag = TRUE;

  /* maximum training epochs */
  max_epochs = (DTime) plist.parameter[7].value.uslgintvalue;

  /* savepoint epochs */
  save_epochs = (DTime) plist.parameter[8].value.uslgintvalue;

  /* first epoch */
  first_epoch = (DTime) plist.parameter[9].value.uslgintvalue;

  /* MSE error */
  if (plist.parameter[10].passed == TRUE)
    {
      mse_file = plist.parameter[10].value.stringvalue;
      mse_flag = TRUE;
    }

  /* log file name */
  if (plist.parameter[11].passed == TRUE)
    log_file = plist.parameter[11].value.stringvalue;

  /* training table directory */
  if (plist.parameter[12].passed == TRUE)
    dat_dir = plist.parameter[12].value.stringvalue;

  /* network configuration files directory */
  if (plist.parameter[13].passed == TRUE)
    net_dir = plist.parameter[13].value.stringvalue;

  /* acceptable MSE error */
  if (plist.parameter[14].passed == TRUE)
    {
      stop_mse_error = plist.parameter[14].value.realvalue;
      mse_stop = TRUE;
    }


  /* Parameter consistency check */
  if (trn_flag == TRUE && net_file == NULL)
    return error_failure (__PROG_NAME_, "no input network file passed\n");

  if (save_epochs > max_epochs)
    return error_failure (__PROG_NAME_, "savepoint epochs out of range\n");

  if (first_epoch < 1)
    return error_failure (__PROG_NAME_,
                          "initial epoch must be 1 or greater\n");



/******************************************************************************
 *                                                                            *
 *                               INITIALIZATION                               *
 *                                                                            *
 ******************************************************************************/

  /* using log file? */
  if (log_file != NULL)
    {
      log_fd = fopen (log_file, "a");
      if (log_fd == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating log file '%s': %s\n",
                              log_file, strerror (errno));
    }
  else
    {
      log_fd = stdout;
    }



/******************************************************************************
 *                                                                            *
 *                           NEURAL NETWORKS CREATION                         *
 *                                                                            *
 ******************************************************************************/

  /* loads the class table */
  fprintf (log_fd, "Using file '%s' as class table control file... ",
           cls_tab_file);

  fflush (log_fd);

  errno = 0;

  cls_table = table_create_from_control_file (cls_tab_file);

  if (cls_table == NULL)
    {
      fputs ("FAILED\n", log_fd);
      return error_failure (__PROG_NAME_,
                            "error creting class table from file '%s': %s\n",
                            cls_tab_file, strerror (errno));
    }
  else
    {
      fputs ("OK\n", log_fd);
    }

  fflush (log_fd);

  if (table_read_data (cls_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "error loading class table data\n");

  /* creates the networks by the table of networks */
  fprintf (log_fd, "Using file '%s' as networks table...\n", net_tab_file);

  fflush (log_fd);

  net_table = create_networks_from_table (net_tab_file, net_file, net_dir,
                                          trn_flag, cls_table, &input_dim);

  if (net_table == NULL)
    {
      fputs ("FAILED\n", log_fd);

      return error_failure (__PROG_NAME_,
                            "error creating neural networks table from file '%s'\n",
                            net_tab_file);
    }
  else
    {
      fputs ("OK\n", log_fd);
    }

  fflush (log_fd);



/******************************************************************************
 *                                                                            *
 *                                  TRAINING                                  *
 *                                                                            *
 ******************************************************************************/

  if (trn_flag == TRUE)
    {
      /* creates the networks by the table of networks */
      if (train_table_of_networks (net_table, mse_file, net_dir, dat_dir,
                                   first_epoch, max_epochs, save_epochs,
                                   rnd_flag, mse_flag,
                                   mse_stop, stop_mse_error) != EXIT_SUCCESS)


        return error_failure (__PROG_NAME_,
                              "error executing training stage\n");
    }



/******************************************************************************
 *                                                                            *
 *                                    TESTING                                 *
 *                                                                            *
 ******************************************************************************/

  /*
   * creates test set
   * propagates test set
   * shows MSE error
   */
  if (tst_flag == TRUE)

    if (test_table_of_networks (net_table, cls_table, tst_tab_file,
                                dat_dir, input_dim, seg_size, &correct_rate)
        != EXIT_SUCCESS)

      return error_failure (__PROG_NAME_, "error executing test stage\n");

  fprintf (log_fd, "Test set correct rate is %3.2f%%\n",
           correct_rate * 100.0);



/******************************************************************************
 *                                                                            *
 *                                FINALIZATION                                *
 *                                                                            *
 ******************************************************************************/

  /* closes log file */
  if (log_fd != stdout)
    if (fclose (log_fd) != 0)
      return error_failure (__PROG_NAME_, "error writing log file '%s': %s\n",
                            log_file, strerror (errno));

  return EXIT_SUCCESS;
}
