#ifndef __TABLE_H_
#define __TABLE_H_ 1

#include <stdio.h>
#include "../common/types.h"



/******************************************************************************
 *                                                                            *
 *                               TABLE DATATYPES                              *
 *                                                                            *
 ******************************************************************************/

/* Basic pointers */
typedef struct TableStruct *Table;      /* Table */
typedef struct ColumnStruct *Column;    /* Column */
typedef struct RowStruct *Row;  /* Row */
typedef CompositeUnion *RowValues;      /* Row Values */
typedef BoolValue *BoolValues;  /* Boolean Values */


/*
 * RowVector: vector of row values
 */
typedef struct
{
  UsLgIntValue columns;
  RowValues value;
}
RowVector;


/*
 * BoolVector: vector of booleans
 */
typedef struct
{
  UsLgIntValue columns;
  BoolValues value;
}
BoolVector;



/*
 * Table Structure
 *
 * name         : table name
 * columns      : number of columns
 * rows         : number of rows
 * first_column : pointer to the first column
 * last_column  : pointer to the last column
 * first_row    : pointer to the first row
 * last_row     : pointer to the last row
 * last_colid   : last column ID
 * last_rowid   : last row ID
 */
struct TableStruct
{
  char *name;
  UsLgIntValue columns;
  UsLgIntValue rows;
  Column first_column;
  Column last_column;
  Row first_row;
  Row last_row;
  UsLgIntValue last_colid;
  UsLgIntValue last_rowid;
};


/*
 * Column Structure
 *
 * table       : table the column is part of
 * colid       : column ID
 * name        : column name
 * type        : column data type
 * next_column : pointer to the next column
 * prev_column : pointer to the previous column
 */
struct ColumnStruct
{
  Table table;
  UsLgIntValue colid;
  char *name;
  CompositeDataType type;
  BoolValue unique;
  Column next_column;
  Column prev_column;
};


/*
 * Row Structure
 *
 * table    : table the row is part of
 * rowid    : row ID
 * next_row : pointer to the next row
 * prev_row : pointer to the previous row
 * cell     : row cells
 * null     : cell null status
 */
struct RowStruct
{
  Table table;
  UsLgIntValue rowid;
  Row next_row;
  Row prev_row;
  RowValues cell;
  BoolValues null;
};



/******************************************************************************
 *                                                                            *
 *                          DATA DEFINITION OPERATIONS                        *
 *                                                                            *
 ******************************************************************************/

/*
 * table_create
 *
 * Creates a new table
 */
extern Table table_create (const char *name);



/*
 * table_drop
 *
 * Drops the given table
 */
extern int table_drop (Table * table);



/*
 * table_describe
 *
 * Outputs information about a table
 */
extern void table_describe (FILE * fp, const Table table);



/*
 * table_add_column
 *
 * Adds a new column to the given table.
 *
 * - table must have no rows
 */
extern int
table_add_column (Table table, const char *column_name,
                  const CompositeDataType type,
                  const BoolValue unique_values);



/*
 * table_drop_column
 *
 * Drops a column, given by its name, from the given table.
 *
 * - table must have no rows
 */
extern int table_drop_column (Table table, const char *column_name);



/******************************************************************************
 *                                                                            *
 *                          DATA DEFINITION OPERATIONS                        *
 *                                                                            *
 ******************************************************************************/

/*
 * table_insert_row
 *
 * Inserts a new row into the given table
 */
extern int table_insert_row (Table table, RowVector values, BoolVector null);



#endif /* __TABLE_H_ */
