#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "nnet_files.h"

#include "nnet_types.h"
#include "../common/types.h"
#include "../strutils/strutils.h"



/******************************************************************************
 *                                                                            *
 *                               PUBLIC VARIABLES                             *
 *                                                                            *
 ******************************************************************************/

FILE *input_fd;                 /* input file descriptor */
int cur_section = -1;           /* current section being processed */
int cur_attr = 0;               /* current section attribute */
char buf[NNET_BUF_SIZE];        /* read buffer */



/******************************************************************************
 *                                                                            *
 *                              PUBLIC OPERATIONS                             *
 *                                                                            *
 ******************************************************************************/

/*
 * nnet_file_read_line
 *
 * Reads one line to the input buffer
 */
int
nnet_file_read_buffer (const char ignore_char, const char *stdin_prompt)
{
  int exit_status;              /* auxiliary function return status */


  /* If reading from standard input, displays the prompt */
  if (input_fd == stdin)
    printf ("%s", stdin_prompt);

  /* Reads one line from the input stream */
  exit_status =
    read_valid_file_line (input_fd, NNET_BUF_SIZE, ignore_char, 0, buf);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr,
               "nnet_file_read_buffer: error reading valid file line\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*
 * nnet_file_write_vector
 *
 * Writes a vector to the output stream
 */
void
nnet_file_write_vector (UsLgIntValue dim, RValue * components,
                        FILE * output_fd)
{
  UsLgIntValue cur_comp;        /* vector component */

  if (dim > 0)
    {
      fprintf (output_fd, "%ld", dim);

      for (cur_comp = 0; cur_comp < dim; cur_comp++)
        fprintf (output_fd, " %f", components[cur_comp]);

      fprintf (output_fd, "\n");
    }
}



/*
 * nnet_file_parse_vector
 *
 * Creates a vector according to the given configuration line
 */
Vector
nnet_file_parse_vector (char *line)
{
  UsLgIntValue dim = 0;         /* vector dimension */
  UsLgIntValue cur_comp = 1;    /* current component */
  Vector new_vector = NULL;     /* new vector to create */
  RValue cur_value;             /* current component value */
  char *word = NULL;            /* word currently being read */
  const char delim[] = " ,";    /* word delimiters */
  int exit_status;              /* auxiliary function return status */


  /* Reads the dimension */
  word = (char *) strtok (line, delim);
  if (word == NULL)
    {
      fprintf (stderr,
               "nnet_file_parse_vector: error parsing vector dimension\n");
      return NULL;
    }

  dim = (UsLgIntValue) strtol (word, NULL, 10);

  /* Creates the vector */
  new_vector = vector_create (dim);

  /* Populates the vector */
  while (word != NULL && cur_comp <= dim)
    {
      /* Reads the component value */
      word = (char *) strtok (NULL, delim);

      if (word != NULL)
        {
          /* Converts into real */
          cur_value = strtod (word, NULL);

          /* Sets the vector component */
          exit_status = vector_set_value (new_vector, cur_comp, cur_value);

          if (exit_status != EXIT_SUCCESS)
            {
              fprintf (stderr,
                       "nnet_file_parse_vector: error setting vector component %ld\n",
                       cur_comp);
              vector_destroy (&new_vector);
              return NULL;
            }
          ++cur_comp;
        }
    }

  if (cur_comp != dim + 1)
    {
      fprintf (stderr,
               "nnet_file_parse_vector: insufficient vector components (expected %ld)\n",
               dim);
      vector_destroy (&new_vector);
      return NULL;
    }

  return new_vector;
}



/*
 * nnet_file_parse_attribute
 *
 * Parses the current buffer and extracts the attribute value
 */
void
nnet_file_parse_attribute (NFile nfile, CompositeUnion * value)
{
/*  char line[NNET_BUF_SIZE]; *//* auxiliar line */
  size_t start_pos;             /* buffer start position */
  CompositeDataType datatype;   /* current attribute data type */


  /* Initialization */
  start_pos = strlen (nfile[cur_section].attr[cur_attr].name);
  datatype = nfile[cur_section].attr[cur_attr].datatype;

  /* Copies the buffer to the line starting from the given position */
  if ((strcpy (buf, buf + start_pos) == NULL))
    {
      fprintf (stderr,
               "nnet_file_parse_attribute: error parsing attribute\n");
      return;
    }

  /* Wipes off left empty spaces */
  ltrim (buf);

  /* Converts to the correct datatype */
  switch (datatype)
    {
    case BOOL:
      if (strcmp (buf, "TRUE") == 0 || strcmp (buf, "true") == 0 ||
          strcmp (buf, "True") == 0)
        value->boolvalue = TRUE;
      else
        if (strcmp (buf, "FALSE") == 0 || strcmp (buf, "false") == 0 ||
            strcmp (buf, "False") == 0)
        value->boolvalue = FALSE;
      else
        {
          fprintf (stderr,
                   "nnet_file_parse_attribute: error converting attribute to boolean\n");
          return;
        }
      break;

    case UNSIGNED_INT:
      value->usintvalue = (UsIntValue) strtol (buf, NULL, 10);
      break;

    case LONG_INT:
      value->lgintvalue = (LgIntValue) strtol (buf, NULL, 10);
      break;

    case UNSIGNED_LONG_INT:
      value->uslgintvalue = (UsLgIntValue) strtol (buf, NULL, 10);
      break;

    case REAL:
      value->realvalue = (RValue) strtod (buf, NULL);
      break;

    case STRING:
      if ((strncpy (value->stringvalue, buf, strlen (buf) + 1)) == NULL)
        {
          fprintf (stderr,
                   "nnet_file_parse_attribute: error copying string attribute\n");
          return;
        }
      break;

    case REAL_VECTOR:
      value->rvectorvalue = nnet_file_parse_vector (buf);

      if (value->rvectorvalue == NULL)
        {
          fprintf (stderr,
                   "nnet_file_parse_attribute: error parsing vector attribute\n");
          return;
        }
      break;

    default:
      fprintf (stderr, "nnet_file_parse_attribute: unknown datatype\n");
      return;
    }

  return;
}



/*
 * nnet_file_read_section
 *
 * Searches a section start in the configuration file and returns its ID
 */
int
nnet_file_read_section (NFile nfile, const UsIntValue nu_sections,
                        const char ignore_char, const char *stdin_prompt)
{
  BoolValue section_found = FALSE;      /* section found flag */


  /* Reads the input file until a valid section tag is found */
  while (section_found == FALSE && !feof (input_fd))
    {
      /* Reads one line */
      if (nnet_file_read_buffer (ignore_char, stdin_prompt) != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_read_section: error reading valid line\n");
          return EXIT_FAILURE;
        }

      /* Checks if it is a section start tag */
      if (!feof (input_fd))
        {
          cur_section = 0;
          while (cur_section < (int) nu_sections && section_found == FALSE)
            {
              if (strcmp (nfile[cur_section].begin, buf) == 0)
                section_found = TRUE;
              else
                ++cur_section;
            }
        }

      /* Displays an error for invalid section */
      if (section_found == FALSE && *buf != '\0')
        fprintf (stderr, "nnet_file_read_section: invalid section: '%s'\n",
                 buf);
    }

  /* Section start tag not found */
  if (section_found == FALSE)
    cur_section = -1;

  return EXIT_SUCCESS;
}



/*
 * nnet_file_read_attribute
 *
 * Reads one of the possible attributes of the current session
 */
int
nnet_file_read_attribute (NFile nfile,
                          UsIntValue * attribute_id,
                          BoolValue * end_of_section,
                          const char ignore_char, const char *stdin_prompt)
{
  BoolValue attribute_found = FALSE;    /* attribute found flag */
  BoolValue end_section_found = FALSE;  /* end of section flag */


  /* Checks if parsing is within a session */
  if (cur_section == -1)
    {
      fprintf (stderr,
               "nnet_file_read_attribute: no current section defined\n");
      return EXIT_FAILURE;
    }

  /* Reinitializes the current attribute */
  cur_attr = 0;

  /* Reads the input file until a valid attribute tag is found */
  while (attribute_found == FALSE && end_section_found == FALSE
         && !feof (input_fd))
    {
      /* Reads one line */
      if (nnet_file_read_buffer (ignore_char, stdin_prompt) != EXIT_SUCCESS)
        {
          fprintf (stderr,
                   "nnet_file_read_attribute: error reading valid line\n");
          return EXIT_FAILURE;
        }

      /* Checks if it is a section start tag */
      if (!feof (input_fd))
        {
          /* Checks if the attribute was found */
          while (cur_attr < nfile[cur_section].nu_attr
                 && attribute_found == FALSE)
            {
              if (strncmp
                  (nfile[cur_section].attr[cur_attr].name, buf,
                   strlen (nfile[cur_section].attr[cur_attr].name)) == 0)
                attribute_found = TRUE;
              else
                ++cur_attr;
            }

          /* Checks if the end of the current section was found */
          if (strcmp (nfile[cur_section].end, buf) == 0)
            end_section_found = TRUE;

          /* Invalid line */
          if (attribute_found == FALSE && end_section_found == FALSE &&
              *buf != '\0')
            {
              fprintf (stderr,
                       "nnet_file_read_attribute: invalid attribute: '%s'\n",
                       buf);
            }
        }
    }

  /* Sets the attribute ID found */
  if (attribute_found == TRUE)
    *attribute_id = cur_attr;
  else
    *attribute_id = -1;

  /* Sets the end_of_section flag */
  *end_of_section = end_section_found;

  return EXIT_SUCCESS;
}
