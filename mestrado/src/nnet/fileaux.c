/*
 * read_raw_file
 *
 * Reads a raw format samples file
 */
static int
read_raw_file (FILE * fp, sample_list_type * smp_list)
{
  /* Read buffer */
  char buf[__SMP_NAME_SIZE_];

  /* Characters to be parsed */
  const char delimiters[] = " ,;()ijIJ[]{}";

  /* Auxiliary complex number read */
  cmp_complex aux_z;

  /* Auxiliary character */
  char *cp;

  /* Auxiliary function return status */
  int exit_status;


  /* Reads the samples */
  do
    {
      if (fgets (buf, __SMP_NAME_SIZE_, fp) != NULL)
        {
          /* Avoid empty lines */
          if (*buf != '\0')
            {
              cp = (char *) strtok (buf, delimiters);

              /* Reads the real part - supposed to be first */
              if (cp != NULL)
                aux_z.re = strtod (cp, NULL);
              else
                aux_z.re = 0;

              /* Reads the complex part - supposed to be next */
              if (smp_list->data_type == SMP_COMPLEX)
                {
                  if (cp != NULL)
                    {
                      cp = (char *) strtok (NULL, delimiters);

                      if (cp != NULL)
                        aux_z.im = strtod (cp, NULL);
                      else
                        aux_z.im = 0;
                    }
                }

              /* Stores the new value on the list */
              exit_status = add_list_value (&smp_list, aux_z);
              if (exit_status != EXIT_SUCCESS)
                {
                  fprintf (stderr,
                           "read_raw_file: error storing last value read\n");
                  return (EXIT_FAILURE);
                }
            }
        }
    }
  while (!feof (fp));

  return (EXIT_SUCCESS);
}
