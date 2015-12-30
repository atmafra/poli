#ifndef __S_CEPSTRUM_H_
#define __S_CEPSTRUM_H_ 1

#include "s_smptypes.h"
#include "s_window.h"

/* C1 = 1000.0 / ln (1.0 + 1000.0 / 700.0) */
#define __C1_ 1127.010480334157438654463368029022

/*
 * Parameter structure
 *
 * Set of parameters required for all the operations involving MFCC
 * calculation.
 *
 * in_preemphasis: indicates if preemphasis should be applied
 * alpha_preemphasis: value of alpha of the preemphasis filter
 * frame_width: number of samples of each generated frame
 * superposing_samples: number of samples that are common to two consecutive
 * frames
 * windowing_function: windowing function to be applied (Hamming, Hanning, ...)
 * triangular_window_low: low value for triangular window
 * triangular_window_center: center value for triangular window
 * triangular_window_high: high value for triangular window
 * triangular_window_central_value: value of the window at its peak
 * kaiser_window_B: B parameter for Kaiser-Bessel windowing function
 * purge_zero_power: purge frames with no energy (power)
 * log_basis: logarithm basis value for cepstrum logarithm
 * delta_mel: mel frequency interval between filters
 * total_filters: number of filters in the bank
 */
typedef struct
{
  /* Feature extraction parameters */
  swin_yes_no in_preemphasis;
  smp_real alpha_preemphasis;
  swin_width_type frame_width;
  swin_width_type superposing_samples;
  swin_window_type windowing_function;
  smp_real triangular_window_low;
  smp_real triangular_window_center;
  smp_real triangular_window_high;
  smp_real triangular_window_central_value;
  smp_real kaiser_window_B;
  swin_purge_type purge_zero_power;
  cmp_real log_basis;
  cmp_real delta_mel;
  smp_num_samples total_filters;
  /* Writing parameters */
  smp_write_index_type write_index;
  smp_write_lists_type write_lists;
  smp_write_time_type write_time;
  smp_write_files_type write_files;
  smp_breaklines_type write_break_lines;
  smp_yes_no write_real_part;
  smp_yes_no write_img_part;
}
scep_parameter_type;




/*
 * scep_f_hz_to_mel
 *
 * Returns the Mel frequency associated to the given input frequency (in Hz)
 */
extern cmp_real scep_f_hz_to_mel (cmp_real f_hz);



/*
 * scep_f_mel_to_hz
 *
 * Returns the frequency in Hz associated to the given mel-scale frequency
 */
extern cmp_real scep_f_mel_to_hz (cmp_real f_mel);


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
extern int
scep_mel_scale (const cmp_real delta_mel,
                const smp_num_samples total_filters,
                const smp_num_samples filter,
                cmp_real * low_freq, cmp_real * mid_freq, cmp_real * hi_freq);



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
 * - delta_mel: mel frequency interval between filterrs
 * - total_filters: number of filters in the bank
 * - out_index: index to which the MFCC coefficients list should be appended to
 */
extern int
scep_mfcc (const index_entry_type list,
           const cmp_real delta_mel,
           const smp_num_samples total_filters, index_list_type * out_index);


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
 * - delta_mel: mel frequency interval between filters
 * - total_filters: number of filters in the bank
 * - mfcc_index: output index, with the same number of lists of the input
 * index, containing the MFCC's of the input lists respectively
 */
extern int
scep_mfcc_index (const index_list_type in_index,
                 const cmp_real delta_mel,
                 const smp_num_samples total_filters,
                 index_list_type * mfcc_index);



/*
 * scep_mfcc_file
 *
 * Calculates the MFCC's (Mel Frequency Cepstral Coefficients)
 * for the given input file, according to the given parameter structure.
 * The resulting lists are stored in the output 'mfcc_index'
 *
 * Parameters:
 * - in_file_name: input file name
 * - param: parameters structure
 * - file_index: index where the file entry will be appended
 * - mfcc_index: index where the MFCC's lists will be stored
 */
extern int
scep_mfcc_file (const char *in_file_name,
                const scep_parameter_type param,
                index_list_type * file_index, index_list_type * mfcc_index);



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
extern int
scep_mfcc_file_write (const char *in_file_name,
                      const scep_parameter_type param, char *out_file_name);



#endif /* __S_CEPSTRUM_H_ */
