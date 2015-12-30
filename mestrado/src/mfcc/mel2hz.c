#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "s_smptypes.h"
#include "s_cepstrum.h"

int
main (int argc, char **argv)
{
  size_t cur_arg;
  cmp_real f_mel;
  cmp_real f_hz;
  char *tailptr;

  if (argc < 2)
    {
      fprintf (stderr, "mel2hz: a frequency in mel-scale must be passed\n");
      return EXIT_FAILURE;
    }

  for (cur_arg = 1; cur_arg < argc; cur_arg++)
    {
      errno = 0;
      f_mel = strtod (argv[cur_arg], &tailptr);

      if (tailptr == NULL)
        {
          fprintf (stderr, "mel2hz: invalid frequency: '%s'\n",
                   argv[cur_arg]);
          return EXIT_FAILURE;
        }

      if (errno != 0)
        {
          fprintf (stderr, "mel2hz: invalid frequency: '%s': %s\n",
                   argv[cur_arg], strerror (errno));
          return EXIT_FAILURE;
        }

      f_hz = scep_f_mel_to_hz (f_mel);

      printf ("%f\n", f_hz);
    }

  return EXIT_SUCCESS;
}
