INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	$(SOCKETLIBS) \
	$(MATHLIB)

EXTRA_DIST = .geomview-sweep panel.fd  sweep.1 sweep.3

module_PROGRAMS = sweep
sweep_SOURCES = sweep.c sweep.main.c panel.c panel.h sweep.h

module_DATA = .geomview-sweep

.geomview-sweep:
	echo '(emodule-define "Sweep" "sweep")' > .geomview-sweep

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Sweep" "$(subdir)/sweep")' > $(top_builddir)/modules/.geomview-sweep

