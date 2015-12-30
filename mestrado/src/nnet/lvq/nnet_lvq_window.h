#ifndef __NNET_LVQ_WINDOW_H_
#define __NNET_LVQ_WINDOW_H_ 1

#include "../../common/types.h"
#include "../nnet_types.h"

/******************************************************************************
 *                                                                            *
 *                             FUNCTIONAL OPERATIONS                          *
 *                                                                            *
 ******************************************************************************/

extern BoolValue
nnet_lvq_winfunction (const Vector x, const Vector w1, const Vector w2,
                      const RValue w);



/*
 * nnet_lvq_window
 *
 * Evaluates if the input vector is inside the window defined
 * between the two winners, according to the window width
 */
extern int
nnet_lvq_window (const Vector input_vector,
                 const Unit winner1, const Unit winner2,
                 const RValue window_width, BoolValue * result);


#endif /* __NNET_LVQ_WINDOW_H_ */
