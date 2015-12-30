#ifndef __TYPES_H_
#define __TYPES_H_ 1

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

#ifndef IGNORE_TOKEN
#define IGNORE_TOKEN '#'
#endif

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
  STRING
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
}
CompositeUnion;

typedef struct
{
  CompositeDataType datatype;
  CompositeUnion value;
}
CompositeValue;


#endif /* __TYPES_H_ */
