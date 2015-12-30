dnl Process this file with autoconf to produce a configure script.
AC_INIT(som_vq,0.1))

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(mfcc.c)
AC_CONFIG_SRCDIR(som_vq.c)

AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

dnl Checks for libraries.
AC_CHECK_LIB(m,main)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h math.h sys/time.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS(strerror strtod)

dnl Output the makefile
AC_OUTPUT(Makefile errorh/Makefile incstat/Makefile ftrxtr/Makefile strutils/Makefile vector/Makefile matrix/Makefile table/Makefile trmap/Makefile function/Makefile inparse/Makefile nnet/Makefile nnet/som/Makefile nnet/lvq/Makefile)
