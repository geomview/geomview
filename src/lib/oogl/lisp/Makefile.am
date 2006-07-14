INCLUDES = $(default_includes)
EXTRA_DIST = Headers lisp2c # $(BUILT_SOURCES)

noinst_LTLIBRARIES = liblisp.la

BUILT_SOURCES = clisp.c clisp.h

liblisp_la_SOURCES = \
    lisp.c \
    clisp.h lisp2.h lisp.h

all-local: clisp

clisp.c clisp.h clisp: lisp.c
	$(srcdir)/lisp2c -o clisp $(srcdir)/lisp.c

DISTCLEANFILES = $(BUILT_SOURCES)
