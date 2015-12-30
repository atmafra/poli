#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "errorh.h"
#include "strutils.h"
#include "nnet_lvq.h"
#include "nnet_types.h"
#include "nnet_nnet.h"
#include "nnet_actv.h"
#include "nnet_weights.h"
#include "nnet_sets.h"
#include "nnet_train.h"
#include "nnet_files.h"
#include "nnet_files_nnet.h"
#include "vector.h"
#include "trmap.h"
#include "inparse.h"
#include "table.h"

#ifdef __PROG_NAME_
#undef __PROG_NAME_
#endif
#define __PROG_NAME_ "lvq_rec"

#define __FILE_NAME_SIZE_ 255
#define __PROGRESS_BAR_WIDTH_ 50
#define __PROGRESS_BAR_CHAR_ '#'


void
usage (void)
{
  puts ("");
  puts ("Usage: lvq_rec [-in | --input-network <file>]");
  puts ("               [-on | --output-network <file>]");
  puts ("               [-ct | --class-table <control file>]");
  puts ("               [-rt | --train-table <control file>]");
  puts ("               [-st | --test-table <control file>]");
  puts ("               [-t  | --train]");
  puts ("               [-e  | --max-epochs <number>]");
  puts ("               [-ie | --initial-epoch <number>]");
  puts ("               [-se | --save-epochs <number>]");
  puts ("               [-ef | --error-file <file>]");
  puts ("               [-rm | --train-matrix <file>]");
  puts ("               [-sm | --test-matrix <file>]");
  puts ("               [-h  | --help]\n");
  puts ("Options are:\n");
  puts ("  -in | --input-network   input neural network file name");
  puts ("  -on | --output-network  output trained neural network file name");
  puts ("  -ct | --class-table     output class table control file");
  puts ("  -rt | --train-table     table of training input files");
  puts ("  -st | --test-table      table of testing input files");
  puts ("  -t  | --train           execute training of input neural network");
  puts ("  -e  | --max-epochs      maximum training epochs");
  puts ("  -ie | --initial-epoch   initial epoch for resume training");
  puts ("  -se | --save-epochs     save network status each n epochs");
  puts ("  -ef | --error-file      current error rate file");
  puts ("  -rm | --train-matrix    training set classification results file");
  puts ("  -sm | --test-matrix     test set classification results file");
  puts ("  -h  | --help            outputs this help message and exit\n");
}



