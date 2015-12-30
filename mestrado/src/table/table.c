#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "errorh.h"
#include "vector.h"
#include "strutils.h"

#include "table.h"


/******************************************************************************
 *                                                                            *
 *                              PRIVATE OPERATIONS                            *
 *                                                                            *
 ******************************************************************************/

/*
 * column_get_by_name
 *
 * Gets a column by its table and its name
 */
static Column
column_get_by_name (const Table table, const char *column_name)
{
  Column column = NULL;         /* auxiliary column */

  /* searches for a column with the same name */
  column = table->first_column;

  while (column != NULL)
    {
      if (strcmp (column_name, column->name) == 0)
        break;

      column = column->next_column;
    }

  return column;
}



/*
 * column_create
 *
 * Creates a new uninitialized column
 */
static Column
column_create (const char *name, const CompositeDataType type,
               const BoolValue unique)
{
  Column new_column;            /* auxiliary new column */


  /* allocates memory for the new column */
  new_column = (Column) malloc (sizeof (struct ColumnStruct));
  if (new_column == NULL)
    return null_failure ("column_create",
                         "can't get memory for a new column\n");

  /* allocates memory for the column name */
  new_column->name = (char *) malloc (strlen (name) + 1);
  if (new_column->name == NULL)
    {
      free (new_column);
      return null_failure ("column_create",
                           "can't get memory for new column name\n");
    }

  /* initialization */
  new_column->table = NULL;
  new_column->colid = 0;
  new_column->colnum = 0;
  strcpy (new_column->name, name);
  new_column->type = type;
  if (unique == TRUE || unique == FALSE)
    new_column->unique = unique;
  else
    {
      free (new_column);
      return null_failure ("column_create",
                           "invalid value for unique values attribute\n");
    }
  new_column->next_column = NULL;
  new_column->prev_column = NULL;

  return new_column;
}



/*
 * column_destroy
 *
 * Destroys a previously created column
 */
static int
column_destroy (Column * column)
{
  /* checks if the column is passed */
  if (column == NULL || *column == NULL)
    return error_failure ("column_destroy", "no column passed to destroy\n");

  /* checks if the column is attached to a table */
  if ((*column)->table != NULL)
    return error_failure ("column_destroy",
                          "column is attached to a table\n");

  /* destroys the column */
  if ((*column)->name != NULL)
    free ((*column)->name);

  free (*column);
  *column = NULL;

  return EXIT_SUCCESS;
}



/*
 * column_set_table
 *
 * Attaches the given column to the given table
 */
static int
column_set_table (Column column, Table table)
{
  Column last_col = NULL;       /* auxiliary last column */


  /* checks if the column was passed */
  if (column == NULL)
    return error_failure ("column_set_table", "no column passed\n");

  /* column must be free */
  if (column->table != NULL)
    return error_failure ("column_set_table",
                          "column is already attached to table %s\n",
                          column->table->name);

  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("column_set_table", "table not informed\n");

  /* checks if the new column has a name */
  if (column->name == NULL)
    return error_failure ("column_set_table", "column has no name\n");

  /* checks if the table already has a column with the same name */
  if (column_get_by_name (table, column->name) != NULL)
    return error_failure ("column_set_table",
                          "table already has a column with the same name\n");

  /* saves table's last column */
  last_col = table->last_column;

  /* attaches the column at the end of the table's column list */
  if (table->first_column == NULL)
    table->first_column = column;
  else
    table->last_column->next_column = column;

  table->last_column = column;

  /* increments table's counters */
  table->last_colid++;
  table->columns++;

  /* sets the columns attributes */
  column->table = table;
  column->colid = table->last_colid;
  column->colnum = table->columns;
  column->prev_column = last_col;
  column->next_column = NULL;

  return EXIT_SUCCESS;
}



/*
 * column_unset_table
 *
 * Removes the given column from its table
 */
static int
column_unset_table (Column column)
{
  Table table = NULL;           /* auxiliary column's table */
  Column prev_col = NULL;       /* auxiliary previous column */
  Column next_col = NULL;       /* auxiliary next column */


  /* checks if the column was passed */
  if (column == NULL)
    return error_failure ("column_unset_table", "no column passed\n");

  /* column must be attached to a table */
  if (column->table == NULL)
    return error_failure ("column_unset_table",
                          "column is not attached to a table\n");

  /* initialization */
  table = column->table;
  prev_col = column->prev_column;
  next_col = column->next_column;

  /* dettaches the column from the table */
  if (prev_col != NULL)
    column->prev_column->next_column = next_col;

  if (next_col != NULL)
    column->next_column->prev_column = prev_col;

  if (column == table->first_column)
    table->first_column = next_col;

  if (column == table->last_column)
    table->last_column = prev_col;

  /* decrements table's column counter */
  table->columns--;

  /* unsets the column's attributes */
  column->table = NULL;
  column->colid = 0;
  column->colnum = 0;
  column->prev_column = NULL;
  column->next_column = NULL;

  return EXIT_SUCCESS;
}



/*
 * column_drop
 *
 * Unsets the column's table and then destroys it
 */
