#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "errorh.h"
#include "strutils.h"
#include "vector.h"
#include "frmap.h"
#include "inparse.h"
#include "table.h"

#include "nnet_som.h"
#include "nnet_lvq.h"
#include "nnet_types.h"
#include "nnet_nnet.h"
#include "nnet_actv.h"
#include "nnet_weights.h"
#include "nnet_sets.h"
#include "nnet_train.h"
#include "nnet_files.h"
#include "nnet_files_nnet.h"

#ifdef __PROG_NAME_
#undef __PROG_NAME_
#endif
#define __PROG_NAME_ "som_vq_f"

#define __PROGRESS_BAR_WIDTH_ 50
#define __PROGRESS_BAR_CHAR_ '#'

typedef enum
{ SINGLE_FILE, MULTI_FILE }
file_mode_type;


void
usage (void)
{
  puts ("");
  puts ("Usage: som_vq [-in | --input-network <file>]");
  puts ("              [-on | --output-network <file>]");
  puts ("              [-i  | --input <file>]");
  puts ("              [-rt | --train-table <control file>]");
  puts ("              [-st | --test-table <control file>]");
  puts ("              [-s  | --states-file <file>]");
  puts ("              [-sd | --states-dir <directory>]");
  puts ("              [-m  | --map-file <file>]");
  puts ("              [-md | --map-dir <directory>]");
  puts ("              [-t  | --train]");
  puts ("              [-r  | --randomize-set]");
  puts ("              [-e  | --max-epochs <number>]");
  puts ("              [-ie | --initial-epoch <number>]");
  puts ("              [-se | --save-epochs <number>]");
  puts ("              [-ef | --mse-error-file <file>]");
  puts ("              [-h  | --help]\n");
  puts ("Options are:\n");
  puts ("  -in | --input-network   input neural network file name");
  puts ("  -on | --output-network  output trained neural network file name");
  puts ("  -i  | --input           input training set file name");
  puts ("  -rt | --train-table     table of training input files");
  puts ("  -st | --test-table      table of test input files");
  puts ("  -s  | --states-file     list of states file name");
  puts
    ("  -sd | --states-dir      output directory for multiple state files");
  puts ("  -m  | --map-file        transition map output file name");
  puts ("  -md | --map-dir         output directory for multiple map files");
  puts ("  -t  | --train           execute training of input neural network");
  puts ("  -r  | --randomize-set   randomize training set before train");
  puts ("  -e  | --max-epochs      maximum training epochs");
  puts ("  -ie | --initial-epoch   initial epoch for resume training");
  puts ("  -se | --save-epochs     save network status each n epochs");
  puts ("  -ef | --mse-error-file  MSE error log file");
  puts ("  -h  | --help            outputs this help message and exit\n");
}