int
main (int argc, char **argv)
{
  char *net_file = NULL;        /* neural network file name */
  char *tr_net_file = NULL;     /* trained neural network file name */
  char *sv_net_file = NULL;     /* savepoint neural network file name */
  char *cls_tab_file = NULL;    /* class table control file */
  char *trn_tab_file = NULL;    /* training files table control file */
  char *tst_tab_file = NULL;    /* test files table control file */
  char *error_file = NULL;      /* current error rate file */
  char *trn_rslt_file = NULL;   /* training results matrix file */
  char *tst_rslt_file = NULL;   /* test results matrix file */
  char *tr_net_dir = NULL;      /* trained network output directory */
  char *tr_net_base = NULL;     /* trained network base name */

  FILE *net_fd = NULL;          /* input network file descriptor */
  FILE *tr_net_fd = NULL;       /* trained network file descriptor */
  FILE *sv_net_fd = NULL;       /* savepoint network file descriptor */
  FILE *err_fd = NULL;          /* error rate file descriptor */
  FILE *trn_rslt_fd = NULL;     /* training results file descriptor */
  FILE *tst_rslt_fd = NULL;     /* test results file descriptor */

  char *net_fext = ".net";      /* network file extension */
  char *tr_net_fext = NULL;     /* trained network file extension */
  char *sv_fext = NULL;         /* savepoint network file extension */

  Table class_table = NULL;     /* output units class table */
  Table trn_table = NULL;       /* train files table */
  Table tst_table = NULL;       /* test files table */
  char *ctl_dir = NULL;         /* control file directory */
  char *ctl_base = NULL;        /* control file base name */
  char *ctl_ext = NULL;         /* control file extension */

  NNetwork nnet = NULL;         /* neural network created */
  LvqNNetwork lvq_nnet = NULL;  /* LVQ extension */
  LvqAttributes lvq_attr = NULL;        /* LVQ attributes */
  /*LRateFunction lrate_function = NULL; *//* learning rate function */
  UnitIndex input_dim;          /* input layer dimension */
  /*UnitIndex output_dim; *//* output layer dimension */
  UsLgIntValue nu_classes;      /* number of classes */

  BoolValue trn_flag = FALSE;   /* flag: execute training */
  BoolValue tst_flag = FALSE;   /* flag: execute tests */
  BoolValue rnd_flag = FALSE;   /* flag: randomize training set */
  BoolValue rst_flag = TRUE;    /* flag: reset initial epoch */
  BoolValue sav_flag = FALSE;   /* flag: use savepoint networks */
  DTime epoch = 0;              /* current epoch */
  DTime max_epochs;             /* maximum training epochs */
  DTime first_epoch = 0;        /* start with this epoch */
  DTime save_epochs = 0;        /* save network status each n epochs */
  /*DTime max_epochs_order; *//* number of digits of max_epochs */
  TSet trn_set = NULL;          /* training set */
  TSet tst_set = NULL;          /* test set */
  time_t t_start, t_stop;       /* training start and stop times */
  double training_duration;     /* training duration */

  RValue trn_error_rate = 0.0;  /* current epoch error rate */
  RValue tst_error_rate = 0.0;  /* current epoch error rate */
  RValue trn_mse_error = 0.0;   /* current epoch MSE error */
  RValue tst_mse_error = 0.0;   /* current epoch MSE error */
  TransitionMap cl_map = NULL;  /* classification map */


  /* command line parameters */
  InputParameterSet pset = {
    {"-h", "--help", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-in", "--input-network", STRING, FALSE, TRUE,
     {.stringvalue = (char *) NULL}},
    {"-on", "--output-network", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-rt", "--train-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-t", "--train", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-e", "--max-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1000}},
    {"-se", "--save-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-ie", "--initial-epoch", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-ct", "--class-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-r", "--randomize-set", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-ef", "--error-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-rm", "--train-matrix", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-st", "--test-table", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-sm", "--test-matrix", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
  };

  InputParameterList plist = { 14, pset };



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

  /* table of training input files */
  if (plist.parameter[3].passed == TRUE)
    trn_tab_file = plist.parameter[3].value.stringvalue;

  /* execute training? */
  if (plist.parameter[4].passed == TRUE)
    trn_flag = TRUE;

  /* maximum training epochs */
  max_epochs = (DTime) plist.parameter[5].value.uslgintvalue;

  /* savepoint epochs */
  if (plist.parameter[6].passed == TRUE)
    {
      save_epochs = (DTime) plist.parameter[6].value.uslgintvalue;
      sav_flag = TRUE;
    }

  /* first epoch */
  if (plist.parameter[7].passed == TRUE)
    first_epoch = (DTime) plist.parameter[7].value.uslgintvalue;

  /* output class table */
  cls_tab_file = plist.parameter[8].value.stringvalue;

  /* randomize training set? */
  if (plist.parameter[9].passed == TRUE && trn_flag == TRUE)
    rnd_flag = TRUE;

  /* current error rate file */
  if (plist.parameter[10].passed == TRUE)
    error_file = plist.parameter[10].value.stringvalue;

  /* training results matrix file */
  if (plist.parameter[11].passed == TRUE)
    trn_rslt_file = plist.parameter[11].value.stringvalue;

  /* table of test input files */
  if (plist.parameter[12].passed == TRUE)
    {
      tst_tab_file = plist.parameter[12].value.stringvalue;
      tst_flag = TRUE;
    }

  /* test results matrix file */
  if (plist.parameter[13].passed == TRUE && tst_flag == TRUE)
    tst_rslt_file = plist.parameter[13].value.stringvalue;

  /* parameter consistency check */
  if (net_file == NULL)
    return error_failure (__PROG_NAME_,
                          "no network configuration file passed\n");

  if (trn_flag == TRUE && trn_tab_file == NULL)
    return error_failure (__PROG_NAME_,
                          "training requested but no file table passed\n");

  if (sav_flag == TRUE && save_epochs < 1)
    return error_failure (__PROG_NAME_,
                          "savepoint epochs must be 1 or greater\n");

  if (save_epochs > max_epochs)
    return error_failure (__PROG_NAME_, "savepoint epochs out of range\n");

  if (cls_tab_file == NULL)
    return error_failure (__PROG_NAME_,
                          "no output class table file passed\n");

  /* Initialization */
  /*max_epochs_order = (DTime) log10 (max_epochs); */

  sv_fext = (char *) malloc (max_epochs + strlen (net_fext) + 3);
  if (sv_fext == NULL)
    return error_failure (__PROG_NAME_, strerror (errno));

  if (tr_net_file != NULL)
    split_file_name (tr_net_file, &tr_net_dir, &tr_net_base, &tr_net_fext);

  /* training files table */
  if (trn_tab_file != NULL && trn_flag == TRUE)
    {
      trn_table = table_create_from_control_file (trn_tab_file);
      if (trn_table == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating table of training files\n");

      if (table_read_data (trn_table) != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error loading data into training files table\n");

      split_file_name (trn_table->ctlfile, &ctl_dir, &ctl_base, &ctl_ext);
    }

  /* test files table */
  if (tst_flag == TRUE)
    {
      tst_table = table_create_from_control_file (tst_tab_file);
      if (tst_table == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating table of test files\n");

      if (table_read_data (tst_table) != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error loading data into test files table\n");
    }


/******************************************************************************
 *                                                                            *
 *                           NEURAL NETWORK CREATION                          *
 *                                                                            *
 ******************************************************************************/

  /* Creates the network by the configuration file */
  net_fd = fopen (net_file, "r");

  if (net_fd == NULL)
    return error_failure (__PROG_NAME_,
                          "error opening network configuration file '%s': %s\n",
                          net_file, strerror (errno));

  printf ("Using file '%s' to create neural network... ", net_file);
  fflush (stdout);

  nnet = nnet_file_create_nnetwork (net_fd);
  fclose (net_fd);

  if (nnet == NULL)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_,
                            "error creating neural network using file '%s'\n",
                            net_file);
    }
  else
    {
      puts ("OK");
    }

  /* Initialization */
  lvq_nnet = (LvqNNetwork) nnet->extension;
  lvq_attr = (LvqAttributes) lvq_nnet->attr;
  /*lrate_function = lvq_attr->lrate_function; */
  input_dim = nnet->first_layer->nu_units;
  /*output_dim = nnet->last_layer->nu_units; */
  nu_classes = lvq_attr->output_classes;

  /*
   * loads output class table by its control file
   * loads class table data
   * attaches it to the LVQ
   */
  printf ("Using file '%s' as output classes table... ", cls_tab_file);
  fflush (stdout);

  class_table = table_create_from_control_file (cls_tab_file);

  if (class_table == NULL)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_,
                            "error creating output class table from file '%s'\n",
                            cls_tab_file);
    }

  if (table_read_data (class_table) != EXIT_SUCCESS)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_, "error loading class table data\n");
    }

  if (nnet_lvq_attach_class_table (lvq_nnet, class_table) != EXIT_SUCCESS)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_,
                            "error attaching class table to LVQ\n");
    }

  puts ("OK");