static int
column_drop (Column * column)
{
  /* unsets the table */
  if (column_unset_table (*column) != EXIT_SUCCESS)
    return EXIT_FAILURE;

  /* destroys the column */
  if (column_destroy (column) != EXIT_SUCCESS)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}



/*
 * column_write_control
 *
 * Outputs column's control data
 */
static void
column_write_control (FILE * ctl_fd, const Column column)
{
  if (ctl_fd == NULL)
    return;

  if (column == NULL)
    return;

  fprintf (ctl_fd, "%s\n", column->name);
  fprintf (ctl_fd, "%s\n", type_to_str (column->type));
  fprintf (ctl_fd, "%s\n", bool_to_str (column->unique));
}



/*
 * row_create
 *
 * Creates a new row in the given table
 */
static Row
row_create (Table table)
{
  Row new_row = NULL;           /* auxiliary new row */
  UsLgIntValue cnt_col;         /* auxiliary column counter */


  /* checks if the table was passed */
  if (table == NULL)
    return null_failure ("row_create", "table not informed\n");

  /* checks if the table has any columns */
  if (table->columns == 0)
    return null_failure ("row_create", "table '%s' has no columns\n",
                         table->name);

  /* allocates the new row */
  new_row = (Row) malloc (sizeof (struct RowStruct));
  if (new_row == NULL)
    return null_failure ("row_create", "can't get memory for another row\n");

  /* initializes the new row */
  new_row->table = table;
  new_row->rowid = table->last_rowid + 1;
  new_row->next_row = NULL;
  new_row->prev_row = table->last_row;

  /* allocates the new row's cells */
  new_row->cell =
    (CompositeUnion *) malloc (table->columns * sizeof (CompositeUnion));
  if (new_row->cell == NULL)
    {
      free (new_row);
      return null_failure ("row_create",
                           "can't get memory for another row\n");
    }

  /* allocates the new row's null vector */
  new_row->null = (BoolValue *) malloc (table->columns * sizeof (BoolValue));
  if (new_row->null == NULL)
    {
      free (new_row->cell);
      free (new_row);
      return null_failure ("row_create",
                           "can't get memory for new row's nulls\n");
    }

  /* set all cells to null */
  for (cnt_col = 0; cnt_col < table->columns; cnt_col++)
    new_row->null[cnt_col] = TRUE;

  /* attaches the row at the end of the table's row list */
  if (table->first_row == NULL)
    table->first_row = new_row;
  else
    table->last_row->next_row = new_row;

  table->last_row = new_row;

  /* updates table's counters */
  table->rows++;
  table->last_rowid++;

  return new_row;
}



/*
 * row_destroy
 *
 * Frees up all memory allocated to the given row
 */
static void
row_destroy (Row row)
{
  Column cur_col = row->table->first_column;    /* auxiliary column */

  /* frees any strings allocated to the row */
  while (cur_col != NULL)
    {
      if (cur_col->type == STRING)
        if (row->cell[cur_col->colnum - 1].stringvalue != NULL)
          free (row->cell[cur_col->colnum - 1].stringvalue);

      cur_col = cur_col->next_column;
    }

  /* frees row values */
  free (row->cell);
  free (row->null);
}



/*
 * row_unset_table
 *
 * Unsets the row's table
 */
static int
row_unset_table (Row row)
{
  Table table = NULL;           /* auxiliary row's table */
  Row prev_row = NULL;          /* auxiliary previous row */
  Row next_row = NULL;          /* auxiliary next row */


  /* checks if the column was passed */
  if (row == NULL)
    return error_failure ("row_unset_table", "no row passed\n");

  /* column must be attached to a table */
  if (row->table == NULL)
    return error_failure ("row_unset_table",
                          "row is not attached to a table\n");

  /* initialization */
  table = row->table;
  prev_row = row->prev_row;
  next_row = row->next_row;

  /* dettaches the row from the table */
  if (prev_row != NULL)
    row->prev_row->next_row = next_row;

  if (next_row != NULL)
    row->next_row->prev_row = prev_row;

  if (row == table->first_row)
    table->first_row = next_row;

  if (row == table->last_row)
    table->last_row = prev_row;

  /* decrements table's column counter */
  table->rows--;

  /* unsets the column's attributes */
  row->table = NULL;
  row->rowid = 0;
  row->prev_row = NULL;
  row->next_row = NULL;

  return EXIT_SUCCESS;
}



/*
 * row_delete
 *
 * Deletes the given row from its table and then destroys it
 */
static int
row_delete (Row * row)
{
  /* checks if the row was passed */
  if (row == NULL || *row == NULL)
    return error_failure ("row_delete", "row not informed\n");

  /* frees row allocated memory */
  row_destroy (*row);

  /* dettaches from its table */
  if (row_unset_table (*row) != EXIT_SUCCESS)
    return error_failure ("row_delete",
                          "error removing row from its table\n");

  free (*row);
  *row = NULL;

  return EXIT_SUCCESS;
}



/*
 * row_set_cell_value
 *
 * Sets the value of one cell
 */
