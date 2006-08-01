INCLUDES = $(default_includes) $(XFORMSINCLUDE)

man_MANS = cplxview.1

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)      \
	$(OOGLLIB) 

EXTRA_DIST = cplxpanels2.fd cplxview.1

module_PROGRAMS = cplxview
cplxview_SOURCES = computegraph.c cplxpanels2.c cplxpanels2.h cplxview.c
cplxview_DEPENDENCIES = $(OOGLLIB)
#cplxview_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-cplxview

.geomview-cplxview: Makefile
	echo '(emodule-define "Cplxview" "cplxview")' > .geomview-cplxview

all-local: $(top_builddir)/modules/.geomview-cplxview

$(top_builddir)/modules/.geomview-cplxview: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Cplxview" "$(subdir)/cplxview")' > $@

CLEANFILES = $(top_builddir)/modules/.geomview-cplxview $(module_DATA)
