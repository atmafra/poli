#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "s_files.h"
#include "s_fmt_wav.h"
#include "s_samples.h"
#include "s_smptypes.h"
#include "../common/types.h"
#include "../strutils/strutils.h"
#include "../incstat/incstat.h"

/*
 * write_index
 *
 * Outputs the contents of all the lists of the given index to dest_file.
 * The flag raw_data means that no header info will be written.
 * If 'dest_file' is passed NULL, output will be directed to standard output.
 */
int
write_index (const index_list_type index,
             char *dest_file,
             const smp_write_index_type write_index,
             const smp_write_lists_type write_lists,
             const smp_write_time_type write_time,
             const smp_write_files_type write_files,
             const smp_breaklines_type break_lines,
             const smp_yes_no real_part, const smp_yes_no img_part)
{
  int exit_status;              /* auxiliary function return status flag */
  smp_index_pos cur_cnt;        /* auxiliary position counter */
  index_entry_type cur;         /* auxiliary pointer for the current entry */
  FILE *fp = (FILE *) NULL;     /* output file descriptors */
  FILE *fplist = (FILE *) NULL; /* output file descriptors */
  FileName filename;            /* output file name */


  /* Opens the index destination file for output */
  if (write_index == SMP_WRITE_INDEX)
    {
      if (dest_file == NULL)
        fp = stdout;
      else
        {
          fp = fopen (dest_file, "w");
          if (fp == (FILE *) NULL)
            {
              fprintf (stderr, "write_index: %s\n", strerror (errno));
              return EXIT_FAILURE;
            }
        }

      /*
       * Index header info
       * Outputs the index ID and its parent ID
       */
      fprintf (fp, "Index ID       : %lu\n", index.index_id);
      if (index.parent_index != NULL)
        fprintf (fp, "Parent Index ID: %lu\n", index.parent_index->index_id);

      /* Outputs the number of lists under the index */
      fprintf (fp, "Number of lists: %ld\n", index.num_entries);

      if (index.num_entries > 0)
        fprintf (fp, "\n");
    }

  /*
   * If lists should be printed to a single file, defines its name and opens it
   * for writing
   */
  if (write_lists == SMP_WRITE_LISTS && write_files == SMP_WRITE_SINGLE_FILE)
    {
      /* The lists file name is the index file name plus an extension */
      sprintf (filename, "%s", dest_file);

      /* Open the lists file */
      fplist = fopen (dest_file, "w");
      if (fplist == (FILE *) NULL)
        {
          fprintf (stderr, "write_index: %s\n", strerror (errno));
          return EXIT_FAILURE;
        }
    }

  if (write_index == SMP_WRITE_INDEX || write_lists == SMP_WRITE_LISTS)
    {
      /* Position the current entry to the head of the list */
      cur = index.head;
      cur_cnt = 0;

      /* Lists loop */
      while (cur != NULL)
        {
          /* Increment list position counter */
          cur_cnt++;

          if (write_index == SMP_WRITE_INDEX)
            {
              /* Outputs list header info */
              fprintf (fp, "List #%lu\n", cur_cnt);
              fprintf (fp, "Position   : %ld\n", cur->position);
              fprintf (fp, "Name       : %s\n", cur->name);
              fprintf (fp, "File       : %s\n", cur->file);
              fprintf (fp, "Samples    : %ld\n", cur->list->samples);
              fprintf (fp, "Parent Pos.: ");

              if (cur->parent == NULL)
                fprintf (fp, "None\n");
              else
                fprintf (fp, "%ld\n", cur->parent->position);

              fprintf (fp, "Parent Name: ");

              if (cur->parent == NULL)
                fprintf (fp, "None\n");
              else
                fprintf (fp, "%s\n", cur->parent->name);

              fprintf (fp, "Init. Time : %E\n", cur->list->ini_time);
              fprintf (fp, "Time incr. : %E\n", cur->list->inc_time);
              fprintf (fp, "Init. ntime: %ld\n", cur->list->ini_norm_time);

              /* If the statistics are valid, print them */
              if (cur->list->valid_stats == SMP_YES)
                {
                  fprintf (fp, "Average    : (%f, %f)\n", cur->list->avg.re,
                           cur->list->avg.im);
                  fprintf (fp, "Variance   : (%f, %f)\n", cur->list->var.re,
                           cur->list->var.im);
                  fprintf (fp, "Std. Dev.  : (%f, %f)\n", cur->list->std.re,
                           cur->list->std.im);
                  fprintf (fp, "Maximums   : (%f, %f)\n", cur->list->max.re,
                           cur->list->max.im);
                  fprintf (fp, "Minimums   : (%f, %f)\n", cur->list->min.re,
                           cur->list->min.im);
                }
            }

          if (write_lists == SMP_WRITE_LISTS)
            {
              /* Open list output file */
              if (dest_file == NULL)
                {
                  fplist = stdout;
                }
              else
                {
                  if (write_files == SMP_WRITE_MANY_FILES)
                    {
                      /* Defines the output file name */
                      sprintf (filename, "%s", cur->file);

                      /* Opens the destination file */
                      fplist = fopen (filename, "w");
                      if (fplist == (FILE *) NULL)
                        {
                          fprintf (stderr, "write_index: %s\n",
                                   strerror (errno));
                          return EXIT_FAILURE;
                        }
                    }
                }

              /* Outputs the list contents */
              exit_status =
                write_list (*(cur->list), fplist, break_lines, write_time,
                            write_files, real_part, img_part);

              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr, "write_index: error writing list\n");
                  return EXIT_FAILURE;
                }

              /* Close list output file */
              if (write_files == SMP_WRITE_MANY_FILES)
                {
                  if (dest_file != NULL)
                    fclose (fplist);
                }
              else
                {
                  fprintf (fplist, "\n");
                }
            }

          /* Proceed to next list */
          cur = cur->next;

          if (write_index == SMP_WRITE_INDEX)
            fprintf (fp, "\n");
        }
    }

  /* If single lists file was selected, close it */
  if (write_lists == SMP_WRITE_LISTS && write_files == SMP_WRITE_SINGLE_FILE)
    fclose (fplist);

  /* Close index file */
  if (write_index == SMP_WRITE_INDEX && dest_file != NULL)
    fclose (fp);

  return EXIT_SUCCESS;
}



