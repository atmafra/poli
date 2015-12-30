#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "s_samples.h"
#include "s_complex.h"
#include "s_smptypes.h"
#include "s_fft.h"
#include "../incstat/incstat.h"

/*
 * sfft_sup_power
 *
 * Returns the minimum power of 'basis', and its corresponding exponent,
 * that is bigger than 'x'.
 */
int
sfft_sup_power (const smp_num_samples basis, const smp_num_samples x,
                smp_num_samples * power, smp_num_samples * exponent)
{
  smp_num_samples aux_power;    /* Auxiliary power */
  smp_num_samples aux_exponent; /* Auxiliary exponent */


  /* Checks the validity of 'basis' */
  if (basis <= 0)
    {
      fprintf (stderr, "sfft_sup_power: %ld is an invalid basis\n", basis);
      return EXIT_FAILURE;
    }

  /* Checks the validity of 'x' */
  /*
  if (x < 0)
    {
      fprintf (stderr, "sfft_sup_power: %ld is an invalid sample index\n", x);
      return EXIT_FAILURE;
    }
   */

  /* Calculates the supreme of 'x' in the power series of 'basis' */
  aux_power = 1;
  aux_exponent = 0;

  while (aux_power < x)
    {
      aux_power *= basis;
      aux_exponent++;
    }

  /* Sets the return variables */
  *power = aux_power;
  *exponent = aux_exponent;

  return EXIT_SUCCESS;
}



/*
 * sfft_bit_vector_to_int
 *
 * Converts a N-bit-vector to the corresponding integer.
 * Internal function, serving sfft_bit_reverse.
 */
void
sfft_bit_vector_to_int (const smp_bit_vector bit_vector,
                        const smp_num_samples N,
                        smp_num_samples * converted_integer)
{
  smp_num_samples aux_converted = 0;    /* Auxiliary return value */
  smp_num_samples aux_exp, aux_power = 1;       /* Auxiliary counters */


  /* Conversion loop */
  for (aux_exp = N; aux_exp > 0; aux_exp--)
    {
      /* Increments the converted index with the bit times the power */
      aux_converted += *(bit_vector + aux_exp - 1) * aux_power;

      /* Duplicates the power */
      aux_power *= 2;
    }

  /* Sets the return value */
  *converted_integer = aux_converted;
}



/*
 * sfft_next_index
 *
 * Given the current bit-vector index, the current position N_cur,
 * and N, recursively calculates the next N-bit-reversed bit vector.
 * The recursion should be started with N_cur = 0.
 * Internal function, serving sfft_bit_reverse.
 */
int
sfft_next_index (smp_bit_vector * bit_vector,
                 const smp_num_samples N_cur, const smp_num_samples N)
{
  unsigned char aux_bit;        /* auxiliary current bit */
  int exit_status;              /* auxiliary function return status */


  /* Validates N_cur */
  /*if (N_cur > N || N_cur < 0)*/
  if (N_cur > N)
    {
      fprintf (stderr, "sfft_next_index: invalid exponent\n");
      return EXIT_FAILURE;
    }

  /* Checks if bit_vector was initialized */
  if (bit_vector == NULL)
    {
      fprintf (stderr, "sfft_next_index: uninitialized bit vector\n");
      return EXIT_FAILURE;
    }

  /* Gets the bit at current position */
  aux_bit = *((*bit_vector) + N_cur);

  /* Switches the bit value */
  if (aux_bit == 0)
    aux_bit = 1;
  else
    aux_bit = 0;

  /*
   * When a 1 to 0 occurred, and not at the last position,
   * recursively call sfft_next_index for the next N_cur
   */
  if (aux_bit == 0 && N_cur < N)
    {
      exit_status = sfft_next_index (bit_vector, N_cur + 1, N);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_next_index: error during recursion\n");
          return EXIT_FAILURE;
        }
    }

  /* Updates the bit at the current position */
  *((*bit_vector) + N_cur) = aux_bit;

  return EXIT_SUCCESS;
}



/*
 * sfft_bit_reverse
 *
 * Returns the N bit-reversed sample index, required for the
 * initialization of the FFT algorithm.
 *
 * For better performance, the N-bit-reversed indexes are implemented
 * as a look-up table
 *
 * Inputs
 * - index: is the sample index
 * - N: integer positive exponent, representing the sample size as two to the
 *      N-th power
 *
 * Output
 * - reversed_index: N-bit-reversed sample index
 */
