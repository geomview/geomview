INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-tackdown tackdown.1 panel.fd origin.panel.fd

bin_PROGRAMS = tackdown origin

bin_SCRIPTS = .geomview-tackdown

tackdown_SOURCES = tackdown.main.c \
                   panel.h panel.c


origin_SOURCES = origin.main.c origin.common.c origin.common.h \
                   origin.panel.c origin.panel.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: tackdown
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Tackdown" "$(subdir)/tackdown")' > $(top_builddir)/modules/.geomview-tackdown


.geomview-tackdown:
	echo '(emodule-define "Tackdown" "tackdown")' > .geomview-tackdown
