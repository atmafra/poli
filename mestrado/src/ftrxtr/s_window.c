#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include "s_samples.h"
#include "s_complex.h"
#include "s_preemph.h"
#include "../incstat/incstat.h"
#include "s_window.h"

/*
 * Windowing Functions
 */

/*
 * swin_win_rectangular
 *
 * Implements the triangular (Bartlett) windowing function.
 */
static int
swin_win_rectangular (const long int i, const smp_num_samples N,
                      spre_real * value)
{
  *value = (spre_real) 1.0;

  return EXIT_SUCCESS;
}



/*
 * swin_win_bartlett
 *
 * Implements the triangular Bartlett windowing function.
 */
static int
swin_win_bartlett (const long int i, const smp_num_samples N,
                   spre_real * value)
{
  *value = (i <= 0 ? (spre_real) 1.0 + ((2.0 / (spre_real) N) * (spre_real) i)
            : (spre_real) 1.0 - ((2.0 / (spre_real) N) * (spre_real) i));

  return EXIT_SUCCESS;
}




/*
 * swin_win_triangular
 *
 * Implements the generic triangular windowing function
 */
static int
swin_win_triangular (const long int i, const spre_real central_i,
                     const spre_real low_i, const spre_real high_i,
                     const spre_real central_value, spre_real * value)
{
  if ((spre_real) i - low_i <= DBL_EPSILON
      || high_i - (spre_real) i <= DBL_EPSILON)
    *value = 0.0;
  else
    {
      if (central_i - (spre_real) i > DBL_EPSILON)
        *value =
          central_value * (((spre_real) i - low_i) / (central_i - low_i));
      else if ((spre_real) i - central_i > DBL_EPSILON)
        *value =
          central_value * ((high_i - (spre_real) i) / (high_i - central_i));
      else
        *value = central_value;
    }

  return EXIT_SUCCESS;
}




/*
 * swin_win_raised_cosine
 *
 * The general form for the Hamming and Hanning windowing functions.
 * Implements the equation:
 * w[i] = a + (1-a) cos (2 * PI * i / N)
 */
static int
swin_win_raised_cosine (const spre_real a, const long int i,
                        const smp_num_samples N, spre_real * value)
{
  /* Checks the validity of a */
  if (a < DBL_EPSILON || a > 1.0 - DBL_EPSILON)
    {
      fprintf (stderr,
               "swin_win_raised_cosine: value for a (%f) is out of range\n",
               a);
      return EXIT_FAILURE;
    }

  /* Checks the validity of N */
  if (N < 1)
    {
      fprintf (stderr,
               "swin_win_raised_cosine: N must be a positive value\n");
      return EXIT_FAILURE;
    }

  *value = (spre_real) (a + ((spre_real) 1.0 - a) *
                        (spre_real) cos ((double) (2.0 * PI * (double) i) /
                                         (double) N));

  return EXIT_SUCCESS;
}



/*
 * swin_win_hamming
 *
 * Implements the Hamming windowing function.
 * This function is a particular raised cosine function, with a=0.54.
 */