int
sfft_bit_reverse (const smp_num_samples index,
                  const smp_num_samples N, smp_num_samples * reversed_index)
{
  /* The N-bit reversed indexes vector is implemented as a static list */
  static sample_list_type *R_indexes = NULL;

  /* The last N used for the vector construction is also stored statically */
  static smp_num_samples N_internal = 0;

  /* Internal bit-vector */
  smp_bit_vector bit_vector;

  /* Auxiliary indexes */
  smp_num_samples aux_index, aux_reversed_index;

  /* Auxiliary powers */
  smp_num_samples aux_power;

  /* Auxiliary complex */
  cmp_complex aux_z;

  /* Auxiliary N */
  smp_num_samples aux_exp;

  /* Auxiliary function exit status */
  int exit_status;


  /* Checks the validity of N */
  /*
  if (N < 0)
    {
      fprintf (stderr, "sfft_bit_reverse: invalid N\n");
      *reversed_index = 0;
      return EXIT_FAILURE;
    }
   */

  /* Checks the validity of the sample index */
  /*
  if (index < 0)
    {
      fprintf (stderr, "sfft_bit_reverse: invalid sample index\n");
      *reversed_index = 0;
      return EXIT_FAILURE;
    }
   */

  /* If a different N was requested, the list needs reconstruction */
  if (N != N_internal)
    {
      /* Frees the old list */
      if (N_internal > 0)
        destroy_list (&R_indexes);

      /* Clears the statistics */
      istt_clear_stat ();

      /* If N = 0, no list should be created */
      if (N > 0)
        {
          /* Allocate memory space for the bit vector */
          bit_vector = (smp_bit_vector) malloc (N * sizeof (unsigned char));
          if (bit_vector == NULL)
            {
              fprintf (stderr,
                       "sfft_bit_reverse: error allocating bit vector\n");
              *reversed_index = 0;
              return EXIT_FAILURE;
            }

          /* Initialize the bit vector with 0's */
          for (aux_index = 0; aux_index < N; aux_index++)
            *(bit_vector + aux_index) = 0;

          /* Initialize the power counter */
          aux_power = 1;
          for (aux_exp = 0; aux_exp < N; aux_exp++)
            aux_power *= 2;

          /* Checks the consistency between 'N' and 'index' */
          if (index > (2 * aux_power) - 1)
            {
              fprintf (stderr,
                       "sfft_bit_reverse: the index %ld can't be %ld-bit-reversed\n",
                       index, N);
              *reversed_index = 0;
              return EXIT_FAILURE;
            }

          /* Create the new look-up table N-bit-reversed indexes */
          exit_status = create_list (&R_indexes, SMP_REAL, 0.0, 0.0, 0);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_bit_reverse: error creating %ld-bit-reversed look-up table\n",
                       N);
              *reversed_index = 0;
              return EXIT_FAILURE;
            }

          /* Allocates memory for the list */
          exit_status = resize_list (&R_indexes, aux_power);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_bit_reverse: error allocating memory space\n");
              return EXIT_FAILURE;
            }

          /* Look-up Table construction loop */
          for (aux_index = 0; aux_index < aux_power; aux_index++)
            {
              /* Converts the current bit vector into an integer */
              sfft_bit_vector_to_int (bit_vector, N, &aux_reversed_index);

              /* Defines the artificial complex value */
              aux_z.re = aux_reversed_index;
              aux_z.im = 0.0;

              /* Sets the value in the look-up table */
              exit_status = set_list_value (R_indexes, aux_index + 1, aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_bit_reverse: error adding %ld-th value\n",
                           aux_index);
                  return EXIT_FAILURE;
                }

              /* Increments the current bit vector */
              exit_status = sfft_next_index (&bit_vector, 0, N);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_bit_reverse: error incrementing bit vector\n");
                  return EXIT_FAILURE;
                }
            }                   /* Look-up table construction */

        }                       /* End of the list creation */

      /* Update internal N value */
      N_internal = N;

      /* Clear the statistics */
      istt_clear_stat ();

    }                           /* N != N_internal */

  /* Get the return value from the look-up table */
  if (N > 0)
    {
      exit_status = get_list_value (*R_indexes, index + 1, &aux_z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_bit_reverse: error retrieving reversed index\n");
          return EXIT_FAILURE;
        }
    }
  else
    aux_z.re = 0.0;

  /* Set the return value */
  *reversed_index = (smp_num_samples) aux_z.re;

  return EXIT_SUCCESS;
}



