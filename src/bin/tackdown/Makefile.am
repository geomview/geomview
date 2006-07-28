INCLUDES = $(default_includes) $(XFORMSINCLUDE)

man_MANS = tackdown.1

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)      \
	$(OOGLLIB)


EXTRA_DIST = .geomview-tackdown tackdown.1 panel.fd origin.panel.fd

module_PROGRAMS = tackdown origin

tackdown_SOURCES = tackdown.main.c \
                   panel.h panel.c
tackdown_DEPENDENCIES = $(OOGLLIBS)
tackdown_LDFLAGS = -rpath $(libdir)

origin_SOURCES = origin.main.c origin.common.c origin.common.h \
                   origin.panel.c origin.panel.h
origin_DEPENDENCIES = $(OOGLLIBS)
origin_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-tackdown

.geomview-tackdown:
	echo '(emodule-define "Tackdown" "tackdown")' > .geomview-tackdown

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Tackdown" "$(subdir)/tackdown")' > $(top_builddir)/modules/.geomview-tackdown
