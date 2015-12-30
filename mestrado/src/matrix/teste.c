#include <stdio.h>
#include <stdlib.h>

#include "errorh.h"
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

  /* Cria a matriz */
  m1 = matrix_create (rows1, columns1, TRUE, 5.0);

  if (m1 == NULL)
    return error_failure (argv[0], "erro criando a matriz\n");

  /* Seta os valores dos elementos igual ao produto da linha pela coluna */
  for (cur_row = 1; cur_row <= rows1; cur_row++)
    for (cur_col = 1; cur_col <= columns1; cur_col++)
      {
        if (matrix_set_value (m1, cur_row, cur_col,
                              (RValue) cur_row * cur_col) != EXIT_SUCCESS)

          return error_failure (argv[0],
                                "erro setando o valor do elemento (%ld,%ld)\n",
                                cur_row, cur_col);

        /* soma 2 a todos os elementos */
        matrix_sum_value (m1, cur_row, cur_col, 2.0);

        /* subtrai 2 de todos os elementos */
        matrix_subt_value (m1, cur_row, cur_col, 2.0);

        /* multiplica por 2 */
        matrix_mult_value (m1, cur_row, cur_col, 2.0);

        /* divide por 2 */
        matrix_div_value (m1, cur_row, cur_col, 2.0);
      }

  /* Cria a matriz identidade */
  id = matrix_identity (id_order, 5.0);

  if (id == NULL)
    return error_failure (argv[0], "erro criando matriz identidade\n");

  /* soma m1 e a identidade */
  if (matrix_sum (m1, id, &sum) != EXIT_SUCCESS)
    return error_failure (argv[0], "erro somando m1 + id\n");

  /* multiplica a m1 pela identidade */
  if (matrix_product (m1, id, &prod) != EXIT_SUCCESS)
    return error_failure (argv[0], "erro multiplicando m1 * id\n");

  /* Exibe os valores da matriz soma */
  for (cur_row = 1; cur_row <= id_order; cur_row++)
    for (cur_col = 1; cur_col <= id_order; cur_col++)
      {
        if (matrix_get_value (sum, cur_row, cur_col, &value) != EXIT_SUCCESS)
          return error_failure (argv[0],
                                "erro buscando o valor do elemento (%ld,%ld)\n",
                                cur_row, cur_col);

        if (rows1 * columns1 <= 10000)
          printf ("(%ld,%ld) = %f\n", cur_row, cur_col, value);
      }

  /* Destroi as matrizes */
  if (matrix_destroy (&m1) != EXIT_SUCCESS)
    return error_failure (argv[0], "erro destruindo a matriz m1\n");

  if (matrix_destroy (&id) != EXIT_SUCCESS)
    return error_failure (argv[0], "erro destruindo a matriz identidade\n");

  if (matrix_destroy (&sum) != EXIT_SUCCESS)
    return error_failure (argv[0], "erro destruindo a matriz soma\n");

  return EXIT_SUCCESS;
}
