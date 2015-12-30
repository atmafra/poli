/*
 * Complex Numbers support
 */
#ifndef __CMP_COMPLEX_
#define __CMP_COMPLEX_ 1

#include <stdarg.h>
#include <stdlib.h>
#include "../common/types.h"

#define cmp_re(z1) ((z1).re)
#define cmp_im(z1) ((z1).im)
#define logb(z,b) (log(z) / log(b))

/*
 * Data type for the real domains
 */
typedef RValue cmp_real;


/*
 * Data type for the complexes
 */
typedef CValue cmp_complex;


/*
 * cmp_fcn_mode_type
 *
 * Complex function execution mode. Selects the behaviour of the calculation,
 * defining if Real or Complex numbers should be assumed.
 */
typedef enum
{ CMP_FCN_REAL, CMP_FCN_COMPLEX }
cmp_fcn_mode_type;


/*
 * cmp_function
 *
 * Pointer to Complex Function data type
 *
 *
 * The complex function data type is defined for functions like:
 *   z = f (z1, ... , zi, ..., zN)
 * where:
 *   zi is a complex input parameter
 *   z is the return value
 *
 * Parameters
 * - retval: is the complex returned value (z)
 * - mode: selects Real or Complex calculation mode
 * - inpc: inputs count. Value of 'N'.
 * - inlist: the list of arguments
 * - parc: parameters count. Number of parameters for the function.
 * - parlist: the list of parameters
 *
 * Returned Value
 * - 'EXIT_SUCCESS', if the calculation performed successfully;
 * - 'EXIT_FAILURE', otherwise.
 */
typedef int (*cmp_function) (cmp_complex * retval,
                             const cmp_fcn_mode_type mode, const size_t inpc,
                             va_list * inlist, const size_t parc,
                             va_list * parlist);

/*
 * cmp_fcn_exec
 *
 * Executes the given complex function with the list of arguments passed.
 *
 * Parameters:
 * - function: is the complex function to be executed
 * - retval: is the complex value returned by the function
 * - mode: selects Real or Complex calculation mode
 * - parc: number of input arguments to process
 * - parlist: list of parameters to the function
 * - inpc: number of inputs
 * - ...: the list of arguments
 */
int cmp_fcn_exec (cmp_function function, cmp_complex * retval,
                  const cmp_fcn_mode_type mode, const size_t parc,
                  va_list * parlist, const size_t inpc, ...);


/*
 * Operations defined for complexes
 *
 * For a given complex z of the form z = a + jb,
 * cmp_mod: Modulus of z. sqrt (a2 + b2)
 * cmp_sqr: Squared modulus of z. a2 + b2
 * cmp_arg: Argument of z. atan (b/a)
 * cmp_min: Minus z. -a - jb
 * cmp_con: Conjugate of z. a - jb
 * cmp_sum: Sum of the input list. z1 + z2 + ... + zN
 * cmp_sub: Subtraction. First complex minus all others. z1 - z2 - ... - zN
 * cmp_rpd: Real product. Complex times list of reals. z * k1 * k2 * ... * kN
 * cmp_tms: Product of the list. z1 * z2 * ... * zN
 * cmp_inv: Inverse of z. 1/z
 * cmp_exp: Exponential. exp z = e^a * (cos(b) + j sin(b))
 * cmp_rlg: Logarithm of the real and imaginary parts. log(a) + j log(b)
 * cmp_abs: Absolute value of the real and imaginary parts.
 * cmp_rlm: Logarithm of the modulus
 * TODO cmp_log: logarithm of z, at the given basis
 */
int cmp_mod (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_sqr (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_arg (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_min (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_cnj (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_sum (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_sbt (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_rpd (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_tms (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_inv (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_exp (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_rlg (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_abs (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);
int cmp_rlm (cmp_complex * retval, const cmp_fcn_mode_type mode,
             const size_t inpc, va_list * inlist,
             const size_t parc, va_list * parlist);

#endif /* !__CMP_COMPLEX_ */
