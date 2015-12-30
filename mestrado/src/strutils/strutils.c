#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "strutils.h"


/*
 * strip_nl
 *
 * Strips off the first new line character in the string
 */
void
strip_nl (char *in_string)
{
  char *np = NULL;

  np = (char *) strchr (in_string, '\n');

  if (np != NULL)
    {
      if (*np == '\n')
        *np = '\0';
    }

  return;
}



/*
 * ltrim
 *
 * Strips off white spaces at the beggining of the string
 */
void
ltrim (char *in_string)
{
  size_t init_pos, cur_pos, len;

  init_pos = strspn (in_string, " ");
  len = strlen (in_string) - init_pos;

  for (cur_pos = 0; cur_pos <= len; cur_pos++)
    in_string[cur_pos] = in_string[cur_pos + init_pos];

  return;
}



/*
 * rtrim
 *
 * Strips off white spaces at the ending of the string
 */
void
rtrim (char *in_string)
{
  size_t pos = 0;

  pos = strlen (in_string) - 1;

  while (in_string[pos] == ' ' && pos != 0)
    {
      in_string[pos] = '\0';
      pos--;
    }

  return;
}



/*
 * split_file_name
 *
 * Splits the given file name into its path, base name and extension
 */
void
split_file_name (char *file_name, char **directory,
                 char **basename, char **extension)
{
  char c;
  long pos = -1;
  long dot_pos = -1, bar_pos = -1;
  size_t len;
  size_t basename_size;
  int dot_fl = 0, base_fl = 0, bar_fl = 0;


  /* initialization */
  *directory = NULL;
  *basename = NULL;
  *extension = NULL;

  /* trivial case: empty file name */
  if (file_name == (char *) NULL || len == 0)
    return;

  /* another trivial case: file name is '\0' */
  len = strlen (file_name);
  if (len == 0)
    return;

  /* searches for a dot and/or a bar */
  pos = len;
  while (pos >= 0 && !bar_fl)
    {
      /* sets current character */
      c = file_name[pos];

      /* is it a dot? */
      if (c == '.' && !dot_fl && !bar_fl)
        {
          dot_pos = pos;
          dot_fl = 1;
        }

      /* is it a bar? */
      if ((c == '\\' || c == '/') && !bar_fl)
        {
          bar_pos = pos;
          bar_fl = 1;
        }

      pos--;
    }

  /* file names may start with a dot */
  if (dot_pos == bar_pos + 1)
    {
      dot_fl = 0;
      dot_pos = -1;
    }

  /* is there a base name? */
  if ((bar_fl && dot_fl && dot_pos - bar_pos > 1) ||
      (bar_fl && !dot_fl && len - bar_pos > 1) ||
      (!bar_fl && dot_pos > 0) || (!bar_fl && !dot_fl && len > 0))
    base_fl = 1;

  /* part memory allocation */
  if (bar_fl)
    *directory = (char *) realloc (*directory, bar_pos + 2);

  if (dot_fl)
    *extension = (char *) realloc (*extension, len - dot_pos + 1);

  if (base_fl)
    {
      if (bar_fl && dot_fl && dot_pos - bar_pos > 1)
        {
          basename_size = dot_pos - bar_pos;
        }
      else if (bar_fl && !dot_fl && len - bar_pos > 0)
        {
          basename_size = len - bar_pos;
        }
      else if (!bar_fl && dot_fl)
        {
          basename_size = dot_pos + 1;
        }
      else if (!bar_fl && !dot_fl && len > 0)
        {
          basename_size = len + 1;
        }
      *basename = (char *) realloc (*basename, basename_size);
    }

  /* sets the parts */
  for (pos = 0; pos <= len; pos++)
    {
      /* directory part */
      if (pos <= bar_pos && bar_fl)
        (*directory)[pos] = file_name[pos];

      /* directory end */
      if (pos == bar_pos && bar_fl)
        (*directory)[pos + 1] = '\0';

      /* base name part */
      if ((pos > bar_pos) &&
          ((pos < dot_pos && dot_fl) || (pos < len && !dot_fl)) && base_fl)
        {
          (*basename)[pos - bar_pos - 1] = file_name[pos];
        }

      /* end of base name */
      if ((pos == dot_pos || (pos == len && !dot_fl)) && base_fl)
        {
          (*basename)[basename_size - 1] = '\0';
        }

      /* extension part */
      if (pos >= dot_pos && pos < len && dot_fl)
        (*extension)[pos - dot_pos] = file_name[pos];

      /* end of extension */
      if (pos == len && dot_fl)
        (*extension)[pos - dot_pos] = '\0';
    }

  /* trims the parts */
  if (bar_fl)
    ltrim (*directory);

  if (dot_fl)
    rtrim (*extension);

  return;
}



/*
 * get_file_extension
 *
 * Returns the last part of a string that begins with a dot;
 * if no extension is found, return NULL
 */
