DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes) $(default_includes) \
 -I$(TCL_INCLUDE_PATH) -I$(TK_INCLUDE_PATH)

EXTRA_DIST = NDview NDdemo

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

module_PROGRAMS = ndview
ndview_SOURCES = ndview.c testwish.c
#ndview_DEPENDENCIES =
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

CLEANFILES = $(module_DATA)