int
main (int argc, char **argv)
{
  char *net_fext = ".net";      /* network file extension */
  char *tr_net_fext = NULL;     /* trained network file extension */
  char *sv_fext = NULL;         /* savepoint network file extension */
  char *tm_fext = ".map";       /* transition map file extension */
  char *sl_fext = ".sl";        /* status list file extension */

  char *net_file = NULL;        /* neural network file name */
  char *tr_net_file = NULL;     /* trained neural network file name */
  char *sv_net_file = NULL;     /* savepoint neural network file name */
  char *set_file = NULL;        /* training set file name */
  char *tm_file = NULL;         /* transition map file name */
  char *sl_file = NULL;         /* status list file name */
  char *trn_tab_file = NULL;    /* training table control file */
  char *tst_tab_file = NULL;    /* test table control file */
  char *mse_file = NULL;        /* MSE errors file */

  FILE *net_fd = NULL;          /* input network file descriptor */
  FILE *tr_net_fd = NULL;       /* trained network file descriptor */
  FILE *sv_net_fd = NULL;       /* savepoint network file descriptor */
  FILE *tm_fd = NULL;           /* transition map file descriptor */
  FILE *sl_fd = NULL;           /* status list file descriptor */
  FILE *mse_fd = NULL;          /* MSE errors file descriptor */

  char *tr_net_dir = NULL;      /* trained network output directory */
  char *tr_net_base = NULL;     /* trained network base name */
  char *tm_dir = NULL;          /* transition map output directory */
  char *sl_dir = NULL;          /* status list output directory */

  file_mode_type file_mode;     /* single/multi-file input */

  Table trn_table = NULL;       /* training files table */
  Table tst_table = NULL;       /* test files table */
  Row cur_row = NULL;           /* current file table row */
  RValue desired_class;         /* desired class for file */
  BoolValue last_row = FALSE;   /* last file in table? */
  CompositeUnion cellval;       /* file name cell */
  BoolValue nullval = TRUE;     /* file name cell is null? */
  char *ctl_dir = NULL;         /* control file directory */
  char *ctl_base = NULL;        /* control file base name */
  char *ctl_ext = NULL;         /* control file extension */

  NNetwork nnet = NULL;         /* neural network created */
  SomNNetwork som_nnet = NULL;  /* SOM extension */
  /*SomAttributes som_attr = NULL; *//* SOM attributes */
  /*LRateFunction lrate_function = NULL; *//* learning rate function */
  UnitIndex input_dim;          /* input layer dimension */
  UnitIndex output_dim;         /* output layer dimension */

  BoolValue trn_flag = FALSE;   /* flag: execute training */
  BoolValue tst_flag = FALSE;   /* flag: execute testing */
  BoolValue rnd_flag = FALSE;   /* flag: randomize training set */
  BoolValue rst_flag = TRUE;    /* flag: reset initial epoch */
  DTime epoch = 0;              /* current epoch */
  DTime max_epochs;             /* maximum training epochs */
  DTime first_epoch = 1;        /* start with this epoch */
  DTime save_epochs = 0;        /* save network status each n epochs */
  /*DTime max_epochs_order; *//* number of digits of max_epochs */
  TSet trn_set = NULL;          /* training set */
  TSet tst_set = NULL;          /* test set */
  TSet aux_set = NULL;          /* auxiliary training set */
  time_t t_start, t_stop;       /* training start and stop times */
  double training_duration;     /* training duration */
  BoolValue mse_flag = FALSE;   /* calculate MSE error? */
  RValue trn_mse_error = 0.0;   /* training set MSE error */
  RValue tst_mse_error = 0.0;   /* test set MSE error */

  Vector winners = NULL;        /* list of states */
  FrequencyMap frmap = NULL;    /* state frequency map */
  BoolValue tm_flag = FALSE;    /* flag: output transition map to file */
  BoolValue sl_flag = FALSE;    /* flag: output status list to file */



  /* command line parameters */
  InputParameterSet pset = {
    {"-h", "--help", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-in", "--input-network", STRING, FALSE, TRUE,
     {.stringvalue = (char *) NULL}},
    {"-on", "--output-network", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-i", "--input", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-rt", "--train-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-s", "--states-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-sd", "--states-dir", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-m", "--map-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-md", "--map-dir", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-t", "--train", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-e", "--max-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1000}},
    {"-se", "--save-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-ie", "--initial-epoch", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1}},
    {"-ef", "--mse-error-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-r", "--randomize-set", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-st", "--test-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
  };

  InputParameterList plist = { 16, pset };