/*
 * sfft_W
 *
 * Returns the FFT's twiddle factors W.
 *
 * A twiddle factor is defined as
 * W(k,N) = exp (-j*2*PI*k/N) = cos (2*PI*k/N) - j * sin (2*PI*k/N)
 * Note that the twiddle factors are N-periodic. So W(k+N,N) = W(k,N).
 *
 * Input values
 * - k: discrete frequency value
 * - N: the number of samples
 *
 * Output value
 * - W: the complex twiddle factor W at frequency k
 */
int
sfft_W (const smp_num_samples k, const smp_num_samples N, cmp_complex * W)
{
  /* The twiddle factors vector is implemented as a static list */
  static sample_list_type *W_factors = NULL;

  /* The last N used for the vector construction is also stored statically */
  static smp_num_samples N_internal = 0;

  /* Auxiliary internal exponent of N */
  smp_num_samples Nexp;

  /* Auxiliary function return status */
  int exit_status;

  /* Auxiliary list creation counter */
  smp_num_samples aux_k;

  /* Auxiliary N-transposed index */
  smp_num_samples k_transp;

  /* Auxiliary list creation complex value */
  cmp_complex aux_W, return_W;



  /* Decide what to do, depending on the requested N */
  /*
  if (N < 0)
    {
      fprintf (stderr, "sfft_W: %ld is not a valid value for N\n", N);
      return EXIT_FAILURE;
    }
   */

  if (N > N_internal)
    {
      /* The vector needs reconstruction */
      exit_status = destroy_list (&W_factors);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_W: error destroying old W-factors list\n");
          return EXIT_FAILURE;
        }

      /* Clear the statistics */
      istt_clear_stat ();

      /* If N = 0, no list should be created */
      if (N > 0)
        {
          /* Creates the new list */
          exit_status = create_list (&W_factors, SMP_COMPLEX, 0.0, 0.0, 0);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_W: error creating twiddle factors list\n");
              return EXIT_FAILURE;
            }

          /* Calculates the number of bits of N */
          Nexp = 0;
          for (aux_k = N; aux_k > 1; aux_k /= 2)
            Nexp++;

          /* Resizes the list */
          exit_status = resize_list (&W_factors, N);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr, "sfft_W: error allocating memory space\n");
              return EXIT_FAILURE;
            }

          /* Loop for list value setting */
          for (aux_k = 0; aux_k < N; aux_k++)
            {
              /* Define the real and complex parts of the factor */
              aux_W.re = (cmp_real) cos (2 * PI * aux_k / N);
              aux_W.im = (cmp_real) - sin (2 * PI * aux_k / N);

              /* Add the current value to the list */
              exit_status = set_list_value (W_factors, aux_k + 1, aux_W);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr, "sfft_W: error setting %ld-th factor\n",
                           aux_k);
                  return EXIT_FAILURE;
                }
            }
        }

      /* Update the internal value of N */
      N_internal = N;

      /* Clear the statistics */
      istt_clear_stat ();
    }


  /* Get the list value at the transposed position */
  if (N > 0)
    {
      k_transp = (k * (N_internal / N)) % N_internal;

      exit_status = get_list_value (*W_factors, k_transp + 1, &return_W);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_W: error retrieving twiddle factor\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      return_W.re = 0.0;
      return_W.im = 0.0;
    }

  /* Set the return value */
  *W = return_W;

  return EXIT_SUCCESS;
}


/*
 * sfft_exec
 *
 * Executes the FFT of the signal on the current list of the given
 * 'in_index'
 * The returning list is the list at position 'out_pos'
 * of the index 'out_index'
 * If the flag 'in_place' is set to yes, then the in-place calculation
 * will be done:
 *   - 'out_index' will return in_index';
 *   - 'out_pos' will return 'in_pos'
 * The flag 'inverse' makes the inverse FFT to be calculated: the coefficients
 * will be rescaled by a 1/N factor
 * If 'domain' is set to 'SMP_REAL', then the real FFT will be computed;
 * otherwise ('SMP_COMPLEX') the complex transform will be computed.
 */
