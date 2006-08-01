EXTRA_DIST = README example.1 panel.fd tutorial.ps tutorial.tex

INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(OOGLLIB) $(XFORMSLIBS) $(XLIBS)

module_PROGRAMS = example
example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h
example_DEPENDENCIES = $(OOGLLIB)
#example_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-example

.geomview-example:
	echo '(emodule-define "Example" "example")' > .geomview-example

all-local: $(top_builddir)/modules/.geomview-example

$(top_builddir)/modules/.geomview-example: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Example" "$(subdir)/example")' > $@

CLEANFILES = $(module_DATA) $(top_builddir)/modules/.geomview-example