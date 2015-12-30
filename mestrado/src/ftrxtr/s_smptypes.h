#ifndef __SMPTYPES_H_
#define __SMPTYPES_H_ 1

#include "s_complex.h"

#define __SMP_MAX_SAMPLES_ 1000000

/* Basic real type */
typedef cmp_real smp_real;

/* Number of samples in a sample list */
typedef unsigned long smp_num_samples;

/* Index of an entry at an index */
typedef size_t smp_index_pos;

/* Data type for the elements of the list */
typedef enum
{ SMP_REAL, SMP_COMPLEX }
smp_entries_type;

/* Enumerated data type indicating Yes or No */
typedef enum
{ SMP_YES, SMP_NO }
smp_yes_no;

/* Break lines between values? */
typedef enum
{ SMP_BREAK_LINES, SMP_DONT_BREAK_LINES }
smp_breaklines_type;

/* Write index to output file? */
typedef enum
{ SMP_WRITE_INDEX, SMP_DONT_WRITE_INDEX }
smp_write_index_type;

/* Write lists to output files? */
typedef enum
{ SMP_WRITE_LISTS, SMP_DONT_WRITE_LISTS }
smp_write_lists_type;

/* Write time scale to output files? */
typedef enum
{ SMP_WRITE_TIME, SMP_WRITE_NORM_TIME, SMP_DONT_WRITE_TIME }
smp_write_time_type;

/* Write lists to one single file? */
typedef enum
{ SMP_WRITE_SINGLE_FILE, SMP_WRITE_MANY_FILES }
smp_write_files_type;

/* Set the newly created index entry to be current? */
typedef enum
{ SMP_SET_CURRENT, SMP_DONT_SET_CURRENT }
smp_set_current_type;

/* Types of signal files supported */
typedef enum
{ RAW, WAVE }
list_file_type;


/*****************************************************************************
 *                                                                           *
 *                            WAVE RELATED TYPES                             *
 *                                                                           *
 *****************************************************************************/

/* Type definition for the WAVE formats */
typedef enum
{ NONE, PCM, MULAW, ALAW, ADPCM, UNKNOWN }
swav_wave_fmt_type;

/* Sample value data type */
typedef int swav_sample_type;

/*
 * swav_wave_pcm_specific
 *
 * Structure for the PCM-specific format
 */
struct swav_wave_pcm_specific
{
  size_t bits_per_sample;
};

/*
 * swav_wave_struct
 *
 * Structure of the WAVE chunk of the RIFF data structure
 * Parameters:
 * - wave: the WAVE magic word
 * - fmt: the WAVE format word
 * - length: length of the WAVE format chunk
 * - channels: number of channels
 * - samples_per_second: number of samples per second. Sampling frequency, in Hz * - average_bytes_per_second: bytes_per_sample * samples_per_second
 * - bytes_per_sample: number of bytes used to store each sample
 */
struct swav_wave_struct
{
  char *wave;
  char *fmt;
  size_t length;
  swav_wave_fmt_type format_tag;
  size_t channels;
  size_t samples_per_second;
  size_t average_bytes_per_second;
  size_t bytes_per_sample;
  struct swav_wave_pcm_specific pcm_specific;
};


/*
 * swav_list_type
 *
 * Type for the list of samples
 * Samples will be addressed directly
 */
typedef swav_sample_type *swav_list_type;


/*
 * swav_riff_data_struct
 *
 * Data stucture for the actual data chunk
 */
struct swav_riff_data_struct
{
  char *data;
  size_t length;
  swav_list_type samples;
};


/*
 * swav_riff_wave_struct
 *
 * riff: RIFF magic word
 * size: size of the RIFF file, without the RIFF chunk
 * wave_c: embedded WAVE chunk
 */
struct swav_riff_wave_struct
{
  char *riff;
  size_t size;
  struct swav_wave_struct wave_c;
  struct swav_riff_data_struct data_c;
};


typedef struct swav_riff_wave_struct swav_type;



/*****************************************************************************
 *                                                                           *
 *                          INDEX AND LIST TYPES                             *
 *                                                                           *
 *****************************************************************************/

/*
 * sample_list_type
 *
 * Addressed list of samples
 *
 * type: indicates if the elements are real or complex valued
 * r: addressed list of reals
 * z: addressed list of complexes
 * samples: number of samples
 * ini_time: time associated to the first sample
 * inc_time: time interval (increment) between consequent samples
 * ini_norm_time: normalized time associated to the first sample
 * sum: sum
 * avg: average
 * var: variance
 * std: standard deviation
 * max: maxima
 * min: minima
 * valid_stats: indicates the current status of the list's statistics
 */
typedef struct
{
  smp_entries_type data_type;
  cmp_real *r;
  cmp_complex *z;
  smp_num_samples samples;
  cmp_real ini_time;
  cmp_real inc_time;
  smp_num_samples ini_norm_time;
  cmp_complex sum;
  cmp_complex avg;
  cmp_complex var;
  cmp_complex std;
  cmp_complex max;
  cmp_complex min;
  smp_yes_no valid_stats;
}
sample_list_type;

/*
 * index_entry
 *
 * Bi-linked list of sample lists
 * Each index_entry points to a sample list
 * - list: the list of samples
 * - position: the position of the list in the index
 * - name: the name of the list
 * - file: associated file name
 * - wave: associated RIFF WAVE data
 * - next: next index entry
 * - previous: previous index entry
 * - parent: parent list index entry
 */
struct index_entry_struct
{
  sample_list_type *list;
  smp_num_samples position;
  char name[NAME_SIZE + 1];
  char file[FILE_NAME_SIZE + 1];
  list_file_type file_type;
  swav_type wave;
  struct index_entry_struct *next;
  struct index_entry_struct *previous;
  struct index_entry_struct *parent;
};

typedef struct index_entry_struct index_entry;

/*
 * index_entry_type
 *
 * A pointer to an index entry
 */
typedef index_entry *index_entry_type;



/*
 * index_struct
 *
 * Control structure for index entries
 * - index_id: unique identifier of the index
 * - head: pointer to the first entry
 * - tail: pointer to the last entry
 * - current: pointer to the current entry
 * - num_entries: number of index entries
 * - parent_index: pointer to the parent of the current index
 */
struct index_struct
{
  smp_index_pos index_id;
  index_entry_type head;
  index_entry_type tail;
  index_entry_type current;
  smp_num_samples num_entries;
  struct index_struct *parent_index;
};

typedef struct index_struct index_list_type;

/*
 * index_list_ptr
 *
 * A pointer to an index
 */
typedef index_list_type *index_list_ptr;


#endif /* __SMPTYPES_H_ */
