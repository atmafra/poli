EXTRA_DIST = bootstrap
SUBDIRS = \
errorh vector matrix table trmap incstat function strutils ftrxtr inparse nnet

bin_PROGRAMS = mfcc som_vq

mfcc_SOURCES = mfcc.c
mfcc_LDADD = \
$(top_builddir)/errorh/liberrorh.a \
$(top_builddir)/vector/libvector.a \
$(top_builddir)/matrix/libmatrix.a \
$(top_builddir)/function/libfunction.a \
$(top_builddir)/ftrxtr/libftrxtr.a \
$(top_builddir)/inparse/libinparse.a \
$(top_builddir)/strutils/libstrutils.a \
$(top_builddir)/incstat/libincstat.a

som_vq_SOURCES = som_vq.c
som_vq_LDADD = \
$(top_builddir)/nnet/som/libnnetsom.a \
$(top_builddir)/nnet/lvq/libnnetlvq.a \
$(top_builddir)/errorh/liberrorh.a \
$(top_builddir)/strutils/libstrutils.a \
$(top_builddir)/matrix/libmatrix.a \
$(top_builddir)/table/libtable.a \
$(top_builddir)/nnet/libnnet.a \
$(top_builddir)/trmap/libtrmap.a \
$(top_builddir)/vector/libvector.a \
$(top_builddir)/incstat/libincstat.a \
$(top_builddir)/function/libfunction.a \
$(top_builddir)/inparse/libinparse.a

AM_CFLAGS = -ansi -Wall -Werror -ggdb
