#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../strutils/strutils.h"
#include "s_samples.h"
#include "s_smptypes.h"
#include "s_fmt_wav.h"

/*
 * swav_get_magic
 *
 * Gets the magic word of the given input file
 */
static int
swav_get_magic (FILE * in_file, char **magic)
{
  /* Reallocates the given parameter to the correct size */
  (*magic) = (char *) realloc ((*magic), __SWAV_MAGIC_SIZE_);
  if ((*magic) == NULL)
    {
      fprintf (stderr, "swav_get_magic: virtual memory exhausted\n");
      return (EXIT_FAILURE);
    }


  /* RIFF WAVE magic word */
  strcpy ((*magic), getsafe (in_file, __SWAV_MAGIC_SIZE_ - 1));

  return (EXIT_SUCCESS);
}




/*
 * swav_fmt_enum_to_tag
 *
 * Returns the value corresponding to the enumerated WAVE format
 */
static size_t
swav_fmt_enum_to_tag (const swav_wave_fmt_type enum_format)
{
  switch (enum_format)
    {
    case NONE:
      return (size_t) __SWAV_WAVE_FMT_NONE_;
      break;

    case PCM:
      return (size_t) __SWAV_WAVE_FMT_PCM_;
      break;

    case MULAW:
      return (size_t) __SWAV_WAVE_FMT_IBM_MULAW_;
      break;

    case ALAW:
      return (size_t) __SWAV_WAVE_FMT_IBM_ALAW_;
      break;

    case ADPCM:
      return (size_t) __SWAV_WAVE_FMT_IBM_ADPCM_;
      break;

    default:
      fprintf (stderr, "swav_fmt_tag: undefined WAVE format\n");
      return (size_t) __SWAV_WAVE_FMT_UNKNOWN_;
      break;
    }
}




/*
 * swav_fmt_tag_to_enum
 *
 * Returns the enumerated WAVE format corresponding to the given tag
 */
static swav_wave_fmt_type
swav_fmt_tag_to_enum (const size_t tag)
{
  switch (tag)
    {
    case __SWAV_WAVE_FMT_NONE_:
      return NONE;
      break;

    case __SWAV_WAVE_FMT_PCM_:
      return PCM;
      break;

    case __SWAV_WAVE_FMT_IBM_MULAW_:
      return MULAW;
      break;

    case __SWAV_WAVE_FMT_IBM_ALAW_:
      return ALAW;
      break;

    case __SWAV_WAVE_FMT_IBM_ADPCM_:
      return ADPCM;
      break;

    default:
      fprintf (stderr, "swav_fmt_tag: undefined WAVE format\n");
      return UNKNOWN;
      break;
    }
}




/*
 * swav_fmt_enum_to_str
 *
 * Returns a string description of the WAVE format
 */
static char *
swav_fmt_enum_to_str (const swav_wave_fmt_type type)
{
  switch (type)
    {
    case NONE:
      return __SWAV_WAVE_FMT_DESC_NONE_;
      break;

    case PCM:
      return __SWAV_WAVE_FMT_DESC_PCM_;
      break;

    case MULAW:
      return __SWAV_WAVE_FMT_DESC_IBM_MULAW_;
      break;

    case ALAW:
      return __SWAV_WAVE_FMT_DESC_IBM_ALAW_;
      break;

    case ADPCM:
      return __SWAV_WAVE_FMT_DESC_IBM_ADPCM_;
      break;

    default:
      fprintf (stderr, "swav_fmt_tag: undefined WAVE format\n");
      return __SWAV_WAVE_FMT_DESC_UNKNOWN_;
      break;
    }
}




/*
 * swav_num_samples
 *
 * Gets the number of samples for the given structure
 */
size_t
swav_num_samples (const swav_type riff)
{
  if (riff.wave_c.bytes_per_sample != 0)
    return (riff.data_c.length / riff.wave_c.bytes_per_sample);
  else
    return (0);
}




/*
 * swav_samples_per_second
 *
 * Gets the number of samples per second for the given structure
 */
size_t
swav_samples_per_second (const swav_type riff)
{
  return (riff.wave_c.samples_per_second);
}




/*
 * swav_get_riff_chunk
 *
 * Allocates, reads and validates the RIFF chunk from the given file
 */