/*
 * write_list
 *
 * Outputs the contents of the given list to 'dest_file'.
 * The flag 'raw_data' means that only data will be written.
 */
int
write_list (const sample_list_type smp_list,
            FILE * dest_file,
            const smp_breaklines_type break_lines,
            const smp_write_time_type write_time,
            const smp_write_files_type write_files,
            const smp_yes_no real_part, const smp_yes_no img_part)
{
  /* Auxiliary function return status flag */
  int exit_status;

  /* Auxiliary samples counter */
  smp_num_samples smp_cnt;

  /* Auxiliary time */
  cmp_real aux_time;

  /* Auxiliary sample value */
  cmp_complex z;


  /* Outputs the samples */
  for (smp_cnt = 1; smp_cnt <= smp_list.samples; smp_cnt++)
    {
      /* Fetches the value into z */
      exit_status = get_list_value (smp_list, smp_cnt, &z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "write_list: error getting list value at position %ld\n",
                   smp_cnt);
          return EXIT_FAILURE;
        }

      /* First sample time */
      if (write_files == SMP_WRITE_SINGLE_FILE)
        {
          if (write_time != SMP_DONT_WRITE_TIME)
            {
              switch (write_time)
                {
                case SMP_WRITE_TIME:
                  fprintf (dest_file, "%E", smp_list.ini_time);
                  break;

                case SMP_WRITE_NORM_TIME:
                  fprintf (dest_file, "%E",
                           (cmp_real) smp_list.ini_norm_time);
                  break;

                default:
                  fprintf (stderr, "write_list: unknown time format\n");
                  return EXIT_FAILURE;
                }
            }

          if (write_time == SMP_WRITE_TIME ||
              write_time == SMP_WRITE_NORM_TIME ||
              real_part == SMP_YES || img_part == SMP_YES)
            fprintf (dest_file, " ");
          else
            fprintf (dest_file, "\n");
        }

      /* Time scale */
      if (write_time != SMP_DONT_WRITE_TIME)
        {
          /*
           * If SMP_WRITE_TIME was selected, then the sample time will be used;
           * otherwise, the normalized time (sample index) will be
           */
          switch (write_time)
            {
            case SMP_WRITE_TIME:
              aux_time =
                smp_list.ini_time +
                (smp_list.inc_time * (cmp_real) (smp_cnt - 1));
              break;

            case SMP_WRITE_NORM_TIME:
              aux_time = (cmp_real) (smp_cnt - 1);
              break;

            default:
              fprintf (stderr, "write_list: unknown time format\n");
              return EXIT_FAILURE;
            }

          /* Formatted output */
          fprintf (dest_file, "%E", aux_time);

          if (real_part == SMP_YES || img_part == SMP_YES)
            fprintf (dest_file, " ");
          else
            fprintf (dest_file, "\n");
        }

      /* Values info */
      if (real_part == SMP_YES)
        fprintf (dest_file, "%E", z.re);

      if (real_part == SMP_YES && img_part == SMP_YES
          && smp_list.data_type == SMP_COMPLEX)
        fprintf (dest_file, " ");

      if (img_part == SMP_YES && smp_list.data_type == SMP_COMPLEX)
        fprintf (dest_file, "%E", z.im);

      if (real_part == SMP_YES || img_part == SMP_YES)
        {
          if (break_lines == SMP_BREAK_LINES)
            fprintf (dest_file, "\n");
          else
            fprintf (dest_file, " ");
        }
    }

  if (write_files == SMP_WRITE_SINGLE_FILE &&
      break_lines == SMP_DONT_BREAK_LINES)
    fprintf (dest_file, "\n");

  return EXIT_SUCCESS;
}




