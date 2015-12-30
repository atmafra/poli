#include <stdio.h>
#include <stdlib.h>
#include "strutils.h"

#define __MAX_SIZE_ 1024

int
main (void)
{
  char *filename = "testdir/teste.txt";
  char *extension = NULL;
  char *basename = NULL;
  FILE *input_fd;
  const char token = '#';
  char line[__MAX_SIZE_];
  long cnt;
  long min = 1;
  long max = 10000000;
  int exit_status;


  /* Abre o arquivo */
  input_fd = fopen (filename, "r");
  printf ("arquivo de entrada: %s\n", filename);
  extension = get_file_extension (filename);
  printf ("extensao: %s\n", extension);
  basename = get_file_basename (filename, extension);
  printf ("basename: %s\n", basename);
  free (extension);
  free (basename);

  if (input_fd == NULL)
    {
      fprintf (stderr, "deu pau abrindo o arquivo\n");
      return EXIT_FAILURE;
    }

  /* Mostra as linhas validas */
  while (!feof (input_fd))
    {
      exit_status = read_valid_file_line (input_fd, __MAX_SIZE_, token, line);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "deu pau lendo linha\n");
          return EXIT_FAILURE;
        }

      if (!feof (input_fd))
        printf ("%s\n", line);
    }

  /* Fecha o arquivo */
  fclose (input_fd);

  /* Teste de progress bar */
  for (cnt = min; cnt <= max; cnt++)
    {
      exit_status = display_progress (min, max, cnt, 50, '#', EXIT_FAILURE);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "deu pau mostrando a barra de progresso\n");
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}
