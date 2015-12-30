#include <stdio.h>
#include <stdlib.h>

#include "errorh.h"
#include "inparse.h"

int
main (int argc, char **argv)
{
  /* conjunto de parametros */
  InputParameterSet teste_pset = {
    {"-i", "--input", STRING, FALSE, FALSE, {.stringvalue = "input.txt"}},
    {"-o", "--output", STRING, FALSE, FALSE, {.stringvalue = "output.txt"}},
    {"-n", "--network", STRING, FALSE, TRUE, {.stringvalue = "network.txt"}},
    {"-b", "--boolean", BOOL, FALSE, FALSE, {.boolvalue = FALSE}},
    {"-ui", "--unsigned-int", UNSIGNED_INT, FALSE, FALSE, {.usintvalue = 0}},
    {"-li", "--long-int", LONG_INT, FALSE, FALSE, {.lgintvalue = 0}},
    {"-ul", "--unsigned-long-int", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 0}},
    {"-r", "--real", REAL, FALSE, FALSE, {.realvalue = 1.0}},
  };

  /* lista de parametros */
  InputParameterList teste_plist = { 8, teste_pset };

  /* decodifica a chamada do programa */
  if (inpr_parse (argc, argv, teste_plist) != EXIT_SUCCESS)

    return error_failure (argv[0],
                          "erro decodificando parametros da linha de comando\n");

  /* imprime a lista de parametros */
  inpr_list_info (teste_plist, stdout);

  return EXIT_SUCCESS;
}
