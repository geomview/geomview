INCLUDES = $(default_includes)
EXTRA_DIST = Headers lisp2c

noinst_LIBRARIES = liblisp.a

liblisp_a_SOURCES = \
    lisp.c \
    clisp.h lisp2.h lisp.h

lisp.o: clisp.c

clisp.c:  lisp.c
	./lisp2c -o clisp lisp.c
	touch $@