static int
swav_get_riff_chunk (FILE * in_file, swav_type * riff)
{
  /* Auxiliary function return status */
  int exit_status;


  /* Initialize the RIFF data structure */
  riff->riff = (char *) malloc (__SWAV_MAGIC_SIZE_);
  if (riff->riff == NULL)
    {
      fprintf (stderr, "swav_get_riff_chunk: virtual memory exhausted\n");
      return (EXIT_FAILURE);
    }

  /* Gets the magic word */
  exit_status = swav_get_magic (in_file, &(riff->riff));
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "swav_get_riff_chunk: error reading file magic word\n");
      return (EXIT_FAILURE);
    }

  /* RIFF chunk length */
  riff->size = getsafe_dword (in_file);

  /* Validates the RIFF magic word */
  if (strcmp (riff->riff, __SWAV_RIFF_MAGIC_) != 0)
    return (EXIT_FAILURE);

  return (EXIT_SUCCESS);
}



/*
 * swav_destruct
 *
 * Frees up memory allocated for given WAVE
 */
static int
swav_destruct (swav_type * wave)
{
  free (wave->riff);
  return (EXIT_SUCCESS);
}




/*
 * swav_init_wave
 *
 * Initializes the format chunk of the data structure
 */
static int
swav_init_wave (struct swav_wave_struct *wave)
{
  /* WAVE magic word */
  wave->wave = (char *) malloc (sizeof (__SWAV_WAVE_MAGIC_));

  /* WAVE format word */
  wave->fmt = (char *) malloc (sizeof (__SWAV_WAVE_FMT_));

  /* Checks for allocation errors */
  if (wave->wave == NULL || wave->fmt == NULL)
    {
      fprintf (stderr,
               "swav_init_wave: error allocating memory for the WAVE format chunk\n");
      return (EXIT_FAILURE);
    }

  /* Length of the WAVE format chunk */
  wave->length = 0;

  /* Format tag: undefined */
  wave->format_tag = swav_fmt_tag_to_enum (__SWAV_WAVE_FMT_NONE_);

  /* Number of channels */
  wave->channels = 0;

  /* Samples per second */
  wave->samples_per_second = 0;

  /* Average bytes per second */
  wave->average_bytes_per_second = 0;

  /* Bytes per sample */
  wave->bytes_per_sample = 0;

  /* Initialize PCM specific structure */
  wave->pcm_specific.bits_per_sample = 0;

  return (EXIT_SUCCESS);
}



/*
 * swav_init_data
 *
 * Initializes the data chunk
 */
static int
swav_init_data (struct swav_riff_data_struct *data)
{
  /* data word */
  data->data = (char *) malloc (sizeof (__SWAV_WAVE_DATA_));
  if (data->data == NULL)
    {
      fprintf (stderr,
               "swav_init_data: error allocating memory for 'data' word\n");
      return (EXIT_FAILURE);
    }

  /* Data segment length */
  data->length = 0;

  return (EXIT_SUCCESS);
}



/*
 * swav_read_pcm_specific
 *
 * Read the PCM specific parameters
 */
static int
swav_read_pcm_specific (FILE * in_file, swav_type * wave)
{
  /* Number of bits per sample */
  wave->wave_c.pcm_specific.bits_per_sample = getsafe_word (in_file);

  return (EXIT_SUCCESS);
}



/*
 * swav_read_format
 *
 * Read the WAVE format chunk
 */
static int
swav_read_format (FILE * in_file, swav_type * wave, char *subtype)
{
  /* Auxiliary function exit status */
  int exit_status;

  /* WAVE magic word */
  strcpy (wave->wave_c.wave, subtype);

  /* Reads the WAVE format word */
  strcpy (wave->wave_c.fmt, getsafe (in_file, 4));

  /* Length of the format chunk */
  wave->wave_c.length = getsafe_dword (in_file);

  /* Format tag */
  wave->wave_c.format_tag = swav_fmt_tag_to_enum (getsafe_word (in_file));

  /* Number of channels */
  wave->wave_c.channels = getsafe_word (in_file);

  /* Samples per second */
  wave->wave_c.samples_per_second = getsafe_dword (in_file);

  /* Average bytes per second */
  wave->wave_c.average_bytes_per_second = getsafe_dword (in_file);

  /* Bytes per sample (block align) */
  wave->wave_c.bytes_per_sample = getsafe_word (in_file);

  /* WAVE format specific */
  switch (wave->wave_c.format_tag)
    {
      /* PCM specific parameters */
    case (PCM):
      exit_status = swav_read_pcm_specific (in_file, wave);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swav_read: error reading PCM specific parameters\n");
          return (EXIT_FAILURE);
        }
      break;

    default:
      fprintf (stderr, "swav_read: unsupported format: %s\n",
               swav_fmt_enum_to_str (wave->wave_c.format_tag));
      return (EXIT_FAILURE);
      break;
    }

  return (EXIT_SUCCESS);
}




