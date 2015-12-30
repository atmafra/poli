#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "../errorh/errorh.h"

#define __PROG_NAME_ "tstab"

int
main (int argc, char **argv)
{
  Table table1 = NULL;
  char *table1_name = "TABELA TESTE";
  char *column1_name = "COLUNA 1";
  char *column2_name = "COLUNA 2";
  char *column3_name = "COLUNA 3";
  char *column4_name = "COLUNA 4";


  /* cria uma tabela nova */
  table1 = table_create (table1_name);
  if (table1 == NULL)
    return error_failure (__PROG_NAME_, "erro criando tabela %s\n",
                          table1_name);

  /* cria uma coluna */
  if (table_add_column (table1, column1_name, UNSIGNED_INT, TRUE) !=
      EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro acrescentando coluna %s\n",
                          column1_name);

  /* cria uma coluna */
  if (table_add_column (table1, column2_name, STRING, FALSE) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro acrescentando coluna %s\n",
                          column2_name);


  /* cria uma coluna */
  if (table_add_column (table1, column3_name, BOOL, FALSE) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro acrescentando coluna %s\n",
                          column3_name);

  /* cria uma coluna */
  if (table_add_column (table1, column4_name, STRING, REAL) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro acrescentando coluna %s\n",
                          column4_name);

  /* dropa a coluna 3 */
  if (table_drop_column (table1, column3_name) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro dropando coluna %s\n",
                          column3_name);

  /* recria a coluna dropada */
  if (table_add_column (table1, column3_name, BOOL, FALSE) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro acrescentando coluna %s\n",
                          column3_name);

  /* cria uma nova linha */
  if (table_insert_row (table1, NULL, NULL) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_,
                          "erro inserindo nova linha na tabela '%s'\n",
                          table1->name);

  /* descreve a tabela */
  puts ("\n### DESC ###\n");
  table_describe (stdout, table1);

  /* dropa a tabela */
  if (table_drop (&table1) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro dropando tabela %s\n",
                          table1_name);

  return EXIT_SUCCESS;
}