int
sfft_exec (index_list_type * in_index,
           index_list_type * out_index,
           smp_index_pos * out_pos,
           const sfft_place_type place,
           const sfft_direction_type direction, const sfft_domain_type domain)
{
  /* Auxiliary function return status */
  int exit_status;

  /* Position of the current index entry */
  smp_index_pos in_pos;

  /* Position of the index entry containing the FFT */
  smp_index_pos fft_pos;

  /* List name and file name for the FFT list */
  Name fft_list_name = "";
  FileName fft_file_name = "";

  /* Auxiliary number of samples of the signal list, before zero padding */
  smp_num_samples signal_samples;

  /* Zero padding counter */
  smp_num_samples zero_counter;

  /* Auxiliary samples counter */
  smp_num_samples aux_index, aux_reversed_index;

  /* Auxiliary pointers to the signal and the FFT lists */
  sample_list_type *signal_list, *fft_list;

  /* Auxiliary N values */
  smp_num_samples N, N_exponent;

  /* Time and frequency increments */
  cmp_real inc_time = 0.0, inc_freq = 0.0;

  /* FFT file and signal extensions */
  char fft_extension[10], file_extension[10];

  /* Auxiliary complex values */
  cmp_complex aux_z, aux_z2;


  /*
   * FFT Control Variables
   */

  /* Current pass */
  smp_num_samples cur_pass;

  /* Number of blocks and current block */
  smp_num_samples blocks, cur_block;

  /* Number of butterflies and current butterfly */
  smp_num_samples bflies, cur_bfly;

  /* Number of samples in the current block */
  smp_num_samples points;

  /* Top block and bottom block first sample indexes */
  smp_num_samples top_block_start, bot_block_start;

  /* Top and bottom butterfly results */
  cmp_complex top_bfly_value, bot_bfly_value;

  /* Auxiliary twiddle factor */
  cmp_complex aux_W;



  /* Gets the current list position */
  in_pos = in_index->current->position;

  /* Stores the list address at the pointer to the signal list */
  signal_list = in_index->current->list;

  /* Defines the signal and the file extensions */
  if (direction == SFFT_DIRECT)
    {
      strcpy (fft_extension, ", DFT");
      strcpy (file_extension, ".dft");
    }
  else
    {
      strcpy (fft_extension, ", IDFT");
      strcpy (file_extension, ".idft");
    }

  /* Defines the name and the file of the new FFT list */
  strcpy (fft_list_name, in_index->current->name);
  strcat (fft_list_name, fft_extension);
  strcpy (fft_file_name, in_index->current->file);
  strcat (fft_file_name, file_extension);

  /* According to 'place', creates or not a new list */
  switch (place)
    {
    case SFFT_OFF_PLACE:
      /*
       * Off-place calculation selected.
       * Creates a new list in the out index to contain the FFT.
       */
      exit_status =
        add_index_entry (out_index, fft_list_name, fft_file_name,
                         NULL, SMP_COMPLEX, 0.0, 0.0, 0, &fft_pos,
                         SMP_SET_CURRENT);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec: error creating FFT list\n");
          return EXIT_FAILURE;
        }
      break;

    case SFFT_IN_PLACE:
      /* In-place calculation for a real valued list is impossible */
      if (signal_list->data_type == SMP_REAL)
        {
          fprintf (stderr,
                   "sfft_exec: in-place calculation of a real valued list requested\n");
          return EXIT_FAILURE;
        }

      /*
       * In-place calculation selected.
       * Use the same index and position for the FFT.
       */
      *out_index = *in_index;
      fft_pos = in_pos;

      /* Updates the list name and the file name */
      strcpy (in_index->current->name, fft_list_name);
      strcpy (in_index->current->file, fft_file_name);
      break;

    default:
      fprintf (stderr,
               "sfft_exec: invalid in/off placing computation option\n");
      return EXIT_FAILURE;
    }


  /* Stores the FFT list address at the pointer to the FFT list */
  fft_list = out_index->current->list;



  /***************************************************************************
   *                                                                         *
   *                               FFT Algorithm                             *
   *                                                                         *
   ***************************************************************************/

  /*
   * 1. Determine the number of samples supreme value
   *    from the power series of 2.
   *    Define the frequency increment.
   */
  exit_status = sfft_sup_power (2, signal_list->samples, &N, &N_exponent);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "sfft_exec: error determining N\n");
      return EXIT_FAILURE;
    }

  /* Calculates the frequency increment */
  inc_time = signal_list->inc_time;
  if (inc_time > DBL_EPSILON)
    inc_freq = 1.0 / (inc_time * (cmp_real) N);
  else
    inc_freq = 0.0;

  /* Set the frequency increment */
  fft_list->ini_time = signal_list->ini_time;
  fft_list->inc_time = inc_freq;




  /*
   * 2. Allocate necessary extra memory space
   */

  /*
   * If in-place computation was selected and the list is smaller than
   * 'N', add zeroes to the list until it gets to 'N' samples
   */
  if ((place == SFFT_IN_PLACE && signal_list->samples < N) ||
      place == SFFT_OFF_PLACE)
    {
      /* Define the zeroed element */
      aux_z.re = 0.0;
      aux_z.im = 0.0;

      /* Stores the number of samples of the signal list */
      signal_samples = signal_list->samples;

      /* Resizes the list to the desired number of samples */
      exit_status = resize_list (&fft_list, N);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec: error resizing list to %ld elements\n",
                   N);
          return EXIT_FAILURE;
        }

      /* Sets the zeroed elements */
      for (zero_counter = (place == SFFT_IN_PLACE ? signal_samples + 1 : 1);
           zero_counter <= N; zero_counter++)
        {
          exit_status = set_list_value (fft_list, zero_counter, aux_z);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_exec: error setting zeroes on the FFT list\n");
              return EXIT_FAILURE;
            }
        }
    }



  /*
   * 3. Copy the sample list values in N-bit-reversed order to the FFT list
   *    Consider if in-place computation was selected
   */

  /* Bit reversion loop */
  for (aux_index = 0; aux_index < N; aux_index++)
    {
      /* Get the N-bit-reversed index */
      exit_status =
        sfft_bit_reverse (aux_index, N_exponent, &aux_reversed_index);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec: error reverting bits of index %ld\n",
                   aux_index);
          return EXIT_FAILURE;
        }

      /* Switches the values if necessary */
      if ((aux_index < aux_reversed_index && place == SFFT_IN_PLACE) ||
          place == SFFT_OFF_PLACE)
        {
          /* Get the signal list value at the reversed index */
          if (aux_reversed_index < signal_list->samples)
            {
              exit_status =
                get_list_value (*signal_list, aux_reversed_index + 1, &aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error getting signal value at %ld\n",
                           aux_reversed_index + 1);
                  return EXIT_FAILURE;
                }
            }
          else
            {
              aux_z.re = 0;
              aux_z.im = 0;
            }

          /* Get the signal list value at the direct index */
          if (aux_index < signal_list->samples)
            {
              exit_status =
                get_list_value (*signal_list, aux_index + 1, &aux_z2);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error getting signal value at %ld\n",
                           aux_index);
                  return EXIT_FAILURE;
                }
            }
          else
            {
              aux_z2.re = 0;
              aux_z2.im = 0;
            }

          /* Switches the values at the list */
          exit_status = set_list_value (fft_list, aux_index + 1, aux_z);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_exec: error setting the value at position %ld of the FFT list\n",
                       aux_index + 1);
              return EXIT_FAILURE;
            }

          exit_status =
            set_list_value (fft_list, aux_reversed_index + 1, aux_z2);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "sfft_exec: error setting the value at position %ld of the FFT list\n",
                       aux_index + 1);
              return EXIT_FAILURE;
            }
        }
    }



  /*
   * 4. Create the twiddle vector list for the number of samples 'N'
   */

  exit_status = sfft_W (0, N, &aux_W);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "sfft_exec: error creating twiddle vector\n");
      return EXIT_FAILURE;
    }



  /*
   * 5. Perform the Fast Fourier Transform on the FFT list
   */

  /* First pass initialization */
  blocks = N / 2;
  points = 2;

  /* Passes loop */
  for (cur_pass = 0; cur_pass < N_exponent; cur_pass++)

    {
      /* The number of butterflies is half the number of points */
      bflies = points / 2;

      /* Reset top block first sample index to zero */
      top_block_start = 0;

      /* The bottom block starts at the middle of the list */
      bot_block_start = bflies; /* bflies is points/2 */

      /* Blocks loop */
      for (cur_block = 0; cur_block < blocks; cur_block++)
        {
          /* Butterflies loop */
          for (cur_bfly = 0; cur_bfly < bflies; cur_bfly++)
            {
              /*
               * Butterfly in-place computation
               * Samples must be in N-bit-reversed order
               */

              /* Calculate top butterfly value */
              exit_status =
                get_list_value (*fft_list, top_block_start + cur_bfly + 1,
                                &top_bfly_value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error getting top bfly list value\n");
                  return EXIT_FAILURE;
                }

              /* Calculate bottom butterfly value */
              exit_status =
                get_list_value (*fft_list, bot_block_start + cur_bfly + 1,
                                &bot_bfly_value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error getting bottom bfly list value\n");
                  return EXIT_FAILURE;
                }

              /* Get the twiddle factor */
              exit_status = sfft_W (cur_bfly, points, &aux_W);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error getting twiddle factor\n");
                  return EXIT_FAILURE;
                }

              /* Bottom bfly value is multiplicated by the twiddle factor */
              exit_status =
                cmp_fcn_exec (cmp_tms, &bot_bfly_value, CMP_FCN_COMPLEX, 0,
                              (va_list *) NULL, 2, bot_bfly_value, aux_W);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error multiplying complexes\n");
                  return EXIT_FAILURE;
                }

              /* New top value */
              exit_status =
                cmp_fcn_exec (cmp_sum, &aux_z, CMP_FCN_COMPLEX,
                              0, (va_list *) NULL, 2,
                              top_bfly_value, bot_bfly_value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr, "sfft_exec: error adding complexes\n");
                  return EXIT_FAILURE;
                }

              exit_status =
                set_list_value (fft_list, top_block_start + cur_bfly + 1,
                                aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error setting top bfly value\n");
                  return EXIT_FAILURE;
                }

              /* New bottom value */
              exit_status =
                cmp_fcn_exec (cmp_sbt, &aux_z, CMP_FCN_COMPLEX, 0,
                              (va_list *) NULL, 2, top_bfly_value,
                              bot_bfly_value);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error subtracting complexes\n");
                  return EXIT_FAILURE;
                }

              exit_status =
                set_list_value (fft_list, bot_block_start + cur_bfly + 1,
                                aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "sfft_exec: error setting bottom bfly value\n");
                  return EXIT_FAILURE;
                }
            }                   /* Butterfly loop */

          /* Update top and bottom first sample indexes */
          top_block_start += points;
          bot_block_start = top_block_start + bflies;
        }                       /* Block loop */

      /* The number of blocks for the next pass halves */
      blocks /= 2;

      /* The number of samples per block doubles */
      points *= 2;
    }                           /* Pass loop */



  /*
   * 6. Dumps the second half of the list if the real transform was requested
   */
  switch (domain)
    {
    case SFFT_REAL:
      /* Real transform requested: dump the second half of the list */
      exit_status = resize_list (&fft_list, N / 2);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec: error resizing FFT list\n");
          return EXIT_FAILURE;
        }
      break;


    case SFFT_COMPLEX:
      /* Complex transform requested: nothing to be done */
      break;


    default:
      /* Undefined domain */
      fprintf (stderr, "sfft_exec: unknown FFT input domain\n");
      return EXIT_FAILURE;
    }



  /*
   * 7. Only for inverse FFT: rescale the FFT values by the 1/N factor
   */
  if (direction == SFFT_INVERSE)
    {
      exit_status =
        apply_list_function (&fft_list, cmp_rpd, CMP_FCN_COMPLEX, 1,
                             1.0 / (cmp_real) N);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec: error rescaling FFT list by 1/N\n");
          return EXIT_FAILURE;
        }
    }

  /* Set the return value */
  *out_pos = fft_pos;

  return EXIT_SUCCESS;
}




