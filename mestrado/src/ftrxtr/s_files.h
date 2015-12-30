#ifndef __S_FILES_H_
#define __S_FILES_H_ 1

#include "s_smptypes.h"

/******************************************************************************
 *                                                                            *
 *                              File Operations                               *
 *                                                                            *
 ******************************************************************************/

/*
 * write_list
 *
 * Outputs the contents of the given list to 'dest_file'.
 * The flag 'raw_data' means that only data will be written.
 */
extern int
write_list (const sample_list_type smp_list,
            FILE * dest_file,
            const smp_breaklines_type break_lines,
            const smp_write_time_type write_time,
            const smp_write_files_type write_files,
            const smp_yes_no real_part, const smp_yes_no img_part);



/*
 * smp_translate_wave
 *
 * Translate data from a wave file to a list of samples
 */
extern int
smp_translate_wave (const swav_type riff, sample_list_type * smp_list);



/*
 * write_index
 *
 * Outputs the contents of the index to the destination file.
 *
 * Parameters:
 * - index: is the index that contain the lists to be written
 * - dest_file: destination file
 * - real_part: output the real part
 * - img_part: output the imaginary part
 */
extern int
write_index (const index_list_type index,
             char *dest_file,
             const smp_write_index_type write_index,
             const smp_write_lists_type write_lists,
             const smp_write_time_type write_time,
             const smp_write_files_type write_files,
             const smp_breaklines_type break_lines,
             const smp_yes_no real_part, const smp_yes_no img_part);



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
extern int
read_samples_file (index_list_type * index, const smp_index_pos pos);



/*
 * detect_file_type
 *
 * Tries to detect if the input file is at one of the supported formats
 */
extern int
detect_file_type (const index_entry_type list, list_file_type * type);



#endif /* __S_FILES_H_ */