/*
 * read_raw_file
 *
 * Reads a raw format samples file
 */
static int
read_raw_file (FILE * fp, sample_list_type * smp_list)
{
  FileName buf;                 /* read buffer */
  const char delimiters[] = " ,;()ijIJ[]{}";    /* characters to be parsed */
  cmp_complex aux_z;            /* auxiliary complex number read */
  char *cp;                     /* auxiliary character */
  int exit_status;              /* auxiliary function return status */


  /* Reads the samples */
  do
    {
      if (fgets (buf, FILE_NAME_SIZE, fp) != NULL)
        {
          /* Avoid empty lines */
          if (*buf != '\0')
            {
              cp = (char *) strtok (buf, delimiters);

              /* Reads the real part - supposed to be first */
              if (cp != NULL)
                aux_z.re = strtod (cp, NULL);
              else
                aux_z.re = 0;

              /* Reads the complex part - supposed to be next */
              if (smp_list->data_type == SMP_COMPLEX)
                {
                  if (cp != NULL)
                    {
                      cp = (char *) strtok (NULL, delimiters);

                      if (cp != NULL)
                        aux_z.im = strtod (cp, NULL);
                      else
                        aux_z.im = 0;
                    }
                }

              /* Stores the new value on the list */
              exit_status = add_list_value (&smp_list, aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "read_raw_file: error storing last value read\n");
                  return EXIT_FAILURE;
                }
            }
        }
    }
  while (!feof (fp));

  return EXIT_SUCCESS;
}




/*
 * smp_translate_wave
 *
 * Translate data from a wave file to a list of samples
 */