static int
swin_win_hamming (const long int i, const smp_num_samples N,
                  spre_real * value)
{
  /* Auxiliary function exit status flag */
  int exit_status;

  /* The value of a for the Hamming function */
  const spre_real a = 0.54;


  /* Dispatches the execution for swin_win_raised_cosine */
  exit_status = swin_win_raised_cosine (a, i, N, value);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swin_win_hamming: error calculating raised cosine\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * swin_win_hanning
 *
 * Implements the Hanning windowing function.
 * This function is a particular raised cosine function, with a=0.50.
 */
static int
swin_win_hanning (const long int i, const smp_num_samples N,
                  spre_real * value)
{
  /* Auxiliary function exit status flag */
  int exit_status;

  /* The value of a for the Hamming function */
  const spre_real a = 0.50;


  /* Dispatches the execution for swin_win_raised_cosine */
  exit_status = swin_win_raised_cosine (a, i, N, value);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swin_win_hanning: error calculating raised cosine\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}




/*
 * swin_win_zero_order_bessel_function
 *
 * Calculates the zero order modified Bessel function of the first kind.
 * The infinite sum expansion is made up to 10 decimal places.
 * The function is given by:
 *   I_0(x) = 1 + sum_x (0->10) [(x/2)^2L / (L!)^2)]
 */
static int
swin_win_zero_order_bessel_function (spre_real x, spre_real * i0)
{
  errno = 0;

  *i0 =
    1.0 + pow (x, 2) / 4.0 + pow (x, 4) / 64.0 + pow (x, 6) / 2304.0 +
    pow (x, 8) / 147456.0 + pow (x, 10) / 14745600.0;

  if (errno != 0)
    {
      fprintf (stderr, "swin_win_zero_order_bessel_function: %s\n",
               strerror (errno));
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}




/*
 * swin_win_kaiser_bessel
 *
 * Implements the Kaiser-Bessel windowing function.
 */
static int
swin_win_kaiser_bessel (const long int i, const smp_num_samples N,
                        const spre_real B, spre_real * value)
{
  /* Auxiliary variables, defined to avoid re-calculation of I_0 (B) */
  static spre_real last_B = 0.0;
  static spre_real i0_B = 0.0;

  /* Value of the numerator */
  spre_real num;

  /* Auxiliary value for the numerator */
  long double aux_num;

  /* Auxiliary function return status */
  int exit_status;



  /* Checks if i0_B needs to be re-calculated */
  if (fabs (last_B - B) > DBL_EPSILON)
    {
      /* B changed. Re-calculate i0_B */
      exit_status = swin_win_zero_order_bessel_function (B, &i0_B);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_win_kaiser_bessel: error calculating zero order bessel function\n");
          return EXIT_FAILURE;
        }
      last_B = B;
    }

  /* Calculates the numerator of the expression */
  if (labs (2 * i) >= N - 1)
    aux_num = 0.0;
  else
    aux_num =
      B * sqrt (1.0 -
                __SWIN_SQR_ ((2.0 * (spre_real) i) / ((spre_real) N - 1.0)));

  /* Evaluates the zero order bessel function of the auxiliary numerator */
  exit_status =
    swin_win_zero_order_bessel_function ((spre_real) aux_num, &num);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "swin_win_kaiser_bessel: error calculating numerator zero order bessel function\n");
      return EXIT_FAILURE;
    }

  /* Calculates the return value */
  *value = (spre_real) (num / i0_B);

  return EXIT_SUCCESS;
}




/*
 * swin_win_blackman
 *
 * Implements the Blackman windowing function.
 */
static int
swin_win_blackman (const long int i, const smp_num_samples N,
                   spre_real * value)
{
  *value =
    (spre_real) 0.42 +
    0.5 * cos ((2.0 * PI * (spre_real) i) / ((spre_real) N)) +
    0.08 * cos ((4.0 * PI * (spre_real) i) / ((spre_real) N));

  return EXIT_SUCCESS;
}




/*
 * swin_windowed_value
 *
 * Calculates the windowed value of the given value.
 */
