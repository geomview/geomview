#
# $(OPENGLINCLUDE) is needed for bsptree.c to get the correct
# location of the glu.h header file.
#
INCLUDES = $(default_includes) $(OPENGLINCLUDE)
EXTRA_DIST = Headers

noinst_LTLIBRARIES = libgeom.la

libgeom_la_SOURCES = \
 bound.c boundsphere.c class.c copy.c create.c delete.c dice.c draw.c \
 evert.c extend.c geomstream.c handlescan.c iterate.c knownclass.c message.c \
 name.c pick.c replace.c transform.c \
 geomclass.h geomclass_sm.h geom.h pick.h pickP.h vert.h \
 bsptree.c bsptree.h bsptreeP.h nodedata.h