char *
get_file_extension (const char *file_name)
{
  char *dot_ptr;
  char *extension;

  if (file_name == (char *) NULL)
    return (char *) NULL;

  dot_ptr = strrchr (file_name, '.');

  if (dot_ptr != NULL)
    {
      extension = (char *) malloc (strlen (dot_ptr) + 1);

      if (extension == NULL)
        {
          fprintf (stderr, "get_file_extension: virtual memory exhausted\n");
          return NULL;
        }
      else
        {
          strcpy (extension, dot_ptr);
          return extension;
        }
    }
  else
    {
      return NULL;
    }
}



/*
 * get_file_basename
 *
 * Returns the file name, stripping off its path, and, if not null,
 * strips suffix
 */
char *
get_file_basename (const char *file_name, const char *suffix)
{
  char *bar_ptr = NULL;
  char *extension = NULL;
  char *basename = NULL;
  size_t basename_size;

  if (file_name == NULL)
    return (char *) NULL;

  bar_ptr = strrchr (file_name, '/');

  if (bar_ptr == NULL)
    bar_ptr = strrchr (file_name, '\\');

  if (bar_ptr != NULL)
    {
      basename = (char *) realloc (basename, strlen (bar_ptr));

      if (basename == NULL)
        {
          fprintf (stderr, "get_file_basename: virtual memory exhausted\n");
          return NULL;
        }

      strcpy (basename, bar_ptr + 1);
    }
  else
    {
      basename = (char *) realloc (basename, strlen (file_name) + 1);

      if (basename == NULL)
        {
          fprintf (stderr, "get_file_basename: virtual memory exhausted\n");
          return NULL;
        }

      strcpy (basename, file_name);
    }

  if (suffix != NULL)
    {
      extension = get_file_extension (file_name);

      if (strcmp (extension, suffix) == 0)
        {
          basename_size = strlen (basename) - strlen (extension);
          basename = (char *) realloc (basename, basename_size + 1);

          if (basename == NULL)
            {
              fprintf (stderr,
                       "get_file_basename: virtual memory exhausted\n");
              return NULL;
            }

          basename[basename_size] = '\0';
        }

      free (extension);
    }

  return basename;
}



/*
 * get_file_name
 *
 * Returns the file name, given its parts. File extension is changed if
 * 'new_extension' is passed.
 */
char *
get_file_name (const char *dir, const char *filename,
               const char *new_extension)
{
  char *file_name_new = NULL;   /* composite file name */
  char *file_extension = NULL;  /* extracted file extension */
  char *file_basename = NULL;   /* extracted file basename */
  size_t file_name_size = 1;    /* new file name size */


  /* checks if file name was actually passed */
  if (filename == (char *) NULL)
    {
      fprintf (stderr, "get_file_name: no file name passed\n");
      return NULL;
    }

  /* extracts file extension */
  if (new_extension != NULL)
    {
      file_extension = get_file_extension (filename);
      file_basename = get_file_basename (filename, file_extension);
    }

  /* file name size calculation */
  if (dir != NULL)
    {
      if (dir[strlen (dir) - 1] == '/')
        file_name_size += strlen (dir);
      else
        file_name_size += strlen (dir) - 1;
    }

  file_name_size += strlen (filename);

  if (new_extension != NULL)
    file_name_size += strlen (new_extension) - strlen (file_extension);

  /* allocates new file name */
  file_name_new = (char *) malloc (file_name_size);
  if (file_name_new == NULL)
    {
      fprintf (stderr, "get_file_name: '%s'\n", strerror (errno));
      return NULL;
    }
  file_name_new[0] = '\0';

  /* defines the new file name */
  if (dir != NULL)
    {
      if (dir[strlen (dir) - 1] == '/')
        sprintf (file_name_new, "%s", dir);
      else
        sprintf (file_name_new, "%s/", dir);
    }

  if (new_extension != NULL)
    sprintf (file_name_new, "%s%s%s", file_name_new, file_basename,
             new_extension);
  else
    sprintf (file_name_new, "%s%s", file_name_new, filename);

  /* destroy auxiliary variables */
  if (new_extension != NULL)
    {
      free (file_extension);
      free (file_basename);
    }

  return file_name_new;
}



/*
 * getsafe
 *
 * Returns one string from the input stream, limiting
 * its size to 'max_size'
 */
char *
getsafe (FILE * input_fd, const size_t max_size)
{
  char *result = NULL;          /* resulting string */


  if (max_size >= 1)
    {
      /* Allocates space for the result */
      result = (char *) malloc (max_size + 1);

      if (result == NULL)
        {
          fprintf (stderr, "getsafe: virtual memory exhausted\n");
          return NULL;
        }

      /* Trivial case */
      if (max_size == 1)
        {
          *result = '\0';
        }
      else
        {
          /* Gets the string from the input stream */
          result = fgets (result, (int) max_size + 1, input_fd);

          if (result != NULL)
            {
              /* Substitutes the EOL character by an end of string */
              strip_nl (result);
            }
        }
    }

  return result;
}



/*
 * read_file_line
 *
 * Reads one line of the input stream and stores it in the given string
 */
