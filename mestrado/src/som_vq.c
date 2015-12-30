#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "errorh/errorh.h"
#include "strutils/strutils.h"
#include "nnet/som/nnet_som.h"
#include "nnet/nnet_types.h"
#include "nnet/nnet_nnet.h"
#include "nnet/nnet_actv.h"
#include "nnet/nnet_weights.h"
#include "nnet/nnet_sets.h"
#include "nnet/nnet_train.h"
#include "nnet/nnet_files.h"
#include "nnet/nnet_files_nnet.h"
#include "vector/vector.h"
#include "trmap/trmap.h"
#include "inparse/inparse.h"

#ifdef __PROG_NAME_
#undef __PROG_NAME_
#endif
#define __PROG_NAME_ "som_vq"

#define __FILE_NAME_SIZE_ 255
#define __IGNORE_TOKEN_ '#'
#define __PROGRESS_BAR_WIDTH_ 50
#define __PROGRESS_BAR_CHAR_ '#'

typedef enum
{ SINGLE_FILE, MULTI_FILE }
file_mode_type;


void
usage (void)
{
  puts ("");
  puts ("Usage: som_vq [-fp | --file-preffix <file>]");
  puts ("              [-in | --input-network <file>]");
  puts ("              [-on | --output-network <file>]");
  puts ("              [-i  | --input <file>]");
  puts ("              [-il | --input-list <file>]");
  puts ("              [-s  | --states-file <file>]");
  puts ("              [-sd | --states-dir <directory>]");
  puts ("              [-m  | --map-file <file>]");
  puts ("              [-md | --map-dir <directory>]");
  puts ("              [-t  | --train]");
  puts ("              [-e  | --max-epochs <number>]");
  puts ("              [-ie | --initial-epoch <number>]");
  puts ("              [-se | --save-epochs <number>]");
  puts ("              [-h  | --help]\n");
  puts ("Options are:\n");
  puts ("  -fp | --file-preffix    common name for single network, input,");
  puts ("                          output, and transition map file names");
  puts ("  -in | --input-network   input neural network file name");
  puts ("  -on | --output-network  output trained neural network file name");
  puts ("  -i  | --input           input training set file name");
  puts ("  -il | --input-list      file containing list of input file names");
  puts ("  -s  | --states-file     list of states file name");
  puts
    ("  -sd | --states-dir      output directory for multiple state files");
  puts ("  -m  | --map-file        transition map output file name");
  puts ("  -md | --map-dir         output directory for multiple map files");
  puts ("  -t  | --train           execute training of input neural network");
  puts ("  -e  | --max-epochs      maximum training epochs");
  puts ("  -ie | --initial-epoch   initial epoch for resume training");
  puts ("  -se | --save-epochs     save network status each n epochs");
  puts ("  -h  | --help            outputs this help message and exit\n");

  return;
}



