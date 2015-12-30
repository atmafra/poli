#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "s_cepstrum.h"
#include "s_complex.h"
#include "s_samples.h"
#include "s_window.h"
#include "s_files.h"
#include "s_preemph.h"
#include "s_fft.h"

/*
 * scep_f_hz_to_mel
 *
 * Returns the Mel frequency associated to the given input frequency (in Hz)
 */
cmp_real
scep_f_hz_to_mel (cmp_real f_hz)
{
  return __C1_ * log (1.0 + f_hz / 700.0);
}



/*
 * scep_f_mel_to_hz
 *
 * Returns the frequency in Hz associated to the given mel-scale frequency
 */
cmp_real
scep_f_mel_to_hz (cmp_real f_mel)
{
  return 700.0 * (exp (f_mel / __C1_) - 1.0);
}



/*
 * scep_mel_scale
 *
 * Constructs and returns values from a look-up table,
 * containing the frequencies in Hz obtained from
 * the conversion of a series of equally spaced mel frequencies.
 *
 * Parameters:
 * - delta_mel: mel frequency interval between filters
 * - total_filters: number of filters in the bank
 * - filter: input filter number (1 <= 'filter' <= 'total_filters')
 * - low_freq: low frequency of the input filter
 * - mid_freq: central frequency of the input filter
 * - hi_freq: high frequency of the input filter
 */
int
scep_mel_scale (const cmp_real delta_mel,
                const smp_num_samples total_filters,
                const smp_num_samples filter,
                cmp_real * low_freq, cmp_real * mid_freq, cmp_real * hi_freq)
{
  /* Internal mel frequency interval */
  static cmp_real int_delta_mel = 0.0;

  /* Internal current number of filters */
  static smp_num_samples int_total_filters = 0;

  /* Internal Hz frequency series */
  static sample_list_type *mel_list = NULL;

  /* Auxiliary internal filter counter */
  smp_num_samples cur_filter;

  /* Auxiliary first filter to create */
  smp_num_samples aux_first_filter, aux_last_filter;

  /* Auxiliary list element value */
  cmp_complex aux_z;

  /* Auxiliary frequencies */
  cmp_complex aux_low_freq, aux_mid_freq, aux_hi_freq;

  /* Auxiliary function return status */
  int exit_status;


  /* Checks the input parameters domains */
  if (delta_mel < DBL_EPSILON)
    {
      fprintf (stderr,
               "scep_mel_scale: non-positive mel frequency interval requested\n");
      return EXIT_FAILURE;
    }

  /*
  if (total_filters < 0)
    {
      fprintf (stderr,
               "scep_mel_scale: negative number of filters requested\n");
      return EXIT_FAILURE;
    }
   */

  if (filter > total_filters)
    {
      fprintf (stderr,
               "scep_mel_scale: requested input filter (%ld) is beyond the number of filters (%ld)\n",
               filter, total_filters);
      return EXIT_FAILURE;
    }

  /* If 'delta_mel' changed, then the table needs to be reconstructed */
  if (fabs (delta_mel - int_delta_mel) > DBL_EPSILON)
    {
      /* Look-up table has to be reconstructed */

      /* If there is a reminder list, destroy it */
      if (int_total_filters > 0)
        {
          exit_status = destroy_list (&mel_list);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "scep_mel_scale: error destroyin old look-up mel-frequencies table\n");
              return EXIT_FAILURE;
            }
        }

      /*
       * Recreates the list, if the total number of filters
       * is greater than zero
       */
      if (total_filters > 0)
        {
          /* Creates the new list */
          exit_status = create_list (&mel_list, SMP_REAL, 0.0, delta_mel, 0);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "scep_mel_scale: error creating new frequency list\n");
              return EXIT_FAILURE;
            }
        }

      /*
       * All the elements should be reconstructed
       * One aditional value must be created for the return of the high
       * frequency associated to the last filter
       */
      aux_first_filter = 1;
      aux_last_filter = total_filters + 1;

      /* Updates the internal 'delta_mel' and 'total_filters' */
      int_delta_mel = delta_mel;
    }                           /* Table reconstruction */

  else
    {
      /* 'delta_mel' hasn't changed. Check if the table must be resized */
      if (total_filters > int_total_filters)
        {
          /*
           * Table must grow
           * One aditional value must be created for the return of the high
           * frequency associated to the last filter
           */
          aux_first_filter = int_total_filters + 1;
          aux_last_filter = total_filters + 1;
        }
      else
        {
          /* No elements should be created */
          aux_first_filter = 1;
          aux_last_filter = 0;
        }
    }

  /* Resizes the list if necessary */
  if (total_filters != int_total_filters)
    {
      exit_status = resize_list (&mel_list, total_filters + 1);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mel_scale: error resizing list from %ld to %ld elements\n",
                   int_total_filters, total_filters);
          return EXIT_FAILURE;
        }
    }


  for (cur_filter = aux_first_filter; cur_filter <= aux_last_filter;
       ++cur_filter)
    {
      /* Calculates the element value */
      aux_z.re = scep_f_mel_to_hz ((cmp_real) cur_filter * delta_mel);
      aux_z.im = 0.0;

      /* Includes the value in the list */
      exit_status = set_list_value (mel_list, cur_filter, aux_z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mel_scale: error including value %f in filter %ld of the mel-frequency filter\n",
                   aux_z.re, cur_filter);
          return EXIT_FAILURE;
        }
    }                           /* inclusion loop */

  /* Updates the internal 'total_filters' */
  if (total_filters != int_total_filters)
    int_total_filters = total_filters;

  /* Gets the requested frequencies */

  if (total_filters > 0)
    {
      /* Low frequency */
      if (filter <= 1)
        aux_low_freq.re = 0.0;
      else
        {
          exit_status = get_list_value (*mel_list, filter - 1, &aux_low_freq);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "scep_mel_scale: error getting low frequency for filter %ld\n",
                       filter);
              return EXIT_FAILURE;
            }
        }

      /* Mid frequency */
      exit_status = get_list_value (*mel_list, filter, &aux_mid_freq);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mel_scale: error getting mid frequency for filter %ld\n",
                   filter);
          return EXIT_FAILURE;
        }

      /* High frequency */
      exit_status = get_list_value (*mel_list, filter + 1, &aux_hi_freq);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mel_scale: error getting high frequency for filter %ld\n",
                   filter);
          return EXIT_FAILURE;
        }

      /* Sets the return values */
      *low_freq = aux_low_freq.re;
      *mid_freq = aux_mid_freq.re;
      *hi_freq = aux_hi_freq.re;
    }
  else
    {
      /* List was destroyed */
      *low_freq = 0.0;
      *mid_freq = 0.0;
      *hi_freq = 0.0;
    }

  return EXIT_SUCCESS;
}