static int
row_set_cell_value (Row row, UsLgIntValue colnum,
                    CompositeDataType type, CompositeUnion value,
                    BoolValue null)
{
  /* 1's logic */
  colnum--;

  /* sets null flag */
  row->null[colnum] = null;

  /* checks if to set to null */
  if (null == FALSE)
    {
      switch (type)
        {
        case BOOL:
          row->cell[colnum].boolvalue = value.boolvalue;
          break;

        case UNSIGNED_INT:
          row->cell[colnum].usintvalue = value.usintvalue;
          break;

        case LONG_INT:
          row->cell[colnum].lgintvalue = value.lgintvalue;
          break;

        case UNSIGNED_LONG_INT:
          row->cell[colnum].uslgintvalue = value.uslgintvalue;
          break;

        case REAL:
          row->cell[colnum].realvalue = value.realvalue;
          break;

        case REAL_VECTOR:
          row->cell[colnum].rvectorvalue =
            vector_create (value.rvectorvalue->dimension);
          if (row->cell[colnum].rvectorvalue == NULL)
            return error_failure ("row_set_cell_value",
                                  "error creating vector value\n");

          if (vector_copy
              (value.rvectorvalue,
               row->cell[colnum].rvectorvalue) != EXIT_SUCCESS)
            return error_failure ("row_set_cell_value",
                                  "error setting vector value\n");
          break;

        case STRING:
          row->cell[colnum].stringvalue = value.stringvalue;
          break;

        case HANDLER:
          row->cell[colnum].handlervalue = value.handlervalue;
          break;

        default:
          return error_failure ("row_set_cell_value", "invalid data type\n");
        }
    }

  return EXIT_SUCCESS;
}



/*
 * table_input_data
 *
 * Reads table data from its datafile
 */
int
table_input_data (FILE * df, Table table)
{
  RowVector rowvec;             /* record read */
  BoolVector nullvec;           /* null vector */
  UsLgIntValue row = 1;         /* record counter */

  /* checks if table was passed */
  if (table == NULL)
    return error_failure ("table_input_data", "table not informed\n");

  /* checks if the data file descriptor was passed */
  if (df == NULL)
    return error_failure ("table_input_data", "no input stream passed\n");

  /* checks if table has columns */
  if (table->columns == 0)
    return error_failure ("table_input_data", "table %s has no columns\n",
                          table->name);

  /* allocates memory */
  rowvec.columns = table->columns;
  rowvec.value =
    (RowValues) malloc (rowvec.columns * sizeof (CompositeUnion));
  if (rowvec.value == NULL)
    return error_failure ("table_input_data",
                          "can't get memory for a new row\n");

  nullvec.columns = table->columns;
  nullvec.value = (BoolValues) malloc (nullvec.columns * sizeof (BoolValue));
  if (nullvec.value == NULL)
    return error_failure ("table_input_data",
                          "can't get memory for a new boolean vector\n");

  /* reads the rows */
  while (!feof (df))
    {
      /* reads one row from datafile */
      if (table_read_row_vector (df, table, &rowvec, &nullvec) !=
          EXIT_SUCCESS)
        return error_failure ("table_input_data",
                              "error reading row %ld for table %s\n",
                              row, table->name);

      /* inserts into the table */
      if (table_insert_row (table, rowvec, nullvec) != EXIT_SUCCESS)
        return error_failure ("table_input_data",
                              "error inserting row %ld into table %s\n",
                              row, table->name);

      row++;
    }

  /* frees auxiliary memory */
  free (rowvec.value);
  free (nullvec.value);

  return EXIT_SUCCESS;
}



/*
 * row_select_by_column
 *
 * Returns one row with the given value in the given table
 */
static Row
row_select_by_column (Column column, CompositeUnion value)
{
  Table table = NULL;           /* auxiliary column's table */
  Row cur_row = NULL;           /* auxiliary current row */
  BoolValue last_row = FALSE;   /* last row in table */
  BoolValue found = FALSE;      /* row found */


  table = column->table;

  if (column->unique == FALSE)
    return null_failure ("row_select_by_column",
                         "column doesn't have unique values\n");

  if (table->rows == 0)
    return NULL;

  while (last_row == FALSE && found == FALSE)
    {
      table_fetch (table, &last_row, &cur_row);

      if (cur_row != NULL)
        found = composite_compare (cur_row->cell[column->colnum - 1],
                                   value, column->type);
    }

  if (found == FALSE)
    return NULL;
  else
    return cur_row;
}



/*
 * row_check_duplicated_value
 *
 * Checks if there already is another row with the same value in the column
 */
static int
row_check_duplicated_value (Row row, Column column,
                            CompositeUnion value, BoolValue * dup_found)
{
  Table table = NULL;           /* auxiliary column's table */
  Row cur_row = NULL;           /* auxiliary current row */
  BoolValue last_row = FALSE;   /* last row in table */
  BoolValue found = FALSE;      /* row found */


  *dup_found = FALSE;

  /* checks parameters */
  if (column == NULL)
    return error_failure ("row_select_by_column", "column not informed\n");

  table = column->table;

  if (column->unique == FALSE)
    return error_failure ("row_select_by_column",
                          "column doesn't have unique values\n");

  if (table->rows == 0)
    return EXIT_SUCCESS;

  while (last_row == FALSE && found == FALSE)
    {
      table_fetch (table, &last_row, &cur_row);

      if (cur_row != NULL && cur_row != row)
        found = composite_compare (cur_row->cell[column->colnum - 1],
                                   value, column->type);
    }

  *dup_found = found;
  return EXIT_SUCCESS;
}



