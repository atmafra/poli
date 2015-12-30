#ifndef __S_LOG_H_
#define __S_LOG_H_ 1

#include <math.h>
#include "s_samples.h"

/*
 * slog_list
 *
 * Applies the logarithm to the samples in a list.
 *
 * Parameters:
 * - in_index: input index. Index where the input list is in
 * - in_pos: position of the input list at the input index
 * - in_place: Yes/No, defining if the input list must be used as output
 * - out_index: output index.
 *   Index where the output list is in.
 *   Same as 'in_index', if * 'in_place' was passed Yes
 * - out_pos: position of the output list at the output index
 */
extern int
slog_list (index_list_type * in_index, const smp_index_pos in_pos,
           const smp_yes_no in_place, index_list_type * out_index,
           smp_index_pos * out_pos);


#endif /* __S_LOG_H */
