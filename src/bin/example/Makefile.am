EXTRA_DIST = README example.1 panel.fd tutorial.ps tutorial.tex

INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(FORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIBS)

modules_PROGRAMS = example

modules_DATA = .geomview-example

example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h
