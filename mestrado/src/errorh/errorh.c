#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "errorh.h"



/*
 * error_failure
 *
 * Returns EXIT_FAILURE, outputting error message to standard error output
 */
int
error_failure (const char *host_function_name, const char *error_message, ...)
{
  va_list error_attributes;

  fprintf (stderr, "%s: ", host_function_name);
  va_start (error_attributes, error_message);
  vfprintf (stderr, error_message, error_attributes);
  va_end (error_attributes);

  return EXIT_FAILURE;
}



/*
 * error_if_null
 *
 * Returns EXIT_SUCCESS if 'function_output' is not null;
 * otherwise, outputs the error message with its corresponding attributes
 */
int
error_if_null (void *function_output,
               const char *host_function_name, const char *error_message, ...)
{
  va_list error_attributes;

  if (function_output == (void *) NULL)
    {
      fprintf (stderr, "%s: ", host_function_name);
      va_start (error_attributes, error_message);
      vfprintf (stderr, error_message, error_attributes);
      va_end (error_attributes);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * error_if_failure
 *
 * Returns EXIT_SUCCESS if 'function_output' is EXIT_SUCCESS;
 * otherwise, outputs the error message with its corresponding attributes
 */
int
error_if_failure (int function_output,
                  const char *host_function_name,
                  const char *error_message, ...)
{
  va_list error_attributes;

  if (function_output != EXIT_SUCCESS)
    {
      fprintf (stderr, "%s: ", host_function_name);
      va_start (error_attributes, error_message);
      vfprintf (stderr, error_message, error_attributes);
      va_end (error_attributes);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
