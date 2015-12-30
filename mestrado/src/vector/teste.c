#include <stdlib.h>
#include "vector.h"

int
main (void)
{
  Vector v1 = NULL, v2 = NULL;
  const UsLgIntValue DIMENSION = 3;
  RValue distance;
  int exit_status;


  /* cria v1 */
  v1 = vector_create (DIMENSION);
  if (v1 == NULL)
    {
      fprintf (stderr, "deu pau criando v1\n");
      return EXIT_FAILURE;
    }

  /* cria v2 */
  v2 = vector_create (DIMENSION);
  if (v2 == NULL)
    {
      fprintf (stderr, "deu pau criando v2\n");
      return EXIT_FAILURE;
    }

  /* povoa v1 */
  exit_status = vector_set_value (v1, 1, 1.0);
  exit_status = vector_set_value (v1, 2, 0.0);
  exit_status = vector_set_value (v1, 3, -1.0);

  /* povoa v2 */
  exit_status = vector_set_value (v2, 1, -1.0);
  exit_status = vector_set_value (v2, 2, 1.0);
  exit_status = vector_set_value (v2, 3, -1.0);

  /* calcula a metrica entre v1 e v2 */
  exit_status = vector_metric (v1, v2, VECTOR_METR_EUCLIDEAN, &distance);
  printf ("a distancia euclidiana entre v1 e v2 é %f\n", distance);

  exit_status = vector_metric (v1, v2, VECTOR_METR_INNER_PRODUCT, &distance);
  printf ("o produto interno entre v1 e v2 é %f\n", distance);

  /* normaliza os 2 vetores */
  exit_status = vector_normalize (v1, v2);

  vector_info (v1, stdout);
  vector_info (v2, stdout);

  exit_status = vector_metric (v1, v2, VECTOR_METR_EUCLIDEAN, &distance);
  printf ("a distancia euclidiana entre v1 e v2 é %f\n", distance);

  exit_status = vector_metric (v1, v2, VECTOR_METR_INNER_PRODUCT, &distance);
  printf ("o produto interno entre v1 e v2 é %f\n", distance);

  vector_destroy (&v1);
  vector_destroy (&v2);

  return EXIT_SUCCESS;
}