int
main (int argc, char **argv)
{
  char *preffix = NULL;         /* file names preffix */
  size_t preffix_length;        /* preffix string length */

  char *net_fext = ".net";      /* network file extension */
  char *tr_net_fext = NULL;     /* trained network file extension */
  char *sv_fext = NULL;         /* savepoint network file extension */
  char *set_fext = ".set";      /* test set file extension */
  char *tm_fext = ".map";       /* transition map file extension */
  char *sl_fext = ".sl";        /* status list file extension */

  char *net_file = NULL;        /* neural network file name */
  char *tr_net_file = NULL;     /* trained neural network file name */
  char *sv_net_file = NULL;     /* savepoint neural network file name */
  char *set_file = NULL;        /* training set file name */
  char *tm_file = NULL;         /* transition map file name */
  char *sl_file = NULL;         /* status list file name */
  char *inlist_file = NULL;     /* file containing multiple input files */

  char *tr_net_dir = NULL;      /* trained network output directory */
  char *tm_dir = NULL;          /* transition map output directory */
  char *sl_dir = NULL;          /* status list output directory */

  char *tr_net_base = NULL;     /* trained network base name */

  FILE *net_fd = NULL;          /* input network file descriptor */
  FILE *tr_net_fd = NULL;       /* trained network file descriptor */
  FILE *sv_net_fd = NULL;       /* savepoint network file descriptor */
  FILE *tm_fd = NULL;           /* transition map file descriptor */
  FILE *sl_fd = NULL;           /* status list file descriptor */
  FILE *inlist_fd = NULL;       /* input list file descriptor */

  BoolValue tm_flag = FALSE;    /* flag: output transition map to file */
  BoolValue sl_flag = FALSE;    /* flag: output status list to file */

  NNetwork nnet = NULL;         /* neural network created */
  SomNNetwork som_nnet = NULL;  /* SOM extension */
  SomAttributes som_attr = NULL;        /* SOM attributes */
  LRateFunction lrate_function = NULL;  /* learning rate function */

  UnitIndex input_dim;          /* input layer dimension */
  UnitIndex output_dim;         /* output layer dimension */

  BoolValue trn_flag = FALSE;   /* flag: execute training */
  BoolValue rst_flag = TRUE;    /* flag: reset initial epoch */
  DTime epoch = 0;              /* current epoch */
  DTime max_epochs;             /* maximum training epochs */
  DTime first_epoch = 0;        /* start with this epoch */
  DTime save_epochs = 0;        /* save network status each n epochs */
  DTime max_epochs_order;       /* number of digits of max_epochs */
  TSet t_set = NULL;            /* training set */
  TSet aux_set = NULL;          /* auxiliary training set */
  time_t t_start, t_stop;       /* training start and stop times */
  double training_duration;     /* training duration */

  Vector winners = NULL;        /* list of states */
  TransitionMap trmap = NULL;   /* state transition map */

  file_mode_type file_mode;     /* single/multi-file input */


  /* command line parameters */
  InputParameterSet pset = {
    {"-h", "--help", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-fp", "--file-preffix", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-in", "--input-network", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-on", "--output-network", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-i", "--input", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-il", "--input-list", STRING, FALSE, FALSE,
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
     {.uslgintvalue = 0}},
  };

  InputParameterList plist = { 14, pset };



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

  /* Group of file names given by preffix */
  if (plist.parameter[1].passed == TRUE)
    {
      preffix = plist.parameter[1].value.stringvalue;
      preffix_length = strlen (preffix);

      net_file = (char *) malloc (preffix_length + strlen (net_fext) + 1);
      set_file = (char *) malloc (preffix_length + strlen (set_fext) + 1);
      tm_file = (char *) malloc (preffix_length + strlen (tm_fext) + 1);
      sl_file = (char *) malloc (preffix_length + strlen (sl_fext) + 1);

      if (net_file == NULL || set_file == NULL ||
          tm_file == NULL || sl_file == NULL)
        return error_failure (__PROG_NAME_, strerror (errno));

      /* standard file names according to preffix */
      sprintf (net_file, "%s%s", preffix, net_fext);
      sprintf (set_file, "%s%s", preffix, set_fext);
      sprintf (tm_file, "%s%s", preffix, tm_fext);
      sprintf (sl_file, "%s%s", preffix, sl_fext);

      tm_flag = TRUE;
      sl_flag = TRUE;
    }

  /* network configuration file name */
  if (plist.parameter[2].passed == TRUE)
    net_file = plist.parameter[2].value.stringvalue;

  /* trained neural network output file name */
  if (plist.parameter[3].passed == TRUE)
    tr_net_file = plist.parameter[3].value.stringvalue;

  /* training set (input) file name */
  if (plist.parameter[4].passed == TRUE)
    set_file = plist.parameter[4].value.stringvalue;

  /* file containing a list of input files */
  if (plist.parameter[5].passed == TRUE)
    inlist_file = plist.parameter[5].value.stringvalue;

  /* list of states (output) file name */
  if (plist.parameter[6].passed == TRUE)
    {
      sl_file = plist.parameter[6].value.stringvalue;
      sl_flag = TRUE;
    }

  /* multiple output list of states directory */
  if (plist.parameter[7].passed == TRUE)
    {
      sl_dir = plist.parameter[7].value.stringvalue;
      sl_flag = TRUE;
    }

  /* state transition map file name */
  if (plist.parameter[8].passed == TRUE)
    {
      tm_file = plist.parameter[8].value.stringvalue;
      tm_flag = TRUE;
    }

  /* output directory for multiple files */
  if (plist.parameter[9].passed == TRUE)
    {
      tm_dir = plist.parameter[9].value.stringvalue;
      tm_flag = TRUE;
    }

  /* execute training? */
  if (plist.parameter[10].passed == TRUE)
    trn_flag = TRUE;

  /* maximum training epochs */
  max_epochs = (DTime) plist.parameter[11].value.uslgintvalue;

  /* savepoint epochs */
  save_epochs = (DTime) plist.parameter[12].value.uslgintvalue;

  /* first epoch */
  first_epoch = (DTime) plist.parameter[13].value.uslgintvalue;

  /* Running single or multi-file? */
  if (inlist_file != NULL)
    file_mode = MULTI_FILE;
  else
    file_mode = SINGLE_FILE;

  /* Parameter consistency check */
  if (file_mode == SINGLE_FILE)
    {
      if (set_file == NULL)
        return error_failure (__PROG_NAME_, "no input file passed\n");

      if (sl_file == NULL)
        return error_failure (__PROG_NAME_, "no output file passed\n");
    }

  if (net_file == NULL)
    return error_failure (__PROG_NAME_,
                          "no network configuration file passed\n");

  if (save_epochs < 0)
    return error_failure (__PROG_NAME_, "negative savepoint epochs\n");

  if (save_epochs > max_epochs)
    return error_failure (__PROG_NAME_, "savepoint epochs out of range\n");

  if (first_epoch < 0)
    return error_failure (__PROG_NAME_, "negative initial epoch\n");

  /* Initialization */
  max_epochs_order = (DTime) log10 (max_epochs);

  if (error_if_null
      (sv_fext = (char *) malloc (max_epochs + strlen (net_fext) + 3),
       __PROG_NAME_, strerror (errno)))
    return EXIT_FAILURE;

  if (tr_net_file != NULL)
    split_file_name (tr_net_file, &tr_net_dir, &tr_net_base, &tr_net_fext);


/******************************************************************************
 *                                                                            *
 *                           NEURAL NETWORK CREATION                          *
 *                                                                            *
 ******************************************************************************/

  /* Creates the network by the configuration file */
  if (net_file != NULL)
    {
      if (error_if_null (net_fd = fopen (net_file, "r"),
                         __PROG_NAME_, strerror (errno)))
        return EXIT_FAILURE;
    }

  printf ("Using file '%s' to create neural network... ", net_file);
  fflush (stdout);

  if (error_if_null
      (nnet = nnet_file_create_nnetwork (net_fd), __PROG_NAME_,
       "error creating neural network using file '%s'\n", net_file))
    {
      puts ("FAILED");
      return EXIT_FAILURE;
    }
  else
    {
      fclose (net_fd);
      puts ("OK");
      fflush (stdout);
    }

  /* Initialization */
  som_nnet = (SomNNetwork) nnet->extension;
  som_attr = (SomAttributes) som_nnet->attr;
  lrate_function = som_attr->lrate_function;
  input_dim = nnet->first_layer->nu_units;
  output_dim = nnet->last_layer->nu_units;


/******************************************************************************
 *                                                                            *
 *                             TRAINING SET CREATION                          *
 *                                                                            *
 ******************************************************************************/

  if (file_mode == SINGLE_FILE)
    {
      /* creates a new training set to the given file */
      printf ("Using file '%s' to create training set... ", set_file);
      fflush (stdout);

      if (error_if_null (t_set = nnet_tset_create_from_file
                         ("SOM Training Set", input_dim, 0, FALSE, FALSE,
                          TRUE, TRUE, FALSE, set_file), __PROG_NAME_,
                         "error creating training set from file '%s'\n",
                         set_file))
        {
          puts ("FAILED");
          return EXIT_FAILURE;
        }
      else
        {
          puts ("OK");
        }
    }
  else
    {
      /* creates the target training set */
      printf ("Using file '%s' as input file list... ", inlist_file);
      fflush (stdout);

      if (error_if_null (t_set = nnet_tset_create_from_list
                         ("SOM Training Set", input_dim, 0, FALSE, FALSE,
                          TRUE, TRUE, FALSE, inlist_file), __PROG_NAME_,
                         "error creating training set from file '%s'\n",
                         inlist_file))
        {
          puts ("FAILED");
          return EXIT_FAILURE;
        }
      else
        {
          puts ("OK");
        }
    }

  /* randomizes training set elements */
  printf ("Randomizing training set... ");
  fflush (stdout);

  if (error_if_failure (nnet_tset_randomize (t_set), __PROG_NAME_,
                        "error randomizing training set\n"))
    {
      puts ("FAILED");
      return EXIT_FAILURE;
    }
  else
    {
      puts ("OK");
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
      fflush (stdout);

      for (epoch = first_epoch; epoch < max_epochs; epoch++)
        {
          /* time reset */
          if (epoch == first_epoch)
            rst_flag = TRUE;
          else
            rst_flag = FALSE;

          /* executes one pass through the entire training set */
          if (error_if_failure
              (nnet_som_train_set
               (som_nnet, t_set, first_epoch, max_epochs, rst_flag, TRUE,
                __PROGRESS_BAR_WIDTH_, __PROGRESS_BAR_CHAR_), __PROG_NAME_,
               "error executing training epoch %ld\n", epoch))
            return EXIT_FAILURE;

          /* checks if savepoint is reached */
          if (save_epochs > first_epoch && tr_net_file != NULL)
            {
              if (epoch % save_epochs == 0 && epoch > 0)
                {
                  /* savepoint network configuration name */
                  sprintf (sv_fext, ".t%ld%s", epoch, net_fext);
                  sv_net_file =
                    get_file_name (tr_net_dir, tr_net_base, sv_fext);

                  /* creates savepoint file */
                  if (error_if_null
                      (sv_net_fd = fopen (sv_net_file, "w"), __PROG_NAME_,
                       "error creating savepoint network file\n"))
                    return EXIT_FAILURE;

                  /* writes the current neural network to savepoint file */
                  nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE,
                                            TRUE, sv_net_fd);

                  fclose (sv_net_fd);
                }
            }
        }

      t_stop = time (NULL);
      printf ("Neural network training finished at %s", ctime (&t_stop));

      /* Calculates training stage elapsed time */
      training_duration = difftime (t_stop, t_start);
      printf ("Training stage lasted %ld seconds\n",
              (long) training_duration);
    }



