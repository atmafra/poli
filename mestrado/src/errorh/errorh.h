#ifndef __ERROR_HANDLING_H_
#define __ERROR_HANDLING_H_ 1

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C"
{
#endif
/* *INDENT-ON* */



/*
 * error_failure
 *
 * Returns EXIT_FAILURE, outputting error message to standard error output
 */
extern int
error_failure (const char *host_function_name,
               const char *error_message, ...);



/*
 * error_if_null
 *
 * Returns EXIT_SUCCESS if 'function_output' is not null;
 * otherwise, outputs the error message with its corresponding attributes
 */
extern int
error_if_null (void *function_output,
               const char *host_function_name,
               const char *error_message, ...);



/*
 * error_if_failure
 *
 * Returns EXIT_SUCCESS if 'function_output' is EXIT_SUCCESS;
 * otherwise, outputs the error message with its corresponding attributes
 */
extern int
error_if_failure (int function_output,
                  const char *host_function_name,
                  const char *error_message, ...);


/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* errorh.h */
