INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(top_builddir)/src/lib/fexpr/libfexpr.a                    \
	$(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-cplxview cplxhelp.txt cplxpanels2.fd cplxview.1

modules_PROGRAMS = cplxview

modules_DATA = .geomview-cplxview

cplxview_SOURCES = computegraph.c cplxpanels2.c cplxpanels2.h cplxview.c