/*
 * row_set_values
 *
 * Set the row's cells values
 */
static int
row_set_values (Row row, RowVector values, BoolVector null)
{
  Table table = NULL;           /* row's table */
  Column column = NULL;         /* auxiliary column */
  BoolValue dup_row = TRUE;     /* flag: duplicated value found */
  UsLgIntValue col_count;       /* auxiliary column counter */


  /* checks if the row was passed */
  if (row == NULL)
    return error_failure ("row_set_values", "row not informed\n");

  table = row->table;

  /* checks row values against table's number of columns */
  if (values.columns != table->columns)
    return error_failure ("row_set_values", "wrong number of values\n");

  /* checks null vector against table's number of columns */
  if (null.columns != table->columns)
    return error_failure ("row_set_values", "incompatible null vector\n");

  /* columns loop */
  column = table->first_column;
  col_count = 1;

  while (column != NULL)
    {
      /* checks uniqueness */
      if (column->unique == TRUE)
        {
          if (row_check_duplicated_value (row, column,
                                          values.value[col_count - 1],
                                          &dup_row) != EXIT_SUCCESS)
            return error_failure ("row_set_values",
                                  "error checking for duplicated values\n");

          if (dup_row == TRUE)
            return error_failure ("row_set_values",
                                  "duplicated value in column '%s'\n",
                                  column->name);
        }

      /* sets the cell value */
      if (row_set_cell_value (row, col_count, column->type,
                              values.value[col_count - 1],
                              null.value[col_count - 1]) != EXIT_SUCCESS)

        return error_failure ("row_set_values",
                              "error setting column %ld value\n", col_count);

      column = column->next_column;
      col_count++;
    }


  return EXIT_SUCCESS;
}



/*
 * row_output
 *
 * Outputs the cells of the given row, one cell per line
 */
static void
row_output (FILE * fp, Row row)
{
  Table table = NULL;           /* row's table */
  Column cur_col = NULL;        /* auxiliary column */
  UsLgIntValue cur_colnum;      /* current column number */

  if (row == NULL)
    return;

  if (fp == NULL)
    return;

  table = row->table;

  if (table->columns == 0)
    return;

  /* cells loop */
  cur_col = table->first_column;
  while (cur_col != NULL)
    {
      cur_colnum = cur_col->colnum - 1;
      composite_output (fp, row->cell[cur_colnum], cur_col->type);
      fprintf (fp, "\n");
      cur_col = cur_col->next_column;
    }
}



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
Table
table_create (const char *name, const char *ctlfile, const char *datafile)
{
  Table new_table;              /* new table */

  /* must have a name */
  if (name == NULL)
    return null_failure ("table_create", "table name not informed\n");

  /* table allocation */
  new_table = (Table) malloc (sizeof (struct TableStruct));
  if (new_table == NULL)
    return null_failure ("table_create",
                         "can't get memory for a new table\n");

  /* initialization */
  new_table->name = (char *) malloc (strlen (name) + 1);

  if (new_table->name == NULL)
    return null_failure ("table_create",
                         "can't get memory for new table name\n");

  strcpy (new_table->name, name);

  if (ctlfile != NULL)
    {
      new_table->ctlfile = (char *) malloc (strlen (ctlfile) + 1);

      if (new_table->ctlfile == NULL)
        return null_failure ("table_crete",
                             "can't get memory for new table control file name\n");

      strcpy (new_table->ctlfile, ctlfile);
    }
  else
    {
      new_table->ctlfile = NULL;
    }

  if (datafile != NULL)
    {
      new_table->datafile = (char *) malloc (strlen (datafile) + 1);

      if (new_table->datafile == NULL)
        return null_failure ("table_create",
                             "can't get memory for new table datafile name\n");

      strcpy (new_table->datafile, datafile);
    }
  else
    {
      new_table->datafile = NULL;
    }

  new_table->columns = 0;
  new_table->rows = 0;
  new_table->first_column = NULL;
  new_table->last_column = NULL;
  new_table->first_row = NULL;
  new_table->last_row = NULL;
  new_table->last_colid = 0;
  new_table->last_rowid = 0;

  return new_table;
}



/*
 * table_drop
 *
 * Drops the given table
 */