/*
 * scep_filter_value
 *
 * Returns the filter value at the input position
 */
int
scep_filter_value (const cmp_real i, const cmp_real central_i,
                   const cmp_real low_i, const cmp_real high_i,
                   const cmp_real central_value, cmp_real * value)
{
  if (i - low_i <= DBL_EPSILON || high_i - i <= DBL_EPSILON)
    *value = 0.0;
  else
    {
      if (central_i - i > DBL_EPSILON)
        *value = central_value * ((i - low_i) / (central_i - low_i));
      else if (i - central_i > DBL_EPSILON)
        *value = central_value * ((high_i - i) / (high_i - central_i));
      else
        *value = central_value;
    }

  return EXIT_SUCCESS;
}




/*
 * scep_mfcc
 *
 * Calculates the MFCC's (Mel Frequency Cepstral Coefficients)
 * of the input signal given by 'in_list', and then appends the
 * generated MFCC list to the given 'out_index'.
 *
 * Parameters:
 * - list: the input list, representing the signal to which the mel filters
 * bank should be applied to
 * - delta_mel: mel frequency interval between filters
 * - total_filters: number of filters in the bank
 * - out_index: index to which the MFCC coefficients list should be appended to
 */
int
scep_mfcc (const index_entry_type list, const cmp_real delta_mel,
           const smp_num_samples total_filters, index_list_type * out_index)
{
  /* Sample associated to the discretized low frequency */
  smp_num_samples low_sample;

  /* Sample associated to the discretized high frequency */
  smp_num_samples hi_sample;

  /* Auxiliary current sample */
  smp_num_samples cur_sample;

  /* Time scale attributes from the source list */
  cmp_real ini_time, inc_time;
  smp_num_samples ini_norm_time;

  /* Auxiliary current filter */
  smp_num_samples cur_filter;

  /* Mel filter frequencies */
  cmp_real low_freq, mid_freq, hi_freq;

  /* Auxiliary frequency associated to the current sample */
  cmp_real cur_freq;

  /* Auxiliary MFCC list position in the output index */
  smp_index_pos mfcc_list_pos;

  /* Auxiliary MFCC value */
  cmp_complex aux_mfcc;

  /* Auxiliary input list sample value */
  cmp_complex aux_in_value;

  /* Auxiliary filter value at sample position */
  cmp_real filter_value;

  /* Auxiliary list and file names */
  Name list_name = "";
  FileName file_name = "";

  /* Auxiliary list */
  sample_list_type in_list;

  /* Auxiliary function return status */
  int exit_status;



  /* Initializes the auxiliary list */
  in_list = *(list)->list;

  /* Initializes the input list time scale auxiliary variables */
  ini_time = in_list.ini_time;
  inc_time = in_list.inc_time;
  ini_norm_time = in_list.ini_norm_time;

  /* Defines the list and the file names */
  sprintf (list_name, "%s%s", list->name, ", MFCC");
  sprintf (file_name, "%s%s", list->file, ".mfcc");

  /*
   * Adds a new entry to the output index
   * The new list will be created with complex values for future in-place Fast
   * DCT computing
   */
  exit_status =
    add_index_entry (out_index, list_name, file_name, NULL, SMP_COMPLEX,
                     ini_time, inc_time, ini_norm_time, &mfcc_list_pos,
                     SMP_SET_CURRENT);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc: error adding new entry to the output index\n");
      return EXIT_FAILURE;
    }

  /* Resizes the list to the number of MFCC's it will contain */
  exit_status = resize_list (&(out_index->current->list), total_filters);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc: error allocating memory to the MFCC list\n");
      return EXIT_FAILURE;
    }

  /* Filter loop */
  for (cur_filter = 1; cur_filter <= total_filters; cur_filter++)
    {
      /* Gets the mel frequencies of the filter */
      exit_status =
        scep_mel_scale (delta_mel, total_filters, cur_filter, &low_freq,
                        &mid_freq, &hi_freq);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mfcc: error getting frequencies for mel filter %ld\n",
                   cur_filter);
          return EXIT_FAILURE;
        }

      /*
       * Calculates the limiting discretized samples associated to the low and
       * high mel filter frequencies.
       * C-style rounding: integer result is rounded downwards
       */
      low_sample = (smp_num_samples) (low_freq / inc_time) + 1;
      hi_sample = (smp_num_samples) (hi_freq / inc_time);

      /* Initializes the auxiliary mel coefficients */
      aux_mfcc.re = 0.0;
      aux_mfcc.im = 0.0;

      /* Samples loop */
      for (cur_sample = low_sample; cur_sample <= hi_sample; cur_sample++)
        {
          /*
           * If the sample is still in the input list domain,
           * gets its value; otherwise, the value is assumed zero.
           */
          if (cur_sample <= in_list.samples)
            {
              exit_status =
                get_list_value (in_list, cur_sample, &aux_in_value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "scep_mfcc: error getting input list value at position %ld for filter %ld\n",
                           cur_sample, cur_filter);
                  return EXIT_FAILURE;
                }
            }
          else
            {
              aux_in_value.re = 0.0;
              aux_in_value.im = 0.0;
            }

          /* Calculates the frequency associated to the current sample */
          cur_freq = cur_sample * inc_time;

          /* Calculates the filter falue at the sample frequency */
          exit_status =
            scep_filter_value (cur_freq, mid_freq, low_freq, hi_freq, 1.0,
                               &filter_value);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "scep_mfcc: error getting filter value for sample %ld at filter %ld\n",
                       cur_sample, cur_filter);
              return EXIT_FAILURE;
            }

          /* Increments the MFCC */
          aux_mfcc.re += aux_in_value.re * filter_value;

          if (in_list.data_type == SMP_COMPLEX)
            aux_mfcc.im += aux_in_value.im * filter_value;
        }

      /* Adds the MFCC to the current list of the output index */
      exit_status =
        set_list_value (out_index->current->list, cur_filter, aux_mfcc);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "scep_mfcc: error setting MFCC value at position %ld\n",
                   cur_filter);
          return EXIT_FAILURE;
        }
    }                           /* filter loop */

  return EXIT_SUCCESS;
}




