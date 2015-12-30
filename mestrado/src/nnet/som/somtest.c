#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "nnet_som.h"
#include "../nnet_types.h"
#include "../nnet_nnet.h"
#include "../nnet_actv.h"
#include "../nnet_weights.h"
#include "../nnet_sets.h"
#include "../nnet_train.h"
#include "../nnet_files.h"
#include "../nnet_files_nnet.h"
#include "../../vector/vector.h"
#include "../../trmap/trmap.h"
#include "../../inparse/inparse.h"

int
main (int argc, char **argv)
{
  char *preffix = NULL;         /* file names preffix */
  size_t preffix_length;        /* preffix string length */
  char *net_fext = ".net";      /* network file extension */
  char *set_fext = ".set";      /* test set file extension */
  char *tm_fext = ".tm";        /* transition map file extension */
  char *sl_fext = ".sl";        /* status list file extension */
  char *net_fname = NULL;       /* input file name */
  char *set_fname = NULL;       /* training set file name */
  char *tm_fname = NULL;        /* transition map file name */
  char *sl_fname = NULL;        /* status list file name */
  FILE *net_fd = NULL;          /* input file descriptor */
  FILE *tm_fd = NULL;           /* transition map file descriptor */
  FILE *sl_fd = NULL;           /* status list file descriptor */
  BoolValue tm_flag = FALSE;    /* flag: output transition map to file */
  BoolValue sl_flag = FALSE;    /* flag: output status list to file */

  NNetwork nnet = NULL;         /* neural network created */
  SomNNetwork som_nnet = NULL;  /* rede neural do tipo SOM */
  SomAttributes som_attr = NULL;        /* atributos da SOM */
  UnitIndex input_dim;          /* dimensao da camada de entrada */
  UnitIndex output_dim;         /* dimensao da camada de saida */

  TSet t_set = NULL;            /* conjunto de treinamento */
  LRateFunction lrate_function = NULL;  /* learning rate function */
  DTime max_epocas;             /* limite de epocas de treinamento */

  TElement cur_element = NULL;  /* elemento sendo treinado */
  TElement cur_element2 = NULL; /* elemento sendo treinado */
  RValue etha;                  /* taxa de aprendizado atual */
  Unit winner = NULL;           /* unidade vencedora atual */
  TMState s1, s2;               /* indices das ultimas unidades vencedoras */
  DTime t;                      /* epoca atual */

  Vector slist = NULL;          /* lista de estados */
  TransitionMap trmap = NULL;   /* mapa de transicao de estados */

  int exit_status;              /* status de retorno de funcao */

  /* command line parameters */
  InputParameterSet pset = {
    {"-fp", "--file-preffix", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-n", "--network-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-s", "--training-set-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-tm", "--transition-map-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-sl", "--status-list-file", STRING, FALSE, FALSE,
     {.stringvalue = (char *) NULL}},
    {"-e", "--max-epochs", UNSIGNED_LONG_INT, FALSE, FALSE,
     {.uslgintvalue = 1000}},
  };

  InputParameterList plist = { 6, pset };




/******************************************************************************
 *                                 PARAMETROS                                 *
 ******************************************************************************/

  /* Decodifica a linha de comando */
  exit_status = inpr_parse (argc, argv, plist);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro decodificando a linha de comando\n");
      return EXIT_FAILURE;
    }

  if (plist.parameter[0].passed == TRUE)
    {
      preffix = plist.parameter[0].value.stringvalue;
      preffix_length = strlen (preffix);

      net_fname = (char *) malloc (preffix_length + strlen (net_fext) + 1);
      set_fname = (char *) malloc (preffix_length + strlen (set_fext) + 1);
      tm_fname = (char *) malloc (preffix_length + strlen (tm_fext) + 1);
      sl_fname = (char *) malloc (preffix_length + strlen (sl_fext) + 1);

      if (net_fname == NULL || set_fname == NULL || tm_fname == NULL
          || sl_fname == NULL)
        {
          fprintf (stderr, "erro alocando nomes de arquivos pelo prefixo\n");
          return EXIT_FAILURE;
        }

      /* nomes padrao dos arquivos segundo o prefixo */
      sprintf (net_fname, "%s%s", preffix, net_fext);
      sprintf (set_fname, "%s%s", preffix, set_fext);
      sprintf (tm_fname, "%s%s", preffix, tm_fext);
      sprintf (sl_fname, "%s%s", preffix, sl_fext);

      tm_flag = TRUE;
      sl_flag = TRUE;
    }

  /* nome do arquivo de configuracao da rede */
  if (plist.parameter[1].passed == TRUE)
    net_fname = plist.parameter[1].value.stringvalue;

  /* nome do arquivo do conjunto de treinamento */
  if (plist.parameter[2].passed == TRUE)
    set_fname = plist.parameter[2].value.stringvalue;

  /* nome do arquivo do mapa de transicao de estados */
  if (plist.parameter[3].passed == TRUE)
    {
      tm_fname = plist.parameter[3].value.stringvalue;
      tm_flag = TRUE;
    }

  /* nome do arquivo da lista de estados */
  if (plist.parameter[4].passed == TRUE)
    {
      sl_fname = plist.parameter[4].value.stringvalue;
      sl_flag = TRUE;
    }
  max_epocas = (DTime) plist.parameter[5].value.uslgintvalue;



