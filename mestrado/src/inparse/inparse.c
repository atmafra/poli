#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorh.h"
#include "inparse.h"

/******************************************************************************
 *                                                                            *
 *                             PRIVATE FUNCTIONS                              *
 *                                                                            *
 ******************************************************************************/

static void
inpr_param_info (const InputParameter p, FILE * output_fd)
{
  fprintf (output_fd, "%s (%s): ", p.tag, p.tag_syn);

  switch (p.datatype)
    {
    case BOOL:
      fprintf (output_fd, "%s",
               (p.value.boolvalue == TRUE ? "true" : "false"));
      break;

    case UNSIGNED_INT:
      fprintf (output_fd, "%d", p.value.usintvalue);
      break;

    case LONG_INT:
      fprintf (output_fd, "%ld", p.value.lgintvalue);
      break;

    case UNSIGNED_LONG_INT:
      fprintf (output_fd, "%ld", p.value.uslgintvalue);
      break;

    case REAL:
      fprintf (output_fd, "%f", p.value.realvalue);
      break;

    case STRING:
      fprintf (output_fd, "%s", p.value.stringvalue);
      break;

    default:
      error_failure ("inpr_param_info", "invalid parameter type\n");
      break;
    }

  if (p.passed == TRUE)
    fprintf (stdout, "\n");
  else
    fprintf (stdout, " (default)\n");

  return;
}



/*
 * inpr_check_mandatory
 *
 * Checks if any mandatory parameter is missing
 */
static int
inpr_check_mandatory (const InputParameterList plist)
{
  UsIntValue cur_param;         /* auxiliary current parameter */
  BoolValue missing_flag = FALSE;       /* missing mandatory parameter flag */

  for (cur_param = 0; cur_param < plist.nu_parameters; cur_param++)
    {
      if (plist.parameter[cur_param].mandatory == TRUE &&
          plist.parameter[cur_param].passed == FALSE)
        {
          error_failure ("inpr_check_mandatory",
                         "mandatory parameter '%s' ('%s') missing\n",
                         plist.parameter[cur_param].tag,
                         plist.parameter[cur_param].tag_syn);

          missing_flag = TRUE;
        }
    }

  if (missing_flag == TRUE)
    return EXIT_FAILURE;
  else
    return EXIT_SUCCESS;
}



/******************************************************************************
 *                                                                            *
 *                              PUBLIC FUNCTIONS                              * *                                                                            *
 ******************************************************************************/

/*
 * inpr_parse
 *
 * Parses the parameters given by 'argc' and 'argv' against the given
 * parameter list
 */
int
inpr_parse (int argc, char **argv, InputParameterList plist)
{
  UsIntValue cp;                /* current test parameter */
  UsIntValue val_param = 0;     /* parameter waiting value */
  BoolValue value_flag = FALSE; /* indicates next argv must be a value */
  BoolValue valid_flag = FALSE; /* valid parameter found */
  int cur_arg;                  /* current argument */


  for (cur_arg = 1; cur_arg < argc; cur_arg++)
    {
      if (value_flag == FALSE)
        {
          /* Waiting parameter tag */
          cp = 0;
          valid_flag = FALSE;

          while (cp < plist.nu_parameters && valid_flag == FALSE)
            {
              if (strcmp (plist.parameter[cp].tag, argv[cur_arg]) == 0 ||
                  strcmp (plist.parameter[cp].tag_syn, argv[cur_arg]) == 0)
                {
                  /* Valid parameter found */
                  switch (plist.parameter[cp].datatype)
                    {
                    case BOOL:
                      plist.parameter[cp].value.boolvalue = TRUE;
                      plist.parameter[cp].passed = TRUE;
                      valid_flag = TRUE;
                      break;

                    case UNSIGNED_INT:
                    case LONG_INT:
                    case UNSIGNED_LONG_INT:
                    case REAL:
                    case STRING:
                      val_param = cp;
                      valid_flag = TRUE;
                      value_flag = TRUE;
                      break;

                    default:
                      return error_failure ("inpr_parse",
                                            "invalid parameter type\n");
                    }
                }
              cp++;
            }

          /* Checks if no valid parameter found */
          if (valid_flag == FALSE)
            return error_failure ("inpr_parse", "invalid parameter: %s\n",
                                  argv[cur_arg]);
        }
      else
        {
          /* Waiting parameter value */
          /*
             switch (plist.parameter[val_param].datatype)
             {
             case UNSIGNED_INT:
             plist.parameter[val_param].value.usintvalue =
             (UsIntValue) strtol (argv[cur_arg], NULL, 10);
             break;

             case LONG_INT:
             plist.parameter[val_param].value.lgintvalue =
             strtol (argv[cur_arg], NULL, 10);
             break;

             case UNSIGNED_LONG_INT:
             plist.parameter[val_param].value.uslgintvalue =
             (UsLgIntValue) strtol (argv[cur_arg], NULL, 10);
             break;

             case REAL:
             plist.parameter[val_param].value.realvalue =
             (RValue) strtod (argv[cur_arg], NULL);
             break;

             case STRING:
             plist.parameter[val_param].value.stringvalue = argv[cur_arg];
             break;

             default:
             fprintf (stderr, "inpr_parse: invalid parameter data type\n");
             return EXIT_FAILURE;
             break;
             }
           */
          if (str_to_composite (argv[cur_arg],
                                plist.parameter[val_param].datatype,
                                &(plist.parameter[val_param].value))
              != EXIT_SUCCESS)

            return error_failure ("inpr_parse", "invalid argument format\n");

          plist.parameter[val_param].passed = TRUE;
          value_flag = FALSE;
        }
    }

  /* Checks if any mandatory parameter is missing */
  if (inpr_check_mandatory (plist) != EXIT_SUCCESS)
    return error_failure ("inpr_parse", "missing mandatory parameter\n");

  return EXIT_SUCCESS;
}



/*
 * inpr_list_info
 *
 * Outputs info about the parameter list
 */
void
inpr_list_info (const InputParameterList plist, FILE * output_fd)
{
  UsIntValue cur_param;         /* current parameter */

  for (cur_param = 1; cur_param <= plist.nu_parameters; cur_param++)
    inpr_param_info (plist.parameter[cur_param - 1], output_fd);
}
