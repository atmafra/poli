#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include "vectorstat.h"


/******************************************************************************
 *                                                                            *
 *                              PRIVATE VARIABLES                             *
 *                                                                            *
 ******************************************************************************/

/*
 * Internal parameters
 */
static UsLgIntValue int_samples = 0;    /* number of samples */
static UsLgIntValue int_dim = 0;        /* vector dimension */


/*
 * Internal vectors
 */
static Vector zvector = NULL;   /* zeroes vector */
static Vector int_sum = NULL;   /* sum vector */
static Vector int_max = NULL;   /* maximums vector */
static Vector int_min = NULL;   /* minimums vector */
static Vector int_avg = NULL;   /* average vector */
static Vector int_avg_old = NULL;       /* previous average vector */
static Vector int_sqr_diff = NULL;      /* sum of difference to average */
static Vector int_sum_sqr = NULL;       /* sum of squares vector */



/******************************************************************************
 *                                                                            *
 *                              PRIVATE OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * vcst_create_all_vectors
 *
 * Allocates all the internal vectors
 */
static void
vcst_create_all_vectors (const UsLgIntValue dim)
{
  UsLgIntValue cur_comp;        /* current vector component */
  int exit_status;              /* auxiliary function return status */


  /* Dimension must be valid */
  if (dim == 0)
    return;

  /* Vectors must not exist */
  if (int_dim != 0)
    return;

  /* Creates the vectors */
  zvector = vector_create (dim);
  int_sum = vector_create (dim);
  int_avg = vector_create (dim);
  int_avg_old = vector_create (dim);
  int_max = vector_create (dim);
  int_min = vector_create (dim);
  int_sqr_diff = vector_create (dim);
  int_sum_sqr = vector_create (dim);

  if (zvector == NULL || int_sum == NULL || int_avg == NULL ||
      int_avg_old == NULL || int_max == NULL || int_min == NULL ||
      int_sqr_diff == NULL || int_sum_sqr == NULL)
    {
      fprintf (stderr,
               "vcst_create_all_vectors: error creating internal vectors\n");
      return;
    }

  /* Initializes the zeroes vector */
  for (cur_comp = 1; cur_comp <= dim; cur_comp++)
    {
      exit_status = vector_set_value (zvector, cur_comp, 0.0);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_create_all_vectors: error initializing zeroes vector\n");
          return;
        }
    }

  /* Initializes all vectors with the zeroes vector */
  exit_status = vector_copy (zvector, int_sum);
  exit_status &= vector_copy (zvector, int_avg);
  exit_status &= vector_copy (zvector, int_avg_old);
  exit_status &= vector_copy (zvector, int_max);
  exit_status &= vector_copy (zvector, int_min);
  exit_status &= vector_copy (zvector, int_sqr_diff);
  exit_status &= vector_copy (zvector, int_sum_sqr);

  if (exit_status != EXIT_SUCCESS)
    fprintf (stderr,
             "vcst_create_all_vectors: error initializing internal vectors\n");

  return;
}



/*
 * vcst_destroy_all_vectors
 *
 * Destroys all internal vectors
 */
static void
vcst_destroy_all_vectors (void)
{
  int exit_status;              /* auxiliary function return status */


  /* Vectors must exist */
  if (int_dim == 0)
    return;

  /* Destroys the vectors */
  exit_status = vector_destroy (&zvector);
  exit_status &= vector_destroy (&int_sum);
  exit_status &= vector_destroy (&int_avg);
  exit_status &= vector_destroy (&int_avg_old);
  exit_status &= vector_destroy (&int_max);
  exit_status &= vector_destroy (&int_min);
  exit_status &= vector_destroy (&int_sqr_diff);
  exit_status &= vector_destroy (&int_sum_sqr);

  if (exit_status != EXIT_SUCCESS)
    fprintf (stderr,
             "vcst_destroy_all_vectors: error destroying internal vectors\n");

  return;
}



/*
 * vcst_get_component_values
 *
 * Gets the values of the given components of the internal vectors
 */
static int
vcst_get_component_values (const UsLgIntValue component,
                           RValue * cur_sum, RValue * cur_avg,
                           RValue * cur_avg_old,
                           RValue * cur_max, RValue * cur_min,
                           RValue * cur_sqr_diff, RValue * cur_sum_sqr)
{
  int exit_status;              /* auxiliary function return status */

  /* Gets the values */
  exit_status = vector_get_value (int_sum, component, cur_sum);
  exit_status &= vector_get_value (int_avg, component, cur_avg);
  exit_status &= vector_get_value (int_avg_old, component, cur_avg_old);
  exit_status &= vector_get_value (int_max, component, cur_max);
  exit_status &= vector_get_value (int_min, component, cur_min);
  exit_status &= vector_get_value (int_sqr_diff, component, cur_sqr_diff);
  exit_status &= vector_get_value (int_sum_sqr, component, cur_sum_sqr);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_get_component_values: error getting values for component %ld\n",
               component);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * vcst_set_component_values
 *
 * Sets the values of the given components of the internal vectors
 */
