#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "errorh.h"

#include "s_cepstrum.h"
#include "s_complex.h"
#include "s_samples.h"
#include "s_window.h"
#include "s_files.h"
#include "s_preemph.h"
#include "s_fft.h"
#include "s_deriv.h"

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
  static cmp_real int_delta_mel = 0.0;  /* mel frequency interval */
  static smp_num_samples int_total_filters = 0; /* current number of filters */
  static sample_list_type *mel_list = NULL;     /* Hz frequency series */

  smp_num_samples cur_filter;   /* filter counter */
  smp_num_samples aux_first_filter, aux_last_filter;    /* first filter to create */
  cmp_complex aux_z;            /* list element value */

  cmp_complex aux_low_freq, aux_mid_freq, aux_hi_freq;  /* Auxiliary frequencies */


  /* Checks the input parameters domains */
  if (delta_mel < DBL_EPSILON)
    return error_failure ("scep_mel_scale",
                          "non-positive mel frequency interval requested\n");

  if (filter > total_filters)
    return error_failure ("scep_mel_scale",
                          "requested input filter (%ld) out of range (%ld..%ld)\n",
                          filter, total_filters);

  /* If 'delta_mel' changed, then the table needs to be reconstructed */
  if (fabs (delta_mel - int_delta_mel) > DBL_EPSILON)
    {
      /* Look-up table has to be reconstructed */
      /* If there is a reminder list, destroy it */
      if (int_total_filters > 0)

        if (destroy_list (&mel_list) != EXIT_SUCCESS)

          return error_failure ("scep_mel_scale",
                                "error destroyin old look-up mel-frequencies table\n");

      /*
       * Recreates the list, if the total number of filters
       * is greater than zero
       */
      if (total_filters > 0)
        {
          /* Creates the new list */
          if (create_list (&mel_list, SMP_REAL, 0.0, delta_mel, 0)
              != EXIT_SUCCESS)

            return error_failure ("scep_mel_scale",
                                  "error creating mel-frequency list\n");
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
      if (resize_list (&mel_list, total_filters + 1) != EXIT_SUCCESS)

        return error_failure ("scep_mel_scale",
                              "error resizing list from %ld to %ld elements\n",
                              int_total_filters, total_filters);
    }


  for (cur_filter = aux_first_filter; cur_filter <= aux_last_filter;
       ++cur_filter)
    {
      /* Calculates the element value */
      aux_z.re = scep_f_mel_to_hz ((cmp_real) cur_filter * delta_mel);
      aux_z.im = 0.0;

      /* Includes the value in the list */
      if (set_list_value (mel_list, cur_filter, aux_z) != EXIT_SUCCESS)

        return error_failure ("scep_mel_scale",
                              "error including value %f in filter %ld of the mel-frequency filter\n",
                              aux_z.re, cur_filter);
    }

  /* Updates the internal 'total_filters' */
  if (total_filters != int_total_filters)
    int_total_filters = total_filters;

  /* Gets the requested frequencies */
  if (total_filters > 0)
    {
      /* Low frequency */
      if (filter <= 1)
        {
          aux_low_freq.re = 0.0;
        }
      else
        {
          if (get_list_value (*mel_list, filter - 1, &aux_low_freq)
              != EXIT_SUCCESS)
            return error_failure ("scep_mel_scale",
                                  "error getting low frequency for filter %ld\n",
                                  filter);
        }

      /* Mid frequency */
      if (get_list_value (*mel_list, filter, &aux_mid_freq) != EXIT_SUCCESS)
        return error_failure ("scep_mel_scale",
                              "error getting mid frequency for filter %ld\n",
                              filter);

      /* High frequency */
      if (get_list_value (*mel_list, filter + 1, &aux_hi_freq) !=
          EXIT_SUCCESS)
        return error_failure ("scep_mel_scale",
                              "error getting high frequency for filter %ld\n",
                              filter);

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
    {
      *value = 0.0;
    }
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
scep_mfcc (const index_entry_type list,
           const cmp_real delta_mel,
           const smp_num_samples total_filters,
           const smp_num_samples first_filter, index_list_type * out_index)
{
  smp_num_samples low_sample;   /* low frequency sample */
  smp_num_samples hi_sample;    /* high frequency sample */
  smp_num_samples cur_sample;   /* auxiliary current sample */

  /* Time scale attributes from the source list */
  cmp_real ini_time, inc_time;
  smp_num_samples ini_norm_time;

  smp_num_samples cur_filter;   /* current filter */
  smp_num_samples last_filter;  /* last filter */
  cmp_real low_freq, mid_freq, hi_freq; /* Mel filter frequencies */
  cmp_real cur_freq;            /* current sample frequency */

  /* Auxiliary MFCC list position in the output index */
  smp_index_pos mfcc_list_pos;

  cmp_complex aux_mfcc;         /* MFCC value */
  cmp_complex aux_in_value;     /* input list sample value */
  cmp_real filter_value;        /* filter value at sample position */

  /* Auxiliary list and file names */
  Name list_name = "";
  FileName file_name = "";

  sample_list_type in_list;     /* Auxiliary list */

  /* initialization */
  in_list = *(list)->list;
  ini_time = in_list.ini_time;
  inc_time = in_list.inc_time;
  ini_norm_time = in_list.ini_norm_time;
  last_filter = first_filter + total_filters - 1;

  /* Defines the list and the file names */
  sprintf (list_name, "%s%s", list->name, ", MFCC");
  sprintf (file_name, "%s%s", list->file, ".mfcc");

  /*
   * Adds a new entry to the output index
   * The new list will be created with complex values
   * for future in-place Fast DCT computing
   */
  if (add_index_entry (out_index, list_name, file_name, NULL, SMP_COMPLEX,
                       ini_time, inc_time, ini_norm_time, &mfcc_list_pos,
                       SMP_SET_CURRENT) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc",
                          "error adding new entry to the output index\n");

  /* Resizes the list to the number of MFCC's it will contain */
  if (resize_list (&(out_index->current->list), total_filters)
      != EXIT_SUCCESS)

    return error_failure ("scep_mfcc",
                          "error allocating memory to the MFCC list\n");

  /* Filter loop */
  for (cur_filter = first_filter; cur_filter <= last_filter; cur_filter++)
    {
      /*
       * Gets the mel frequencies of the filter
       * Calculates the limiting discretized samples associated to the low and
       * high mel filter frequencies.
       * C-style rounding: integer result is rounded downwards
       */
      if (scep_mel_scale (delta_mel, last_filter, cur_filter,
                          &low_freq, &mid_freq, &hi_freq) != EXIT_SUCCESS)

        return error_failure ("scep_mfcc",
                              "error getting frequencies for mel filter %ld\n",
                              cur_filter);

      low_sample = (smp_num_samples) (low_freq / inc_time) + 1.0;
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
              if (get_list_value (in_list, cur_sample, &aux_in_value)
                  != EXIT_SUCCESS)

                return error_failure ("scep_mfcc",
                                      "error getting input list value at position %ld for filter %ld\n",
                                      cur_sample, cur_filter);
            }
          else
            {
              aux_in_value.re = 0.0;
              aux_in_value.im = 0.0;
            }

          /* Calculates the frequency associated to the current sample */
          cur_freq = cur_sample * inc_time;

          /* Calculates the filter falue at the sample frequency */
          if (scep_filter_value (cur_freq, mid_freq, low_freq, hi_freq, 1.0,
                                 &filter_value) != EXIT_SUCCESS)

            return error_failure ("scep_mfcc",
                                  "error getting filter value for sample %ld at filter %ld\n",
                                  cur_sample, cur_filter);

          /* Increments the MFCC */
          aux_mfcc.re += aux_in_value.re * filter_value;

          if (in_list.data_type == SMP_COMPLEX)
            aux_mfcc.im += aux_in_value.im * filter_value;
        }

      /* Adds the MFCC to the current list of the output index */
      if (set_list_value (out_index->current->list,
                          cur_filter - first_filter + 1, aux_mfcc)
          != EXIT_SUCCESS)

        return error_failure ("scep_mfcc",
                              "error setting MFCC value at position %ld\n",
                              cur_filter - first_filter + 1);
    }

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
int
scep_mfcc_index (const index_list_type * in_index,
                 const cmp_real delta_mel,
                 const smp_num_samples total_filters,
                 const smp_num_samples first_filter,
                 index_list_type * mfcc_index)
{
  index_entry_type cur;         /* current index entry */


  /* If the input index has no entries, there's nothing to be done */
  if (in_index->num_entries == 0)
    return EXIT_SUCCESS;

  /* Creates the MFCC index */
  if (create_index (mfcc_index, (index_list_ptr) in_index) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_index", "error creating MFCC index\n");

  /* Points 'cur' to the head of the input index */
  cur = in_index->head;

  /* Lists loop */
  while (cur != NULL)
    {
      /* Calculates the list's MFCC's */
      if (scep_mfcc (cur, delta_mel, total_filters, first_filter, mfcc_index)
          != EXIT_SUCCESS)

        return error_failure ("scep_mfcc_index",
                              "error calculating MFCC's\n");

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
  index_list_type frames_index; /* Generated frames index */
  smp_index_pos file_pos;       /* Position of the input file on index */
  const index_entry_type NO_PARENT_ENTRY = NULL;
  const smp_entries_type COMPLEX_FRAMES = SMP_COMPLEX;
  const smp_entries_type REAL_WINDOWING_OPERATIONS = SMP_REAL;
  smp_num_samples mfcc_size;    /* MFCC list size */


  /* Adds an entry to the file index */
  if (add_index_entry (file_index, in_file_name, in_file_name,
                       NO_PARENT_ENTRY, SMP_REAL, 0.0, 0.0, 0,
                       &file_pos, SMP_SET_CURRENT) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file",
                          "error adding index entry for the file list\n");

  /* Reads the input file */
  if (read_samples_file (file_index, file_pos) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_file", "error reading input file\n");

  /* Windowing of the input signal */
  if (swin_window (file_index,
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
                   param.purge_zero_power, &frames_index) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file",
                          "error performing windowing of the input file\n");

  /* Executes the FFT on the frames */
  if (sfft_exec_index (&frames_index, &frames_index, SFFT_FFT, SFFT_IN_PLACE,
                       SFFT_DIRECT, SFFT_REAL) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file", "error executing FFT\n");

  /* Power Density Spectrum */
  if (apply_index_function (&frames_index, cmp_sqr, CMP_FCN_COMPLEX, 0)
      != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file", "error calculating PSD\n");

  /* Applies critical mel-frequency filters bank */
  if (scep_mfcc_index (&frames_index, param.delta_mel,
                       param.total_filters, param.first_filter, mfcc_index)
      != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file", "error calcutating MFCCs\n");

  /* Calculates the log of the module on the frequency domain */
  if (apply_index_function (mfcc_index, cmp_rlg, CMP_FCN_COMPLEX,
                            1, param.log_basis) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file", "error applying log function\n");

  /* Destroy the auxiliary frames index */
  if (destroy_index (&frames_index) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file",
                          "error releasing auxiliary frames index\n");

  /* Executes the inverse DCT */
  if (sfft_exec_index (mfcc_index, mfcc_index, SFFT_FCT, SFFT_IN_PLACE,
                       SFFT_DIRECT, SFFT_REAL) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file", "error executing the DCT\n");

  /*
   * Resizes MFCC lists, according to the last used MFCC and
   * the use or not of derivatives
   */
  if (param.use_derivatives == SMP_YES)
    mfcc_size = 2 * param.last_mfcc_used;
  else
    mfcc_size = param.last_mfcc_used;

  if (resize_index (mfcc_index, mfcc_size) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_file", "error resizing MFCC index\n");

  /* Calculates the derivatives */
  if (param.use_derivatives == SMP_YES)

    if (sder_index_derivative (*mfcc_index) != EXIT_SUCCESS)
      return error_failure ("scep_mfcc_file",
                            "error calculating MFCCs derivatives\n");

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
  index_list_type file_index;   /* input file signal */
  index_list_type mfcc_index;   /* MFCC index */


  /* Creates the input file index */
  if (create_index (&file_index, NULL) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_file_write",
                          "error creating file index\n");

  /* Perform the MFCC feature extraction */
  if (scep_mfcc_file (in_file_name, param, &file_index, &mfcc_index)
      != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file_write",
                          "error performing MFCC extraction\n");

  /* Destroys the file index */
  if (destroy_index (&file_index) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_file_write",
                          "error releasing file index\n");

  /* Writes the MFCC's to the selected output */
  if (write_index (mfcc_index, out_file_name, param.write_index,
                   param.write_lists, param.write_time, param.write_files,
                   param.write_break_lines, param.write_real_part,
                   param.write_img_part) != EXIT_SUCCESS)

    return error_failure ("scep_mfcc_file_write",
                          "error in MFCC file output\n");

  /* Destroys the MFCC's index */
  if (destroy_index (&mfcc_index) != EXIT_SUCCESS)
    return error_failure ("scep_mfcc_file_write",
                          "error releasing MFCC's index\n");

  return EXIT_SUCCESS;
}
