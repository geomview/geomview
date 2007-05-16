INCLUDES = @default_includes@

bin_PROGRAMS = oogl2rib
man_MANS = oogl2rib.1gv

oogl2rib_SOURCES = oogl2rib.c
oogl2rib_DEPENDENCIES = $(OOGLLIB)
#oogl2rib_LDFLAGS = -rpath $(libdir)

LDADD = $(OOGLLIB)

EXTRA_DIST = $(man_MANS)
