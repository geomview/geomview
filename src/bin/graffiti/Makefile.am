INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIB)

EXTRA_DIST = .geomview-graffiti graffiti.1

module_PROGRAMS = graffiti
graffiti_SOURCES = graffiti.c gui.c lang.h panel.h graffiti.h lang.c panel.c
graffiti_DEPENDENCIES = $(OOGLLIB)
graffiti_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-graffiti

.geomview-graffiti:
	echo '(emodule-define "Graffiti" "graffiti")' > .geomview-graffiti

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Graffiti" "$(subdir)/graffiti")' > $(top_builddir)/modules/.geomview-graffiti