int
table_drop (Table * table)
{
  Table aux_table = NULL;       /* auxiliary pointer to the table */
  Column aux_column = NULL;     /* auxiliary current column */


  /* checks if the table was passed */
  if (table == NULL || *table == NULL)
    return error_failure ("table_drop", "no table passed to drop\n");

  aux_table = *table;

  /* drops the rows */
  if (table_truncate (aux_table) != EXIT_SUCCESS)
    return error_failure ("table_drop", "error deleting table %s data\n",
                          aux_table->name);

  /* drops the columns */
  while (aux_table->columns > 0)
    {
      aux_column = aux_table->last_column;

      if (column_drop (&aux_column) != EXIT_SUCCESS)
        return error_failure ("table_drop",
                              "error dropping table's columns\n");
    }

  /* frees up memory */
  free (aux_table->name);

  if (aux_table->ctlfile != NULL)
    free (aux_table->ctlfile);

  if (aux_table->datafile != NULL)
    free (aux_table->datafile);

  free (*table);
  *table = NULL;

  return EXIT_SUCCESS;
}



/*
 * table_describe
 *
 * Outputs information about a table
 */
void
table_describe (FILE * fp, const Table table)
{
  Column cur_col = NULL;        /* auxiliary column */


  if (fp == NULL)
    return;

  if (table == NULL)
    return;

  /* table head data */
  fprintf (fp, "Table  : %s\n", table->name);
  fprintf (fp, "Columns: %ld\n", table->columns);
  fprintf (fp, "Rows   : %ld\n", table->rows);
  fprintf (fp, "Control: %s\n", table->ctlfile);
  fprintf (fp, "Data   : %s\n", table->datafile);

  if (table->columns > 0)
    fprintf (fp, "\n");

  /* columns */
  cur_col = table->first_column;
  while (cur_col != NULL)
    {
      fprintf (fp, "Column : %s\n", cur_col->name);
      fprintf (fp, "ID     : %ld\n", cur_col->colid);
      fprintf (fp, "Number : %ld\n", cur_col->colnum);
      fprintf (fp, "Type   : %s\n", type_to_str (cur_col->type));
      fprintf (fp, "Unique : %s\n", bool_to_str (cur_col->unique));

      cur_col = cur_col->next_column;

      if (cur_col != NULL)
        fprintf (fp, "\n");
    }
}



/*
 * table_add_column
 *
 * Adds a new column to the given table.
 *
 * - table must have no rows
 */
int
table_add_column (Table table, const char *column_name,
                  const CompositeDataType type, const BoolValue unique_values)
{
  Column new_column = NULL;     /* auxiliary new column */


  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_add_column", "table not passed\n");

  /* checks if the table already has rows */
  if (table->rows > 0)
    return error_failure ("table_add_column", "table must have no rows\n");

  /* checks if the column name was passed */
  if (column_name == NULL)
    return error_failure ("table_add_column", "column name not informed\n");

  /* allocates the new column */
  new_column = column_create (column_name, type, unique_values);
  if (new_column == NULL)
    return error_failure ("table_add_column", "error creating new column\n");

  /* attaches the column to the table */
  if (column_set_table (new_column, table) != EXIT_SUCCESS)
    {
      if (column_destroy (&new_column) != EXIT_SUCCESS)
        error_failure ("table_add_column", "error destroying new column\n");

      return error_failure ("table_add_column",
                            "error setting new column's table\n");
    }

  return EXIT_SUCCESS;
}



/*
 * table_drop_column
 *
 * Drops a column, given by its name, from the given table.
 *
 * - table must have no rows
 */
int
table_drop_column (Table table, const char *column_name)
{
  Column column;                /* auxiliary column */


  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_drop_column", "table not informed\n");

  /* checks if the column name was passed */
  if (column_name == NULL)
    return error_failure ("table_drop_column", "column name not informed\n");

  /* checks if the table has any rows */
  if (table->rows > 0)
    return error_failure ("table_drop_column",
                          "table '%s' must have no rows\n", table->name);

  /* searches the column by the name */
  column = column_get_by_name (table, column_name);
  if (column == NULL)
    return error_failure ("table_drop_column",
                          "table '%s' has no column '%s'\n",
                          table->name, column_name);

  /* drops the column */
  if (column_drop (&column) != EXIT_SUCCESS)
    return error_failure ("table_drop_column",
                          "error dropping column '%s' from table '%s'\n",
                          column_name, table->name);

  return EXIT_SUCCESS;
}



/*
 * table_create_from_control_file
 *
 * Creates table and its columns from the given control file
 */
