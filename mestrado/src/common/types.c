#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>

#include "errorh.h"
#include "types.h"

/******************************************************************************
 *                                                                            *
 *                              BOOLEAN OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * str_to_bool
 *
 * Converts a string into its corresponding boolean value
 */
int
str_to_bool (const char *str, BoolValue * bool)
{
  if (str == NULL)
    return error_failure ("str_to_bool", "string not informed\n");

  if (strcmp (str, __TRUE_STR_) == 0)
    {
      *bool = TRUE;
      return EXIT_SUCCESS;
    }


  if (strcmp (str, __FALSE_STR_) == 0)
    {
      *bool = FALSE;
      return EXIT_SUCCESS;
    }

  return error_failure ("str_to_bool", "invalid boolean: '%s'\n", str);
}



/*
 * bool_to_str
 *
 * Converts a boolean value into its corresponding string
 */
char *
bool_to_str (const BoolValue bool)
{
  switch (bool)
    {
    case TRUE:
      return __TRUE_STR_;

    case FALSE:
      return __FALSE_STR_;

    default:
      return NULL;
    }
}



/******************************************************************************
 *                                                                            *
 *                             COMPOSITE OPERATIONS                           *
 *                                                                            *
 ******************************************************************************/

/*
 * type_to_str
 *
 * Converts a type to its identifying string
 */
char *
type_to_str (const CompositeDataType type)
{
  switch (type)
    {
    case BOOL:
      return __BOOL_STR_;

    case UNSIGNED_INT:
      return __UNSIGNED_INT_STR_;

    case LONG_INT:
      return __LONG_INT_STR_;

    case UNSIGNED_LONG_INT:
      return __UNSIGNED_LONG_INT_STR_;

    case REAL:
      return __REAL_STR_;

    case REAL_VECTOR:
      return __REAL_VECTOR_STR_;

    case STRING:
      return __STRING_STR_;

    case HANDLER:
      return __HANDLER_STR_;

    default:
      error_failure ("type_to_str", "unknown datatype\n");
      return NULL;
    }
}



/*
 * str_to_type
 *
 * Returns the composite data type corresponding to the given string
 */
int
str_to_type (const char *str, CompositeDataType * type)
{
  if (strcmp (str, __BOOL_STR_) == 0)
    {
      *type = BOOL;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __UNSIGNED_INT_STR_) == 0)
    {
      *type = UNSIGNED_INT;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __LONG_INT_STR_) == 0)
    {
      *type = LONG_INT;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __UNSIGNED_LONG_INT_STR_) == 0)
    {
      *type = UNSIGNED_LONG_INT;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __REAL_STR_) == 0)
    {
      *type = REAL;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __REAL_VECTOR_STR_) == 0)
    {
      *type = REAL_VECTOR;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __STRING_STR_) == 0)
    {
      *type = STRING;
      return EXIT_SUCCESS;
    }

  if (strcmp (str, __HANDLER_STR_) == 0)
    {
      *type = HANDLER;
      return EXIT_SUCCESS;
    }

  return error_failure ("str_to_type", "unknown type: '%s'\n", *type);
}



/*
 * composite_compare
 *
 * Compares two composite values, given their data type
 */
BoolValue
composite_compare (const CompositeUnion c1, const CompositeUnion c2,
                   const CompositeDataType type)
{
  BoolValue result = FALSE;     /* comparison result */

  switch (type)
    {
    case BOOL:
      if (c1.boolvalue == c2.boolvalue)
        result = TRUE;
      break;

    case UNSIGNED_INT:
      if (c1.usintvalue == c2.usintvalue)
        result = TRUE;
      break;

    case LONG_INT:
      if (c1.lgintvalue == c2.lgintvalue)
        result = TRUE;
      break;

    case UNSIGNED_LONG_INT:
      if (c1.uslgintvalue == c2.uslgintvalue)
        result = TRUE;
      break;

    case REAL:
      if (fabs (c1.realvalue - c2.realvalue) <= DBL_EPSILON)
        result = TRUE;
      break;

    case REAL_VECTOR:
      result = FALSE;
      break;

    case STRING:
      if (c1.stringvalue != NULL && c2.stringvalue != NULL)
        if (strcmp (c1.stringvalue, c2.stringvalue) == 0)
          result = TRUE;
      break;

    case HANDLER:
      if (c1.handlervalue != NULL && c2.handlervalue != NULL)
        if (c1.handlervalue == c2.handlervalue)
          result = TRUE;
      break;

    default:
      return error_failure ("composite_compare", "invalid data type\n");
    }

  return result;
}



