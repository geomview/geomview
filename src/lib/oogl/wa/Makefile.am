INCLUDES = $(default_includes)
EXTRA_DIST = Headers fsaparse.3 fsaparse.y fsaparse.l $(BUILT_SOURCES)

noinst_LTLIBRARIES = libwa.la

BUILT_SOURCES = wa.lex.c wa.yacc.c y.tab.h y.tab.c lex.yy.c

libwa_la_SOURCES = \
    wa.lex.c wa.yacc.c \
    wa.h y.tab.h yystype.h

y.tab.c y.tab.h wa.yacc.c: $(srcdir)/fsaparse.y
	rm -f y.tab.h y.tab.c $@
	($(YACC) -d $(srcdir)/fsaparse.y; \
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' y.tab.c) >$@

wa.lex.c: $(srcdir)/fsaparse.l
	rm -f lex.yy.c $@
	($(LEX) $(srcdir)/fsaparse.l; \
	 sed -e '/^\#include/!s/yy/wafsa/g' \
	     -e 's/YYSTYPE wafsalval/extern &/' \
             -e '/#ident/d' \
             -e 's/lex.wafsa.c/wa.lex.c/g' lex.yy.c) >$@

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