Table
table_create_from_control_file (const char *ctl_filename)
{
  Table table = NULL;           /* new table */
  FILE *cf;                     /* control file descriptor */
  char line[FILE_LINE_SIZE];    /* read buffer */
  char *table_name = NULL;      /* new table's name */
  char *datafile = NULL;        /* new table's data file */
  Table col_table = NULL;       /* columns auxiliary table */
  Row col_row = NULL;           /* current row from columns table */
  BoolValue last_row = FALSE;   /* last columns table row */
  char *column_name = NULL;     /* new column's name */
  CompositeDataType type;       /* new column's type */
  BoolValue unique;             /* new column's unique */

  /* check is filename provided */
  if (ctl_filename == NULL)
    return null_failure ("table_create_from_control_file",
                         "control file name not informed\n");

  /* opens the control file */
  cf = fopen (ctl_filename, "r");
  if (cf == NULL)
    return null_failure ("table_create_from_control_file",
                         "error opening control file '%s': %s\n",
                         ctl_filename, strerror (errno));

  /* table's header data */
  if (read_valid_file_line (cf, FILE_LINE_SIZE, IGNORE_TOKEN, 0, line) !=
      EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error reading table name from file '%s'\n'",
                     ctl_filename);
      goto close_n_return;
    }

  table_name = (char *) malloc (strlen (line) + 1);
  if (table_name == NULL)
    {
      error_failure ("table_create_from_control_file",
                     "can't get memory to store table name '%s'\n", line);
      goto close_n_return;
    }
  strcpy (table_name, line);

  if (read_valid_file_line (cf, FILE_LINE_SIZE, IGNORE_TOKEN, 0, line) !=
      EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error reading table data file name from file '%s'\n'",
                     ctl_filename);
      goto close_n_return;
    }

  datafile = (char *) malloc (strlen (line) + 1);
  if (datafile == NULL)
    {
      error_failure ("table_create_from_control_file",
                     "can't get memory to store table data file name '%s'\n",
                     line);
      goto close_n_return;
    }

  strcpy (datafile, line);

  /* creates the table */
  table = table_create (table_name, ctl_filename, datafile);
  if (table == NULL)
    {
      error_failure ("table_create_from_control_file",
                     "error creating table %s from control file '%s'\n",
                     table_name, ctl_filename);
      goto close_n_return;
    }

  /* frees up auxiliary memory */
  free (table_name);
  free (datafile);
  table_name = NULL;
  datafile = NULL;

  /* reads the columns */
  col_table = table_create ("TAB_COLUMNS", NULL, NULL);
  if (col_table == NULL)
    {
      error_failure ("table_create_from_control_file",
                     "error creating internal columns table\n");
      goto close_n_return;
    }

  if (table_add_column (col_table, "COL_NAME", STRING, FALSE) != EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error adding column COL_NAME to columns table\n");
      goto close_n_return;
    }

  if (table_add_column (col_table, "COL_TYPE", STRING, FALSE) != EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error adding column COL_TYPE to columns table\n");
      goto close_n_return;
    }

  if (table_add_column (col_table, "COL_UNIQUE", BOOL, FALSE) != EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error adding column COL_UNIQUE to columns table\n");
      goto close_n_return;
    }

  if (table_input_data (cf, col_table) != EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error reading table %s's columns from control file '%s'\n",
                     table->name, ctl_filename);
      goto close_n_return;
    }

  /* creates the columns */
  while (last_row == FALSE)
    {
      table_fetch (col_table, &last_row, &col_row);

      column_name = col_row->cell[0].stringvalue;

      if (str_to_type (col_row->cell[1].stringvalue, &type) != EXIT_SUCCESS)
        {
          error_failure ("table_create_from_control_file",
                         "invalid data type for column %s in table %s\n",
                         column_name, table->name);
          goto close_n_return;
        }

      unique = col_row->cell[2].boolvalue;

      if (table_add_column (table, column_name, type, unique) != EXIT_SUCCESS)
        {
          error_failure ("table_create_from_control_file",
                         "error adding column %s to table %s\n",
                         column_name, table->name);
          goto close_n_return;
        }
    }

  /* drops columns table */
  if (table_drop (&col_table) != EXIT_SUCCESS)
    {
      error_failure ("table_create_from_control_file",
                     "error dropping columns table for table %s\n",
                     table->name);
      goto close_n_return;
    }

  if (fclose (cf) != 0)
    {
      error_failure ("table_create_from_control_file",
                     "error closing control file '%s'\n", ctl_filename);
      goto close_n_return;
    }

  return table;

close_n_return:

  if (table != NULL)
    table_drop (&table);

  if (table_name != NULL)
    free (table_name);

  if (datafile != NULL)
    free (datafile);

  if (col_table != NULL)
    table_drop (&col_table);

  fclose (cf);
  return NULL;
}



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
int
table_insert_row (Table table, RowVector values, BoolVector null)
{
  Row new_row = NULL;           /* new row */


  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_insert_row", "table not informed\n");

  /* creates the row */
  new_row = row_create (table);
  if (new_row == NULL)
    return error_failure ("table_insert_row", "error creating new row\n");

  /* sets the row's column values */
  if (row_set_values (new_row, values, null) != EXIT_SUCCESS)
    return error_failure ("table_insert_row", "error setting row's values\n");

  return EXIT_SUCCESS;
}



/*
 * table_fetch
 *
 * Opens a cursor with the rows in the given table and fetches the first row
 * If already open, fetches another row
 * If fetched row is last, makes last_row TRUE
 * If *row is passed NULL, fetches first row
 */
void
table_fetch (Table table, BoolValue * last_row, Row * row)
{
  if (table == NULL)
    {
      *row = NULL;
      return;
    }

  if (*row == NULL)
    *row = table->first_row;
  else if ((*row)->table != table)
    *row = table->first_row;
  else if (*row == table->last_row)
    *row = table->first_row;
  else
    *row = (*row)->next_row;

  if (*row == table->last_row)
    *last_row = TRUE;
  else
    *last_row = FALSE;
}



