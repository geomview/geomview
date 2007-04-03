DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(TCL_INCLUDE_PATH)\
 -I$(TK_INCLUDE_PATH)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

module_PROGRAMS = ndview
ndview_SOURCES = ndview.c testwish.c
#ndview_DEPENDENCIES =
#ndview_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = NDview NDdemo

EMODULES = .geomview-ndview .geomview-nddemo

ndviewdir          = $(geomdatadir)/NDview
ndview_scriptsdir  = $(ndviewdir)/scripts
ndview_geomdir     = $(ndviewdir)/geom
ndview_geomdemodir = $(ndview_geomdir)/demo
ndview_textdir     = $(ndviewdir)/text

if LOCAL_EMODULE
  homedir = $(HOME)
  home_DATA = $(EMODULES)
  tclpath = $(module_tcldir)
  modulepath = $(moduledir)/
else
  tclpath = $$GEOMVIEW_EMODULE_DIR/tcl
  module_DATA = $(EMODULES)
if !GEOMVIEW_QUERY
  modulepath = $(moduledir)/
else
  modulepath =
endif
endif

.geomview-ndview: Makefile
	echo '(emodule-define "NDview" "$(modulepath)ndview $(tclpath)/NDview")' > $@

.geomview-nddemo: Makefile
	echo '(emodule-define "NDdemo" "$(modulepath)nddemo $(tclpath)/NDdemo")' > $@

.geomview: Makefile
	echo '(emodule-define "NDview" "$(abs_builddir)/ndview $(abs_srcdir)/NDview")' > $@
	echo '(emodule-define "NDdemo" "$(abs_builddir)/nddemo $(abs_srcdir)/NDdemo")' >> $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

EXTRA_DIST = $(module_tcl_DATA) $(man_MANS)