/******************************************************************************
 *                                                                            *
 *                                 PARAMETERS                                 *
 *                                                                            *
 ******************************************************************************/

  /* Parses the command line */
  if (error_if_failure (inpr_parse (argc, argv, plist), __PROG_NAME_,
                        "error parsing command line\n"))
    {
      usage ();
      return EXIT_FAILURE;
    }

  /* Usage request */
  if (plist.parameter[0].passed == TRUE)
    {
      usage ();
      return EXIT_SUCCESS;
    }

  /* network configuration file name */
  if (plist.parameter[1].passed == TRUE)
    net_file = plist.parameter[1].value.stringvalue;

  /* trained neural network output file name */
  if (plist.parameter[2].passed == TRUE)
    tr_net_file = plist.parameter[2].value.stringvalue;

  /* training set (input) file name */
  if (plist.parameter[3].passed == TRUE)
    set_file = plist.parameter[3].value.stringvalue;

  /* file containing a list of input files */
  if (plist.parameter[4].passed == TRUE)
    trn_tab_file = plist.parameter[4].value.stringvalue;

  /* list of states (output) file name */
  if (plist.parameter[5].passed == TRUE)
    {
      sl_file = plist.parameter[5].value.stringvalue;
      sl_flag = TRUE;
    }

  /* multiple output list of states directory */
  if (plist.parameter[6].passed == TRUE)
    {
      sl_dir = plist.parameter[6].value.stringvalue;
      sl_flag = TRUE;
    }

  /* state frequency map file name */
  if (plist.parameter[7].passed == TRUE)
    {
      tm_file = plist.parameter[7].value.stringvalue;
      tm_flag = TRUE;
    }

  /* output directory for multiple files */
  if (plist.parameter[8].passed == TRUE)
    {
      tm_dir = plist.parameter[8].value.stringvalue;
      tm_flag = TRUE;
    }

  /* execute training? */
  if (plist.parameter[9].passed == TRUE)
    trn_flag = TRUE;

  /* maximum training epochs */
  max_epochs = (DTime) plist.parameter[10].value.uslgintvalue;

  /* savepoint epochs */
  save_epochs = (DTime) plist.parameter[11].value.uslgintvalue;

  /* first epoch */
  first_epoch = (DTime) plist.parameter[12].value.uslgintvalue;

  /* MSE error */
  if (plist.parameter[13].passed == TRUE)
    {
      mse_file = plist.parameter[13].value.stringvalue;
      mse_flag = TRUE;
    }

  /* randomize training set? */
  if (plist.parameter[14].passed == TRUE && trn_flag == TRUE)
    rnd_flag = TRUE;

  /* test set table control file */
  if (plist.parameter[15].passed == TRUE)
    {
      tst_flag = TRUE;
      tst_tab_file = plist.parameter[15].value.stringvalue;
    }

  /* Running single or multi-file? */
  if (trn_tab_file != NULL)
    file_mode = MULTI_FILE;
  else
    file_mode = SINGLE_FILE;

  /* Parameter consistency check */
  if (file_mode == SINGLE_FILE && tst_flag == FALSE)
    {
      if (set_file == NULL)
        return error_failure (__PROG_NAME_, "no input file passed\n");

      if (sl_file == NULL)
        return error_failure (__PROG_NAME_, "no output file passed\n");
    }

  if (save_epochs > max_epochs)
    return error_failure (__PROG_NAME_, "savepoint epochs out of range\n");

  if (first_epoch < 1)
    return error_failure (__PROG_NAME_,
                          "initial epoch must be 1 or greater\n");

  /* Initialization */
  /*max_epochs_order = (DTime) log10 (max_epochs); */

  sv_fext = (char *) malloc (max_epochs + strlen (net_fext) + 3);
  if (sv_fext == NULL)
    return error_failure (__PROG_NAME_,
                          "error creating save file extension: %s\n",
                          strerror (errno));

  if (tr_net_file != NULL)
    split_file_name (tr_net_file, &tr_net_dir, &tr_net_base, &tr_net_fext);

  if (trn_tab_file != NULL)
    {
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


      split_file_name (trn_table->ctlfile, &ctl_dir, &ctl_base, &ctl_ext);
    }

/******************************************************************************
 *                                                                            *
 *                           NEURAL NETWORK CREATION                          *
 *                                                                            *
 ******************************************************************************/

  /* Creates the network by the configuration file */
  if (net_file != NULL)
    {
      if (error_if_null (net_fd = fopen (net_file, "r"),
                         __PROG_NAME_,
                         "error opening network file '%s': %s\n", net_file,
                         strerror (errno)))
        return EXIT_FAILURE;
    }

  printf ("Using file '%s' to create neural network... ", net_file);
  fflush (stdout);

  nnet = nnet_file_create_nnetwork (net_fd);
  if (nnet == NULL)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_,
                            "error creating neural network using file '%s'\n",
                            net_file);
    }
  else
    {
      fclose (net_fd);
      puts ("OK");
      fflush (stdout);
    }

  /* Initialization */
  som_nnet = (SomNNetwork) nnet->extension;
  /*som_attr = (SomAttributes) som_nnet->attr; */
  /*lrate_function = som_attr->lrate_function; */
  input_dim = nnet->first_layer->nu_units;
  output_dim = nnet->last_layer->nu_units;