int
swin_windowed_value (const cmp_complex in_value,
                     const smp_num_samples pos,
                     const smp_num_samples samples,
                     const swin_window_type window_type,
                     const smp_entries_type data_type,
                     const spre_real trg_central,
                     const spre_real trg_low,
                     const spre_real trg_high,
                     const spre_real trg_central_value,
                     const spre_real B_kaiser, cmp_complex * out_value)
{
  /* Correct position of the sample for the windowing function */
  long int aux_pos;

  /* Auxiliary function exit status flag */
  int exit_status;


  /* If the index is out of the range of the sample, returns 0 */
  if (pos < 1 || pos > samples)
    {
      (*out_value).re = (spre_real) 0.0;
      (*out_value).im = (spre_real) 0.0;
      return EXIT_SUCCESS;
    }

  /* Calculates the correct sample index */
  aux_pos = pos - (smp_num_samples) (samples / 2);

  /* Dispatches for the correct windowing function */
  switch (window_type)
    {
    case SWIN_RECTANGULAR:
      exit_status = swin_win_rectangular (aux_pos, samples, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Rectangular windowing function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status =
            swin_win_rectangular (aux_pos, samples, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Rectangular windowing function\n");
              return EXIT_FAILURE;
            }
        }
      break;


    case SWIN_BARTLETT:
      exit_status = swin_win_bartlett (aux_pos, samples, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Bartlett windowing function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status =
            swin_win_bartlett (aux_pos, samples, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Bartlett windowing function\n");
              return EXIT_FAILURE;
            }
        }
      break;


    case SWIN_TRIANGULAR:
      exit_status =
        swin_win_triangular (aux_pos, trg_central, trg_low, trg_high,
                             trg_central_value, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Triangular windowing function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status =
            swin_win_triangular (aux_pos, trg_central, trg_low, trg_high,
                                 trg_central_value, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Triangular windowing function\n");
              return EXIT_FAILURE;
            }
        }
      break;


    case SWIN_KAISER:
      exit_status =
        swin_win_kaiser_bessel (aux_pos, samples, B_kaiser, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Kaiser-Bessel function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status =
            swin_win_kaiser_bessel (aux_pos, samples, B_kaiser,
                                    &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Kaiser-Bessel function\n");
              return EXIT_FAILURE;
            }
        }
      break;


    case SWIN_HAMMING:
      exit_status = swin_win_hamming (aux_pos, samples, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Hamming function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status = swin_win_hamming (aux_pos, samples, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Hamming function\n");
              return EXIT_FAILURE;
            }
        }
      break;

    case SWIN_HANNING:
      exit_status = swin_win_hanning (aux_pos, samples, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Hanning function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status = swin_win_hanning (aux_pos, samples, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Hanning function\n");
              return EXIT_FAILURE;
            }
        }
      break;

    case SWIN_BLACKMAN:
      exit_status = swin_win_blackman (aux_pos, samples, &(*out_value).re);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_windowed_value: error on Blackman function\n");
          return EXIT_FAILURE;
        }
      if (data_type == SMP_COMPLEX)
        {
          exit_status =
            swin_win_blackman (aux_pos, samples, &(*out_value).im);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_windowed_value: error on Blackman function\n");
              return EXIT_FAILURE;
            }
        }
      break;

    default:
      (*out_value).re = 1.0;
      (*out_value).im = 1.0;
      break;
    }

  /* Multiplies the input value by the window function value */
  (*out_value).re *= in_value.re;
  if (data_type == SMP_COMPLEX)
    (*out_value).im *= in_value.im;

  return EXIT_SUCCESS;
}



/*
 * swin_window_list
 *
 * Generates a windowed frame from the input list and appends it to the given
 * output index
 *
 * Parameters:
 * - source_list: index entry containing the source list
 * - frame_number: index of the generated frame
 * - width: window width
 * - samples_between_frames: number of samples between consecutive frames
 * - frame_data_type: data type (real/complex) of the generated frames
 * - windowing_data_type: data type (real/complex) for windowing operations
 * - window_type: one of the possible windowing functions
 * - trg_central: triangle peak value for triangular window
 * - trg_low: low value for triangular window
 * - trg_high: high value for triangular window
 * - trg_central_value: value of the window at 'trg_central'
 * - B_kaiser: B parameter for the Kaiser-Bessel windowing function
 * - out_index: output index to which the generated frame will be appended
 */
