INCLUDES = $(default_includes)

LDADD = \
 $(TK_LIB) \
 $(TCL_LIB) \
 $(XLIBS) \
 $(OOGLLIB)

man_MANS = crayola.1

EXTRA_DIST = crayola.1 Crayola

module_PROGRAMS = crayola
crayola_SOURCES = tkcrayola.c testwish.c callbacks.h callbacks.c ui.h
crayola_DEPENDENCIES = $(OOGLLIB)
#crayola_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = Crayola
module_DATA = .geomview-crayola

.geomview-crayola: Makefile
	echo '(emodule-define "Crayola" "crayola $$GEOMVIEW_EMODULE_PATH/tcl/Crayola")' > .geomview-crayola

all-local: $(top_builddir)/modules/.geomview-crayola

$(top_builddir)/modules/.geomview-crayola: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Crayola" "$(subdir)/crayola $(subdir)/Crayola")' > $@

CLEANFILES = $(module_DATA) $(top_builddir)/modules/.geomview-crayola