/*
 * str_to_composite
 *
 * Converts a string into a composite value according to the type
 */
int
str_to_composite (char *instr, CompositeDataType type,
                  CompositeUnion * outcmp)
{
  CompositeUnion cv;            /* composite value to return */
  char *tailptr;                /* for integer conversions */

  if (instr == NULL)
    return error_failure ("str_to_composite", "no string to parse\n");

  switch (type)
    {
    case BOOL:
      if (str_to_bool (instr, &(cv.boolvalue)) != EXIT_SUCCESS)
        return error_failure ("str_to_composite", "invalid boolean\n");
      break;

    case UNSIGNED_INT:
      errno = 0;
      cv.usintvalue = (UsIntValue) strtoul (instr, &tailptr, 10);
      if (errno != 0)
        return error_failure ("str_to_composite", strerror (errno));
      if (*tailptr != '\0')
        return error_failure ("str_to_composite",
                              "invalid unsigned integer: '%s'\n", instr);
      break;

    case LONG_INT:
      errno = 0;
      cv.lgintvalue = (LgIntValue) strtol (instr, &tailptr, 10);
      if (errno != 0)
        return error_failure ("str_to_composite", strerror (errno));
      if (*tailptr != '\0')
        return error_failure ("str_to_composite",
                              "invalid long integer: '%s'\n", instr);
      break;

    case UNSIGNED_LONG_INT:
      errno = 0;
      cv.uslgintvalue = (UsLgIntValue) strtoul (instr, &tailptr, 10);
      if (errno != 0)
        return error_failure ("str_to_composite", strerror (errno));
      if (*tailptr != '\0')
        return error_failure ("str_to_composite",
                              "invalid unsigned long integer: '%s'\n", instr);
      break;

    case REAL:
      errno = 0;
      cv.realvalue = (RValue) strtod (instr, &tailptr);
      if (errno != 0)
        return error_failure ("str_to_composite", strerror (errno));
      if (*tailptr != '\0')
        return error_failure ("str_to_composite",
                              "invalid unsigned long integer: '%s'\n", instr);
      break;

    case REAL_VECTOR:
      return error_failure ("str_to_composite",
                            "real vector conversion not implemented\n");
      break;

    case STRING:
      cv.stringvalue = (char *) malloc (strlen (instr) + 1);
      if (cv.stringvalue == NULL)
        return error_failure ("str_to_composite",
                              "can't get memory to store the string '%s'\n",
                              instr);
      strcpy (cv.stringvalue, instr);
      break;

    case HANDLER:
      if (strcmp (instr, __NULL_STR_) != 0)
        return error_failure ("str_to_composite",
                              "handler values must be explicitly allocated\n");
      else
        cv.handlervalue = NULL;
      break;

    default:
      return error_failure ("str_to_composite", "invalid data type\n");
    }

  *outcmp = cv;
  return EXIT_SUCCESS;
}



/*
 * composite_output
 *
 * Formatted output of composite values
 */
void
composite_output (FILE * fp, CompositeUnion value, CompositeDataType type)
{
  if (fp == NULL)
    return;

  switch (type)
    {
    case BOOL:
      if (value.boolvalue == TRUE)
        fprintf (fp, "%s", __TRUE_STR_);
      else if (value.boolvalue == FALSE)
        fprintf (fp, "%s", __FALSE_STR_);
      break;

    case UNSIGNED_INT:
      fprintf (fp, "%ld", value.uslgintvalue);
      break;

    case LONG_INT:
      fprintf (fp, "%ld", value.lgintvalue);
      break;

    case UNSIGNED_LONG_INT:
      fprintf (fp, "%ld", value.uslgintvalue);
      break;

    case REAL:
      fprintf (fp, "%f", value.realvalue);
      break;

    case REAL_VECTOR:
      break;

    case STRING:
      fprintf (fp, "%s", value.stringvalue);
      break;

    case HANDLER:
      fprintf (fp, "%p", value.handlervalue);
      break;
    }
}
