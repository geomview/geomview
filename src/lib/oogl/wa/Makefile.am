INCLUDES = $(default_includes)
EXTRA_DIST = Headers fsaparse.3 fsaparse.y fsaparse.l

noinst_LIBRARIES = libwa.a

libwa_a_SOURCES = \
    wa.lex.c wa.yacc.c \
    wa.h y.tab.h yystype.h

wa.yacc.c: fsaparse.y
	rm -f y.tab.h y.tab.c $@
	(yacc -d fsaparse.y; \
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' y.tab.c) >$@

wa.lex.c: fsaparse.l
	rm -f lex.yy.c $@
	(lex fsaparse.l; \
	 sed -e '/^\#include/!s/yy/wafsa/g' \
	     -e 's/YYSTYPE wafsalval/extern &/' -e '/#ident/d' lex.yy.c) >$@
