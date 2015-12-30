#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

#define ORD 100

int
main (int argc, char **argv)
{
  Matrix m1 = NULL;
  Matrix id = NULL;
  Matrix sum = NULL;
  Matrix prod = NULL;
  const MIndex rows1 = ORD;
  const MIndex columns1 = ORD;
  const MIndex id_order = ORD;
  MIndex cur_row, cur_col;
  RValue value;
  int exit_status;

  /* Cria a matriz */
  m1 = matrix_create (rows1, columns1, TRUE, 5.0);
  if (m1 == NULL)
    {
      fprintf (stderr, "erro criando a matriz\n");
      return EXIT_FAILURE;
    }

  /* Seta os valores dos elementos igual ao produto da linha pela coluna */
  for (cur_row = 1; cur_row <= rows1; cur_row++)
    for (cur_col = 1; cur_col <= columns1; cur_col++)
      {
        exit_status =
          matrix_set_value (m1, cur_row, cur_col, (RValue) cur_row * cur_col);
        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr, "erro setando o valor do elemento (%ld,%ld)\n",
                     cur_row, cur_col);
            return EXIT_FAILURE;
          }

        /* soma 2 a todos os elementos */
        exit_status = matrix_sum_value (m1, cur_row, cur_col, 2.0);

        /* subtrai 2 de todos os elementos */
        exit_status = matrix_subt_value (m1, cur_row, cur_col, 2.0);

        /* multiplica por 2 */
        exit_status = matrix_mult_value (m1, cur_row, cur_col, 2.0);

        /* divide por 2 */
        exit_status = matrix_div_value (m1, cur_row, cur_col, 2.0);
      }

  /* Cria a matriz identidade */
  id = matrix_identity (id_order, 5.0);
  if (id == NULL)
    {
      fprintf (stderr, "erro criando matriz identidade\n");
      return EXIT_FAILURE;
    }

  /* soma m1 e a identidade */
  exit_status = matrix_sum (m1, id, &sum);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro somando m1 + id\n");
      return EXIT_FAILURE;
    }

  /* multiplica a m1 pela identidade */
  exit_status = matrix_product (m1, id, &prod);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro multiplicando m1 * id\n");
      return EXIT_FAILURE;
    }

  /* Exibe os valores da matriz soma */
  for (cur_row = 1; cur_row <= id_order; cur_row++)
    for (cur_col = 1; cur_col <= id_order; cur_col++)
      {
        exit_status = matrix_get_value (sum, cur_row, cur_col, &value);
        if (exit_status != EXIT_SUCCESS)
          {
            fprintf (stderr, "erro buscando o valor do elemento (%ld,%ld)\n",
                     cur_row, cur_col);
            return EXIT_FAILURE;
          }
        if (rows1 * columns1 <= 10000)
          printf ("(%ld,%ld) = %f\n", cur_row, cur_col, value);
      }

  /* Destroi as matrizes */
  exit_status = matrix_destroy (&m1);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro destruindo a matriz m1\n");
      return EXIT_FAILURE;
    }

  exit_status = matrix_destroy (&id);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro destruindo a matriz identidade\n");
      return EXIT_FAILURE;
    }

  exit_status = matrix_destroy (&sum);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro destruindo a matriz soma\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