/*
 * swav_getsafe_pcm
 *
 * Read bytes from the input file, according to the PCM format rules
 */
static int
swav_getsafe_pcm (FILE * in_file, const size_t num_bytes)
{
  size_t i = 0;
  int buf;
  int ret_value = 0;
  int aux_pow = 1;

  do
    {
      /*
       * When reading one byte, return unsigned 8-bit;
       * otherwise, return signed
       */
      if (i < num_bytes - 1 || i == 0)
        buf = fgetc (in_file);
      else
        buf = (char) fgetc (in_file);

      /* Little endian assumption */
      if (buf != EOF)
        {
          ret_value += aux_pow * buf;
          aux_pow *= 256;
          ++i;
        }
    }
  while (i < num_bytes && buf != EOF);

  return (ret_value);
}




/*
 * swav_read_pcm
 *
 * Read samples stored in PCM format and store them on the data chunk
 */
static int
swav_read_pcm (FILE * in_file, index_entry_type * entry)
{
  /* Auxiliary wave structure */
  swav_type wave;

  /* Number of samples */
  smp_num_samples samples;

  /* Number of channels */
  size_t channels;

  /* Number of bytes */
  size_t bytes_per_sample;

  /* Number of bytes per channel */
  size_t bytes_per_channel;

  /* Number of samples per second */
  size_t samples_per_second = 0;

  /* Sampling frequency */
  cmp_real sampling_interval = 0.0;

  /* Current sample */
  size_t cur_sample;

  /* Current channel */
  size_t cur_channel;

  /* Auxiliary sample list value */
  cmp_complex aux_z;

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

  /* Auxiliary wave file sample value */
  swav_sample_type aux_wave_value;

  /* Auxiliary function exit status */
  int exit_status;



  /* Initializes the auxiliary wave structure */
  wave = (*entry)->wave;

  /* Calculate the number of samples */
  samples = swav_num_samples (wave);

  /* Gets the number of samples per second */
  samples_per_second = swav_samples_per_second (wave);

  /* Get the number of bytes per sample */
  bytes_per_sample = wave.wave_c.bytes_per_sample;

  /* Get the number of channels */
  channels = wave.wave_c.channels;

  /* Number of bytes per channel */
  bytes_per_channel = bytes_per_sample / channels;

  /* Calculates the sampling frequency */
  if (samples_per_second == 0)
    sampling_interval = 0.0;
  else
    sampling_interval = 1.0 / (cmp_real) samples_per_second;

  /* Sets the sampling interval */
  if (samples_per_second != 0)
    {
      (*entry)->list->ini_time = 0.0;
      (*entry)->list->inc_time = sampling_interval;
    }

  /* Resizes the list to the number of samples */
  exit_status = resize_list (&((*entry)->list), samples);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_read_pcm: error resizing list to %ld elements\n",
               samples);
      return (EXIT_FAILURE);
    }

  /* Determines the range factor */
  if (bytes_per_sample == 1)
    range_factor = 128.0;
  else
    range_factor = 0.0;

  /* Samples loop */
  for (cur_sample = 1; cur_sample <= samples; cur_sample++)
    {
      aux_z.re = 0.0;
      aux_z.im = 0.0;

      /* Channels loop */
      for (cur_channel = 1; cur_channel <= channels; cur_channel++)
        {
          /* Bytes reading */
          aux_wave_value = swav_getsafe_pcm (in_file, bytes_per_channel);

          /* Attributes the sample value to the complex number */
          if (cur_channel == 1)
            aux_z.re = (cmp_real) aux_wave_value - range_factor;
          else if (cur_channel == 2
                   || (*entry)->list->data_type == SMP_COMPLEX)
            aux_z.im = (cmp_real) aux_wave_value - range_factor;
        }

      /* Adds the sample to the list */
      exit_status = set_list_value ((*entry)->list, cur_sample, aux_z);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "swav_read_pcm: error adding sample list value at position %d\n",
                   cur_sample);
          return (EXIT_FAILURE);
        }
    }

  return (EXIT_SUCCESS);
}




