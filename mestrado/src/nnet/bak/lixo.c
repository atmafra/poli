#include <stdio.h>
#include <stdlib.h>
#include "nnet_types.h"
#include "nnet_nnet.h"
#include "nnet_actv.h"
#include "nnet_layers.h"
#include "nnet_units.h"

int
main ()
{
  int exit_status;

  NNetwork rede;

  Layer input_layer;
  Layer hidden_layer;
  Layer output_layer;

  Unit unidade_entrada;

  nnet_unit_index_type i;
  nnet_layer_index_type j;

/******************************************************************************/

  nnet_actv_init_all_classes ();

  /*
     nnet_actv_class_info (nnet_actv_class_passthrough, stdout);
     puts ("");
     nnet_actv_class_info (nnet_actv_class_threshold, stdout);
     puts ("");
     nnet_actv_class_info (nnet_actv_class_linear, stdout);
     puts ("");
     nnet_actv_class_info (nnet_actv_class_pcwlinear, stdout);
     puts ("");
     nnet_actv_class_info (nnet_actv_class_sigmoidal, stdout);
     puts ("");
     nnet_actv_class_info (nnet_actv_class_tanh, stdout);
     puts ("");
   */

/******************************************************************************/

  nnet_layer_init_all_classes ();

  /*
     nnet_layer_class_info (nnet_layer_class_input, stdout);
     puts ("");
     nnet_layer_class_info (nnet_layer_class_hidden, stdout);
     puts ("");
     nnet_layer_class_info (nnet_layer_class_output, stdout);
     puts ("");
   */

/******************************************************************************/

  rede = nnet_nnetwork_create ("Rede Teste");

  /*
     puts ("");
     nnet_nnetwork_info (*rede, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  input_layer =
    nnet_layer_create (rede, NULL, &nnet_layer_class_input, "Entrada");

  hidden_layer =
    nnet_layer_create (rede, NULL, &nnet_layer_class_hidden, "Oculta");

  output_layer =
    nnet_layer_create (rede, NULL, &nnet_layer_class_output, "Saída");

  /*
     puts ("");
     printf ("input = %p\n", input_layer);
     printf ("hidden = %p\n", hidden_layer);
     printf ("output = %p\n", output_layer);
     puts ("");
   */

  /*
     puts ("");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  for (i = 0; i < 4; i++)
    {
      unidade_entrada =
        nnet_unit_create (input_layer, NULL, NULL, NULL, FALSE, TRUE, TRUE);
      if (unidade_entrada == NULL)
        {
          fprintf (stderr, "deu pau criando a unidade\n");
          return EXIT_FAILURE;
        }
    }

  /*
     puts ("");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  i = 3;

  unidade_entrada =
    nnet_unit_create (input_layer, &i, NULL, NULL, FALSE, TRUE, TRUE);

  if (unidade_entrada == NULL)
    {
      fprintf (stderr, "deu pau criando a unidade\n");
      return EXIT_FAILURE;
    }

  /*
     puts ("#####  ANTES  #####");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

  exit_status = nnet_unit_destroy (&unidade_entrada);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destruindo a unidade\n");
      return EXIT_FAILURE;
    }

  /*
     puts ("#####  DEPOIS  #####");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  i = 3;

  unidade_entrada =
    nnet_unit_create (input_layer, &i, NULL, NULL, FALSE, TRUE, TRUE);

  if (unidade_entrada == NULL)
    {
      fprintf (stderr, "deu pau criando a unidade\n");
      return EXIT_FAILURE;
    }

  /*
     puts ("#####  ANTES  #####");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

  exit_status =
    nnet_unit_attach_to_layer (unidade_entrada, hidden_layer, NULL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau trocando de camada\n");
      return EXIT_FAILURE;
    }

  /*
     puts ("#####  DEPOIS  #####");
     nnet_nnetwork_info (*rede, TRUE, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  /*
     printf ("hidden layer antes = %p\n", hidden_layer);
   */

  exit_status = nnet_layer_destroy (&hidden_layer, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "nao conseguiu destruir a camada\n");
      return EXIT_FAILURE;
    }

  /*
     printf ("hidden layer depois = %p\n", hidden_layer);
   */

  /*
     puts ("");
     nnet_nnetwork_info (rede, TRUE, TRUE, stdout);
     puts ("");
   */

/******************************************************************************/

  unidade_entrada =
    nnet_unit_create (NULL, NULL, &nnet_actv_class_sigmoidal, NULL, FALSE,
                      FALSE, TRUE);

  if (unidade_entrada == NULL)
    {
      fprintf (stderr, "deu pau criando a unidade\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  exit_status =
    nnet_unit_attach_to_layer (unidade_entrada, input_layer, NULL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau trocando de camada\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  exit_status = nnet_unit_remove_from_layer (unidade_entrada);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau tirando da camada\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  i = 3;
  exit_status = nnet_unit_attach_to_layer (unidade_entrada, input_layer, &i);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau colocando na posicao 3 da entrada\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  i = 4;
  exit_status = nnet_unit_attach_to_layer (unidade_entrada, input_layer, &i);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau colocando na posicao 4 da entrada\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  exit_status = nnet_layer_remove_from_nnetwork (output_layer);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau tirando a camada de saida da rede\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  j = 1;
  exit_status = nnet_layer_attach_to_nnetwork (output_layer, rede, &j);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "deu pau grudando de volta a camada de saida na rede\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  j = 2;
  exit_status = nnet_layer_attach_to_nnetwork (output_layer, rede, &j);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "deu pau grudando de volta a camada de saida na rede\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/
  puts ("");
  nnet_nnetwork_info (rede, TRUE, TRUE, stdout);
  puts ("");

  exit_status = nnet_nnetwork_destroy (&rede, TRUE, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destruindo a rede\n");
      return EXIT_FAILURE;
    }

  puts ("");
  nnet_nnetwork_info (rede, TRUE, TRUE, stdout);
  puts ("");

/******************************************************************************/

  return 0;
}
