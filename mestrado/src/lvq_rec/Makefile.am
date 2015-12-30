bin_PROGRAMS = lvq_rec

lvq_rec_SOURCES = lvq_rec.c

LDADD = \
$(top_builddir)/trmap/libtrmap.a \
$(top_builddir)/nnet/libnnet.a \
$(top_builddir)/table/libtable.a \
$(top_builddir)/inparse/libinparse.a \
$(top_builddir)/trmap/libtrmap.a \
$(top_builddir)/matrix/libmatrix.a \
$(top_builddir)/vector/libvector.a \
$(top_builddir)/function/libfunction.a \
$(top_builddir)/incstat/libincstat.a \
$(top_builddir)/strutils/libstrutils.a \
$(top_builddir)/common/libtypes.a \
$(top_builddir)/errorh/liberrorh.a


AM_CPPFLAGS = \
-I$(top_srcdir)/common \
-I$(top_srcdir)/errorh \
-I$(top_srcdir)/strutils \
-I$(top_srcdir)/function \
-I$(top_srcdir)/vector \
-I$(top_srcdir)/matrix \
-I$(top_srcdir)/inparse \
-I$(top_srcdir)/trmap \
-I$(top_srcdir)/table \
-I$(top_srcdir)/nnet

AM_CFLAGS = -std=c89 -Wall -Werror -W -ggdb
