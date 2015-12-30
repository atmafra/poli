#ifndef __S_FMT_WAV_H_
#define __S_FMT_WAV_H_ 1

#include  "s_smptypes.h"

/* RIFF file magic word */
#define __SWAV_RIFF_MAGIC_ "RIFF"

/* Magick word size */
#define __SWAV_MAGIC_SIZE_ sizeof(__SWAV_RIFF_MAGIC_)

/* WAVE file magic word */
#define __SWAV_WAVE_MAGIC_ "WAVE"

/* WAVE chunk format word */
#define __SWAV_WAVE_FMT_ "fmt "

/* DATA chunk word */
#define __SWAV_WAVE_DATA_ "data"

/* WAVE formats */
#define __SWAV_WAVE_FMT_NONE_ 0x0000
#define __SWAV_WAVE_FMT_PCM_ 0x0001
#define __SWAV_WAVE_FMT_IBM_MULAW_ 0x0101
#define __SWAV_WAVE_FMT_IBM_ALAW_ 0x0102
#define __SWAV_WAVE_FMT_IBM_ADPCM_ 0x0103
#define __SWAV_WAVE_FMT_UNKNOWN_ 0x9999

/* WAVE formats description */
#define __SWAV_WAVE_FMT_DESC_NONE_ "None"
#define __SWAV_WAVE_FMT_DESC_PCM_ "Microsoft Pulse Code Modulation (PCM)"
#define __SWAV_WAVE_FMT_DESC_IBM_MULAW_ "IBM mu-law"
#define __SWAV_WAVE_FMT_DESC_IBM_ALAW_ "IBM a-law"
#define __SWAV_WAVE_FMT_DESC_IBM_ADPCM_ "IBM AVC Adaptive Pulse Code Modulation"
#define __SWAV_WAVE_FMT_DESC_UNKNOWN_ "Unknown"


/*****************************************************************************
 *                                                                           *
 *                             Public Functions                              *
 *                                                                           *
 *****************************************************************************/

/*
 * swav_num_samples
 *
 * Gets the number of samples for the given structure
 */
extern size_t swav_num_samples (const swav_type riff);


/*
 * swav_samples_per_second
 *
 * Gets the number of samples per second for the given structure
 */
extern size_t swav_samples_per_second (const swav_type riff);


/*
 * swav_detect
 *
 * Tries to detect if the given file is a RIFF WAVE file
 * 'detected' returns 0 if the file is not RIFF WAVE; returns -1 otherwise
 */
extern int swav_detect (char *file_name, int *detected);


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
extern int swav_read (FILE * in_file, index_entry_type * entry);


/*
 * swav_write_header_info
 *
 * Writes the RIFF WAVE header info (including format-specific info)
 * to the given file
 */
extern int swav_write_header_info (FILE * out_file, const swav_type wave);


/*
 * swav_get_sample_value
 *
 * Takes the sample from the given WAVE structure,
 * number 'sample', channel 'channel' * and returns its value at 'value'
 */
extern int
swav_get_sample_value (const swav_type riff, const size_t sample,
                       const size_t channel, swav_sample_type * value);



#endif /* __S_FMT_WAV_H_ */
