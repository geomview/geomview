INCLUDES = $(default_includes) $(MOTIFINCLUDE) $(OPENGLINCLUDE) $(X_CFLAGS) -I..
SUBDIRS = bitmaps interface

gvx_LDADD = ../common/libgvcommon.a buildinfo.o \
 $(MOTIFUILIB) $(MOTIFLIBS) $(XMULIBS) $(OOGLLIB) $(XLIBS)
gvx_DEPENDENCIES = ../common/libgvcommon.a buildinfo.o $(OOGLLIB)
#gvx_LDFLAGS = -rpath $(libdir)

module_PROGRAMS = gvx

gvx_SOURCES =\
 gvappear.c gvcameras.c gvcamui.c gvcolor.c gvcommands.c gvcredits.c\
 gvevent.c gvfiles.c gvlights.c gvload.c gvmain.c gvmaterial.c gvmnpanel.c\
 gvsave.c gvtoolui.c gvui.c gvui.h


buildinfo.c: $(gvx_OBJECTS) ../common/libgvcommon.a
	(echo 'char builddate[] = "'"`date +%Y%m%d%H%M`"'";'; \
	 echo 'char buildinfo1[] = "'"  By `whoami`@`hostname`[`uname -s`-`uname -r`]"'";'; \
	 echo 'char buildinfo2[] = "'"  On `date`"'";'; \
	) > buildinfo.c

../clang.c ../clang.h:
	cd .. && $(MAKE) $(AM_MAKEFLAGS) clang

DISTCLEANFILES = buildinfo.c clang.c clang.h