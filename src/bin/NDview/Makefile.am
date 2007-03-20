INCLUDES = $(default_includes)

EXTRA_DIST = NDview NDdemo

LDADD = \
 $(TK_LIB) \
 $(TCL_LIB) \
 $(XLIBS) \
 $(OOGLLIB)

module_PROGRAMS = ndview
ndview_SOURCES = ndview.c testwish.c
ndview_DEPENDENCIES = $(OOGLLIB)
#ndview_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = NDview NDdemo
module_DATA = .geomview-ndview .geomview-nddemo

ndviewdir          = $(geomdatadir)/NDview
ndview_scriptsdir  = $(ndviewdir)/scripts
ndview_geomdir     = $(ndviewdir)/geom
ndview_geomdemodir = $(ndview_geomdir)/demo
ndview_textdir     = $(ndviewdir)/text

.geomview-ndview: Makefile
	echo '(emodule-define "NDview" "ndview $$GEOMVIEW_EMODULE_DIR/tcl/NDview")' > .geomview-ndview

.geomview-nddemo: Makefile
	echo '(emodule-define "NDdemo" "ndview $$GEOMVIEW_EMODULE_DIR/tcl/NDdemo")' > .geomview-nddemo

$(top_builddir)/modules/.geomview-ndview: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "NDview" "$(subdir)/ndview $(realpath $(srcdir))/NDview")' > $@

$(top_builddir)/modules/.geomview-nddemo: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "NDdemo" "$(subdir)/ndview $(realpath $(srcdir))/$(srcdir)/NDdemo")' > $@

all-local: $(top_builddir)/modules/.geomview-ndview  $(top_builddir)/modules/.geomview-nddemo

CLEANFILES = \
 $(top_builddir)/modules/.geomview-ndview \
 $(top_builddir)/modules/.geomview-nddemo \
 $(module_DATA)