static int
vcst_set_component_values (const UsLgIntValue component,
                           const RValue cur_sum, const RValue cur_avg,
                           const RValue cur_max, const RValue cur_min,
                           const RValue cur_sqr_diff,
                           const RValue cur_sum_sqr)
{
  int exit_status;              /* auxiliary function return status */

  /* Gets the values */
  exit_status = vector_set_value (int_sum, component, cur_sum);
  exit_status &= vector_set_value (int_avg, component, cur_avg);
  exit_status &= vector_set_value (int_max, component, cur_max);
  exit_status &= vector_set_value (int_min, component, cur_min);
  exit_status &= vector_set_value (int_sqr_diff, component, cur_sqr_diff);
  exit_status &= vector_set_value (int_sum_sqr, component, cur_sum_sqr);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_set_component_values: error setting values for component %ld\n",
               component);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
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
void
vcst_init_stat (const UsLgIntValue dim)
{
  /* If the vectors already exist, destroy them */
  if (int_dim > 0)
    vcst_destroy_all_vectors ();

  /* If a new valid dimension was passed, recreates the vectors */
  if (dim > 0)
    vcst_create_all_vectors (dim);

  /* Sets the number of samples and the internal dimension */
  int_samples = 0;
  int_dim = dim;

  return;
}



/*
 * vcst_add_stat
 *
 * Adds a new observation
 */
int
vcst_add_stat (const Vector v)
{
  UsLgIntValue cur_comp;        /* current component */
  RValue cur_val;               /* current component value */
  RValue cur_sum;               /* sum vector */
  RValue cur_avg;               /* average vector */
  RValue cur_avg_old;           /* previous average vector */
  RValue cur_max;               /* maximums vector */
  RValue cur_min;               /* minimums vector */
  RValue cur_sqr_diff;          /* sum of squared difference to average */
  RValue cur_sum_sqr;           /* sum of squares vector */
  int exit_status;              /* auxiliary function return status */


  /* Checks if the vector was actually passed */
  if (v == NULL)
    {
      fprintf (stderr, "vcst_add_stat: no vector passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the statistics are initialized */
  if (int_dim == 0)
    {
      fprintf (stderr, "vcst_add_stat: vector statistics not initialized\n");
      return EXIT_FAILURE;
    }

  /* Checks dimensional compatibility */
  if (v->dimension != int_dim)
    {
      fprintf (stderr,
               "vcst_add_stat: incompatible dimensions: dim(v) = %ld; internal dimension = %ld\n",
               v->dimension, int_dim);
      return EXIT_FAILURE;
    }

  /* Stores the old averages */
  exit_status = vector_copy (int_avg, int_avg_old);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "vcst_add_stat: error backing up averages\n");
      return EXIT_FAILURE;
    }

  /* Adds statistics for each dimension */
  for (cur_comp = 1; cur_comp <= int_dim; cur_comp++)
    {
      /* Gets the current component value of the given vector */
      exit_status = vector_get_value (v, cur_comp, &cur_val);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_add_stat: error getting value for the given vector component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Gets the current internal vectors component values */
      exit_status = vcst_get_component_values
        (cur_comp, &cur_sum, &cur_avg, &cur_avg_old,
         &cur_max, &cur_min, &cur_sqr_diff, &cur_sum_sqr);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_add_stat: error getting vector component values for component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      cur_avg = ((RValue) int_samples * cur_avg + cur_val)
        / ((RValue) int_samples + 1.0);

      /*
       * Accumulators
       */
      cur_sum += cur_val;
      cur_sum_sqr += sqr (cur_val);

      /*
       * Square Differences
       */
      cur_sqr_diff +=
        (RValue) int_samples / sqr ((RValue) int_samples + 1.0)
        * sqr (cur_avg_old - cur_val) + sqr (cur_avg - cur_val);

      /*
       * Maxima and minima
       */
      if (int_samples == 0)
        {
          cur_min = cur_val;
          cur_max = cur_val;
        }
      else
        {
          cur_min = (cur_min - cur_val < DBL_EPSILON ? cur_min : cur_val);
          cur_max = (cur_max - cur_val > DBL_EPSILON ? cur_max : cur_val);
        }

      /* Updates the internal vectors component values */
      exit_status = vcst_set_component_values
        (cur_comp, cur_sum, cur_avg, cur_max, cur_min,
         cur_sqr_diff, cur_sum_sqr);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_add_stat: error setting internal vector component %ld value\n",
                   cur_comp);
          return EXIT_FAILURE;
        }
    }

  /*
   * Number of observations
   */
  int_samples++;

  return EXIT_SUCCESS;
}



