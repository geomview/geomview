INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	$(SOCKETLIBS) \
	$(MATHLIB)

EXTRA_DIST = .geomview-graffiti graffiti.1

bin_PROGRAMS = graffiti

bin_SCRIPTS = .geomview-graffiti

graffiti_SOURCES = graffiti.c gui.c lang.h panel.h graffiti.h lang.c panel.c

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: graffiti
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Graffiti" "$(subdir)/graffiti")' > $(top_builddir)/modules/.geomview-graffiti


.geomview-graffiti:
	echo '(emodule-define "Graffiti" "graffiti")' > .geomview-graffiti
