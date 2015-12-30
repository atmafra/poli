#ifndef __TABLE_H_
#define __TABLE_H_ 1

#include <stdio.h>

#include "types.h"


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
 * ctlfile      : table's control file
 * datafile     : table's associated data file
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
  char *ctlfile;
  char *datafile;
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
 * colnum      : colunn's order index
 * name        : column name
 * type        : column data type
 * next_column : pointer to the next column
 * prev_column : pointer to the previous column
 */
struct ColumnStruct
{
  Table table;
  UsLgIntValue colid;
  UsLgIntValue colnum;
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
 * cell     : row cells
 * null     : cell null status
 * next_row : pointer to the next row
 * prev_row : pointer to the previous row
 */
struct RowStruct
{
  Table table;
  UsLgIntValue rowid;
  RowValues cell;
  BoolValues null;
  Row next_row;
  Row prev_row;
};



/******************************************************************************
 *                                                                            *
 *                         DATA DEFINITION OPERATIONS                         *
 *                                                                            *
 ******************************************************************************/

/*
 * table_create
 *
 * Creates a new table
 */
extern Table
table_create (const char *name, const char *ctlfile, const char *datafile);



/*
 * table_drop
 *
 * Drops the given table
 */
extern int table_drop (Table * table);



/*
 * table_add_column
 *
 * Adds a new column to the given table.
 * Table must have no rows
 */
extern int
table_add_column (Table table, const char *column_name,
                  const CompositeDataType type,
                  const BoolValue unique_values);



/*
 * table_drop_column
 *
 * Drops a column, given by its name, from the given table.
 * Table must have no rows
 */
extern int table_drop_column (Table table, const char *column_name);



/*
 * table_create_from_control_file
 *
 * Creates table's columns from its control file
 */
extern Table table_create_from_control_file (const char *ctl_filename);



/******************************************************************************
 *                                                                            *
 *                          DATA MANIPULATION OPERATIONS                      *
 *                                                                            *
 ******************************************************************************/

/*
 * table_insert_row
 *
 * Inserts a new row into the given table
 */
extern int table_insert_row (Table table, RowVector values, BoolVector null);



/*
 * table_fetch
 *
 * Opens a cursor with the rows in the given table and fetches the first row
 * If already open, fetches another row
 * If fetched row is last, makes last_row TRUE
 * If *row is passed NULL, fetches first row
 */
extern void table_fetch (Table table, BoolValue * last_row, Row * row);



/*
 * table_set_cell
 *
 * Sets the composite value of one cell, given the column and row
 */
extern int
table_set_cell (Row row, Column column, CompositeUnion value, BoolValue null);



/*
 * table_get_cell
 *
 * Gets the composite value of one cell, given the column and row
 */
extern int
table_get_cell (Row row, Column column,
                CompositeUnion * value, BoolValue * null);



/*
 * table_select
 *
 * Returns the first row which has the given value in the given column
 */
extern Row table_select (Table table, Column column, CompositeUnion value);



/*
 * table_truncate
 *
 * Deletes all the rows from the given table
 */
extern int table_truncate (Table table);



/******************************************************************************
 *                                                                            *
 *                                I/O OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * table_describe
 *
 * Outputs table description
 */
extern void table_describe (FILE * fp, const Table table);



/*
 * table_read_row_vector
 *
 * Reads a row vector from an input file descriptor.
 * Cells are separated by new line characters.
 * Null values are expressed by the null symbol.
 * Row vector ends at an empty line.
 * Cell values are validated against table definition.
 */
extern int
table_read_row_vector (FILE * fp, Table table,
                       RowVector * rowvec, BoolVector * nullvec);



/*
 * table_write_control_file
 *
 * Writes table control data to its control file
 */
extern int table_write_control_file (Table table);



/*
 * table_read_data
 *
 * Reads table data from its datafile
 */
extern int table_read_data (Table table);



/*
 * table_write_data
 *
 * Writes table data to its datafile
 */
extern int table_write_data (Table table);



/*
 * table_output
 *
 * Outputs all rows of the given table to the given stream
 */
extern void table_output (FILE * fp, Table table);



#endif /* __TABLE_H_ */