/*
 * vcst_samples
 *
 * Returns the current number of observations
 */
UsLgIntValue
vcst_samples (void)
{
  return int_samples;
}



/*
 * vcst_average
 *
 * Returns the average vector
 */
int
vcst_average (Vector v)
{
  return vector_copy (int_avg, v);
}



/*
 * vcst_variance
 *
 * Returns the variance vector
 */
int
vcst_variance (Vector v)
{
  int exit_status;              /* auxiliary function return status */


  if (int_samples > 0)
    {
      /* Copies the sum of squared differences to average */
      exit_status = vector_copy (int_sqr_diff, v);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_variance: error calculating variance vector\n");
          return EXIT_FAILURE;
        }

      /* Divides the differences of the components by the number of samples */
      exit_status = vector_scalar_multiply (v, 1.0 / (RValue) int_samples, v);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_variance: error calculating variance vector\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      fprintf (stderr, "vcst_variance: no observations added\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * vcst_stddev
 *
 * Returns the standard deviation vector
 */
int
vcst_stddev (Vector v)
{
  int exit_status;              /* auxiliary function return status */


  if (int_samples > 0)
    {
      /* Calculates the variance vector */
      exit_status = vcst_variance (v);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_stddev: error calculating standard deviation vector\n");
          return EXIT_FAILURE;
        }

      /* Takes the square root of each component */
      exit_status = vector_sqrt (v, v);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_stddev: error taking square root of components\n");
          return EXIT_FAILURE;
        }
    }
  else
    {
      fprintf (stderr, "vcst_stddev: no observations added\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * vcst_max
 *
 * Returns the maximum component values vector
 */
int
vcst_max (Vector v)
{
  return vector_copy (int_max, v);
}



/*
 * vcst_min
 *
 * Returns the minimum component values vector
 */
int
vcst_min (Vector v)
{
  return vector_copy (int_min, v);
}



/*
 * vcst_sum
 *
 * Returns the component sum vector
 */
int
vcst_sum (Vector v)
{
  return vector_copy (int_sum, v);
}



/*
 * vcst_sum_sqr
 *
 * Returns the sum of the squared components vector
 */
int
vcst_sum_sqr (Vector v)
{
  return vector_copy (int_sum_sqr, v);
}


/*
 * vcst_invstd_pond
 *
 * Returns the inverse standard deviations vector
 * (standard deviation ponderation vector)
 */
int
vcst_invstddev (Vector v)
{
  UsLgIntValue cur_comp;        /* current vector component */
  RValue cur_value;             /* current vector component value */
  int exit_status;              /* auxiliary function return status */

  /* Copies the standard deviation vector */
  exit_status = vcst_stddev (v);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_invstd_pond: error calculating ponderation vector\n");
      return EXIT_FAILURE;
    }

  for (cur_comp = 1; cur_comp <= int_dim; cur_comp++)
    {
      /* Gets the component value */
      exit_status = vector_get_value (v, cur_comp, &cur_value);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "vcst_invstd_pond: error getting standard deviation value for component %ld\n",
                   cur_comp);
          return EXIT_FAILURE;
        }

      /* Inverts the component, ignoring constant component */
      if (cur_value > DBL_EPSILON)
        {
          exit_status = vector_set_value (v, cur_comp, 1.0 / cur_value);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "vcst_invstd_pond: error setting ponderation vector component %ld\n",
                       cur_comp);
              return EXIT_FAILURE;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * vcst_stats_create
 *
 * Creates a new set of statistic vectors
 */
VectorStats
vcst_stats_create (const UsLgIntValue dim)
{
  VectorStats new_vstats = NULL;        /* new statistic vectors set */


  /* Checks if the dimension is valid */
  if (dim < 1)
    {
      fprintf (stderr, "vcst_stats_create: invalid dimension: %ld\n", dim);
      return NULL;
    }

  /* Creates the new vector set */
  new_vstats = (VectorStats) malloc (sizeof (vcst_stats_type));

  if (new_vstats == NULL)
    {
      fprintf (stderr, "vcst_stats_create: virtual memory exhausted\n");
      return NULL;
    }

  /* Sets the vectors dimension */
  new_vstats->dimension = dim;

  /* Creates the vectors */
  new_vstats->max = vector_create (dim);
  new_vstats->min = vector_create (dim);
  new_vstats->sum = vector_create (dim);
  new_vstats->sum_sqr = vector_create (dim);
  new_vstats->average = vector_create (dim);
  new_vstats->variance = vector_create (dim);
  new_vstats->stddev = vector_create (dim);
  new_vstats->invstddev = vector_create (dim);

  if (new_vstats->max == NULL || new_vstats->min == NULL ||
      new_vstats->sum == NULL || new_vstats->sum_sqr == NULL ||
      new_vstats->average == NULL || new_vstats->variance == NULL ||
      new_vstats->stddev == NULL || new_vstats->invstddev == NULL)
    {
      fprintf (stderr,
               "vcst_stats_create: error creating statistic vectors\n");
      free (new_vstats);
      return NULL;
    }

  return new_vstats;
}



/*
 * vcst_stats_destroy
 *
 * Destroys a previously created set of statistic vectors
 */
int
vcst_stats_destroy (VectorStats * vstats)
{
  int exit_status;              /* auxiliary function return status */


  /* Checks if the set was actually passed */
  if (vstats == NULL || *vstats == NULL)
    {
      fprintf (stderr,
               "vcst_stats_destroy: no statistic vectors set passed\n");
      return EXIT_FAILURE;
    }

  /* Destroys the vectors */
  exit_status = vector_destroy (&((*vstats)->max));
  exit_status &= vector_destroy (&((*vstats)->min));
  exit_status &= vector_destroy (&((*vstats)->sum));
  exit_status &= vector_destroy (&((*vstats)->sum_sqr));
  exit_status &= vector_destroy (&((*vstats)->average));
  exit_status &= vector_destroy (&((*vstats)->variance));
  exit_status &= vector_destroy (&((*vstats)->stddev));
  exit_status &= vector_destroy (&((*vstats)->invstddev));

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_stats_destroy: error destroying statistic vectors\n");
      return EXIT_FAILURE;
    }

  /* Destroys the set itself */
  free (*vstats);

  /* Makes it point to NULL */
  *vstats = NULL;

  return EXIT_SUCCESS;
}



/*
 * vcst_stats_update
 *
 * Updates the vectors in the set with the current statistics
 */
int
vcst_stats_update (VectorStats vstats)
{
  /* Checks if the set of vectors was passed */
  if (vstats == NULL)
    {
      fprintf (stderr,
               "vcst_stats_update: no statistic vectors set passed\n");
      return EXIT_FAILURE;
    }

  /* Updates each statistic vector */
  if (vcst_max (vstats->max) != EXIT_SUCCESS)
    {
      fprintf (stderr, "vcst_stats_update: error updating maximums vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_min (vstats->min) != EXIT_SUCCESS)
    {
      fprintf (stderr, "vcst_stats_update: error updating minimums vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_sum (vstats->sum) != EXIT_SUCCESS)
    {
      fprintf (stderr, "vcst_stats_update: error updating sums vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_sum_sqr (vstats->sum_sqr) != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_stats_update: error updating squared sums vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_average (vstats->average) != EXIT_SUCCESS)
    {
      fprintf (stderr, "vcst_stats_update: error updating averages vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_variance (vstats->variance) != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_stats_update: error updating variances vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_stddev (vstats->stddev) != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_stats_update: error updating standard deviations vector\n");
      return EXIT_FAILURE;
    }

  if (vcst_invstddev (vstats->invstddev) != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "vcst_stats_update: error updating inverse standard deviations vector\n");
      return EXIT_FAILURE;
    }


  return EXIT_SUCCESS;
}



/*
 * vcst_stats_info
 *
 * Outputs information about the statistic vectors set
 */
void
vcst_stats_info (const VectorStats vstats, FILE * output_fd)
{
  if (output_fd == NULL)
    return;

  if (vstats == NULL)
    return;

  fprintf (output_fd, "Maxima    : ");
  vector_info (vstats->max, output_fd);

  fprintf (output_fd, "Minima    : ");
  vector_info (vstats->min, output_fd);

  fprintf (output_fd, "Sums      : ");
  vector_info (vstats->sum, output_fd);

  fprintf (output_fd, "Sqr sums  : ");
  vector_info (vstats->sum_sqr, output_fd);

  fprintf (output_fd, "Averages  : ");
  vector_info (vstats->average, output_fd);

  fprintf (output_fd, "Variances : ");
  vector_info (vstats->variance, output_fd);

  fprintf (output_fd, "Std Devs  : ");
  vector_info (vstats->stddev, output_fd);

  return;
}
