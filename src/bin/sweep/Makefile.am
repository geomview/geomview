INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-sweep panel.fd  sweep.1 sweep.3

modules_PROGRAMS = sweep

modules_DATA = .geomview-sweep

sweep_SOURCES = sweep.c sweep.main.c panel.c panel.h sweep.h
