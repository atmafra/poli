#ifndef __VECSTAT_H_
#define __VECSTAT_H_ 1

#include <stdio.h>
#include "vector.h"

/******************************************************************************
 *                                                                            *
 *                             PUBLIC DATATYPES                               *
 *                                                                            *
 ******************************************************************************/

/*
 * vcst_stats_type
 *
 * Set of statistic vectors
 */
typedef struct
{
  UsLgIntValue dimension;
  Vector max;
  Vector min;
  Vector sum;
  Vector sum_sqr;
  Vector average;
  Vector variance;
  Vector stddev;
  Vector invstddev;
}
vcst_stats_type;


/* Symbolic Type */
typedef vcst_stats_type *VectorStats;



/******************************************************************************
 *                                                                            *
 *                             PUBLIC OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * vcst_init_stat
 *
 * Initializes all the internal statistic vectors, setting the internal
 * dimension.
 *
 * If the internal dimension is passed 0, all internal vectors will be
 * destroyed
 */
extern void vcst_init_stat (const UsLgIntValue dim);



/*
 * vcst_add_stat
 *
 * Adds a new observation
 */
extern int vcst_add_stat (const Vector v);



/*
 * vcst_samples
 *
 * Returns the current number of observations
 */
extern UsLgIntValue vcst_samples (void);



/*
 * vcst_average
 *
 * Returns the average vector
 */
extern int vcst_average (Vector v);



/*
 * vcst_variance
 *
 * Returns the variance vector
 */
extern int vcst_variance (Vector v);



/*
 * vcst_stddev
 *
 * Returns the standard deviation vector
 */
extern int vcst_stddev (Vector v);



/*
 * vcst_max
 *
 * Returns the maximum component values vector
 */
extern int vcst_max (Vector v);



/*
 * vcst_min
 *
 * Returns the minimum component values vector
 */
extern int vcst_min (Vector v);



/*
 * vcst_sum
 *
 * Returns the component sum vector
 */
extern int vcst_sum (Vector v);



/*
 * vcst_sum_sqr
 *
 * Returns the sum of the squared components vector
 */
extern int vcst_sum_sqr (Vector v);



/*
 * vcst_invstddev
 *
 * Returns the inverse standard deviations vector
 * (standard deviation ponderation vector)
 */
extern int vcst_invstddev (Vector v);



/*
 * vcst_stats_create
 *
 * Creates a new set of statistic vectors
 */
extern VectorStats vcst_stats_create (const UsLgIntValue dim);



/*
 * vcst_stats_destroy
 *
 * Destroys a previously created set of statistic vectors
 */
extern int vcst_stats_destroy (VectorStats * vstats);



/*
 * vcst_stats_update
 *
 * Updates the vectors in the set with the current statistics
 */
extern int vcst_stats_update (VectorStats vstats);



/*
 * vcst_stats_info
 *
 * Outputs information about the statistic vectors set
 */
extern void vcst_stats_info (const VectorStats vstats, FILE * output_fd);



#endif /* __VECSTAT_H_ */
