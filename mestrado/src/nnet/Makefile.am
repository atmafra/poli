noinst_LIBRARIES = libnnet.a

libnnet_a_SOURCES = \
nnet_types.h \
nnet_nnet.h nnet_nnet.c \
nnet_layers.h nnet_layers.c \
nnet_units.h nnet_units.c \
nnet_conns.h nnet_conns.c \
nnet_weights.h nnet_weights.c \
nnet_actv.h nnet_actv.c \
nnet_metrics.h nnet_metrics.c \
nnet_sets.h nnet_sets.c \
nnet_train.h nnet_train.c \
nnet_files.h nnet_files.c \
nnet_files_nnet.h nnet_files_nnet.c \
nnet_som.c nnet_som.h \
nnet_som_ngb.c nnet_som_ngb.h \
nnet_lvq.c nnet_lvq.h \
nnet_lvq_window.c nnet_lvq_window.h

AM_CPPFLAGS = \
-I$(top_srcdir)/vector \
-I$(top_srcdir)/errorh \
-I$(top_srcdir)/common \
-I$(top_srcdir)/function \
-I$(top_srcdir)/incstat \
-I$(top_srcdir)/strutils \
-I$(top_srcdir)/table \
-I$(top_srcdir)/trmap \
-I$(top_srcdir)/frmap \
-I$(top_srcdir)/matrix

AM_CFLAGS = -std=c89 -Wall -Werror -ggdb -W -Wno-unused-parameter