int
read_file_line (FILE * input_fd, const size_t max_size, char *line)
{
  char *line_read;              /* line read from input stream */


  /* Checks if the file descriptor was actually passed */
  if (input_fd == NULL)
    {
      fprintf (stderr, "read_file_line: no input stream passed\n");
      return EXIT_FAILURE;
    }

  /* Checks if the return string was actually passed */
  if (line == NULL)
    {
      fprintf (stderr, "read_file_line: no output string passed\n");
      return EXIT_FAILURE;
    }

  /* Trivial case: nothing to read */
  if (max_size == 0)
    return EXIT_SUCCESS;

  /* Reads the file */
  line_read = fgets (line, max_size + 2, input_fd);

  if (line_read == NULL && !feof (input_fd))
    {
      fprintf (stderr, "read_file_line: error reading line from file\n");
      return EXIT_FAILURE;
    }

  if (feof (input_fd))
    *line = '\0';

  return EXIT_SUCCESS;
}



/*
 * read_valid_file_line
 *
 * Reads one line of the input stream, strips off preceding spaces,
 * ignoring lines that start with the given token.
 * Stores the result in 'line'.
 */
int
read_valid_file_line (FILE * input_fd, const size_t max_line_size,
                      char ignore_token, int ignore_blank_lines, char *line)
{
  int line_ok = 1;              /* flag that indicates that the line is ok */
  int exit_status;              /* auxiliary function return status */


  /* Reads the line */
  while (line_ok == 1 && !feof (input_fd))
    {
      /* Reads one line from the file */
      exit_status = read_file_line (input_fd, max_line_size, line);

      if (exit_status != EXIT_SUCCESS)
        {
          fprintf (stderr, "read_valid_file_line: error reading file line\n");
          return EXIT_FAILURE;
        }

      if (!feof (input_fd))
        {
          /* Strips initial white spaces */
          ltrim (line);

          /* Substitutes \n by \0 */
          strip_nl (line);

          /* Checks if the line starts with the token */
          if (line[0] != ignore_token &&
              !(ignore_blank_lines && line[0] == '\0'))
            line_ok = 0;

          /* A different behaviour for the standard input */
          if (line_ok == 1 && input_fd == stdin)
            {
              *line = '\0';
              line_ok = 0;
            }
        }
    }

  return EXIT_SUCCESS;
}



/*
 * getsafe_hex
 *
 * Binary gets routine
 */
unsigned long
getsafe_hex (FILE * in_file, const size_t count)
{
  size_t i = 0;
  unsigned long aux_pow = 1;
  int buf;
  unsigned long ret_value = 0;

  do
    {
      buf = fgetc (in_file);
      if (buf != EOF)
        {
          ret_value += aux_pow * buf;
          ++i;
          aux_pow *= 256;
        }
    }
  while (i < count && buf != EOF);

  return (ret_value);
}



/*
 * getsafe_word
 *
 */
size_t
getsafe_word (FILE * in_file)
{
  return (size_t) getsafe_hex (in_file, __WORD_SIZE_);
}



/*
 * getsafe_dword
 *
 */
unsigned long
getsafe_dword (FILE * in_file)
{
  return (unsigned long) getsafe_hex (in_file, __DWORD_SIZE_);
}



/*
 * display_progress
 *
 * Displays progress of a process as a character representation
 */
int
display_progress (const long min, const long max, const long cur,
                  const size_t width, const char symbol, const int reinit)
{
  static long int_min = 0, int_max = 0;
  static long range = 0;
  static size_t int_width = 0;
  static size_t nu_symbols_old = 0;
  size_t nu_symbols_new;
  size_t aux;
  char label[256] = "";


  /* reinitialization */
  if (reinit)
    {
      int_min = 0;
      int_max = 0;
      range = 0;
      int_width = 0;
    }

  /* consistency check */
  if (cur < min || cur > max)
    {
      fprintf (stderr,
               "display_progress: current value (%d) out of range (%d..%d)\n",
               cur, min, max);
      return EXIT_FAILURE;
    }

  /* synchronize or not internal variables */
  if (min != int_min || max != int_max || width != int_width)
    {
      if (min > max)
        {
          fprintf (stderr,
                   "display_progress: minimum value is lower than maximum\n");
          return EXIT_FAILURE;
        }
      if (width <= 0)
        {
          fprintf (stderr, "display_progress: invalid width requested\n");
          return EXIT_FAILURE;
        }
      range = max - min + 1;
      int_min = min;
      int_max = max;
      int_width = width;
    }

  /* number of symbols */
  if (cur == max)
    nu_symbols_new = width;
  else
    nu_symbols_new = (long) ((double) cur - (double) min) *
      ((double) width / (double) range);

  /* first value */
  if (cur == min)
    {
      sprintf (label, "[%ld-%ld] ", min, max);
      printf (label, min, max);
      for (aux = 0; aux < width; ++aux)
        printf ("%c", symbol);
      printf ("\n");
      for (aux = 0; aux < strlen (label); ++aux)
        printf (" ");
    }

  for (; nu_symbols_old < nu_symbols_new; ++nu_symbols_old)
    printf ("%c", symbol);

  if (cur == max)
    printf ("\n");

  fflush (stdout);

  return EXIT_SUCCESS;
}
