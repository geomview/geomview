INCLUDES = @default_includes@

bin_PROGRAMS = oogl2vrml oogl2vrml2
man_MANS = oogl2vrml.1

oogl2vrml_SOURCES = oogl2vrml.c
oogl2vrml2_SOURCES = oogl2vrml2.c

LDADD = $(STUBLIB) $(OOGLLIBS) $(SOCKETLIBS) $(MATHLIB)

EXTRA_DIST = $(man_MANS)
