#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "errorh.h"
#include "strutils.h"
#include "s_cepstrum.h"
#include "inparse.h"


#ifdef __PROG_NAME_
#undef __PROG_NAME_
#endif
#define __PROG_NAME_ "mfcc"


typedef enum
{ SINGLE_FILE, MULTI_FILE }
file_mode_type;


void
usage (void)
{
  puts ("");
  puts ("Usage: mfcc [-i  | --input-file <file>]");
  puts ("            [-o  | --output-file <file>]");
  puts ("            [-p  | --parameter-file <file>]");
  puts ("            [-il | --input-files-list <file>]");
  puts ("            [-od | --output-directory <directory>]");
  puts ("            [-h  | --help]\n");
  puts ("Options are:\n");
  puts ("  -i  | --input-file        : input file name");
  puts ("  -o  | --output-file       : output file name");
  puts ("  -p  | --parameter-file    : parameter file name");
  puts ("  -il | --input-files-list  : file containing list of input files");
  puts ("  -od | --output-directory  : output directory for multiple files");
  puts ("  -h  | --help              : outputs this help message and exit");
  puts ("");

  return;
}



int
main (int argc, char **argv)
{
  scep_parameter_type param;    /* feature extraction parameters */

  char *input_file = NULL;      /* current input file name */
  char *input_basename = NULL;  /* input file base name */
  char *input_ext = NULL;       /* input file extension */
  char *output_file = NULL;     /* current output file name */
  char *output_ext = ".mfcc";   /* output file extension */
  /*char *param_file = NULL; *//* parameters file */
  char *inlist_file = NULL;     /* file containing multiple input files */
  char *output_dir = NULL;      /* output directory */

  /*
     FILE *param_fd = NULL;        \* parameter file descriptor *\
   */
  FILE *inlist_fd = NULL;       /* list of inputs file descriptor */
  char buf[FILE_NAME_SIZE + 1]; /* input buffer */

  file_mode_type file_mode;     /* single/multi-file input */

  /* command line parameters */
  InputParameterSet pset = {
    {"-h", "--help", BOOL, FALSE, FALSE,
     {.boolvalue = FALSE}},
    {"-i", "--input-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-o", "--output-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-p", "--parameter-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-il", "--input-files-list", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-od", "--output-directory", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
  };

  InputParameterList plist = { 6, pset };



/******************************************************************************
 *                                                                            *
 *                                 PARAMETERS                                 *
 *                                                                            *
 ******************************************************************************/

  /* Parses the command line */
  if (error_if_failure (inpr_parse (argc, argv, plist), __PROG_NAME_,
                        "error parsing the command line\n"))
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

  /* Input file name */
  if (plist.parameter[1].passed == TRUE)
    input_file = plist.parameter[1].value.stringvalue;

  /* Output file name */
  if (plist.parameter[2].passed == TRUE)
    output_file = plist.parameter[2].value.stringvalue;

  /* Parameter file name */
  /*
     if (plist.parameter[3].passed == TRUE)
     param_file = plist.parameter[3].value.stringvalue;
   */

  /* File containing list of input files */
  if (plist.parameter[4].passed == TRUE)
    inlist_file = plist.parameter[4].value.stringvalue;

  /* Output directory */
  if (plist.parameter[5].passed == TRUE)
    output_dir = plist.parameter[5].value.stringvalue;

  /* Running single or multi-file? */
  if (inlist_file != NULL)
    file_mode = MULTI_FILE;
  else
    file_mode = SINGLE_FILE;

  /* Consistency check */
  if (file_mode == SINGLE_FILE)
    {
      if (input_file == NULL)
        return error_failure (__PROG_NAME_, "no input file passed\n");
      else if (output_file == NULL)
        return error_failure (__PROG_NAME_, "no output file passed\n");
    }

  /* Initializes parameter structure */
  param.in_preemphasis = SWIN_YES;
  param.alpha_preemphasis = 0.95;
  param.frame_width = 512;
  param.superposing_samples = 256;
  param.windowing_function = SWIN_HAMMING;
  param.triangular_window_low = 0.0;
  param.triangular_window_center = 0.0;
  param.triangular_window_high = 0.0;
  param.triangular_window_central_value = 0.0;
  param.kaiser_window_B = 0.0;
  param.purge_zero_power = SWIN_PURGE_ZERO_POWER;
  param.log_basis = 10.0;
  param.delta_mel = 120.0;
  param.total_filters = 32;
  param.first_filter = 4;
  param.last_mfcc_used = 14;
  param.use_derivatives = SMP_YES;
  param.write_index = SMP_DONT_WRITE_INDEX;
  param.write_lists = SMP_WRITE_LISTS;
  param.write_time = SMP_WRITE_NORM_TIME;
  param.write_files = SMP_WRITE_SINGLE_FILE;
  param.write_break_lines = SMP_BREAK_LINES;
  param.write_real_part = SMP_YES;
  param.write_img_part = SMP_NO;



/******************************************************************************
 *                                                                            *
 *                              FEATURE EXTRACTION                            *
 *                                                                            *
 ******************************************************************************/

  /* Executes feature extraction */
  if (file_mode == SINGLE_FILE)
    {
      if (scep_mfcc_file_write (input_file, param, output_file)
          != EXIT_SUCCESS)
        return error_failure (__PROG_NAME_,
                              "error executing feature extraction of file '%s'\n",
                              input_file);
    }
  else
    {
      /* processes all files in list */
      inlist_fd = fopen (inlist_file, "r");

      if (inlist_fd == NULL)
        return error_failure (__PROG_NAME_,
                              "error opening input list file '%s': %s\n",
                              inlist_file, strerror (errno));

      if (output_dir != NULL)
        {
          if (output_dir[strlen (output_dir) - 1] == '/')
            output_dir[strlen (output_dir) - 1] = '\0';

          printf ("Output directory is '%s/'\n", output_dir);
        }

      while (!feof (inlist_fd))
        {
          /* Reads the input file name */
          if (read_valid_file_line (inlist_fd, FILE_NAME_SIZE,
                                    IGNORE_TOKEN, 1, buf) != EXIT_SUCCESS)

            return error_failure (__PROG_NAME_,
                                  "error reading input file name from list '%s'\n",
                                  inlist_file);

          input_file = buf;

          if (!feof (inlist_fd))
            {
              /* gets input base name */
              input_ext = get_file_extension (input_file);
              input_basename = get_file_basename (input_file, input_ext);

              /* determines output file name */
              if (output_dir == NULL)
                {
                  output_file = (char *) malloc (strlen (input_basename)
                                                 + strlen (output_ext) + 1);

                  if (output_file == NULL)
                    return error_failure (__PROG_NAME_, strerror (errno));

                  sprintf (output_file, "%s%s", input_basename, output_ext);
                }
              else
                {
                  output_file = (char *) malloc (strlen (output_dir) +
                                                 strlen (input_basename)
                                                 + strlen (output_ext) + 2);

                  if (output_file == NULL)
                    return error_failure (__PROG_NAME_, strerror (errno));

                  sprintf (output_file, "%s/%s%s",
                           output_dir, input_basename, output_ext);
                }

              printf ("Generating file '%s%s' ... ", input_basename,
                      output_ext);

              /* executes MFCC extraction */
              if (scep_mfcc_file_write (input_file, param, output_file)
                  != EXIT_SUCCESS)
                {
                  puts ("FAILED");

                  return error_failure (__PROG_NAME_,
                                        "error executing feature extraction of file '%s'\n",
                                        input_file);
                }
              else
                {
                  puts ("OK");
                }
              free (input_ext);
              free (input_basename);
              free (output_file);
            }
        }
      fclose (inlist_fd);
    }

  return EXIT_SUCCESS;
}
