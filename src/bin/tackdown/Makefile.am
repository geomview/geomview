INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	$(SOCKETLIBS) \
	$(MATHLIB)

EXTRA_DIST = .geomview-tackdown tackdown.1 panel.fd origin.panel.fd

module_PROGRAMS = tackdown origin

tackdown_SOURCES = tackdown.main.c \
                   panel.h panel.c

origin_SOURCES = origin.main.c origin.common.c origin.common.h \
                   origin.panel.c origin.panel.h

module_DATA = .geomview-tackdown

.geomview-tackdown:
	echo '(emodule-define "Tackdown" "tackdown")' > .geomview-tackdown

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Tackdown" "$(subdir)/tackdown")' > $(top_builddir)/modules/.geomview-tackdown
