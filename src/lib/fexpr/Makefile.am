INCLUDES = $(default_includes)
EXTRA_DIST = Headers parse.tab.c-saved

noinst_LIBRARIES = libfexpr.a

libfexpr_a_SOURCES = \
    binfuncs.c complex.c evaluate.c exprs.c lex.yy.c monfuncs.c parse.tab.c real.c vars.c \
    fexpr.h parse.tab.h

parse.tab.c: parse.tab.c-saved
	cp parse.tab.c-saved parse.tab.c
