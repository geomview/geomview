DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes) \
 -I$(geomviewincludes)/private \
 $(default_includes) \
 -I$(TCL_INCLUDE_PATH) \
 -I$(TK_INCLUDE_PATH)

EXTRA_DIST = $(module_tcl_DATA)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

man_MANS =

module_PROGRAMS = labeler
labeler_SOURCES = labeler.c testwish.c
#labeler_DEPENDENCIES = $(OOGLLIB)
#labeler_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = Labeler
module_DATA = .geomview-labeler

.geomview-labeler:
	echo '(emodule-define "Labeler" "labeler $$GEOMVIEW_EMODULE_DIR/tcl/Labeler")' > .geomview-labeler

CLEANFILES = $(module_DATA)
