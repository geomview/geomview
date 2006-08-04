INCLUDES = $(default_includes)
EXTRA_DIST = Headers fsaparse.3 fsaparse.y fsaparse.l $(BUILT_SOURCES)

noinst_LTLIBRARIES = libwa.la

BUILT_SOURCES = wa.lex.c wa.yacc.c wa.yacc.h y.tab.h y.tab.c lex.yy.c

libwa_la_SOURCES = \
    wa.lex.c wa.yacc.c wa.yacc.h \
    wa.h wa.yystype.h

y.tab.c y.tab.h: $(srcdir)/fsaparse.y Makefile
	$(YACC) -d $(srcdir)/fsaparse.y

wa.yacc.c: y.tab.c Makefile
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' $< >$@

wa.yacc.h:  y.tab.h Makefile
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' $< >$@

wa.lex.c: wa.yacc.h $(srcdir)/fsaparse.l Makefile
	rm -f lex.yy.c $@
	($(LEX) $(srcdir)/fsaparse.l; \
	 sed -e '/^\#include/!s/yy/wafsa/g' \
 -e 's/YYSTYPE wafsalval/extern &/' \
 -e '/#ident/d' \
 -e 's/lex.wafsa.c/wa.lex.c/g' \
 -e 's/y.tab.h/wa.yacc.h/g' lex.yy.c) >$@

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
