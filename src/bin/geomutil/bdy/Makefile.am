INCLUDES = $(default_includes)

LDADD = $(OOGLLIB)

man_MANS = bdy.1gv

EXTRA_DIST = drawbdy.sh $(man_MANS)

bin_PROGRAMS = bdy
bdy_SOURCES = bdy.c
bdy_DEPENDENCIES = $(OOGLLIB)

module_SCRIPTS = drawbdy
module_DATA = .geomview-drawbdy

CLEANFILES = drawbdy

drawbdy: drawbdy.sh Makefile
	sed \
 -e "s|%SHELL%|`eval eval eval echo @SHELL@`|g" \
 -e "s|%bindir%|`eval eval eval echo $(bindir)`|g" \
 -e "s|%builddir%|$(abs_builddir)|g" \
 < $(srcdir)/drawbdy.sh > drawbdy
	chmod +x drawbdy

.geomview-drawbdy:
	echo '(emodule-define "Draw Boundary" "drawbdy")' > .geomview-drawbdy

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Draw Boundary" "$(subdir)/drawbdy")' > $(top_builddir)/modules/.geomview-drawbdy

DISTCLEANFILES = .geomview-drawbdy