int
smp_translate_wave (const swav_type riff, sample_list_type * smp_list)
{
  /* Number of samples */
  size_t samples = 0;

  /* Number of samples per second */
  size_t samples_per_second = 0;

  /* Sampling frequency */
  cmp_real sampling_interval = 0.0;

  /* Auxiliary sample counter */
  size_t cur_sample;

  /* Auxiliary current channel */
  size_t cur_channel;

  /* Auxiliary samples */
  swav_sample_type value;

  /*
   * Range factor
   *
   * The WAVE format associates the sample values range to the
   * number of bytes per second (time resolution) in the following way
   * . if bytes per second = 8, then the range is 0..255
   * . otherwise, the values are zero-centered.
   * Because of this, if bytes per second = 8, the values will be
   * subtracted by a factor of 128, to eliminate the spurious DC component
   * introduced by this.
   */
  swav_sample_type range_factor;

  /* Auxiliary complex value */
  cmp_complex aux_z;

  /* Auxiliary function return status */
  int exit_status = EXIT_FAILURE;



  /* Calculate the number of samples */
  samples = swav_num_samples (riff);

  /* Gets the number of samples per second */
  samples_per_second = swav_samples_per_second (riff);

  /* Calculates the sampling frequency */
  if (samples_per_second == 0)
    sampling_interval = 0.0;
  else
    sampling_interval = 1.0 / (cmp_real) samples_per_second;

  /* Sets the sampling interval */
  if (samples_per_second != 0)
    {
      smp_list->ini_time = 0.0;
      smp_list->inc_time = sampling_interval;
    }

  /* Determines the range factor */
  if (riff.wave_c.bytes_per_sample == 1)
    range_factor = 128.0;
  else
    range_factor = 0.0;

  /* Resizes the list to the number of samples */
  exit_status = resize_list (&smp_list, samples);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "smp_translate_wave: error resizing samples list\n");
      return EXIT_FAILURE;
    }

  if (samples > 0)
    {
      /* Sample conversion loop */
      for (cur_sample = 0; cur_sample < samples; cur_sample++)
        {
          aux_z.re = 0.0;
          aux_z.im = 0.0;

          for (cur_channel = 1; cur_channel <= riff.wave_c.channels;
               cur_channel++)
            {
              /* Gets the sample value */
              exit_status =
                swav_get_sample_value (riff, cur_sample, cur_channel, &value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "smp_translate_wave: error getting sample from WAVE structure\n");
                  return EXIT_FAILURE;
                }

              /* Attributes the sample value to the complex number */
              if (cur_channel == 1)
                aux_z.re = (cmp_real) value - range_factor;
              else if (cur_channel == 2 || smp_list->data_type == SMP_COMPLEX)
                aux_z.im = (cmp_real) value - range_factor;

            }

          /* Sets the sample value on the list */
          exit_status = set_list_value (smp_list, cur_sample + 1, aux_z);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "smp_translate_wave: error setting list value at positioin %lu\n",
                       cur_sample + 1);
              return EXIT_FAILURE;
            }
        }

    }

  return EXIT_SUCCESS;
}




/*
 * read_samples_file
 *
 * Reads a file that is supposed to contain complex numbers.
 * Stores the contents read on the list at position 'pos'
 * of the list index 'list'.
 * The name of the file must be stored at the list entry.
 *
 * Parameters:
 * - index: input index where the samples list will be stored
 * - pos: the position of the index entry where the list will be stored
 */
int
read_samples_file (index_list_type * index, const smp_index_pos pos)
{
  /* File descriptor */
  FILE *fp;

  /* Auxiliary function return status */
  int exit_status;

  /* Auxiliary pointer to the list index entry */
  index_entry_type entry;



  /* Points the auxiliary pointer to the required index entry */
  exit_status = move_to_entry (index, pos, &entry);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "read_samples_file: cannot go to the required list\n");
      return EXIT_FAILURE;
    }


  /* Tries to detect the input file type */
  exit_status = detect_file_type (entry, &(entry->file_type));
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "read_samples_file: error trying to detect file format\n");
      return EXIT_FAILURE;
    }

  /* Open samples file */
  fp = fopen (entry->file, "r");
  if (fp == (FILE *) NULL)
    {
      fprintf (stderr, "read_samples_file: %s\n", strerror (errno));
      return EXIT_FAILURE;
    }

  /* Initialize the list's statistics */
  istt_clear_stat ();

  /* Reads the samples file according to its type */
  switch (entry->file_type)
    {
    case WAVE:
      exit_status = swav_read (fp, &entry);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "read_samples_file: error reading WAVE file '%s'\n",
                   entry->file);
          return EXIT_FAILURE;
        }
      break;

    case RAW:
      exit_status = read_raw_file (fp, entry->list);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "read_samples_file: error reading RAW file '%s'\n",
                   entry->file);
          return EXIT_FAILURE;
        }
      break;

    default:
      fprintf (stderr,
               "read_samples_file: unable to determine file format: '%s'\n",
               entry->file);
      return EXIT_FAILURE;
      break;
    }

  /* Close the input file */
  fclose (fp);

  return EXIT_SUCCESS;
}




/*
 * detect_file_type
 *
 * Tries to detect if the input file is at one of the supported formats
 */
int
detect_file_type (const index_entry_type list, list_file_type * type)
{
  /* auxiliary "format detected" flag */
  int detected;

  /* auxiliary funcion return status */
  int exit_status;


  /* Tries the file as a RIFF WAVE file */
  exit_status = swav_detect (list->file, &detected);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "detect_file_type: error trying to detect file as RIFF WAVE\n");
      return EXIT_FAILURE;
    }

  if (detected)
    {
      *type = WAVE;
      return EXIT_SUCCESS;
    }

  /* Default is RAW */
  *type = RAW;

  return EXIT_SUCCESS;
}
