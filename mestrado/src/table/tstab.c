#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "table.h"
#include "../errorh/errorh.h"

#define __PROG_NAME_ "tstab"

int
main (int argc, char **argv)
{
  Table spk_table = NULL;
  char *spk_table_ctlfile = "teste/speakers.ctl";
  Table phr_table = NULL;
  char *phr_table_ctlfile = "teste/phrases.ctl";

  /* cria a tabela de locutores pelo control file */
  spk_table = table_create_from_control_file (spk_table_ctlfile);
  if (spk_table == NULL)
    return error_failure (__PROG_NAME_, "erro criando tabela de locutores\n");

  /* cria a tabela de frases pelo control file */
  phr_table = table_create_from_control_file (phr_table_ctlfile);
  if (phr_table == NULL)
    return error_failure (__PROG_NAME_, "erro criando tabela de frases\n");

  /* le os data files */
  if (table_read_data (spk_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro lendo os dados da tabela %s\n",
                          spk_table->name);

  if (table_read_data (phr_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro lendo os dados da tabela %s\n",
                          phr_table->name);

  /* mostra as duas tabelas */
  table_output (stdout, spk_table);
  table_output (stdout, phr_table);

  /* dropa as tabelas */
  if (table_drop (&spk_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro dropando tabela %s\n",
                          spk_table->name);

  if (table_drop (&phr_table) != EXIT_SUCCESS)
    return error_failure (__PROG_NAME_, "erro dropando tabela %s\n",
                          phr_table->name);

  return EXIT_SUCCESS;
}