/******************************************************************************
 *                                  CRIACAO                                   *
 ******************************************************************************/

  /* Cria a rede pelo arquivo de configuracao */
  if (net_fname != NULL)
    {
      net_fd = fopen (net_fname, "r");
      if (net_fd == NULL)
        {
          fprintf (stderr, strerror (errno));
          return EXIT_FAILURE;
        }
    }

  if (net_fname != NULL)
    printf ("Criando rede pelo arquivo '%s'\n", net_fname);

  nnet = nnet_file_create_nnetwork (net_fd);

  if (nnet == NULL)
    {
      fprintf (stderr, "deu pau criando a rede pelo arquivo\n");
      if (net_fd != stdin)
        fclose (net_fd);
      return EXIT_FAILURE;
    }

  /* Inicializacao de variaveis internas */
  som_nnet = (SomNNetwork) nnet->extension;
  som_attr = (SomAttributes) som_nnet->attr;
  lrate_function = som_attr->lrate_function;
  input_dim = nnet->first_layer->nu_units;
  output_dim = nnet->last_layer->nu_units;

  /* Cria o conjunto de teste */
  printf ("Criando o conjunto de teste pelo arquivo '%s'\n", set_fname);

  t_set =
    nnet_tset_create_from_file ("Conjunto de Treinamento", input_dim, 0,
                                FALSE, FALSE, TRUE, TRUE, FALSE, set_fname);

  if (t_set == NULL)
    {
      fprintf (stderr, "deu pau criando o conjunto de treinamento \n");
      return EXIT_FAILURE;
    }

/******************************************************************************
 *                                TREINAMENTO                                 *
 ******************************************************************************/

  /* Treina a rede */
  printf ("Treinando a SOM (%ld épocas)...", max_epocas);

  for (t = 0; t < max_epocas; t++)
    {
      /* Calcula a taxa de aprendizado atual */
      etha = nnet_train_lrate_value (lrate_function, (RValue) t);

      /* Atualiza o tempo da funcao de vizinhanca */
      exit_status =
        nnet_som_ngb_set_parameter (som_attr->ngb_function, 0, (RValue) t);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "deu pau atualizando o tempo na ngb_function\n");
          return EXIT_FAILURE;
        }

      /* Varre todos os elementos do conjunto */
      cur_element = t_set->first_element;

      while (cur_element != NULL)
        {
          /* treina o elemento atual */
          exit_status = nnet_som_train_element (som_nnet, cur_element, etha);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr, "deu pau treinando o elemento\n");
              return EXIT_FAILURE;
            }

          /* vai para o proximo elemento */
          cur_element = cur_element->next;
        }
    }

  puts ("concluído");