/******************************************************************************
 *                                                                            *
 *                             TRAINING SET CREATION                          *
 *                                                                            *
 ******************************************************************************/

  if (trn_flag == TRUE)
    {
      if (file_mode == SINGLE_FILE)
        {
          /* creates a new training set to the given file */
          printf ("Using file '%s' to create training set... ", set_file);
          fflush (stdout);

          trn_set = nnet_tset_create_from_file ("SOM Training Set",
                                                input_dim, 0,
                                                FALSE, FALSE, TRUE, TRUE,
                                                FALSE, set_file);
          if (trn_set == NULL)
            {
              puts ("FAILED");
              return error_failure (__PROG_NAME_,
                                    "error creating training set from file '%s'\n",
                                    set_file);
            }
          else
            {
              puts ("OK");
            }
        }
      else
        {
          /* creates the target training set */
          printf ("Using file '%s' as training files table... ",
                  trn_tab_file);
          fflush (stdout);

          trn_set = nnet_tset_create_from_table (trn_table, input_dim, 0,
                                                 FALSE, FALSE, TRUE, TRUE,
                                                 FALSE);
          if (trn_set == NULL)
            {
              puts ("FAILED");
              return error_failure (__PROG_NAME_,
                                    "error creating training set from table %s\n",
                                    trn_table->name);
            }
          else
            {
              puts ("OK");
            }
        }

      printf ("Training set has %ld elements\n", trn_set->nu_elements);

      /* randomizes training set elements */
      if (trn_flag == TRUE && rnd_flag == TRUE)
        {
          printf ("Randomizing training set... ");
          fflush (stdout);

          if (nnet_tset_randomize (trn_set) != EXIT_SUCCESS)
            {
              puts ("FAILED");
              return error_failure (__PROG_NAME_,
                                    "error randomizing training set\n");
            }
          else
            {
              puts ("OK");
            }
        }
    }



/******************************************************************************
 *                                                                            *
 *                                   TRAINING                                 *
 *                                                                            *
 ******************************************************************************/

  if (trn_flag == TRUE)
    {
      /* Performs network training stage */
      t_start = time (NULL);

      printf ("Starting SOM training (maximum epochs = %ld)\n", max_epochs);
      printf ("Neural network training started at %s", ctime (&t_start));
      if (mse_flag == TRUE)
        printf ("Using file '%s' as MSE error log\n", mse_file);

      fflush (stdout);

      for (epoch = first_epoch; epoch <= max_epochs; epoch++)
        {
          /* time reset */
          if (epoch == first_epoch)
            rst_flag = TRUE;
          else
            rst_flag = FALSE;

          /* executes one pass through the entire training set */
          if (nnet_som_train_set (som_nnet, trn_set,
                                  first_epoch, max_epochs,
                                  rst_flag, TRUE,
                                  __PROGRESS_BAR_WIDTH_,
                                  __PROGRESS_BAR_CHAR_,
                                  mse_flag, &trn_mse_error) != EXIT_SUCCESS)

            return error_failure (__PROG_NAME_,
                                  "error executing training epoch %ld\n",
                                  epoch);

          /* writes MSE error */
          if (mse_flag == TRUE)
            {
              mse_fd = fopen (mse_file, "a");
              if (mse_fd == NULL)
                return error_failure (__PROG_NAME_,
                                      "error opening MSE error file '%s': %s\n",
                                      mse_file, strerror (errno));

              fprintf (mse_fd, "%f\n",
                       trn_mse_error / (RValue) trn_set->nu_elements);

              if (fclose (mse_fd) != 0)
                return error_failure (__PROG_NAME_,
                                      "error writing MSE file\n");
            }

          /* checks if savepoint is reached */
          if (tr_net_file != NULL && save_epochs > 0)
            if (epoch % save_epochs == 0 && epoch > first_epoch)
              {
                /*
                 * savepoint network configuration name
                 * creates savepoint file
                 * writes the current neural network to savepoint file
                 */
                sprintf (sv_fext, ".t%ld%s", epoch, net_fext);

                sv_net_file =
                  get_file_name (tr_net_dir, tr_net_base, sv_fext);

                sv_net_fd = fopen (sv_net_file, "w");
                if (sv_net_fd == NULL)
                  return error_failure (__PROG_NAME_,
                                        "error creating savepoint network file\n");

                nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE,
                                          TRUE, sv_net_fd);

                fclose (sv_net_fd);
              }
        }

      t_stop = time (NULL);
      printf ("Neural network training finished at %s", ctime (&t_stop));

      /* Calculates training stage elapsed time */
      training_duration = difftime (t_stop, t_start);

      printf ("Training stage lasted %ld seconds\n",
              (long) training_duration);

      printf ("Final training MSE error is %f\n",
              trn_mse_error / (RValue) trn_set->nu_elements);
    }