/*
 * sfft_dct_extend_list
 *
 * Extends the N-point input list to an even simetric 2N-point list, according
 * to the DCT-2 extension rule
 */
static int
sfft_dct_extend_list (sample_list_type * in_list)
{
  /* Number of samples of the input list */
  smp_num_samples N = 0;

  /* Auxiliary input value */
  cmp_complex in_value;

  /* Current sample counter */
  smp_num_samples cur_sample;

  /* Auxiliary function return status */
  int exit_status;



  /* Initializes the number of samples */
  N = in_list->samples;

  /* Resizes the list to 2N */

  /* Trivial case */
  if (N == 0)
    return EXIT_SUCCESS;

  /* Non-trivial case */
  exit_status = resize_list (&in_list, 2 * N);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "sfft_dct_extend_list: error doubling size of input list \n");
      return EXIT_FAILURE;
    }

  /* Value insertion loop */
  for (cur_sample = 1; cur_sample <= N; cur_sample++)
    {
      /* Gets the original value */
      exit_status = get_list_value (*in_list, cur_sample, &in_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_dct_extend_list: error getting input value at position %ld\n",
                   cur_sample);
          return EXIT_FAILURE;
        }

      /* Sets the extended value */
      exit_status =
        set_list_value (in_list, 2 * N - cur_sample + 1, in_value);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_dct_extend_list: error setting extended value at position %ld\n",
                   2 * N - cur_sample + 1);
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}