/******************************************************************************
 *                                                                            *
 *                             TRAINING SET CREATION                          *
 *                                                                            *
 ******************************************************************************/

  /* creates the training set */
  if (trn_flag == TRUE)
    {
      printf ("Using file '%s' as training files table... ", trn_tab_file);
      fflush (stdout);

      trn_set = nnet_tset_create_from_table (trn_table, input_dim, 1,
                                             FALSE, FALSE, TRUE, TRUE, FALSE);

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

  if (tst_flag == TRUE)
    {
      printf ("Using file '%s' as test files table... ", tst_tab_file);
      fflush (stdout);

      tst_set = nnet_tset_create_from_table (tst_table, input_dim, 1,
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

      printf ("Test set has %ld elements\n", tst_set->nu_elements);
    }

/******************************************************************************
 *                                                                            *
 *                             WEIGHT INITIALIZATION                          *
 *                                                                            *
 ******************************************************************************/

  if (nnet_lvq_init_weights (lvq_nnet, trn_set) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "error in weight initialization\n");


/******************************************************************************
 *                                                                            *
 *                                   TRAINING                                 *
 *                                                                            *
 ******************************************************************************/

  /* classification results matrix */
  if (trn_rslt_file != NULL || tst_flag == TRUE)
    {
      cl_map = trmap_create (nu_classes, 0.0);

      if (cl_map == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating classification matrix\n");
    }

  if (trn_flag == TRUE)
    {
      /* Performs network training stage */
      t_start = time (NULL);

      printf ("Starting LVQ training (maximum epochs = %ld)\n", max_epochs);
      printf ("Neural network training started at %s", ctime (&t_start));
      fflush (stdout);

      for (epoch = first_epoch; epoch < max_epochs; epoch++)
        {
          /* time reset */
          if (epoch == first_epoch)
            rst_flag = TRUE;
          else
            rst_flag = FALSE;

          /* executes one pass through the entire training set */
          if (nnet_lvq_train_set (lvq_nnet, trn_set,
                                  first_epoch, max_epochs,
                                  rst_flag, TRUE,
                                  __PROGRESS_BAR_WIDTH_,
                                  __PROGRESS_BAR_CHAR_,
                                  &trn_error_rate, &trn_mse_error,
                                  cl_map) != EXIT_SUCCESS)

            return error_failure (__PROG_NAME_,
                                  "error executing training epoch %ld\n",
                                  epoch);

          /* propagates test set */
          if (nnet_lvq_propagate_set (lvq_nnet, tst_set,
                                      &tst_error_rate, &tst_mse_error, cl_map)
              != EXIT_SUCCESS)
            return error_failure (__PROG_NAME_,
                                  "error propagating test set elements\n");

          /* writes error rate */
          if (error_file != NULL)
            {
              err_fd = fopen (error_file, "a");
              if (err_fd == NULL)
                return error_failure (__PROG_NAME_,
                                      "error opening error rate file '%s': %s\n",
                                      error_file, strerror (errno));

              fprintf (err_fd, "%f %f %f %f\n", trn_error_rate, trn_mse_error,
                       tst_error_rate, tst_mse_error);

              if (fclose (err_fd) != 0)
                return error_failure (__PROG_NAME_,
                                      "error writing error rate file\n");
            }

          /* checks if savepoint is reached */
          if (tr_net_file != NULL && save_epochs > 0)
            if (epoch % save_epochs == 0 && epoch > first_epoch &&
                epoch != max_epochs)
              {
                /*
                 * assembles savepoint network configuration name
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

      printf ("Final training error rate is %f%%\n", 100.0 * trn_error_rate);
    }

  /* writes the trained network file */
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

  /* writes classification matrices */
  if (trn_flag == TRUE && trn_rslt_file != NULL)
    {
      trn_rslt_fd = fopen (trn_rslt_file, "w");

      if (trn_rslt_fd == NULL)
        return error_failure (__PROG_NAME_,
                              "error creating classification matrix file '%s': %s\n",
                              trn_rslt_file, strerror (errno));

      matrix_info (cl_map, trn_rslt_fd);

      if (fclose (trn_rslt_fd) != 0)
        return error_failure (__PROG_NAME_,
                              "error writing classification matrix file '%s': %s\n",
                              trn_rslt_file, strerror (errno));
    }


/******************************************************************************
 *                                                                            *
 *                            TRAINING FINALIZATION                           *
 *                                                                            *
 ******************************************************************************/

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
 *                                    TEST                                    *
 *                                                                            *
 ******************************************************************************/

  if (tst_flag == TRUE)
    {
      /*
       * creates the test set
       * propagates the elements
       * outputs information to result matrix
       * destroys test set
       */
      if (nnet_lvq_propagate_set (lvq_nnet, tst_set,
                                  &tst_error_rate, &tst_mse_error, cl_map)
          != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error propagating test set elements\n");

      printf ("Final test error rate is %f%%\n", 100.0 * tst_error_rate);

      if (tst_rslt_file != NULL)
        {
          tst_rslt_fd = fopen (tst_rslt_file, "w");

          if (tst_rslt_fd == NULL)
            return error_failure (__PROG_NAME_,
                                  "error creating classification matrix file '%s': %s\n",
                                  tst_rslt_file, strerror (errno));

          matrix_info (cl_map, tst_rslt_fd);

          if (fclose (tst_rslt_fd) != 0)
            return error_failure (__PROG_NAME_,
                                  "error writing classification matrix file '%s': %s\n",
                                  tst_rslt_file, strerror (errno));
        }

      /* Destroys the test set */
      printf ("Destroying test set... ");
      fflush (stdout);

      if (nnet_tset_destroy (&tst_set, TRUE) != EXIT_SUCCESS)
        {
          puts ("FAILED");
          return error_failure (__PROG_NAME_, "error destroying test set\n");
        }
      else
        {
          puts ("OK");
        }
    }


  /* Destroys the LVQ */
  printf ("Destroying LVQ... ");
  fflush (stdout);

  if (nnet_nnetwork_destroy (&nnet, TRUE, TRUE, TRUE, TRUE) != EXIT_SUCCESS)
    {
      puts ("FAILED");
      return error_failure (__PROG_NAME_, "error destroying LVQ network\n");
    }
  else
    {
      puts ("OK");
    }

  /* Destroys the classification matrix */
  if (cl_map != NULL)
    {
      printf ("Destroying classification matrix...");
      fflush (stdout);

      if (trmap_destroy (&cl_map) != EXIT_SUCCESS)
        {
          puts ("FAILED");
          return error_failure (__PROG_NAME_,
                                "error destroying classification matrix\n");
        }
      else
        {
          puts ("OK");
        }
    }

  return EXIT_SUCCESS;
}
