INCLUDES = $(default_includes)

man_MANS = clip.1gv

EXTRA_DIST = $(man_MANS)

bin_PROGRAMS = clip

clip_SOURCES = clip.c clipoogl.c clipmain.c Clip.h
clip_DEPENDENCIES = $(OOGLLIB)
#clip_LDFLAGS = -rpath $(libdir)

LDADD = $(OOGLLIB)

