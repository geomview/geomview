INCLUDES = @default_includes@

LDADD= ${OOGLLIB}
DEPENDENCIES = $(OOGLLIB)
#AM_LDFLAGS = -rpath $(libdir)

bin_PROGRAMS = anytooff bez2mesh offconsol polymerge
man_MANS = anytooff.1gv bez2mesh.1gv offconsol.1gv polymerge.1gv

anytooff_SOURCES = anytooff.main.c
bez2mesh_SOURCES = bez2mesh.c
offconsol_SOURCES = offconsol.main.c
polymerge_SOURCES = polymerge.c

EXTRA_DIST = $(man_MANS)

