INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-flythrough flyhelp flythrough.1 panel.fd

bin_PROGRAMS = flythrough

bin_SCRIPTS = .geomview-flythrough

flythrough_SOURCES = main.c flyhelp.h panel.c panel.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: flythrough
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Flythrough" "$(subdir)/flythrough")' > $(top_builddir)/modules/.geomview-flythrough


.geomview-flythrough:
	echo '(emodule-define "Flythrough" "flythrough")' > .geomview-flythrough
