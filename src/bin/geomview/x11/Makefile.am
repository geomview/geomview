INCLUDES = $(default_includes) $(MOTIFINCLUDE) $(OPENGLINCLUDE) -I..
SUBDIRS = bitmaps interface

LDADD = ../common/libgvcommon.a buildinfo.o ${OOGLLIBS} \
        $(OPENGLLIBS) $(MOTIFLIBS) $(XMULIBS) $(XLIBS) $(SOCKETLIBS) $(MATHLIB)

bin_PROGRAMS = gvx

gvx_SOURCES = \
    gvappear.c	gvcameras.c  gvcolor.c  gvcommands.c      \
    gvcredits.c gvevent.c    gvfiles.c   gvlights.c gvload.c          \
    gvcamui.c \
    gvmain.c	gvmaterial.c gvmnpanel.c gvsave.c   gvtoolui.c gvui.c \
    gvui.h


buildinfo.c: $(gvx_OBJECTS) ../common/libgvcommon.a
	(echo 'char builddate[] = "'"`date +%y%m%d%H%M`"'";'; \
	 echo 'char buildinfo1[] = "'"  By `whoami`@`hostname`[`uname -r`]"'";'; \
	 echo 'char buildinfo2[] = "'"  On `date`"'";'; \
	) > buildinfo.c

../clang.c ../clang.h:
	cd .. && $(MAKE) $(AM_MAKEFLAGS) clang
