#ifndef __NNET_FILES_H_
#define __NNET_FILES_H_ 1

#include "../common/types.h"
#include "nnet_types.h"
#include "som/nnet_som.h"


/* Tag string size */
#define __NNTAGSIZE_ 50

/* Maximum Attributes per Section */
#define __NNMAXATT_ 10


/******************************************************************************
 *                                                                            *
 *                              PUBLIC DATATYPES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * NFileAttribute
 *
 * File Section associated attribute
 */
typedef struct
{
  int attribute_id;
  CompositeDataType datatype;
  char *name;
}
NFileAttribute;



/*
 * NFileSection
 *
 * File Sections corresponding to neural network elements
 */
typedef struct
{
  int section_id;
  char *begin;
  char *end;
  UsIntValue nu_attr;
  NFileAttribute attr[__NNMAXATT_];
}
NFileSection;



/*
 * NFile
 *
 * Set of sections that compound a configuration file
 */
typedef NFileSection NFile[];



/******************************************************************************
 *                                                                            *
 *                               PUBLIC VARIABLES                             *
 *                                                                            *
 ******************************************************************************/

extern FILE *input_fd;              /* input file descriptor */
extern int cur_section;             /* current section being processed */
extern UsIntValue cur_attr;         /* current section attribute */
extern char buf[NNET_BUF_SIZE];     /* read buffer */



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
extern int
nnet_file_read_buffer (const char ignore_char, const char *stdin_prompt);



/*
 * nnet_file_write_vector
 *
 * Writes a vector to the output stream
 */
extern void
nnet_file_write_vector (UsLgIntValue dim,
                        RValue * components, FILE * output_fd);



/*
 * nnet_file_parse_vector
 *
 * Creates a vector according to the given configuration line
 */
extern Vector nnet_file_parse_vector (char *line);



/*
 * nnet_file_parse_attribute
 *
 * Parses the current buffer and extracts the attribute value
 */
extern void nnet_file_parse_attribute (NFile nfile, CompositeUnion * value);



/*
 * nnet_file_read_section
 *
 * Searches a section start in the configuration file and returns its ID
 */
extern int
nnet_file_read_section (NFile nfile,
                        const UsIntValue nu_sections,
                        const char ignore_char, const char *stdin_prompt);



/*
 * nnet_file_read_attribute
 *
 * Reads one of the possible attributes of the current session
 */
extern int
nnet_file_read_attribute (NFile nfile,
                          UsIntValue * attribute_id,
                          BoolValue * end_of_section,
                          const char ignore_char, const char *stdin_prompt);



#endif /* __NNET_FILES_H_ */