/*
 * scep_mfcc_index
 *
 * Calculates the MFCC's (Mel Frequency Cepstral Coefficients)
 * of all the lists in the input index 'in_index', generating MFCC lists
 * under the output 'mfcc_index'
 *
 * Parameters:
 * - in_index: input index, containing lists of the signals to calculate the
 * MFCC's
 * - delta_mel: mel frequency interval between filterrs
 * - total_filters: number of filters in the bank
 * - mfcc_index: output index, with the same number of lists of the input
 * index, containing the MFCC's of the input lists respectively
 */
int scep_mfcc_index
  (const index_list_type in_index, const cmp_real delta_mel,
   const smp_num_samples total_filters, index_list_type * mfcc_index)
{
  /* Auxiliary current index entry */
  index_entry_type cur;

  /* Auxiliary function return status */
  int exit_status;


  /* If the input index has no entries, there's nothing to be done */
  if (in_index.num_entries == 0)
    return EXIT_SUCCESS;

  /* Creates the MFCC index */
  exit_status = create_index (mfcc_index, (index_list_ptr) & in_index);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_index: error creating MFCC index\n");
      return EXIT_FAILURE;
    }

  /* Points 'cur' to the head of the input index */
  cur = in_index.head;

  /* Lists loop */
  while (cur != NULL)
    {
      /* Calculates the list's MFCC's */
      exit_status = scep_mfcc (cur, delta_mel, total_filters, mfcc_index);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "scep_mfcc_index: error calculating MFCC's\n");
          return EXIT_FAILURE;
        }

      /* Goes to the next list */
      cur = cur->next;
    }

  return EXIT_SUCCESS;
}



