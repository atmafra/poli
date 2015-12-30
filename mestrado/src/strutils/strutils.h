#ifndef __STR_UTILS_H_
#define __STR_UTILS_H_ 1

#include <stdio.h>

/* Number of bytes for the WORD data type */
#define __WORD_SIZE_ 2

/* Number of bytes for the DOUBLE WORD data type */
#define __DWORD_SIZE_ 2 * __WORD_SIZE_



/*
 * strip_nl
 *
 * Strips off the first new line character in the string
 */
extern void strip_nl (char *in_string);



/*
 * ltrim
 *
 * Strips off white spaces at the beggining of the string
 */
extern void ltrim (char *in_string);



/*
 * rtrim
 *
 * Strips off white spaces at the ending of the string
 */
extern void rtrim (char *in_string);



/*
 * split_file_name
 *
 * Splits the given file name into its path, base name and extension
 */
extern void
split_file_name (char *file_name, char **directory,
                 char **basename, char **extension);



/*
 * get_file_extension
 *
 * Returns a new string, containing the last part of the given file name
 * that begins with a dot; if no extension is found, return NULL
 */
extern char *get_file_extension (const char *file_name);



/*
 * get_file_basename
 *
 * Returns the file name, stripping off its path, and, if not null,
 * strips suffix
 */
extern char *get_file_basename (const char *file_name, const char *suffix);



/*
 * get_file_name
 *
 * Returns the file name, given its parts. File extension is changed if
 * 'new_extension' is passed.
 */
extern char *get_file_name (const char *dir, const char *filename,
                            const char *new_extension);



/*
 * getsafe
 *
 * Returns one string from the input stream, limiting
 * its size to 'max_size'
 */
extern char *getsafe (FILE * input_fd, const size_t max_size);



/*
 * read_file_line
 *
 * Reads one line of the input stream and stores it in the given string
 */
extern int
read_file_line (FILE * input_fd, const size_t max_size, char *line);



/*
 * read_valid_file_line
 *
 * Reads one line of the input stream, strips off preceding spaces,
 * ignoring lines that start with the given token.
 * Stores the result in 'line'.
 */
extern int
read_valid_file_line (FILE * input_fd, const size_t max_line_size,
                      char ignore_token, int ignore_blank_lines, char *line);



/*
 * getsafe_hex
 *
 * Binary gets routine
 */
extern unsigned long getsafe_hex (FILE * in_file, const size_t count);



/*
 * getsafe_word
 *
 */
extern size_t getsafe_word (FILE * in_file);



/*
 * getsafe_dword
 *
 */
extern unsigned long getsafe_dword (FILE * in_file);



/*
 * display_progress
 *
 * Displays progress of a process as a character representation
 */
extern int
display_progress (const long min, const long max, const long cur,
                  const size_t width, const char symbol, const int reinit);


#endif /* __STR_UTILS_H_ */
