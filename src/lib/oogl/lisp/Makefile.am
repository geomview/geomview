INCLUDES = $(default_includes)
BUILT_SOURCES = clisp.c clisp.h
EXTRA_DIST = Headers lisp2c $(BUILT_SOURCES)

noinst_LTLIBRARIES = liblisp.la

liblisp_la_SOURCES = lisp.c clisp.h lisp2.h lisp.h

all-local: clisp.c clisp.h

clisp.h: clisp.c

clisp.c: lisp.c lisp.h
	$(srcdir)/lisp2c -o $(srcdir)/clisp $(srcdir)/lisp.c

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
