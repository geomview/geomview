INCLUDES = $(default_includes)

man_MANS = fsaparse.3gv

EXTRA_DIST = Headers fsaparse.y fsaparse.l $(BUILT_SOURCES) $(man_MANS)

noinst_LTLIBRARIES = libwa.la

BUILT_SOURCES = wa.lex.c wa.yacc.c wa.yacc.h y.tab.h y.tab.c lex.yy.c

libwa_la_SOURCES = \
    wa.lex.c wa.yacc.c wa.yacc.h \
    wa.h wa.yystype.h

y.tab.c y.tab.h: $(srcdir)/fsaparse.y $(srcdir)/Makefile.in
	$(YACC) -d $(srcdir)/fsaparse.y

wa.yacc.c: y.tab.c $(srcdir)/Makefile.in
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' $< >$@

wa.yacc.h:  y.tab.h $(srcdir)/Makefile.in
	sed -e '/^\#include/!s/yy/wafsa/g' -e '/#ident/d' $< >$@

lex.yy.c: wa.yacc.h $(srcdir)/fsaparse.l $(srcdir)/Makefile.in
	rm -f $@
	$(LEX) $(srcdir)/fsaparse.l

wa.lex.c: lex.yy.c $(srcdir)/Makefile.in
	 sed -e '/^\#include/!s/yy/wafsa/g' \
 -e 's/YYSTYPE wafsalval/extern &/' \
 -e '/#ident/d' \
 -e 's/lex.wafsa.c/wa.lex.c/g' \
 -e 's/y.tab.h/wa.yacc.h/g' lex.yy.c >$@

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
