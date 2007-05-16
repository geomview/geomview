INCLUDES = @default_includes@

bin_PROGRAMS = oogl2vrml oogl2vrml2
man_MANS = oogl2vrml.1gv

oogl2vrml_SOURCES = oogl2vrml.c
oogl2vrml2_SOURCES = oogl2vrml2.c
oogl2vrml2_DEPENDENCIES = $(OOGLLIB)
#oogl2vrml2_LDFLAGS = -rpath $(libdir)

LDADD = $(OOGLLIB)

EXTRA_DIST = $(man_MANS)
