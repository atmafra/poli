#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include "s_complex.h"

/*
 * cmp_fcn_exec
 *
 * Executes the given complex function with the list of arguments passed.
 *
 * Parameters:
 * - function: is the complex function to be executed
 * - retval: is the complex value returned by the function
 * - parc: number of parameters required by the function
 * - parlist: parameter list
 * - inpc: number of inputs
 * - ...: the list of input values
 */
int cmp_fcn_exec
  (cmp_function function, cmp_complex * retval, const cmp_fcn_mode_type mode,
   const size_t parc, va_list * parlist, const size_t inpc, ...)
{
  /* List of input arguments */
  va_list inlist;

  /* Auxiliary function exit status */
  int exit_status;

  /* Initializes the list of arguments */
  va_start (inlist, inpc);

  /* Calls the requested function */
  exit_status = (*function) (retval, mode, inpc, &inlist, parc, parlist);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "cmp_fcn_exec: error in call to complex function\n");
      return (EXIT_FAILURE);
    }

  /* Finalizes the list of arguments */
  va_end (inlist);

  return (EXIT_SUCCESS);
}





/*
 * Standard complex number operations and definitions
 */

/*
 * cmp_mod: modulus of the complex number
 */
int
cmp_mod (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary input value */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_mod: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_mod: only complex mode is allowed for function modulus\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the modulus */
  (*retval).re = sqrt (sqr (aux_z.re) + sqr (aux_z.im));
  (*retval).im = 0.0;

  return (EXIT_SUCCESS);
}




/*
 * cmp_sqr: squared modulus of the complex number
 */
int
cmp_sqr (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary input value */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_sqr: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_sqr: only complex mode is allowed for function square\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the modulus */
  (*retval).re = sqr (aux_z.re) + sqr (aux_z.im);
  (*retval).im = 0.0;

  return (EXIT_SUCCESS);
}




/*
 * cmp_arg: argument of the complex number
 */
int
cmp_arg (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary input value */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_arg: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_arg: only complex mode is allowed for function argument\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the argument */
  if (aux_z.re < DBL_EPSILON && aux_z.re > -DBL_EPSILON)
    (*retval).re = atan ((double) DBL_MAX);
  else
    (*retval).re = atan (aux_z.im / aux_z.re);

  return (EXIT_SUCCESS);
}




/*
 * cmp_min: minus input complex
 */
int
cmp_min (cmp_complex * retval, const cmp_fcn_mode_type mode,
         size_t inpc, va_list * inlist, const size_t parc, va_list * parlist)
{
  /* Auxiliary input value */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_minus: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the opposite */
  (*retval).re = -aux_z.re;
  if (mode == CMP_FCN_COMPLEX)
    (*retval).im = -aux_z.im;

  return (EXIT_SUCCESS);
}




/*
 * cmp_cnj: complex conjugate
 */
int
cmp_cnj (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary input value */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_cnj: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_cnj: only complex mode is allowed for function conjugate\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the conjugate */
  (*retval).re = +aux_z.re;
  (*retval).im = -aux_z.im;

  return (EXIT_SUCCESS);
}




/*
 * cmp_sum: sum of all the input complexes
 *
 * No restrictions on the number of input arguments
 */
int
cmp_sum (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary argument counter */
  size_t aux_arg;

  /* Auxiliary argument value */
  cmp_complex aux_z;


  /* Initializes the return value with the neutral element */
  (*retval).re = 0.0;
  (*retval).im = 0.0;

  /* Trivial case */
  if (inpc == 0)
    return (EXIT_SUCCESS);

  /* Sum loop */
  /* The 'if' was put outside the loop for performance matters */
  if (mode == CMP_FCN_COMPLEX)
    {
      for (aux_arg = 0; aux_arg < inpc; aux_arg++)
        {
          /* Gets the argument from the list */
          aux_z = va_arg (*inlist, cmp_complex);

          /* Accumulates the sum */
          (*retval).re += aux_z.re;
          (*retval).im += aux_z.im;
        }
    }
  else
    {
      for (aux_arg = 0; aux_arg < inpc; aux_arg++)
        {
          /* Gets the argument from the list */
          aux_z = va_arg (*inlist, cmp_complex);

          /* Accumulates the sum */
          (*retval).re += aux_z.re;
        }
    }

  return (EXIT_SUCCESS);
}




/*
 * cmp_sbt: first input minus all the rest
 *
 * No restrictions on the number of input arguments
 */