/******************************************************************************
 *                                                                            *
 *                        STATE FREQUENCY LIST AND MAP                       *
 *                                                                            *
 ******************************************************************************/

  /* Loops through the training elements */
  if (sl_flag == TRUE || tm_flag == TRUE)
    {
      /* single/multi file */
      if (file_mode == SINGLE_FILE)
        {
          /* generate the status list */
          winners = nnet_som_propagate_set (som_nnet, trn_set,
                                            FALSE, &trn_mse_error);
          if (winners == NULL)
            return error_failure (__PROG_NAME_,
                                  "error propagating training set\n");

          /* constructs the transition map */
          if (tm_flag == TRUE)
            {
              /* constructs transition map from winner indexes vector */
              frmap = frmap_create_from_vector (output_dim, winners, TRUE);
              if (frmap == NULL)
                return error_failure (__PROG_NAME_,
                                      "error creating state frequency map\n");

              /* Writes state frequency map to file */
              printf
                ("Using file '%s' to generate state frequency map\n",
                 tm_file);
              tm_fd = fopen (tm_file, "w");
              vector_list_info (frmap, 0.0, tm_fd);
              fclose (tm_fd);

              /* Destroys state frequency map */
              if (frmap_destroy (&frmap) != EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error destroying transition map\n");
            }

          /* writes states list to file */
          if (sl_flag == TRUE)
            {
              printf ("Using file '%s' to generate states list\n", sl_file);
              sl_fd = fopen (sl_file, "w");
              vector_raw_info (winners, sl_fd);
              fclose (sl_fd);
            }

          /* Destroys states list */
          if (vector_destroy (&winners) != EXIT_SUCCESS)
            return error_failure (__PROG_NAME_,
                                  "error destroying states list\n");
        }
      else
        {
          /* multi-file mode */
          cur_row = NULL;

          while (last_row == FALSE)
            {
              /* fetches the file name from file table */
              table_fetch (trn_table, &last_row, &cur_row);
              if (cur_row == NULL)
                return error_failure (__PROG_NAME_,
                                      "empty row in file table\n");

              if (table_get_cell (cur_row, trn_table->first_column,
                                  &cellval, &nullval) != EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error getting file name from table row\n");

              if (nullval == TRUE)
                return error_failure (__PROG_NAME_,
                                      "null file name in table\n");

              set_file =
                (char *) malloc (strlen (ctl_dir) +
                                 strlen (cellval.stringvalue) + 1);
              if (set_file == NULL)
                return error_failure (__PROG_NAME_,
                                      "can't get memory for status list file name\n");

              sprintf (set_file, "%s%s", ctl_dir, cellval.stringvalue);

              /* fetches the desired class from file table */
              if (table_get_cell (cur_row,
                                  trn_table->first_column->next_column,
                                  &cellval, &nullval) != EXIT_SUCCESS)
                return error_failure (__PROG_NAME_,
                                      "error getting desired class from table row\n");

              if (nullval == TRUE)
                return error_failure (__PROG_NAME_,
                                      "null desired class in table\n");

              desired_class = (RValue) cellval.uslgintvalue;

              /* recreates the set by the file */
              aux_set = nnet_tset_create_from_file (NULL, input_dim, 0,
                                                    FALSE, FALSE, TRUE, TRUE,
                                                    FALSE, set_file);
              if (aux_set == NULL)
                return error_failure (__PROG_NAME_,
                                      "error creating training set from file '%s'\n",
                                      set_file);

              /* generate the status list */
              winners = nnet_som_propagate_set (som_nnet, aux_set,
                                                FALSE, &trn_mse_error);

              if (winners == NULL)
                return error_failure (__PROG_NAME_,
                                      "error propagating training set\n");

              /* constructs the transition map */
              if (tm_flag == TRUE)
                {
                  /* determines the transition map file name */
                  tm_file = get_file_name (tm_dir, set_file, tm_fext);
                  if (tm_file == NULL)
                    return error_failure (__PROG_NAME_,
                                          "error defining map file name\n");

                  /* constructs transition map from winners vector */
                  frmap =
                    frmap_create_from_vector (output_dim, winners, TRUE);
                  if (frmap == NULL)
                    return error_failure (__PROG_NAME_,
                                          "error creating state frequency map\n");

                  /* writes state frequency map and desired class to file */
                  tm_fd = fopen (tm_file, "w");
                  vector_list_info (frmap, 0.0, tm_fd);
                  fprintf (tm_fd, "%f %f %f\n", 0.0, 0.0, desired_class);
                  fclose (tm_fd);

                  /* Destroys state frequency map */
                  if (frmap_destroy (&frmap) != EXIT_SUCCESS)
                    return error_failure (__PROG_NAME_,
                                          "error destroying transition map\n");

                  free (tm_file);
                }

              if (sl_flag == TRUE)
                {
                  /* determines the status list file name */
                  sl_file = get_file_name (sl_dir, set_file, sl_fext);
                  if (sl_file == NULL)
                    return error_failure (__PROG_NAME_,
                                          "error defining states file name\n");

                  /* writes states list to file */
                  sl_fd = fopen (sl_file, "w");
                  if (sl_fd == NULL)
                    return error_failure (__PROG_NAME_,
                                          "error creating states file\n");

                  vector_raw_info (winners, sl_fd);
                  fclose (sl_fd);
                  free (sl_file);
                }
            }
          free (set_file);
        }
    }

/******************************************************************************
 *                                                                            *
 *                                FINALIZATION                                *
 *                                                                            *
 ******************************************************************************/

  /* Writes the trained network file */
  if (tr_net_file != NULL && trn_flag == TRUE)
    {
      tr_net_fd = fopen (tr_net_file, "w");
      if (tr_net_fd == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating trained network file: %s\n",
                              strerror (errno));

      printf ("Using file '%s' to write trained neural network ... ",
              tr_net_file);
      fflush (stdout);

      nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE, TRUE,
                                tr_net_fd);

      puts ("OK");
      fclose (tr_net_fd);
    }

  /* Destroys the training set */
  if (trn_flag == TRUE)
    {
      printf ("Destroying training set... ");
      fflush (stdout);

      if (nnet_tset_destroy (&trn_set, TRUE) != EXIT_SUCCESS)
        {
          puts ("FAILED");
          return error_failure (__PROG_NAME_,
                                "error destroying training set\n");
        }
      else
        {
          puts ("OK");
        }
    }

  /* Destroys the file table */
  if (trn_table != NULL)
    {
      printf ("Destroying file table... ");
      fflush (stdout);

      if (table_drop (&trn_table) != EXIT_SUCCESS)
        {
          puts ("FAILED");
          return error_failure (__PROG_NAME_, "error dropping file table\n");
        }
      else
        {
          puts ("OK");
        }
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
  if (tst_flag == TRUE && tst_tab_file != NULL)
    {
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


      printf ("Using file '%s' as test files table... ", tst_tab_file);
      fflush (stdout);

      tst_set = nnet_tset_create_from_table (tst_table, input_dim, 0,
                                             FALSE, FALSE, TRUE, TRUE, FALSE);
      if (tst_set == NULL)
        {
          puts ("FAILED");
          return error_failure (__PROG_NAME_,
                                "error creating test set from table %s\n",
                                tst_table->name);
        }
      else
        {
          puts ("OK");
        }

      winners = nnet_som_propagate_set (som_nnet, tst_set,
                                        TRUE, &tst_mse_error);

      printf ("Test set MSE error is %f\n",
              tst_mse_error / (RValue) tst_set->nu_elements);
    }

  /* Destroys winners vector */
  if (winners != NULL)
    if (vector_destroy (&winners) != EXIT_SUCCESS)
      return error_failure (__PROG_NAME_, "error destroying states list\n");

  /* Destroys the SOM */
  printf ("Destroying SOM... ");
  fflush (stdout);

  if (nnet_nnetwork_destroy (&nnet, TRUE, TRUE, TRUE, TRUE) != EXIT_SUCCESS)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_, "error destroying SOM network\n");
    }
  else
    {
      puts ("OK");
    }

  return EXIT_SUCCESS;
}
