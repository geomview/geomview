INCLUDES = @default_includes@ -I$(srcdir)/lib

SUBDIRS = lib

bin_PROGRAMS = vrml2oogl
man_MANS = vrml2oogl.1gv
EXTRA_DIST = $(man_MANS)

vrml2oogl_SOURCES = vrml2oogl.C

LDADD = ./lib/libQv.a
