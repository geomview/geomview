DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes)/private \
 $(default_includes) \
 $(TK_ALL_INCLUDES)

LDADD = $(OOGLLIB) $(TK_LIB) $(TCL_LIB) $(XLIBS)

module_tcldir = $(moduledir)/tcl

man_MANS = crayola.1gv

module_tcl_DATA = Crayola
module_PROGRAMS = crayola
crayola_SOURCES = tkcrayola.c testwish.c callbacks.h callbacks.c ui.h

EMODULES = .geomview-crayola

if LOCAL_EMODULE
  homedir = $(HOME)
  home_DATA = $(EMODULES)
  tclpath = $(module_tcldir)
  modulepath = $(moduledir)/
else
  module_DATA = $(EMODULES)
  tclpath = $$GEOMVIEW_EMODULE_DIR/tcl
if !GEOMVIEW_QUERY
    modulepath = $(moduledir)/
else
    modulepath =
endif
endif

.geomview-crayola: Makefile
	echo '(emodule-define "Crayola" "$(modulepath)crayola $(tclpath)/Crayola")' > $@

.geomview: Makefile
	echo '(emodule-define "Crayola" "$(abs_builddir)/crayola $(abs_srcdir)/Crayola")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

EXTRA_DIST = $(module_tcl_DATA) $(man_MANS)