int
swin_window_list (const index_entry_type source_list,
                  const swin_number_type frame_number,
                  const swin_width_type width,
                  const smp_num_samples samples_between_frames,
                  const smp_entries_type frame_data_type,
                  const smp_entries_type windowing_data_type,
                  const swin_window_type window_type,
                  const spre_real trg_central,
                  const spre_real trg_low,
                  const spre_real trg_high,
                  const spre_real trg_central_value,
                  const spre_real B_kaiser, index_list_type * out_index)
{
  smp_num_samples samples;                  /* number of samples of the source list */
  FileName frame_list_name;                 /* frames list file name */
  FileName frame_file_name;                 /* frame file name */
  smp_num_samples read_pos, real_read_pos;  /* source list read position */
  smp_num_samples frame_pos;                /* current source list frame position */
  smp_index_pos aux_pos;                    /* new list insert position */
  cmp_complex value;                        /* auxiliary sample value */
  cmp_complex windowed_value;               /* auxiliary windowed sample value */
  cmp_real inc_time;                        /* time scale parameters */
  int exit_status;                          /* auxiliary function return status */


  /* Gets the number of samples */
  samples = source_list->list->samples;
  if (width > samples)
    {
      fprintf (stderr,
               "swin_window_list: frame width (%lu) is larger than the number of samples (%ld)\n",
               width, samples);
      return EXIT_FAILURE;
    }

  /* Determines the list and file names */
  sprintf (frame_list_name, "frame [%s,%lu]", source_list->name, frame_number);
  sprintf (frame_file_name, "%s.%lu", source_list->file, frame_number);

  /* Time scale parameters */
  inc_time = source_list->list->inc_time;

  /* Create a new list for the frame */
  exit_status =
    add_index_entry (out_index, frame_list_name, frame_file_name, NULL,
                     frame_data_type,
                     (cmp_real) frame_number * inc_time *
                     (cmp_real) samples_between_frames, inc_time,
                     frame_number, &aux_pos, SMP_SET_CURRENT);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "swin_window_list: error creating new list for frame %lu\n",
               frame_number);
      return EXIT_FAILURE;
    }

  /* Initialize the sample frame position counter */
  frame_pos = 0;

  /* Resizes the output list */
  exit_status = resize_list (&(out_index->current->list), width);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swin_window_list: error resizing list\n");
      return EXIT_FAILURE;
    }

  /* Value insertion loop */
  for (read_pos = (frame_number * samples_between_frames + 1);
       read_pos <= (frame_number * samples_between_frames + width);
       ++read_pos)
    {
      /* Increments the frame position */
      ++frame_pos;

      /*
       * Get the source list value at read position.
       * If trying to read beyond the end of the signal,
       * the initial values will be padded in a circular way.
       */
      real_read_pos = (read_pos <= samples ? read_pos : read_pos - samples);

      exit_status =
        get_list_value (*(source_list->list), real_read_pos, &value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_window_list: error reading sample from source list at position %ld\n",
                   real_read_pos);
          return EXIT_FAILURE;
        }

      /* Calculates the windowed value */
      exit_status = swin_windowed_value (value, frame_pos, width, window_type,
                                         windowing_data_type,
                                         trg_central, trg_low, trg_high,
                                         trg_central_value, B_kaiser,
                                         &windowed_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_window_list: error calculating windowed value\n");
          return EXIT_FAILURE;
        }

      /* Add the value to the frame list */
      exit_status =
        set_list_value (out_index->current->list, frame_pos, windowed_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swin_window_list: error setting value in frame %lu\n",
                   frame_number);
          return EXIT_FAILURE;
        }
    }                           /* value insertion loop */

  return EXIT_SUCCESS;
}





/*
 * swin_window
 *
 * Creates a new index of sample lists that are the frames of the original list
 * transformed with a windowing function
 *
 * Optionally, applies the preemphasis to the signal
 * (enhances the amplitude of high frequency formants)
 *
 * - index: index of sample lists
 * - preemphasis: do the pre-emphasis of the original signal
 * - alpha_preemphasis: alpha coefficient for the pre-emphasis filter
 * - width: window width
 * - superposing_samples: number of samples to be shared between adjacent frames * - window_type: one of the possible windowing functions
 * - frame_data_type: data type (real/complex) of the generated frames
 * - windowing_data_type: data type (real/complex) for windowing operation
 * - trg_central: triangle peak value for triangular window
 * - trg_low: low value for triangular window
 * - trg_high: high value for triangular window
 * - trg_central_value: value of the window at 'trg_central'
 * - B_kaiser: B parameter for the Kaiser-Bessel windowing function
 * - purge_zero_power: indicates that zero-powered frames should be purged
 * - new_index: the new index with the windowed frames of the original list
 */
