EXTRA_DIST = README example.1 panel.fd tutorial.ps tutorial.tex

INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(FORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIBS)  \
        $(MATHLIB)

bin_PROGRAMS = example

bin_SCRIPTS = .geomview-example

example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: example
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Example" "$(subdir)/example")' > $(top_builddir)/modules/.geomview-example


.geomview-example:
	echo '(emodule-define "Example" "example")' > .geomview-example
