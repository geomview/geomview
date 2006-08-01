INCLUDES = $(default_includes)

EXTRA_DIST = $(module_tcl_DATA)

LDADD = \
 $(TK_LIB) \
 $(TCL_LIB) \
 $(XLIBS) \
 $(OOGLLIB)

man_MANS =

module_PROGRAMS = labeler
labeler_SOURCES = labeler.c testwish.c
labeler_DEPENDENCIES = $(OOGLLIB)
#labeler_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = Labeler
module_DATA = .geomview-labeler

.geomview-labeler:
	echo '(emodule-define "Labeler" "labeler $$GEOMVIEW_EMODULE_PATH/tcl/Labeler")' > .geomview-labeler

all-local: $(top_builddir)/modules/.geomview-labeler

$(top_builddir)/modules/.geomview-labeler: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Labeler" "$(subdir)/labeler $(subdir)/Labeler")' > $@

CLEANFILES = $(top_builddir)/modules/.geomview-labeler $(module_DATA)