/*
 * swav_detect
 *
 * Tries to detect if the given file is a RIFF WAVE file
 * 'detected' returns 0 if the file is not RIFF WAVE; returns -1 otherwise 
 */
int
swav_detect (char *file_name, int *detected)
{
  /* pointer to the file */
  FILE *fp;

  /* internal RIFF */
  swav_type aux_riff;

  /* auxiliary function exit status */
  int exit_status;

  /* Tries to open the file */
  fp = fopen (file_name, "r");
  if (fp == (FILE *) NULL)
    {
      fprintf (stderr, "swav_detect: %s: '%s'\n", strerror (errno),
               file_name);
      return (EXIT_FAILURE);
    }

  /* Tries to get the RIFF chunk of the file */
  exit_status = swav_get_riff_chunk (fp, &aux_riff);
  if (exit_status != EXIT_SUCCESS)
    *detected = (int) 0;
  else
    *detected = (int) -1;

  /* Frees the memory for 'aux_riff' */
  exit_status = swav_destruct (&aux_riff);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_detect: error de-allocating auxiliary RIFF\n");
      return (EXIT_FAILURE);
    }

  /* Closes the input file */
  fclose (fp);

  return (EXIT_SUCCESS);
}




/*
 * swav_read
 *
 * Reads a RIFF WAVE audio data file
 * and stores its information into an index entry.
 *
 * Parameters
 * - in_file: file descriptor of the already opened input file
 * - entry: index entry where the file contents will be stored
 */
int
swav_read (FILE * in_file, index_entry_type * entry)
{
  /* RIFF file sub-type. ex.: WAVE */
  char subtype[sizeof (__SWAV_WAVE_MAGIC_)];

  /* Auxiliary function exit status flag */
  int exit_status;



  /* Gets the RIFF chunk */
  exit_status = swav_get_riff_chunk (in_file, &((*entry)->wave));
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_read: error getting RIFF chunk\n");
      return (EXIT_FAILURE);
    }

  /* Validates the RIFF subtype */
  strcpy (subtype, getsafe (in_file, sizeof (__SWAV_WAVE_MAGIC_) - 1));

  /* Chooses the correct sub-type */
  if (strcmp (subtype, __SWAV_WAVE_MAGIC_) != 0)
    {
      fprintf (stderr, "swav_read: unknown RIFF subtype: %s\n", subtype);
      return (EXIT_FAILURE);
    }

  /* Initialize the WAVE format chunk */
  exit_status = swav_init_wave (&((*entry)->wave.wave_c));
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_read: error initializing WAVE format chunk\n");
      return (EXIT_FAILURE);
    }

  /* Initialization of the data chunk */
  exit_status = swav_init_data (&((*entry)->wave.data_c));
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_read: error initializing data chunk\n");
      return (EXIT_FAILURE);
    }

  /* Read the WAVE format chunk */
  exit_status = swav_read_format (in_file, &((*entry)->wave), subtype);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "swav_read: error reading WAVE format chunk\n");
      return (EXIT_FAILURE);
    }

  /* TWO BYTES LATE: WHAT A HELL? */
  /* printf ("??? = %d\n", getsafe_word (in_file)); */
  getsafe_word (in_file);

  /* Read the data chunk */
  /* data word */
  strcpy ((*entry)->wave.data_c.data, getsafe (in_file, 4));

  /* Data chunk length */
  (*entry)->wave.data_c.length = getsafe_dword (in_file);

  switch ((*entry)->wave.wave_c.format_tag)
    {
      /* PCM format */
    case (PCM):
      exit_status = swav_read_pcm (in_file, entry);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "swav_read: error reading PCM samples\n");
          return (EXIT_FAILURE);
        }
      break;

    default:
      fprintf (stderr, "swav_read: unsupported format: %s\n",
               swav_fmt_enum_to_str ((*entry)->wave.wave_c.format_tag));
      return (EXIT_FAILURE);
    }

  return (EXIT_SUCCESS);
}