/*
 * sfft_dct_coefficient
 *
 * Returns the DCT-2 coefficient for the conversion of the real part of the FFT
 * of the DCT-2 extended list
 *
 * The coefficients are given by the formula
 *   alpha[k] / 2 * cos (PI/N * k/2), where
 *     N is the number of samples in the list
 *     k is the sample index, k = 0, 1, ..., N - 1
 *     alpha[k] = sqrt (1/N), if k = 0
 *                sqrt (2/N), otherwise
 */
static int
sfft_dct_coefficient (const smp_num_samples N, const smp_num_samples k,
                      cmp_real * dct_coeff)
{
  /* Auxiliary alpha */
  cmp_real alpha;


  /* Validates the domain of the inputs */
  if (N <= 0)
    {
      fprintf (stderr,
               "sfft_dct_coefficients: invalid number of samples N = %ld\n",
               N);
      return EXIT_FAILURE;
    }

  /*if (k < 0 || k > N - 1)*/
  if (k > N - 1)
    {
      fprintf (stderr,
               "sfft_dct_coefficients: invalid sample index requested %ld\n",
               k);
      return EXIT_FAILURE;
    }

  /* Determines the value of alpha for the given input */
  if (k == 0)
    alpha = (cmp_real) sqrt (1.0 / (cmp_real) N);
  else
    alpha = (cmp_real) sqrt (2.0 / (cmp_real) N);

  /* Calculates the DCT-2 coefficient */
  *dct_coeff =
    alpha / (2.0 * cos ((PI / (cmp_real) N) * ((cmp_real) k / 2.0)));

  return EXIT_SUCCESS;
}




