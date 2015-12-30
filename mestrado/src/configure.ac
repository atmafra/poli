dnl Process this file with autoconf to produce a configure script.
AC_INIT(THRASH, 1.0)

AC_CONFIG_SRCDIR(mfcc/mfcc.c)
AC_CONFIG_SRCDIR(som_vq_t/som_vq_t.c)
AC_CONFIG_SRCDIR(som_vq_f/som_vq_f.c)
AC_CONFIG_SRCDIR(lvq_rec/lvq_rec.c)
AC_CONFIG_SRCDIR(som_rec/som_rec.c)

AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE

#AC_CONFIG_HEADER

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

dnl Checks for libraries.
AC_CHECK_LIB(m,main)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h limits.h math.h sys/time.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strerror strspn strtod strtol strtoul)

dnl Output the makefile
AC_OUTPUT(matrix/Makefile strutils/Makefile vector/Makefile Makefile common/Makefile trmap/Makefile frmap/Makefile nnet/Makefile mfcc/Makefile som_vq_t/Makefile som_vq_f/Makefile som_rec/Makefile function/Makefile incstat/Makefile errorh/Makefile lvq_rec/Makefile ftrxtr/Makefile inparse/Makefile table/Makefile)
