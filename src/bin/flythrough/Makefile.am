INCLUDES = $(default_includes) $(XFORMSINCLUDE)

man_MANS = flythrough.1

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIB) 

EXTRA_DIST = flyhelp flythrough.1 panel.fd

module_PROGRAMS = flythrough
flythrough_SOURCES = main.c flyhelp.h panel.c panel.h flythrough.h
flythrough_DEPENDENCIES = $(OOGLLIB)
#flythrough_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-flythrough

.geomview-flythrough:
	echo '(emodule-define "Flythrough" "flythrough")' > .geomview-flythrough

all-local: $(top_builddir)/modules/.geomview-flythrough

$(top_builddir)/modules/.geomview-flythrough: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Flythrough" "$(subdir)/flythrough")' > $@

CLEANFILES = $(module_DATA) $(top_builddir)/modules/.geomview-flythrough