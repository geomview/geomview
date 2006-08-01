INCLUDES = $(default_includes)

LDADD = $(OOGLLIB)

EXTRA_DIST = bdy.1 drawbdy.sh

man_MANS = bdy.1

bin_PROGRAMS = bdy
bdy_SOURCES = bdy.c
bdy_DEPENDENCIES = $(OOGLLIB)
#bdy_LDFLAGS = -rpath $(libdir)

module_SCRIPTS = drawbdy
module_DATA = .geomview-drawbdy

CLEANFILES = drawbdy

drawbdy: drawbdy.sh Makefile
	sed -e 's|%GEOMBIN%|$(moduledir)|g'       \
	    < $(srcdir)/drawbdy.sh > drawbdy
	chmod +x drawbdy

.geomview-drawbdy:
	echo '(emodule-define "Draw Boundary" "drawbdy")' > .geomview-drawbdy

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Draw Boundary" "$(subdir)/drawbdy")' > $(top_builddir)/modules/.geomview-drawbdy

# install-geomview-dotfile: drawbdy
# 	$(top_builddir)/mkmodulesdir $(top_builddir)
# 	echo '(emodule-define "Draw Boundary" "$(subdir)/drawbdy")' > $(top_builddir)/modules/.geomview-drawbdy

DISTCLEANFILES = .geomview-drawbdy