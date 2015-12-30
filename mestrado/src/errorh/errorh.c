#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "errorh.h"


/*
 * output_failure
 *
 * Outputs error message to standard error stream
 */
static void
output_failure (const char *host_function_name, const char *error_message,
                va_list error_attributes)
{
  fprintf (stderr, "\n");

  if (host_function_name != NULL)
    fprintf (stderr, "%s: ", host_function_name);

  vfprintf (stderr, error_message, error_attributes);

  va_end (error_attributes);
}



/*
 * error_failure
 *
 * Returns EXIT_FAILURE, outputting error message to standard error output
 */
int
error_failure (const char *host_function_name, const char *error_message, ...)
{
  va_list error_attributes;

  va_start (error_attributes, error_message);
  output_failure (host_function_name, error_message, error_attributes);

  return EXIT_FAILURE;
}



/*
 * null_failure
 *
 * Returns NULL, outputting error message to standard error output
 */
void *
null_failure (const char *host_function_name, const char *error_message, ...)
{
  va_list error_attributes;

  va_start (error_attributes, error_message);
  output_failure (host_function_name, error_message, error_attributes);

  return NULL;
}



/*
 * error_if_null
 *
 * Returns EXIT_SUCCESS if 'function_output' is not null;
 * otherwise, outputs the error message with its corresponding attributes
 */
int
error_if_null (void *function_output, const char *host_function_name,
               const char *error_message, ...)
{
  va_list error_attributes;

  if (function_output == (void *) NULL)
    {
      va_start (error_attributes, error_message);
      output_failure (host_function_name, error_message, error_attributes);

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
      va_start (error_attributes, error_message);
      output_failure (host_function_name, error_message, error_attributes);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