/*
 * swav_write_header_info
 *
 * Writes the RIFF WAVE header info (including format-specific info)
 * to the given file
 */
int
swav_write_header_info (FILE * out_file, const swav_type wave)
{
  /* RIFF chunk info */
  fprintf (out_file, "RIFF Chunk\n");
  fprintf (out_file, "----------\n");
  fprintf (out_file, "RIFF magick              = '%s'\n", wave.riff);
  fprintf (out_file, "RIFF size                = %d\n", wave.size);
  fprintf (out_file, "\n");

  /* WAVE format chunk */
  fprintf (out_file, "WAVE Format Chunk\n");
  fprintf (out_file, "-----------------\n");
  fprintf (out_file, "WAVE magick              = '%s'\n", wave.wave_c.wave);
  fprintf (out_file, "Format chunk length      = %d bytes\n",
           wave.wave_c.length);
  fprintf (out_file, "Format                   = %x (%s)\n",
           swav_fmt_enum_to_tag (wave.wave_c.format_tag),
           swav_fmt_enum_to_str (wave.wave_c.format_tag));
  fprintf (out_file, "Channels                 = %d\n", wave.wave_c.channels);
  fprintf (out_file, "Samples per second       = %d\n",
           wave.wave_c.samples_per_second);
  fprintf (out_file, "Average bytes per second = %d\n",
           wave.wave_c.average_bytes_per_second);
  fprintf (out_file, "Bytes per sample         = %d\n",
           wave.wave_c.bytes_per_sample);
  fprintf (out_file, "\n");

  /* Format specific parameters */
  fprintf (out_file, "Format specific\n");
  fprintf (out_file, "---------------\n");

  switch (wave.wave_c.format_tag)
    {
    case PCM:
      fprintf (out_file, "Bits per sample          = %d\n",
               wave.wave_c.pcm_specific.bits_per_sample);
      break;

    default:
      break;
    }
  fprintf (out_file, "\n");

  /* Data chunk */
  fprintf (out_file, "Data Chunk\n");
  fprintf (out_file, "----------\n");
  fprintf (out_file, "DATA magick              = '%s'\n", wave.data_c.data);
  fprintf (out_file, "Data chunk length        = %d\n", wave.data_c.length);
  fprintf (out_file, "\n");

  /* Complementary info */
  fprintf (out_file, "Complementary Info\n");
  fprintf (out_file, "------------------\n");

  fprintf (out_file, "File size                = ");
  if (wave.size == 0)
    fprintf (out_file, "0 bytes\n");
  else
    fprintf (out_file, "%d bytes\n", wave.size + 8);

  fprintf (out_file, "Number of samples        = ");
  if (wave.wave_c.bytes_per_sample == 0)
    fprintf (out_file, "0\n");
  else
    fprintf (out_file, "%d\n",
             wave.data_c.length / wave.wave_c.bytes_per_sample);

  fprintf (out_file, "Duration                 = ");
  if (wave.wave_c.average_bytes_per_second == 0)
    fprintf (out_file, "0\n");
  else
    fprintf (out_file, "%f\n",
             (double) wave.data_c.length /
             (double) wave.wave_c.average_bytes_per_second);

  return (EXIT_SUCCESS);
}




/*
 * swav_get_sample_value
 *
 * Takes the sample from the given WAVE structure,
 * number 'sample', channel 'channel' and returns its value at 'value'
 */
int
swav_get_sample_value (const swav_type riff, const size_t sample,
                       const size_t channel, swav_sample_type * value)
{
  /* Auxiliary number of samples */
  size_t samples;


  /* Gets the number of samples */
  samples = swav_num_samples (riff);

  /* Validates the sample requested */
  if (sample > samples)
    {
      fprintf (stderr,
               "swav_get_sample_value: invalid sample requested: %d from %d samples\n",
               sample, samples);
      return (EXIT_FAILURE);
    }

  /* Validates the channel requested */
  if (channel > riff.wave_c.channels)
    {
      fprintf (stderr,
               "swav_get_sample_value: invalid channel requested: %d from %d\n",
               channel, riff.wave_c.channels);
      return (EXIT_FAILURE);
    }

  /* Gets the actual value */
  *value =
    *(riff.data_c.samples + (sample * riff.wave_c.channels) + channel - 1);

  return (EXIT_SUCCESS);
}
