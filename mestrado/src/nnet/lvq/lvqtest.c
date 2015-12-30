#include <stdio.h>
#include <stdlib.h>
#include "../../common/types.h"

int
main (int argc, char **argv)
{
  UsLgIntValue dim = 2;
  Vector x = NULL;
  Vector w1 = NULL;
  Vector w2 = NULL;
  int exit_status;

  x = vector_create (dim);
  w1 = vector_create (w1);
  w2 = vector_create (w2);

  if (x == NULL || w1 == NULL || w2 == NULL)
    {
      fprintf (stderr, "Erro criando os vetores de teste\n");
      return EXIT_FAILURE;
    }
