INCLUDES = $(default_includes)

LDADD = $(OOGLLIBS) $(SOCKETLIBS) $(MATHLIB)

EXTRA_DIST = bdy.1 drawbdy.sh

bin_PROGRAMS = bdy

bin_SCRIPTS = drawbdy .geomview-drawbdy

bdy_SOURCES = bdy.c

drawbdy: drawbdy.sh
	sed -e 's|%GEOMBIN%|$(exec_prefix)/bin|g'       \
	    < drawbdy.sh > drawbdy
	chmod +x drawbdy

install-geomview-dotfile: drawbdy
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Draw Boundary" "$(subdir)/drawbdy")' > $(top_builddir)/modules/.geomview-drawbdy


.geomview-drawbdy:
	echo '(emodule-define "Draw Boundary" "drawbdy")' > .geomview-drawbdy
