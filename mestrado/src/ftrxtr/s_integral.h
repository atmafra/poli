/*
 * Integration methods:
 * Trapeze: linear interpolation of the function between two samples
 * Simpson: polinomial interpolation of the function between two samples
 */
#ifndef __INT_INTEGRAL_
#define __INT_INTEGRAL_


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/* Integration Methods */
#define	__INT_TRAPEZE		1
#define __INT_SIMPSON           2
#define __INT_DEFAULT_METHOD	__INT_SIMPSON

/* Error Constants */
#define __INT_ERROR_NOERROR     0
#define __INT_ERROR_BOUNDS      1
#define __INT_ERROR_SEGMENTS    2
#define __INT_ERROR_METHOD      3
#define __INT_ERROR_FUNCTION    4
#define __INT_ERROR_SAMPLES	5
#define __INT_ERROR_FIRST	__INT_ERROR_BOUNDS
#define	__INT_ERROR_LAST	__INT_ERROR_SAMPLES

/* TRUE and FALSE */
#ifndef TRUE
#define TRUE                    0
#endif

#ifndef FALSE
#define FALSE                   1
#endif

/* Default number of segments between two samples */
#define __INT_DEFAULT_SEGMENTS  1000


/*
 * Integral data types
 */

/* Number of segments */
typedef size_t sint_segments_type;

/* Integration method */
typedef unsigned char sint_method_type;

/*
 * func_integral
 *
 * Numeric integral of function 'f', between the limits
 * 'lower_bound' and 'upper_bound'.
 * The parameter segments defines in how many parts the interval
 * between lower_bound and upper_bound will be broken into.
 * The higher the number of segments, the most accurate the interpolation
 * of the function.
 */
double func_integral (const double lower_bound, const double upper_bound,
                      const sint_segments_type segments,
                      double (*f) (const double),
                      const sint_method_type method);


#endif /* !__INT_INTEGRAL */