/*
 * table_set_cell
 *
 * Sets the composite value of one cell, given the column and row
 */
int
table_set_cell (Row row, Column column, CompositeUnion value, BoolValue null)
{
  BoolValue dup_found = TRUE;   /* flag: duplicated value on column */


  if (column == NULL)
    return error_failure ("table_set_cell", "column not informed\n");

  if (row == NULL)
    return error_failure ("table_set_cell", "row not informed\n");

  if (column->table != row->table)
    return error_failure ("table_set_cell", "row and column tables differ\n");

  /* checks for duplicates */
  if (column->unique == TRUE)
    {
      if (row_check_duplicated_value (row, column, value, &dup_found)
          != EXIT_SUCCESS)
        return error_failure ("table_set_cell",
                              "error checking for duplicated values on column %s\n",
                              column->name);

      if (dup_found == TRUE)
        return error_failure ("table_set_cell",
                              "duplicate value found in column %s\n",
                              column->name);
    }

  row->cell[column->colnum - 1] = value;
  row->null[column->colnum - 1] = null;

  return EXIT_SUCCESS;
}



/*
 * table_get_cell
 *
 * Gets the composite value of one cell, given the column and row
 */
int
table_get_cell (Row row, Column column,
                CompositeUnion * value, BoolValue * null)
{
  if (column == NULL)
    return error_failure ("table_get_cell", "column not informed\n");

  if (row == NULL)
    return error_failure ("table_get_cell", "row not informed\n");

  if (column->table != row->table)
    return error_failure ("table_get_cell", "row and column tables differ\n");

  *value = row->cell[column->colnum - 1];
  *null = row->null[column->colnum - 1];

  return EXIT_SUCCESS;
}



/*
 * table_select
 *
 * Returns the first row which has the given value in the given column
 */
Row
table_select (Table table, Column column, CompositeUnion value)
{
  Row row = NULL;               /* row selected */

  /* NULL checks */
  if (table == NULL)
    return null_failure ("table_select", "table not informed\n");

  if (column == NULL)
    return null_failure ("table_select", "column not informed\n");

  /* compatibility */
  if (table != column->table)
    return null_failure ("table_select", "column is from wrong table\n");

  /* fetches the row */
  row = row_select_by_column (column, value);

  return row;
}



/*
 * table_truncate
 *
 * Deletes all the rows from the given table
 */