int
cmp_sbt (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary argument counter */
  size_t aux_arg;

  /* Auxiliary argument value */
  cmp_complex aux_z;


  /* Initializes the return value with the neutral element */
  (*retval).re = 0.0;
  (*retval).im = 0.0;

  /* Trivial case */
  if (inpc == 0)
    return (EXIT_SUCCESS);

  /* Subtraction loop */
  if (mode == CMP_FCN_COMPLEX)
    {
      for (aux_arg = 0; aux_arg < inpc; aux_arg++)
        {
          /* Gets the argument from the list */
          aux_z = va_arg (*inlist, cmp_complex);

          /* Accumulates the result */
          (*retval).re += (aux_arg == 0 ? aux_z.re : -aux_z.re);
          (*retval).im += (aux_arg == 0 ? aux_z.im : -aux_z.im);
        }
    }
  else
    {
      for (aux_arg = 0; aux_arg < inpc; aux_arg++)
        {
          /* Gets the argument from the list */
          aux_z = va_arg (*inlist, cmp_complex);

          /* Accumulates the result */
          (*retval).re += (aux_arg == 0 ? aux_z.re : -aux_z.re);
        }
    }

  return (EXIT_SUCCESS);
}




/*
 * cmp_rpd: multiplies a complex by all of the following real numbers
 *
 * First argument is assumed to be the complex number, which will be
 * multiplied by all of the subsequent real arguments
 */
int
cmp_rpd (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary argument counter */
  size_t aux_arg;

  /* Auxiliary real constant */
  cmp_real k;

  /* Auxiliary complex */
  cmp_complex aux_z;


  /* This function accepts only one input argument */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_rpd: only one argument is accepted for this function\n");
      return (EXIT_FAILURE);
    }

  /* This function requires at least one real coefficient */
  if (parc < 1)
    {
      fprintf (stderr, "cmp_rpd: at least one real parameter is required\n");
      return (EXIT_FAILURE);
    }

  /* Gets the complex parameter */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Initializes the return value */
  (*retval).re = aux_z.re;
  (*retval).im = aux_z.im;

  /* Trivial case */
  if (inpc == 1)
    return (EXIT_SUCCESS);

  /* Product loop */
  if (mode == CMP_FCN_COMPLEX)
    {
      for (aux_arg = 0; aux_arg < parc; aux_arg++)
        {
          /* Gets the real input parameter */
          k = va_arg (*parlist, cmp_real);

          /* Calculates the real product */
          (*retval).re *= k;
          (*retval).im *= k;
        }
    }
  else
    {
      for (aux_arg = 0; aux_arg < parc; aux_arg++)
        {
          /* Gets the real input parameter */
          k = va_arg (*parlist, cmp_real);

          /* Calculates the real product */
          (*retval).re *= k;
        }
    }

  return (EXIT_SUCCESS);
}




/*
 * cmp_tms: complex product of all the inputs
 */
int
cmp_tms (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary argument counter */
  size_t aux_arg;

  /* Auxiliary complexes */
  cmp_complex aux_z, aux_z2;



  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_tms: only complex mode is allowed for function complex times\n");
      return (EXIT_FAILURE);
    }

  /* Initializes the return value with the neutral element */
  (*retval).re = 1.0;
  (*retval).im = 0.0;

  /* Trivial case */
  if (inpc == 0)
    return (EXIT_SUCCESS);

  /* Multiplication loop */
  for (aux_arg = 0; aux_arg < inpc; aux_arg++)
    {
      /* Gets the argument from the list */
      aux_z = va_arg (*inlist, cmp_complex);

      /* Stores the last value of 'retval' */
      aux_z2.re = (*retval).re;
      aux_z2.im = (*retval).im;

      /* Accumulates the product */
      (*retval).re = aux_z2.re * aux_z.re - aux_z2.im * aux_z.im;
      (*retval).im = aux_z2.re * aux_z.im + aux_z2.im * aux_z.re;
    }

  return (EXIT_SUCCESS);
}



/*
 * cmp_inv: inverse of the complex (1/z)
 */
