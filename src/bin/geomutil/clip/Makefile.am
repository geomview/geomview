INCLUDES = $(default_includes)

EXTRA_DIST = clip.1

man_MANS = clip.1

bin_PROGRAMS = clip

clip_SOURCES = clip.c clipoogl.c clipmain.c Clip.h
clip_DEPENDENCIES = $(OOGLLIB)
#clip_LDFLAGS = -rpath $(libdir)

LDADD = $(STUBLIB) $(OOGLLIB)

