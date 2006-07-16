INCLUDES = $(default_includes)
EXTRA_DIST = Headers parse.tab.c-saved

noinst_LTLIBRARIES = libfexpr.la

libfexpr_la_SOURCES = \
    binfuncs.c complex.c evaluate.c exprs.c lex.yy.c monfuncs.c parse.tab.c real.c vars.c \
    fexpr.h parse.tab.h fexprnew.h

parse.tab.c: parse.tab.c-saved
	cp $(srcdir)/parse.tab.c-saved parse.tab.c