/******************************************************************************
 *                                                                            *
 *                        STATE TRANSITION LIST AND MAP                       *
 *                                                                            *
 ******************************************************************************/

  /* Loops through the training elements */
  if (sl_flag == TRUE || tm_flag == TRUE)
    {
      /* single/multi file */
      if (file_mode == SINGLE_FILE)
        {
          /* generate the status list */
          if (error_if_null
              (winners = nnet_som_propagate_set (som_nnet, t_set),
               __PROG_NAME_, "error propagating training set\n"))
            return EXIT_FAILURE;

          /* constructs the transition map */
          if (tm_flag == TRUE)
            {
              /* constructs transition map from winner indexes vector */
              if (error_if_null
                  (trmap =
                   trmap_create_from_vector (output_dim, winners),
                   __PROG_NAME_, "error creating state transition map\n"))
                return EXIT_FAILURE;

              /* Writes state transition map to file */
              printf
                ("Using file '%s' to generate state transition map\n",
                 tm_file);
              tm_fd = fopen (tm_file, "w");
              matrix_raw_info (trmap, tm_fd);
              fclose (tm_fd);

              /* Destroys state transition map */
              if (error_if_failure
                  (trmap_destroy (&trmap), __PROG_NAME_,
                   "error destroying transition map\n"))
                return EXIT_FAILURE;
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
          if (error_if_failure
              (vector_destroy (&winners), __PROG_NAME_,
               "error destroying states list\n"))
            return EXIT_FAILURE;
        }
      else
        {
          /* multi-file mode */
          if (error_if_null (inlist_fd = fopen (inlist_file, "r"),
                             __PROG_NAME_, strerror (errno)))
            return EXIT_FAILURE;

          /* loops through all file names */
          while (!feof (inlist_fd))
            {
              /* reads the current set file name */
              if (error_if_failure (read_valid_file_line
                                    (inlist_fd, FILE_NAME_SIZE,
                                     IGNORE_TOKEN, 1, buf),
                                    __PROG_NAME_,
                                    "error reading input file name from '%s'\n",
                                    inlist_file))
                return EXIT_FAILURE;

              set_file = buf;

              if (!feof (inlist_fd))
                {
                  /* recreates the set by the file */
                  if (error_if_null
                      (aux_set = nnet_tset_create_from_file
                       (NULL, input_dim, 0, FALSE, FALSE, TRUE, TRUE,
                        FALSE, set_file), __PROG_NAME_,
                       "error creating training set from file '%s'\n",
                       set_file))
                    return EXIT_FAILURE;

                  /* generate the status list */
                  if (error_if_null
                      (winners =
                       nnet_som_propagate_set (som_nnet, aux_set),
                       __PROG_NAME_, "error propagating training set\n"))
                    return EXIT_FAILURE;

                  /* constructs the transition map */
                  if (tm_flag == TRUE)
                    {
                      /* determines the transition map file name */
                      if (error_if_null
                          (tm_file = get_file_name
                           (tm_dir, set_file, tm_fext),
                           __PROG_NAME_, "error defining map file name\n"))
                        return EXIT_FAILURE;;

                      /* constructs transition map from winners vector */
                      if (error_if_null
                          (trmap = trmap_create_from_vector
                           (output_dim, winners), __PROG_NAME_,
                           "error creating state transition map\n"))
                        return EXIT_FAILURE;

                      /* Writes state transition map to file */
                      tm_fd = fopen (tm_file, "w");
                      matrix_raw_info (trmap, tm_fd);
                      fclose (tm_fd);

                      /* Destroys state transition map */
                      if (error_if_failure (trmap_destroy (&trmap),
                                            __PROG_NAME_,
                                            "error destroying transition map\n"))
                        return EXIT_FAILURE;
                      free (tm_file);
                    }

                  if (sl_flag == TRUE);
                  {
                    /* determines the transition map file name */
                    if (error_if_null
                        (sl_file = get_file_name
                         (sl_dir, set_file, sl_fext),
                         __PROG_NAME_, "error defining states file name\n"))
                      return EXIT_FAILURE;;

                    /* writes states list to file */
                    if (error_if_null
                        (sl_fd = fopen (sl_file, "w"),
                         __PROG_NAME_, "error creating states file\n"))
                      return EXIT_FAILURE;;
                    vector_raw_info (winners, sl_fd);
                    fclose (sl_fd);

                    free (sl_file);
                  }
                }
            }
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
      if (error_if_null
          (tr_net_fd =
           fopen (tr_net_file, "w"), __PROG_NAME_,
           "error creating trained network file\n"))
        return EXIT_FAILURE;

      printf
        ("Using file '%s' to write trained neural network configuration... ",
         tr_net_file);
      fflush (stdout);

      nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE, TRUE,
                                tr_net_fd);

      puts ("OK");
      fclose (tr_net_fd);
    }

  /* Destroys the SOM */
  printf ("Destroying SOM... ");
  fflush (stdout);
  if (error_if_failure
      (nnet_nnetwork_destroy (&nnet, TRUE, TRUE, TRUE, TRUE),
       __PROG_NAME_, "error destroying SOM network\n"))
    {
      puts ("FAILED");
      return EXIT_FAILURE;
    }
  else
    {
      puts ("OK");
    }

  /* Destroys the training set */
  printf ("Destroying training set... ");
  fflush (stdout);
  if (error_if_failure (nnet_tset_destroy (&t_set, TRUE),
                        __PROG_NAME_, "error destroying training set\n"))
    {
      puts ("FAILED");
      return EXIT_FAILURE;
    }
  else
    {
      puts ("OK");
    }

  return EXIT_SUCCESS;
}