/*
 * sfft_dct_exec
 *
 * Executes the FFT of the signal at the current list of the input index
 * 'in_index'
 * The returning list is the list at position 'out_pos'
 * of the index 'out_index'
 * If the flag 'in_place' is set to yes, then the in-place calculation
 * will be done:
 *   - 'out_index' will return in_index';
 *   - 'out_pos' will return 'in_pos'
 * The flag 'inverse' makes the inverse FFT to be calculated: the coefficients
 * will be rescaled by a 1/N factor
 */
int
sfft_dct_exec (index_list_type * in_index,
               index_list_type * out_index, smp_index_pos * out_pos,
               const sfft_place_type place,
               const sfft_direction_type direction)
{
  /* Auxiliary input list */
  sample_list_type *signal_list;

  /* Input index entry position */
  /*smp_index_pos in_pos;*/

  /* Current sample counter */
  smp_num_samples cur_sample;

  /* Auxiliary input/output value */
  cmp_complex aux_z;

  /* Auxiliary DCT-2 coefficient */
  cmp_real aux_dct_coeff;

  /* Auxiliary function return status */
  int exit_status;




  /* Gets the current list position */
  /*in_pos = in_index->current->position;*/

  /* Stores the list address at the pointer to the signal list */
  signal_list = in_index->current->list;

  /* Extends the input list to the DCT-2 extended list */
  exit_status = sfft_dct_extend_list (signal_list);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "sfft_dct_exec: error DCT-2 extending input signal\n");
      return EXIT_FAILURE;
    }

  /* Executes the FFT of the extended list */
  exit_status =
    sfft_exec (in_index, out_index, out_pos, place, direction, SFFT_REAL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "sfft_dct_exec: error applying the FFT to the extended list\n");
      return EXIT_FAILURE;
    }

  /* Converts the complex list into a real valued list */
  exit_status = convert_complex_list_to_real (out_index->current->list);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "sfft_dct_exec: error converting complex list into real\n");
      return EXIT_FAILURE;
    }

  /* Applies the DCT-2 coefficients to the real part of the list */
  for (cur_sample = 1; cur_sample <= out_index->current->list->samples;
       cur_sample++)
    {
      /* Gets the list value at the position */
      exit_status =
        get_list_value (*(out_index->current->list), cur_sample, &aux_z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_dct_exec: error getting output list value at position %ld\n",
                   cur_sample);
          return EXIT_FAILURE;
        }

      /* Gets the DCT-2 coefficient for the current sample */
      exit_status =
        sfft_dct_coefficient (out_index->current->list->samples,
                              cur_sample - 1, &aux_dct_coeff);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_dct_exec: error getting DCT-2 coefficient for sample %ld\n",
                   cur_sample);
          return EXIT_FAILURE;
        }

      /* Multiplies the sample value by the DCT-2 coefficient */
      aux_z.re *= aux_dct_coeff;
      aux_z.im = 0.0;

      /* Stores the sample value multiplied by the DCT-2 coefficient */
      exit_status =
        set_list_value (out_index->current->list, cur_sample, aux_z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_dct_exec: error setting output list value at position %ld\n",
                   cur_sample);
          return EXIT_FAILURE;
        }
    }                           /* coefficient multiplication loop */

  return EXIT_SUCCESS;
}




