INCLUDES = $(default_includes)
BUILT_SOURCES = clisp.c clisp.h
EXTRA_DIST = Headers lisp2c $(BUILT_SOURCES)

noinst_LTLIBRARIES = liblisp.la

liblisp_la_SOURCES = \
    lisp.c \
    clisp.h lisp2.h lisp.h

if MAINTAINER_MODE
all-local: clisp

clisp.c clisp.h clisp: lisp.c
	$(srcdir)/lisp2c -o clisp $(srcdir)/lisp.c
endif

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
