bin_PROGRAMS = qwertickle

AM_CFLAGS = $(QWERTICKLE_CFLAGS) \
	    -Wall -Wextra -O2 \
	    -DDATADIR=\"$(datadir)/$(PACKAGE)\"

qwertickle_LDADD = $(QWERTICKLE_LIBS)

qwertickle_SOURCES = qwertickle.c qwertickle.h
