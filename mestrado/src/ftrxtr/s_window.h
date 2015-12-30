#ifndef __S_WINDOW_H_
#define __S_WINDOW_H_ 1

#include "s_smptypes.h"
#include "s_preemph.h"

/* Definition of the constant PI */
#undef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI 3.1415926535897932384626434
#endif

/* Definition of the square operation */
#define __SWIN_SQR_(x) (x)*(x)

/*
 * swin_window_type
 *
 *  SWIN_RECTANGULAR: rectangular window
 *  SWIN_BARTLETT: bartlett triangular window
 *  SWIN_TRIANGULAR: generic triangular window
 *  SWIN_KAISER: Kaiser-Bessel window
 *  SWIN_HAMMING: Hamming window
 *  SWIN_HANNING: Hanning window
 *  SWIN_BLACKMAN: Blackman window
 */
typedef enum
{
  SWIN_RECTANGULAR,
  SWIN_BARTLETT,
  SWIN_TRIANGULAR,
  SWIN_KAISER,
  SWIN_HAMMING,
  SWIN_HANNING,
  SWIN_BLACKMAN
}
swin_window_type;


/*
 * swin_purge_type
 *
 * Parameter that indicates that zero-powered frames should or not be purged
 */
typedef enum
{ SWIN_PURGE_ZERO_POWER, SWIN_DONT_PURGE_ZERO_POWER }
swin_purge_type;

/*
 * Yes/No data type
 */
typedef enum
{ SWIN_YES, SWIN_NO }
swin_yes_no;

/* Window or frame width domains */
typedef size_t swin_width_type;

/* Window or frame number domains */
typedef smp_index_pos swin_number_type;



/*
 * swin_windowed_value
 *
 * Calculates the windowed value of the given value.
 */
extern int
swin_windowed_value (const cmp_complex in_value,
                     const smp_num_samples pos,
                     const smp_num_samples samples,
                     const swin_window_type window_type,
                     const smp_entries_type data_type,
                     const spre_real trg_central,
                     const spre_real trg_low,
                     const spre_real trg_high,
                     const spre_real trg_central_value,
                     const spre_real B_kaiser, cmp_complex * out_value);



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
extern int
swin_window_list (const index_entry_type source_list,
                  const swin_number_type frame_number,
                  const swin_width_type width,
                  const smp_num_samples samples_between_frames,
                  const smp_entries_type frame_data_type,
                  const smp_entries_type windowing_data_type,
                  const swin_window_type window_type,
                  const smp_real trg_central,
                  const smp_real trg_low,
                  const smp_real trg_high,
                  const smp_real trg_central_value,
                  const smp_real B_kaiser, index_list_type * out_index);



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
 * - windowing_data_type: data type (real/complex) for windowing operations
 * - trg_central: triangle peak value for triangular window
 * - trg_low: low value for triangular window
 * - trg_high: high value for triangular window
 * - trg_central_value: value of the window at 'trg_central'
 * - B_kaiser: B parameter for the Kaiser-Bessel windowing function
 * - purge_zero_power: indicates that zero-powered frames should be purged
 * - new_index: the new index with the windowed frames of the original list
 */
extern int
swin_window (index_list_type * index,
             const swin_yes_no preemphasis,
             const smp_real alpha_preemphasis,
             const swin_width_type width,
             const swin_width_type superposing_samples,
             const smp_entries_type frame_data_type,
             const smp_entries_type windowing_data_type,
             const swin_window_type window_type,
             const smp_real trg_central,
             const smp_real trg_low,
             const smp_real trg_high,
             const smp_real trg_central_value,
             const smp_real B_kaiser,
             const swin_purge_type purge_zero_power,
             index_list_type * new_index);



#endif /* __S_WINDOW_H_ */
