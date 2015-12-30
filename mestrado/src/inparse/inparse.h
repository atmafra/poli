#ifndef __INPARSE_H_
#define __INPARSE_H_ 1

#include <stdio.h>
#include "../common/types.h"

/******************************************************************************
 *                                                                            *
 *                             PUBLIC DATA TYPES                              *
 *                                                                            *
 ******************************************************************************/

/*
 * InputParameter
 *
 * Data type for one parameter
 * . tag         parameter command line tag. Ex.: -o <filename>
 * . tag_syn     tag synonym. Ex.: --output <filename>
 * . datatype    parameter data type
 * . passed      indicates that the parameter was found in the command line
 * . mandatory   indicates that the parameter is mandatory
 * . value       parameter value
 */
typedef struct
{
  char *tag;
  char *tag_syn;
  CompositeDataType datatype;
  BoolValue passed;
  BoolValue mandatory;
  CompositeUnion value;
}
InputParameter;

/*
 * InputParameterList
 *
 * List of input parameters
 */
typedef InputParameter InputParameterSet[];

typedef struct
{
  UsIntValue nu_parameters;
  InputParameter *parameter;
}
InputParameterList;



/******************************************************************************
 *                                                                            *
 *                              PUBLIC FUNCTIONS                              *
 *                                                                            *
 ******************************************************************************/

/*
 * inpr_parse
 *
 * Parses the parameters given by 'argc' and 'argv' against the given
 * parameter list
 */
extern int inpr_parse (int argc, char **argv, InputParameterList plist);



/*
 * inpr_list_info
 *
 * Outputs info about the parameter list
 */
extern void inpr_list_info (const InputParameterList plist, FILE * output_fd);



#endif /* inparse.h */
