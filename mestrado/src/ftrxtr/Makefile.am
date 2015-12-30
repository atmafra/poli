noinst_LIBRARIES = libftrxtr.a

libftrxtr_a_SOURCES = \
s_smptypes.h \
s_log.h \
s_complex.h s_complex.c \
s_integral.h s_integral.c \
s_files.h s_files.c \
s_fmt_wav.h s_fmt_wav.c \
s_samples.h s_samples.c \
s_window.h s_window.c \
s_preemph.h s_preemph.c \
s_fft.h s_fft.c \
s_cepstrum.h s_cepstrum.c \
s_deriv.h s_deriv.c

AM_CPPFLAGS = \
-I$(top_srcdir)/common \
-I$(top_srcdir)/incstat \
-I$(top_srcdir)/strutils \
-I$(top_srcdir)/errorh

AM_CFLAGS = -std=c89 -Wall -Werror -ggdb -W -Wno-unused-parameter
