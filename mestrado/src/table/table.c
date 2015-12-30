#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "../errorh/errorh.h"
#include "../vector/vector.h"



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
    {
      error_failure ("column_create", "can't get memory for a new column\n");
      return NULL;
    }

  /* allocates memory for the column name */
  new_column->name = (char *) malloc (strlen (name) + 1);
  if (new_column->name == NULL)
    {
      free (new_column);
      error_failure ("column_create",
                     "can't get memory for new column name\n");
      return NULL;
    }

  /* initialization */
  new_column->table = NULL;
  new_column->colid = 0;
  strcpy (new_column->name, name);
  new_column->type = type;
  new_column->unique = unique;
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
  table->columns++;
  table->last_colid++;

  /* sets the columns attributes */
  column->table = table;
  column->colid = table->last_colid;
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
    {
      error_failure ("row_create", "table not informed\n");
      return NULL;
    }

  /* checks if the table has any columns */
  if (table->columns == 0)
    {
      error_failure ("row_create", "table '%s' has no columns\n",
                     table->name);
      return NULL;
    }

  /* allocates the new row */
  new_row = (Row) malloc (sizeof (struct RowStruct));
  if (new_row == NULL)
    {
      error_failure ("row_create", "can't get memory for another row\n");
      return NULL;
    }

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
      error_failure ("row_create", "can't get memory for another row\n");
      return NULL;
    }

  /* allocates the new row's null vector */
  new_row->null = (BoolValue *) malloc (table->columns * sizeof (BoolValue));
  if (new_row->null == NULL)
    {
      free (new_row->cell);
      free (new_row);
      error_failure ("row_create", "can't get memory for new row's nulls\n");
      return NULL;
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
 * row_set_cell_value
 *
 * Sets the value of one cell
 */
static int
row_set_cell_value (Row row, UsLgIntValue colnum,
                    CompositeDataType type, CompositeUnion value)
{
  /* 1's logic */
  colnum--;

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

      if (vector_copy (value.rvectorvalue, row->cell[colnum].rvectorvalue) !=
          EXIT_SUCCESS)
        return error_failure ("row_set_cell_value",
                              "error setting vector value\n");
      break;

    case STRING:
      row->cell[colnum].stringvalue = (char *)
        realloc (row->cell[colnum].stringvalue,
                 strlen (value.stringvalue) + 1);

      if (row->cell[colnum].stringvalue == NULL)
        return error_failure ("row_set_cell_value",
                              "can't get memory to create string cell\n");

      strcpy (row->cell[colnum].stringvalue, value.stringvalue);
      break;

    default:
      return error_failure ("row_set_cell_value", "invalid data type\n");
    }

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
  UsLgIntValue col_count;       /* auxiliary column counter */


  /* checks if the row was passed */
  if (row == NULL)
    return error_failure ("row_set_values", "row not informed\n");

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
      /* sets the cell value */
      if (row_set_cell_value
          (row, col_count, column->type, values.value[col_count])
          != EXIT_SUCCESS)
        return error_failure ("row_set_values",
                              "error setting column %ld value\n", col_count);

      column = column->next_column;
      col_count++;
    }


  return EXIT_SUCCESS;
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
table_create (const char *name)
{
  Table new_table;              /* new table */

  /* must have a name */
  if (name == NULL)
    {
      error_failure ("table_create", "table name not informed\n");
      return NULL;
    }

  /* table allocation */
  new_table = (Table) malloc (sizeof (struct TableStruct));
  if (new_table == NULL)
    {
      error_failure ("table_create", "can't get memory for a new table\n");
      return NULL;
    }

  /* initialization */
  new_table->name = (char *) malloc (strlen (name) + 1);
  if (new_table->name == NULL)
    {
      error_failure ("table_create", "can't get memory for new table name\n");
      return NULL;
    }
  sprintf (new_table->name, "%s", name);

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

  if (table->columns > 0)
    fprintf (fp, "\n");

  /* columns */
  cur_col = table->first_column;
  while (cur_col != NULL)
    {
      fprintf (fp, "Column : %s\n", cur_col->name);
      fprintf (fp, "ID     : %ld\n", cur_col->colid);
      fprintf (fp, "Type   : ");
      switch (cur_col->type)
        {
        case BOOL:
          fprintf (fp, "BOOL");
          break;

        case UNSIGNED_INT:
          fprintf (fp, "UNSIGNED_INT");
          break;

        case LONG_INT:
          fprintf (fp, "LONG_INT");
          break;

        case UNSIGNED_LONG_INT:
          fprintf (fp, "UNSIGNED_LONG_INT");
          break;

        case REAL:
          fprintf (fp, "REAL");
          break;

        case REAL_VECTOR:
          fprintf (fp, "REAL_VECTOR");
          break;

        case STRING:
          fprintf (fp, "STRING");
          break;

        default:
          fprintf (fp, "UNKNOWN");
        }
      fprintf (fp, "\n");
      fprintf (fp, "Unique : %s\n", cur_col->unique == TRUE ? "YES" : "NO");

      cur_col = cur_col->next_column;

      if (cur_col != NULL)
        fprintf (fp, "\n");
    }

  return;
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
                          "table '%s' has no column '%s'\n", table->name,
                          column_name);

  /* drops the column */
  if (column_drop (&column) != EXIT_SUCCESS)
    return error_failure ("table_drop_column",
                          "error dropping column '%s' from table '%s'\n",
                          column_name, table->name);

  return EXIT_SUCCESS;
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
