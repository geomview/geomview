INCLUDES = $(default_includes)

LDADD = $(OOGLLIBS)

EXTRA_DIST = .geomview-bdy bdy.1

bin_PROGRAMS = bdy

modules_DATA = .geomview-drawbdy

bdy_SOURCES = bdy.c

modules_SCRIPTS = drawbdy

drawbdy: drawbdy.sh
	sed -e 's|%GEOMBIN%|$(exec_prefix)/bin|g'       \
	    < drawbdy.sh > drawbdy
	chmod +x drawbdy
