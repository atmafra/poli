#ifndef __COMMON_TYPES_H_
#define __COMMON_TYPES_H_ 1

#include <stdio.h>

#ifndef TRUE_VALUE
#define TRUE_VALUE 1
#endif

#ifndef FALSE_VALUE
#define FALSE_VALUE 0
#endif

#ifndef NAME_SIZE
#define NAME_SIZE 64
#endif

#ifndef FILE_NAME_SIZE
#define FILE_NAME_SIZE FILENAME_MAX
#endif

#ifndef FILE_LINE_SIZE
#define FILE_LINE_SIZE 2048
#endif

#ifndef IGNORE_TOKEN
#define IGNORE_TOKEN '#'
#endif

#define __TRUE_STR_ "TRUE"
#define __FALSE_STR_ "FALSE"

#define __BOOL_STR_ "BOOL"
#define __UNSIGNED_INT_STR_ "UNSIGNED_INT"
#define __LONG_INT_STR_ "LONG_INT"
#define __UNSIGNED_LONG_INT_STR_ "UNSIGNED_LONG_INT"
#define __REAL_STR_ "REAL"
#define __REAL_VECTOR_STR_ "REAL_VECTOR"
#define __STRING_STR_ "STRING"
#define __HANDLER_STR_ "HANDLER"
#define __NULL_STR_ "(null)"

#define sqr(z) ((z) * (z))

/* Boolean values */
typedef enum
{
  TRUE = TRUE_VALUE,
  FALSE = FALSE_VALUE
}
BoolValue;

/* Default sized strings */
typedef char Name[NAME_SIZE + 1];
typedef char FileName[FILE_NAME_SIZE + 1];

/* Unsigned integers */
typedef unsigned int UsIntValue;

/* Signed long integers */
typedef long int LgIntValue;

/* Unsigned long integers */
typedef unsigned long int UsLgIntValue;

/* Real values */
typedef double RValue;

/* Generic pointer */
typedef void *Handler;

/* Complex values */
typedef struct
{
  RValue re;
  RValue im;
}
CValue;

/* Vector of Real values */
typedef struct
{
  UsLgIntValue dimension;
  RValue *value;
}
vector_type;

typedef vector_type *Vector;


/* Composite value */
typedef enum
{
  BOOL,
  UNSIGNED_INT,
  LONG_INT,
  UNSIGNED_LONG_INT,
  REAL,
  REAL_VECTOR,
  STRING,
  HANDLER
}
CompositeDataType;

typedef union
{
  BoolValue boolvalue;
  UsIntValue usintvalue;
  LgIntValue lgintvalue;
  UsLgIntValue uslgintvalue;
  RValue realvalue;
  Vector rvectorvalue;
  char *stringvalue;
  Handler handlervalue;
}
CompositeUnion;

typedef struct
{
  CompositeDataType datatype;
  CompositeUnion value;
}
CompositeValue;


/******************************************************************************
 *                                                                            *
 *                         TYPE MANIPULATION FUNCTIONS                        *
 *                                                                            *
 ******************************************************************************/

/*
 * str_to_bool
 *
 * Converts a string into its corresponding boolean value
 */
extern int str_to_bool (const char *str, BoolValue * bool);



/*
 * bool_to_str
 *
 * Converts a boolean value into its corresponding string
 */
extern char *bool_to_str (const BoolValue bool);



/*
 * type_to_str
 *
 * Converts a type to its identifying string
 */
extern char *type_to_str (const CompositeDataType type);



/*
 * str_to_type
 *
 * Returns the composite data type corresponding to the given string
 */
extern int str_to_type (const char *str, CompositeDataType * type);



/*
 * composite_compare
 *
 * Compares two composite values, given their data type
 */
extern BoolValue
composite_compare (const CompositeUnion c1, const CompositeUnion c2,
                   const CompositeDataType type);



/*
 * str_to_composite
 *
 * Converts a string into a composite value according to the type
 */
extern int
str_to_composite (char *instr, CompositeDataType type,
                  CompositeUnion * outcmp);



/*
 * composite_output
 *
 * Formatted output of composite values
 */
extern void
composite_output (FILE * fp, CompositeUnion value, CompositeDataType type);



#endif /* __COMMON_TYPES_H_ */