/*
 * scep_mfcc_file
 *
 * Calculates the MFCC's (Mel Frequency Cepstral Coefficients)
 * for the given input file, according to the given parameter structure.
 * The resulting lists are stored in the output 'mfcc_index'
 *
 * Parameters:
 * - in_file_name: the input file name
 * - param: the parameters structure
 * - file_index: index where the file entry will be appended
 * - mfcc_index: index where the MFCC's lists will be stored
 */
int
scep_mfcc_file (const char *in_file_name,
                const scep_parameter_type param,
                index_list_type * file_index, index_list_type * mfcc_index)
{
  /* Auxiliary function exit status */
  int exit_status;

  /* Generated frames index */
  index_list_type frames_index;

  /* Position of the input file on index */
  smp_index_pos file_entry_position;

  /* Just to make clear */
  const index_entry_type NO_PARENT_ENTRY = NULL;
  const smp_entries_type COMPLEX_FRAMES = SMP_COMPLEX;
  const smp_entries_type REAL_WINDOWING_OPERATIONS = SMP_REAL;


  /* Adds an entry to the new index */
  exit_status = add_index_entry (file_index,
                                 "Input file index entry",
                                 in_file_name,
                                 NO_PARENT_ENTRY,
                                 SMP_REAL, 0.0, 0.0, 0,
                                 &file_entry_position, SMP_SET_CURRENT);

  if (exit_status != EXIT_SUCCESS)
    {
      /* DESTROY INDEX !!! */
      fprintf (stderr,
               "scep_mfcc_file: error adding index entry for the file list\n");
      return EXIT_FAILURE;
    }

  /* Reads the input file */
  exit_status = read_samples_file (file_index, file_entry_position);
  if (exit_status != EXIT_SUCCESS)
    {
      /* DESTROY INDEX AND ENTRIES!!! */
      fprintf (stderr, "scep_mfcc_file: error reading input file\n");
      return EXIT_FAILURE;
    }

  /* Windowing of the input signal */
  exit_status = swin_window (file_index,
                             param.in_preemphasis,
                             param.alpha_preemphasis,
                             param.frame_width,
                             param.superposing_samples,
                             COMPLEX_FRAMES,
                             REAL_WINDOWING_OPERATIONS,
                             param.windowing_function,
                             param.triangular_window_center,
                             param.triangular_window_low,
                             param.triangular_window_high,
                             param.triangular_window_central_value,
                             param.kaiser_window_B,
                             param.purge_zero_power, &frames_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc_file: error performing windowing of the input file\n");
      return EXIT_FAILURE;
    }

  /* Executes the FFT on the frames */
  exit_status =
    sfft_exec_index (&frames_index, &frames_index, SFFT_FFT, SFFT_IN_PLACE,
                     SFFT_DIRECT, SFFT_REAL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error executing FFT\n");
      return EXIT_FAILURE;
    }

  /* Power Density Spectrum */
  exit_status =
    apply_index_function (&frames_index, cmp_sqr, CMP_FCN_COMPLEX, 0);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error calculating PSD\n");
      return EXIT_FAILURE;
    }

  /* Applies critical mel-frequency filters bank */
  exit_status =
    scep_mfcc_index (frames_index, param.delta_mel, param.total_filters,
                     mfcc_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error calcutating MFCC's\n");
      return EXIT_FAILURE;
    }

  /* Calculates the log of the module on the frequency domain */
  exit_status = apply_index_function
    (mfcc_index, cmp_rlg, CMP_FCN_COMPLEX, 1, param.log_basis);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error applying log function\n");
      return EXIT_FAILURE;
    }

  /* Destroy the auxiliary frames index */
  exit_status = destroy_index (&frames_index);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc_file: error releasing auxiliary frames index\n");
      return EXIT_FAILURE;
    }

  /* Executes the inverse DCT */
  exit_status =
    sfft_exec_index (mfcc_index, mfcc_index, SFFT_FCT, SFFT_IN_PLACE,
                     SFFT_DIRECT, SFFT_REAL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error executing the DCT\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * scep_mfcc_file_write
 *
 * Calculates the MFCC's for the given input file and outputs the list to the
 * given file. If the output file is passed NULL, then 'stdout' should be used.
 *
 * Parameters:
 * - in_file_name: input file name
 * - param: parameters structre
 * - out_file_name: output file name
 */
int
scep_mfcc_file_write (const char *in_file_name,
                      const scep_parameter_type param, char *out_file_name)
{
  /* Auxiliary function return status */
  int exit_status;

  /* File index, used to stored the signal of the input file */
  index_list_type file_index;

  /* MFCC index, returned by feature extraction */
  index_list_type mfcc_index;

  /* Just to make clear */
  index_list_type *NO_PARENT_INDEX = NULL;


  /* Creates the input file index */
  exit_status = create_index (&file_index, NO_PARENT_INDEX);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file_write: error creating file index\n");
      return EXIT_FAILURE;
    }

  /* Perform the MFCC feature extraction */
  exit_status =
    scep_mfcc_file (in_file_name, param, &file_index, &mfcc_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc_file_write: error performing MFCC extraction\n");
      return EXIT_FAILURE;
    }

  /* Destroys the file index */
  exit_status = destroy_index (&file_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file_write: error releasing file index\n");
      return EXIT_FAILURE;
    }

  /* Writes the MFCC's to the selected output */
  exit_status =
    write_index (mfcc_index, out_file_name, param.write_index,
                 param.write_lists, param.write_time, param.write_files,
                 param.write_break_lines, param.write_real_part,
                 param.write_img_part);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file_write: error in MFCC file output\n");
      return EXIT_FAILURE;
    }

  /* Destroys the MFCC's index */
  exit_status = destroy_index (&mfcc_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "scep_mfcc_file_write: error releasing MFCC's index\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