int
cmp_inv (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary denominator value */
  cmp_real den;

  /* Auxiliary complex */
  cmp_complex aux_z;


  /* Validates the number of input arguments */
  if (inpc != 1)
    {
      fprintf (stderr,
               "cmp_inv: this function accepts only one input argument\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_inv: only complex mode is allowed for function complex invert\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input value from list */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the auxiliary denominator */
  den = sqr (aux_z.re) + sqr (aux_z.im);
  if (den < DBL_EPSILON && den > -DBL_EPSILON)
    {
      fprintf (stderr, "cmp_inv: non-invertible complex\n");
      return (EXIT_FAILURE);
    }

  /* Calculates the inverse */
  (*retval).re = aux_z.re / den;
  (*retval).im = -aux_z.im / den;

  return (EXIT_SUCCESS);
}




/*
 * cmp_exp: exponential of z.
 *
 * If z = a + jb, then exp z = e^a (cos(b) + j sin(b))
 */
int
cmp_exp (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary complex */
  cmp_complex aux_z;

  /* Auxiliary real, just to calculate the exponential only once */
  cmp_real aux_exp;


  /* Validates the number of arguments */
  if (inpc != 1)
    {
      fprintf (stderr, "cmp_exp: function requires only one argument\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_exp: only complex mode is allowed for function complex invert\n");
      return (EXIT_FAILURE);
    }

  /* Gets the complex value from the input list */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the exponential */
  aux_exp = exp (aux_z.re);
  (*retval).re = aux_exp * cos (aux_z.im);
  (*retval).im = aux_exp * sin (aux_z.im);

  return (EXIT_SUCCESS);
}




/*
 * cmp_rlg: logarithm of the real part and the imaginary part.
 * log(Re(z)) + j log(Im(z))
 *
 * The first argument must be 'z' and the second (real) argument is the basis.
 */
int
cmp_rlg (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary complex number */
  cmp_complex aux_z;

  /* Logarithm basis */
  cmp_real basis;


  /* Validates the number of input arguments */
  if (inpc != 1)
    {
      fprintf (stderr, "cmp_rlg: this function requires exactly one input\n");
      return (EXIT_FAILURE);
    }

  /* Validates the number of parameters */
  if (parc != 1)
    {
      fprintf (stderr,
               "cmp_rlg: this function requires exactly one parameter\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Corrects zeroed values to one */
  if (aux_z.re < DBL_EPSILON && aux_z.re > -DBL_EPSILON)
    aux_z.re = 1.0;

  if (aux_z.im < DBL_EPSILON && aux_z.im > -DBL_EPSILON)
    aux_z.im = 1.0;

  /*
   * Validates the interval for both real and imaginary parts
   * Must be positive
   */
  if (aux_z.re < DBL_EPSILON || aux_z.im < DBL_EPSILON)
    {
      fprintf (stderr, "cmp_rlg: invalid rlg argument: %f + j %f\n",
               aux_z.re, aux_z.im);
      return (EXIT_FAILURE);
    }

  /* Gets the basis */
  basis = va_arg (*parlist, cmp_real);

  /*
   * Validates the interval of the basis.
   * If the basis is 'b', then: b > 0,  b != 1
   */
  if (basis < DBL_EPSILON
      || (basis < 1.0 + DBL_EPSILON && basis > 1.0 - DBL_EPSILON))
    {
      fprintf (stderr, "cmp_rlg: invalid log basis: %f\n", basis);
      return (EXIT_FAILURE);
    }

  /* Calculates the log */
  (*retval).re = logb (aux_z.re, basis);
  if (mode == CMP_FCN_COMPLEX)
    (*retval).im = logb (aux_z.im, basis);

  return (EXIT_SUCCESS);
}




/*
 * cmp_abs
 *
 * Returns the absolute value of the real and imaginary parts
 */
int
cmp_abs (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary complex */
  cmp_complex aux_z;


  /* This function accepts only one input */
  if (inpc != 1)
    {
      fprintf (stderr, "cmp_abs: this function requires exactly one input\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input from the list */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Calculates the absolute value */
  aux_z.re = fabs (aux_z.re);
  if (mode == CMP_FCN_COMPLEX)
    aux_z.im = fabs (aux_z.im);

  return (EXIT_SUCCESS);
}




/*
 * cmp_rlm: logarithm of the module
 * 0.5 * log(Re(z)^2 + Im(z)^2)
 *
 * The first argument must be 'z' and the second (real) argument is the basis.
 */
int
cmp_rlm (cmp_complex * retval, const cmp_fcn_mode_type mode,
         const size_t inpc, va_list * inlist,
         const size_t parc, va_list * parlist)
{
  /* Auxiliary complex number */
  cmp_complex aux_z;

  /* Logarithm basis */
  cmp_real basis;


  /* Validates the number of input arguments */
  if (inpc != 1)
    {
      fprintf (stderr, "cmp_rlm: this function requires exactly one input\n");
      return (EXIT_FAILURE);
    }

  /* Validates the number of parameters */
  if (parc != 1)
    {
      fprintf (stderr,
               "cmp_rlm: this function requires exactly one parameter\n");
      return (EXIT_FAILURE);
    }

  /* Only complex mode is accepted */
  if (mode != CMP_FCN_COMPLEX)
    {
      fprintf (stderr,
               "cmp_rlm: only complex mode is allowed for function log of modulus\n");
      return (EXIT_FAILURE);
    }

  /* Gets the input complex */
  aux_z = va_arg (*inlist, cmp_complex);

  /* Corrects zeroed values */
  /*
     if (aux_z.re < 1.0 + DBL_EPSILON)
     aux_z.re = 1.0;

     if (aux_z.im < DBL_EPSILON)
     aux_z.im = 1.0;
   */

  /* Gets the basis */
  basis = va_arg (*parlist, cmp_real);

  /*
   * Validates the interval of the basis.
   * If the basis is 'b', then: b > 0,  b != 1
   */
  if (basis < DBL_EPSILON
      || (basis < 1.0 + DBL_EPSILON && basis > 1.0 - DBL_EPSILON))
    {
      fprintf (stderr, "cmp_rlm: invalid log basis: %f\n", basis);
      return (EXIT_FAILURE);
    }

  /* Calculates the log */
  (*retval).re = 0.5 * log (sqr (aux_z.re) + sqr (aux_z.im));
  (*retval).im = 0.0;

  return (EXIT_SUCCESS);
}