/******************************************************************************
 *                     MAPA DE TRANSICAO E LISTA DE ESTADOS                   *
 ******************************************************************************/

  /* Cria a lista de estados */
  if (sl_flag == TRUE)
    {
      slist = vector_create (t_set->nu_elements);
      if (slist == NULL)
        {
          fprintf (stderr, "erro criando a lista de estados\n");
          return EXIT_FAILURE;
        }
    }

  /* Cria o mapa de transicao de estados */
  if (tm_flag == TRUE)
    {
      trmap = trmap_create (output_dim, 0.0);
      if (trmap == NULL)
        {
          fprintf (stderr, "erro criando mapa de transicao de estados\n");
          return EXIT_FAILURE;
        }
      /* Inicializa os indices das unidades vencedoras */
      s1 = 0;
      s2 = 0;
    }

  /* Varre as amostras */
  if (sl_flag == TRUE || tm_flag == TRUE)
    {
      cur_element2 = t_set->first_element;

      while (cur_element2 != NULL)
        {
          exit_status =
            nnet_som_propagate_element (som_nnet, cur_element2, &winner);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr, "erro processando elemento %ld\n",
                       cur_element2->element_index);
              return EXIT_FAILURE;
            }

          /* registra a transicao no mapa */
          if (tm_flag == TRUE)
            {
              s2 = winner->unit_index;

              if (cur_element2 != t_set->first_element)
                {
                  exit_status = trmap_transition (trmap, s1, s2);
                  if (exit_status != EXIT_SUCCESS)
                    {
                      fprintf (stderr,
                               "erro apontando transicao de %ld para %ld\n",
                               s1, s2);
                      return EXIT_FAILURE;
                    }
                }
              s1 = s2;
            }

          /* registra o estado na lista */
          if (sl_flag == TRUE)
            {
              exit_status =
                vector_set_value (slist, cur_element2->element_index,
                                  (RValue) winner->unit_index);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr, "erro registrando estado %ld na lista\n",
                           cur_element2->element_index);
                  return EXIT_FAILURE;
                }
            }

          cur_element2 = cur_element2->next;
        }

      /* Escreve o arquivo do mapa de transicao de estados */
      if (tm_flag == TRUE)
        {
          printf ("Gerando mapa de transição no arquivo '%s'\n", tm_fname);
          tm_fd = fopen (tm_fname, "w");
          matrix_raw_info (trmap, tm_fd);
          fclose (tm_fd);

          /* Destroi o mapa de transicao de estados */
          exit_status = trmap_destroy (&trmap);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "erro destruindo mapa de transicao de estados\n");
              return EXIT_FAILURE;
            }
        }

      /* Escreve o arquivo de lista de estados */
      if (sl_flag == TRUE)
        {
          printf ("Gerando lista de estados no arquivo '%s'\n", sl_fname);
          sl_fd = fopen (sl_fname, "w");
          vector_raw_info (slist, sl_fd);
          fclose (sl_fd);

          /* Destroi a lista de estados */
          exit_status = vector_destroy (&slist);
          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr, "erro destruindo a lista de estados\n");
              return EXIT_FAILURE;
            }
        }
    }

/******************************************************************************
 *                                DESTRUICAO                                  *
 ******************************************************************************/

  /* Destroi a SOM */
  exit_status = nnet_nnetwork_destroy (&nnet, TRUE, TRUE, TRUE, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destruindo a SOM\n");
      return EXIT_FAILURE;
    }

  /* Destroi o conjunto de treinamento */
  exit_status = nnet_tset_destroy (&t_set, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destuindo o conjunto de treinamento\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
