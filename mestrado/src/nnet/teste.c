#include <stdio.h>
#include <stdlib.h>
#include "nnet_types.h"
#include "nnet_nnet.h"
#include "nnet_actv.h"
#include "nnet_layers.h"
#include "nnet_units.h"
#include "nnet_conns.h"
#include "nnet_weights.h"
#include "nnet_metrics.h"
#include "nnet_sets.h"

int
main ()
{
  int exit_status;

  NNetwork rede = NULL;

  Layer input_layer = NULL;
  Layer output_layer = NULL;

  Unit winner = NULL;

  TSet tr_set = NULL, vl_set = NULL;
  /*, ts_set = NULL; */

  TElement cur_element = NULL;

  /*
     TElement element;
     InputOutput io;
     UnitIndex pos;
   */

/******************************************************************************/

  nnet_actv_init_all_classes ();

/******************************************************************************/

  exit_status = nnet_wght_init_all_classes ();

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau inicializando as funcoes de peso\n");
      return EXIT_FAILURE;
    }


/******************************************************************************/

  nnet_layer_init_all_classes ();

/******************************************************************************/

  rede = nnet_nnetwork_create ("Rede Teste");

/******************************************************************************/

  input_layer =
    nnet_layer_create (rede, NULL, &nnet_layer_class_input, "Entrada");

  output_layer =
    nnet_layer_create (rede, NULL, &nnet_layer_class_output, "Saída");

/******************************************************************************/

  exit_status =
    nnet_unit_create_multiple (16, input_layer, NULL, NULL, TRUE, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau criando unidades na camada de entrada\n");
      return EXIT_FAILURE;
    }

  exit_status =
    nnet_unit_create_multiple (5, output_layer, NULL, NULL, TRUE, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau criando unidades na camada de saída\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  exit_status = nnet_conn_connect_layers
    (input_layer, output_layer, 0.0, nnet_wght_init_gaussian_random);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau conectando a entrada com a oculta\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  tr_set = nnet_tset_create_from_file
    ("Conjunto de Teste", input_layer->nu_units, 0, TRUE, FALSE, "teste.txt");

  if (tr_set == NULL)
    {
      fprintf (stderr, "deu pau criando o conjunto de treinamento\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  /*
     nnet_tset_info (tr_set, TRUE, TRUE, TRUE, stdout);

     exit_status =
     nnet_tset_divide (tr_set, &vl_set, "Conjunto de Validação", 20,
     PICK_AT_RANDOM);

     if (exit_status != EXIT_SUCCESS)
     {
     fprintf (stderr, "deu pau dividindo o conjunto de treinamento\n");
     return EXIT_FAILURE;
     }
   */


/******************************************************************************/

  /*
     nnet_tset_info (tr_set, TRUE, TRUE, TRUE, stdout);
     nnet_tset_info (vl_set, TRUE, TRUE, TRUE, stdout);
     nnet_tset_info (set, TRUE, TRUE, TRUE, stdout);
   */

/******************************************************************************/

  cur_element = tr_set->first_element;

  while (cur_element != NULL)
    {
      exit_status = nnet_layer_load_vector (cur_element->input, input_layer);
      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "deu pau carregando elemento na camada de entrada\n");
          return EXIT_FAILURE;
        }

      exit_status = nnet_layer_activate (input_layer, (int) NULL);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "deu pau ativando a camada de entrada\n");
          return EXIT_FAILURE;
        }

      exit_status =
        nnet_layer_activate (output_layer, VECTOR_METR_INNER_PRODUCT);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "deu pau ativando a camada de saída\n");
          return EXIT_FAILURE;
        }

      winner = nnet_metr_layer_winner (output_layer);

      if (winner == NULL)
        {
          fprintf (stderr, "deu pau achando o vencedor\n");
          return EXIT_FAILURE;
        }

      printf ("Venceu a unidade %ld com saída %f\n", winner->unit_index,
              winner->output);

      cur_element = cur_element->next;
    }

/******************************************************************************/

  exit_status = nnet_tset_destroy (&tr_set, TRUE);
  exit_status |= nnet_tset_destroy (&vl_set, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destruindo o conjunto de treinamento\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  exit_status = nnet_nnetwork_destroy (&rede, TRUE, TRUE, TRUE);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "deu pau destruindo a rede\n");
      return EXIT_FAILURE;
    }

/******************************************************************************/

  /* um atexit ? */
  nnet_actv_destroy_all_classes ();
  nnet_wght_destroy_all_classes ();

/******************************************************************************/

  return 0;
}