int
swin_window (index_list_type * index,
             const swin_yes_no preemphasis,
             const spre_real alpha_preemphasis,
             const swin_width_type width,
             const swin_width_type superposing_samples,
             const smp_entries_type frame_data_type,
             const smp_entries_type windowing_data_type,
             const swin_window_type window_type,
             const spre_real trg_central,
             const spre_real trg_low,
             const spre_real trg_high,
             const spre_real trg_central_value,
             const spre_real B_kaiser,
             const swin_purge_type purge_zero_power,
             index_list_type * new_index)
{
  index_entry_type source_list; /* pointer to the original list */
  smp_num_samples samples;      /* number of samples of the source list */
  smp_num_samples samples_between_frames;       /* samples between frames */
  ldiv_t aux_long_div;          /* auxiliary long integer division structure */
  swin_number_type frames;      /* number of frames to be generated */
  swin_number_type current_frame;       /* current frame counter */
  swin_number_type current_valid_frame; /* current valid frame counter */
  int exit_status;              /* auxiliary function return status */



  /* Makes 'list' point to the current list */
  source_list = index->current;

  /* Gets the number of samples */
  samples = source_list->list->samples;
  if (width > samples)
    {
      fprintf (stderr,
               "swin_window: frame width (%lu) is larger than the number of samples (%ld)\n",
               width, samples);
      return EXIT_FAILURE;
    }

  /* Number of samples between frames */
  if (superposing_samples < 0)
    {
      fprintf (stderr,
               "swin_window: negative superposing samples requested\n");
      return EXIT_FAILURE;
    }
  if (superposing_samples >= width)
    {
      fprintf (stderr,
               "swin_window: number of superposing must be less than frame width\n");
      return EXIT_FAILURE;
    }

  /* Calculates the number of frames that will be generated */
  samples_between_frames = width - superposing_samples;
  aux_long_div = ldiv (samples, samples_between_frames);
  frames = aux_long_div.quot;

  /*
   * If the reminder of the division is not zero, one aditional incomplete frame
   * will be generated
   */
  if (aux_long_div.rem > 0)
    ++frames;

  /* Creates the new index, referencing the source index as parent */
  exit_status = create_index (new_index, index);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swin_window: error creating index of frames lists\n");
      return EXIT_FAILURE;
    }

  /* If requested, executes the signal pre-emphasis */
  if (preemphasis == SWIN_YES)
    {
      exit_status =
        spre_list_preemphasis (source_list->list, alpha_preemphasis);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "swin_window: error executing pre-emphasis\n");
          return EXIT_FAILURE;
        }
    }

  /* Frame generation loop */
  current_valid_frame = 0;
  for (current_frame = 0; current_frame < frames; ++current_frame)
    {
      /*
       * Creates the new frame and applies the windowing function to its
       * elements
       */
      exit_status =
        swin_window_list (source_list,
                          current_frame,
                          width, samples_between_frames,
                          frame_data_type, windowing_data_type,
                          window_type,
                          trg_central, trg_low, trg_high, trg_central_value,
                          B_kaiser, new_index);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "swin_window: error generating frame %lu\n",
                   current_frame);
          return EXIT_FAILURE;
        }

      /* Zero-powered frame purging */
      if (purge_zero_power == SWIN_PURGE_ZERO_POWER)
        {
          /* Calculates the list's statistic parameters */
          exit_status =
            calculate_list_statistics (&(new_index->current->list));
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "swin_window: error calculating frame's statistics\n");
              return EXIT_FAILURE;
            }

          /* Verifies the zero-power condition */
          if (new_index->current->list->max.re < DBL_EPSILON &&
              new_index->current->list->min.re > -DBL_EPSILON &&
              new_index->current->list->max.im < DBL_EPSILON &&
              new_index->current->list->min.im > -DBL_EPSILON)
            {
              /* Discards the current frame */
              exit_status = destroy_index_entry (new_index);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "swin_window: error removing zero-powered frame\n");
                  return EXIT_FAILURE;
                }
              --current_valid_frame;
            }
        }                       /* zero-powered frame purging */
      ++current_valid_frame;
    }                           /* frame generation loop */

  return EXIT_SUCCESS;
}
