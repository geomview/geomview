DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes) \
 -I$(geomviewincludes)/private \
 $(default_includes) \
 -I$(TCL_INCLUDE_PATH) \
 -I$(TK_INCLUDE_PATH)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

man_MANS = crayola.1

EXTRA_DIST = $(module_tcl_DATA) $(man_MANS)

module_PROGRAMS = crayola
crayola_SOURCES = tkcrayola.c testwish.c callbacks.h callbacks.c ui.h
#crayola_DEPENDENCIES = $(OOGLLIB)
#crayola_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = Crayola
module_DATA = .geomview-crayola

.geomview-crayola: Makefile
	echo '(emodule-define "Crayola" "crayola $$GEOMVIEW_EMODULE_DIR/tcl/Crayola")' > .geomview-crayola

CLEANFILES = $(module_DATA)

