bin_PROGRAMS = mfcc

mfcc_SOURCES = mfcc.c

LDADD = \
$(top_builddir)/errorh/liberrorh.a \
$(top_builddir)/strutils/libstrutils.a \
$(top_builddir)/ftrxtr/libftrxtr.a \
$(top_builddir)/inparse/libinparse.a \
$(top_builddir)/incstat/libincstat.a \
$(top_builddir)/common/libtypes.a

AM_CPPFLAGS = \
-I$(top_srcdir)/common \
-I$(top_srcdir)/errorh \
-I$(top_srcdir)/strutils \
-I$(top_srcdir)/ftrxtr \
-I$(top_srcdir)/inparse

AM_CFLAGS = -std=c89 -Wall -Werror -ggdb -W