int
table_truncate (Table table)
{
  Row cur_row = NULL;           /* current row being deleted */
  UsLgIntValue rowcount;        /* row counter */

  /* checks if table was passed */
  if (table == NULL)
    return error_failure ("table_truncate", "table not informed\n");

  /* drops the rows */
  rowcount = table->rows;
  while (table->rows > 0)
    {
      cur_row = table->last_row;

      if (row_delete (&cur_row) != EXIT_SUCCESS)
        return error_failure ("table_truncate",
                              "error deleting row %ld from table %s\n",
                              rowcount, table->name);

      rowcount--;
    }

  return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                                I/O OPERATIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * table_read_row_vector
 *
 * Reads a row vector from an input file descriptor.
 * Cells are separated by new line characters.
 * Null values are expressed by the null symbol.
 * Row vector ends at an empty line.
 */
int
table_read_row_vector (FILE * fp, Table table,
                       RowVector * rowvec, BoolVector * nullvec)
{
  RowVector rv;                 /* new row vector */
  BoolVector nv;                /* new null vector */
  char line[FILE_LINE_SIZE];    /* read buffer */
  BoolValue isnull;             /* current cell is null? */
  UsLgIntValue colcnt = 0;      /* columns read counter */
  Column cur_col = NULL;        /* current table column */


  /* checks if file descriptor was passed */
  if (fp == NULL)
    return error_failure ("table_read_row_vector", "no input file\n");

  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_read_row_vector", "table not informed\n");

  /* initialization */
  rv = *rowvec;
  nv = *nullvec;

  /* file reading loop */
  line[0] = '\n';
  cur_col = table->first_column;

  while (!feof (fp) && line[0] != '\0')
    {
      /* skip handler columns */
      if (cur_col != NULL)
        {
          if (cur_col->type == HANDLER)
            {
              colcnt++;
              cur_col = cur_col->next_column;
              continue;
            }
        }

      /* reads a line */
      if (read_file_line (fp, FILE_LINE_SIZE, line) != EXIT_SUCCESS)
        return error_failure ("table_read_row_vector",
                              "error reading file line\n");

      /* strips off CR LF characters */
      strip_nl (line);

      /* parses the line */
      if (!feof (fp) && line[0] != '\0')
        {
          if (colcnt + 1 > table->columns)
            return error_failure ("table_read_row_vector",
                                  "too many columns\n");

          /* checks null values */
          if (strcmp (line, __NULL_STR_) == 0)
            isnull = TRUE;
          else
            isnull = FALSE;

          /* sets the null vector */
          nv.value[colcnt] = isnull;

          /* sets the cell value */
          if (isnull == FALSE)
            {
              if (str_to_composite (line, cur_col->type,
                                    &(rv.value[colcnt])) != EXIT_SUCCESS)

                return error_failure ("table_read_row_vector",
                                      "error reading column '%s'\n",
                                      cur_col->name);
            }
          colcnt++;
          cur_col = cur_col->next_column;
        }
    }

  if (colcnt < table->columns)
    return error_failure ("table_read_row_vector",
                          "too few columns: %ld\n", colcnt);

  return EXIT_SUCCESS;
}



/*
 * table_read_data
 *
 * Reads table data from its datafile
 */
int
table_read_data (Table table)
{
  char *datafile = NULL;        /* data file name */
  char *ctl_file = NULL;        /* control file name */
  char *ctl_dir = NULL;         /* control file directory */
  char *ctl_base = NULL;        /* control file base name */
  char *ctl_ext = NULL;         /* control file extension */
  FILE *df = NULL;              /* datafile descriptor */

  /* checks if table was passed */
  if (table == NULL)
    return error_failure ("table_read_data", "table not informed\n");

  /* checks if table has an associated datafile */
  if (table->datafile == NULL)
    return error_failure ("table_read_data",
                          "table %s doesn't have a datafile\n", table->name);

  /* searches control file directory */
  datafile = table->datafile;
  ctl_file = table->ctlfile;

  if (ctl_file != NULL)
    {
      split_file_name (ctl_file, &ctl_dir, &ctl_base, &ctl_ext);

      if (ctl_dir != NULL)
        {
          datafile =
            (char *) malloc (strlen (ctl_dir) + strlen (table->datafile) + 1);
          sprintf (datafile, "%s%s", ctl_dir, table->datafile);
        }
    }

  /* opens the datafile */
  df = fopen (datafile, "r");
  if (df == NULL)
    return error_failure ("table_read_data",
                          "error opening %s's datafile '%s': %s\n",
                          table->name, datafile, strerror (errno));

  /* inputs data */
  if (table_input_data (df, table) != EXIT_SUCCESS)
    return error_failure ("table_read_data",
                          "error reading table %s's data\n", table->name);

  /* closes the datafile */
  if (fclose (df) != 0)
    return error_failure ("table_read_data",
                          "error closing table %s datafile: %s",
                          table->name, strerror (errno));

  if (table->datafile != datafile)
    free (datafile);

  return EXIT_SUCCESS;
}



/*
 * table_write_data
 *
 * Writes table data to its datafile
 */
int
table_write_data (Table table)
{
  FILE *df;                     /* datafile descriptor */

  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_write_data", "table not informed");

  /* checks if the table has an associated datafile */
  if (table->datafile == NULL)
    return error_failure ("table_write_data",
                          "table %s doesn't have a datafile\n");

  /* writes table data to its datafile */
  df = fopen (table->datafile, "w");
  if (df == NULL)
    return error_failure ("table_write_data",
                          "can't write table %s to datafile '%s'\n",
                          table->name, table->datafile);

  table_output (df, table);

  if (fclose (df) != 0)
    return error_failure ("table_write_data",
                          "error closing datafile '%s' for table %s: %s\n",
                          table->datafile, table->name, strerror (errno));

  return EXIT_SUCCESS;
}



/*
 * table_write_control_file
 *
 * Writes table control data to its control file
 */
int
table_write_control_file (Table table)
{
  FILE *cf;                     /* control file descriptor */
  Column col = NULL;            /* current column */

  /* checks if the table was passed */
  if (table == NULL)
    return error_failure ("table_write_control", "table not informed\n");

  /* checks if table has an associated control file */
  if (table->ctlfile == NULL)
    return error_failure ("table_write_control",
                          "table %s doesn't have a control file\n",
                          table->name);

  /* opens control file */
  cf = fopen (table->ctlfile, "w");
  if (cf == NULL)
    return error_failure ("table_write_control",
                          "error opening table %s's control file '%s': %s\n",
                          table->name, table->ctlfile, strerror (errno));

  /* outputs control data */
  fprintf (cf, "%s\n", table->name);
  fprintf (cf, "%s\n", table->datafile);
  fprintf (cf, "\n");

  col = table->first_column;
  while (col != NULL)
    {
      column_write_control (cf, col);
      col = col->next_column;
      if (col != NULL)
        fprintf (cf, "\n");
    }

  /* closes and exit */
  if (fclose (cf) != 0)
    return error_failure ("table_write_control",
                          "error closing control file '%s' for table %s: %s\n",
                          table->ctlfile, table->name, strerror (errno));

  return EXIT_SUCCESS;
}



/*
 * table_output
 *
 * Outputs all rows of the given table to the given stream
 */
void
table_output (FILE * fp, Table table)
{
  Row cur_row = NULL;           /* current output row */
  BoolValue last_row = FALSE;   /* last row flag */

  if (fp == NULL)
    return;

  if (table == NULL)
    return;

  if (table->columns == 0 || table->rows == 0)
    return;

  while (last_row == FALSE)
    {
      table_fetch (table, &last_row, &cur_row);

      row_output (fp, cur_row);

      if (last_row == FALSE)
        fprintf (fp, "\n");
    }
}
