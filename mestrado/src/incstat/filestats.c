#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "errorh.h"
#include "strutils.h"
#include "incstat.h"

#define __PROG_NAME_ "filestats"
#define __BUF_SIZE_ 1024

int
main (int argc, char **argv)
{
  char *filename;               /* input file name */
  FILE *fd;                     /* input file descriptor */
  RValue x, y;                  /* current line x and y */
  unsigned long line = 0;       /* current file line */
  char buf[__BUF_SIZE_];        /* read buffer */
  char *delim = " ,;&";         /* x y separators in file line */
  char *xstr = NULL;            /* x string */
  char *ystr = NULL;            /* y string */
  char *tailptr = NULL;         /* RValue conversion tail pointer */
  RValue lin_coeff = 0.0;       /* linear regression linear coefficient */
  RValue ang_coeff = 0.0;       /* linear regression angular coefficient */

  if (argc < 2)
    return error_failure (__PROG_NAME_, "no input file passed\n");

  filename = argv[1];

  errno = 0;

  fd = fopen (filename, "r");

  if (fd == NULL)
    return error_failure (__PROG_NAME_, "error opening file '%s': %s\n",
                          filename, strerror (errno));

  istt_clear_stat ();

  while (!feof (fd))
    {
      line++;

      if (read_valid_file_line (fd, __BUF_SIZE_ - 1, '#', TRUE, buf)
          != EXIT_SUCCESS)

        return error_failure (__PROG_NAME_,
                              "error reading file at line %ld\n", line);

      if (feof (fd))
        continue;

      xstr = strtok (buf, delim);

      if (xstr == NULL)
        return error_failure (__PROG_NAME_,
                              "error reading x value at line %ld\n", line);

      ystr = strtok (NULL, delim);

      errno = 0;
      x = strtod (xstr, &tailptr);

      if (errno != 0 || tailptr == NULL)
        return error_failure (__PROG_NAME_,
                              "invalid number format for x: '%s'\n", xstr);

      if (ystr != NULL)
        {
          y = strtod (ystr, &tailptr);

          if (errno != 0 || tailptr == NULL)
            return error_failure (__PROG_NAME_,
                                  "invalid number format for y: '%s'\n",
                                  ystr);
        }
      else
        {
          y = 0.0;
        }

      istt_add_stat (x, y);
    }

  fclose (fd);

  printf ("Samples    = %ld\n", istt_samples ());
  printf ("X Sum      = %f\n", istt_sum_x ());
  printf ("Y Sum      = %f\n", istt_sum_y ());
  printf ("X Average  = %f\n", istt_average_x ());
  printf ("Y Average  = %f\n", istt_average_y ());
  printf ("X Variance = %f\n", istt_variance_x ());
  printf ("Y Variance = %f\n", istt_variance_y ());
  printf ("X Std Dev  = %f\n", istt_stddev_x ());
  printf ("Y Std Dev  = %f\n", istt_stddev_y ());
  printf ("X Maximum  = %f\n", istt_max_x ());
  printf ("Y Maximum  = %f\n", istt_max_y ());
  printf ("X Minimum  = %f\n", istt_min_x ());
  printf ("Y Minimum  = %f\n", istt_min_y ());
  printf ("X Sqr Sum  = %f\n", istt_sum_sqr_x ());
  printf ("Y Sqr Sum  = %f\n", istt_sum_sqr_y ());
  istt_linear_regression (&lin_coeff, &ang_coeff);
  printf ("LR linear  = %f\n", lin_coeff);
  printf ("LR angular = %f\n", ang_coeff);
  printf ("XY Correl  = %f\n", istt_linear_correlation_coeff ());

  return EXIT_SUCCESS;
}