/*
 * sfft_exec_index
 *
 * Executes the FFT for all the signal lists
 * in the given input index 'in_index'.
 * A new index 'out_index' is created with the transformed signals.
 */
int
sfft_exec_index (index_list_type * in_index, index_list_type * out_index,
                 const sfft_transform_type transform,
                 const sfft_place_type place,
                 const sfft_direction_type direction,
                 const sfft_domain_type domain)
{
  /* Current list counter */
  smp_num_samples cur_list;

  /* Auxiliary out list position */
  smp_index_pos out_pos;

  /* Auxiliary reversed index */
  smp_num_samples aux_reversed_index;

  /* Auxiliary W-factor */
  cmp_complex aux_z;

  /* Auxiliary function return status flag */
  int exit_status;


  /*
   * If in-place computation was not selected, then a new index should be
   * created.
   */
  if (place == SFFT_OFF_PLACE)
    {
      exit_status = create_index (out_index, in_index);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "sfft_exec_index: error creating FFT index\n");
          return EXIT_FAILURE;
        }
    }

  /* Goes to the first entry of the index */
  in_index->current = in_index->head;

  /* List transform loop */
  for (cur_list = 1; cur_list <= in_index->num_entries; cur_list++)
    {
      /* Executes the requested transform for the current list */
      switch (transform)
        {
        case SFFT_FFT:
          exit_status =
            sfft_exec (in_index, out_index, &out_pos, place, direction,
                       domain);
          break;

        case SFFT_FCT:
          exit_status =
            sfft_dct_exec (in_index, out_index, &out_pos, place, direction);
          break;

        default:
          {
            fprintf (stderr, "sfft_exec_index: invalid transform domain\n");
            return EXIT_FAILURE;
          }
        }

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "sfft_exec_index: error executing FFT of list %ld\n",
                   cur_list);
          return EXIT_FAILURE;
        }

      /* Moves to the next entry */
      if (in_index->current != in_index->tail)
        in_index->current = in_index->current->next;
    }

  /* Releases memory allocated for bit-reversion */
  exit_status = sfft_bit_reverse (0, 0, &aux_reversed_index);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "sfft_exec_index: error releasing memory for bit-reversion look-up table\n");
      return EXIT_FAILURE;
    }

  /* Releases memory allocated for bit-reversion */
  exit_status = sfft_W (0, 0, &aux_z);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "sfft_exec_index: error releasing memory for W-factors look-up table\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
