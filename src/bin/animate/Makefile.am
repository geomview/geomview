INCLUDES = $(default_includes) $(MOTIFINCLUDE)
SUBDIRS = interface
EXTRA_DIST = animate.1

bin_PROGRAMS = animate
animate_SOURCES = animate.c xanimate.c glob.c animate.h glob.h xanimate.h
LDADD = ../../lib/mib/libmib.a $(MOTIFLIBS) $(XMULIBS) $(XLIBS) $(MATHLIB)
