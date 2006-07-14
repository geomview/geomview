EXTRA_DIST = README example.1 panel.fd tutorial.ps tutorial.tex

INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIB) 

module_PROGRAMS = example
example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h
example_DEPENDENCIES = $(OOGLLIB)
example_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-example

.geomview-example:
	echo '(emodule-define "Example" "example")' > .geomview-example

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Example" "$(subdir)/example")' > $(top_builddir)/modules/.geomview-example

# bin_SCRIPTS = .geomview-example
# 
# noinst_SCRIPTS = install-geomview-dotfile
# 
# install-geomview-dotfile: example
# 	$(top_builddir)/mkmodulesdir $(top_builddir)
# 	echo '(emodule-define "Example" "$(subdir)/example")' > $(top_builddir)/modules/.geomview-example

DISTCLEANFILES = .geomview-example