#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "errorh.h"

#include "nnet_nnet.h"
#include "nnet_types.h"
#include "nnet_files.h"
#include "nnet_files_nnet.h"

int
main (int argc, char *argv[])
{
  NNetwork nnet = NULL;         /* neural network created */
  char *in_fname = NULL;        /* network configuration file name */
  char *out_fname = NULL;       /* output file name */
  FILE *in_fd = stdin;          /* input file descriptor */
  FILE *out_fd = stdout;        /* output file descriptor */


  /* Define os arquivos de entrada e saida */
  if (argc > 1)
    in_fname = argv[1];

  if (argc > 2)
    out_fname = argv[2];

  /* Cria a rede pelo arquivo de configuracao */
  if (in_fname != NULL)
    {
      in_fd = fopen (in_fname, "r");

      if (in_fd == NULL)
        return error_failure (argv[0], "erro abrindo arquivo '%s': %s\n",
                              in_fname, strerror (errno));
    }

  if (in_fname != NULL)
    printf ("Criando rede pelo arquivo '%s'\n", in_fname);

  nnet = nnet_file_create_nnetwork (in_fd);

  if (nnet == NULL)
    {
      if (in_fd != stdin)
        fclose (in_fd);

      return error_failure (argv[0], "deu pau criando a rede pelo arquivo\n");
    }

  if (in_fd != stdin)
    fclose (in_fd);

  /* Mostra a rede que foi criada */
  puts ("Escrevendo novo arquivo de rede...");

  if (out_fname != NULL)
    {
      out_fd = fopen (out_fname, "w");

      if (out_fd == NULL)
        return error_failure (argv[0], "erro criando o arquivo '%s': %s\n",
                              out_fname, strerror (errno));
    }

  nnet_file_write_nnetwork (nnet, TRUE, TRUE, TRUE, TRUE, TRUE, out_fd);

  if (out_fd != stdout)
    fclose (out_fd);

  /* Destroi a rede */
  puts ("Destruindo rede...");
  nnet_nnetwork_destroy (&nnet, TRUE, TRUE, TRUE, TRUE);

  return EXIT_SUCCESS;
}
