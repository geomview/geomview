INCLUDES = $(default_includes) $(MOTIFINCLUDE) $(OPENGLINCLUDE)
SUBDIRS = bitmaps interface

COMMON_OBJS = \
  ../common/comm.o     ../common/main.o      ../common/space.o     	\
  ../common/drawer.o   ../common/motion.o    ../common/transform.o 	\
  ../common/event.o    ../common/mouse.o     ../common/ui.o	   	\
  ../common/lang.o     ../common/ndshade.o   ../common/worldio.o	\
  ../common/lights.o   ../common/rman.o				   	\
  ../common/lispext.o  ../common/shaders.o

# The following rule prevents make from attempting to compile the
# .c files from ../common in this directory; it's triggered if you
# do a 'make' in this directory before doing it in ../common:
$(COMMON_OBJS):
	@echo  ""
	@echo "You need to run 'make' manually in ../common to build"
	@echo "  .o files there before continuing.  Or better yet, always"
	@echo "  build geomview by issuing 'make' from the parent of this"
	@echo "  directory (.../src/bin/geomview); that takes care of"
	@echo "  building both the common/ and x11/ directories in the"
	@echo "  correct order."
	@echo  ""
	exit -1

LDADD = ${COMMON_OBJS} buildinfo.o ${OOGLLIBS} \
        $(MOTIFLIBS) $(XMULIBS) $(XLIBS) $(MATHLIB)

# $(OPENGLLIBS) 

COMMON_SRCS = \
  ../common/comm.c     ../common/main.c      ../common/space.c     	\
  ../common/drawer.c   ../common/motion.c    ../common/transform.c 	\
  ../common/event.c    ../common/mouse.c     ../common/ui.c	   	\
  ../common/lang.c     ../common/ndshade.c   ../common/worldio.c	\
  ../common/lights.c   ../common/rman.c				   	\
  ../common/lispext.c  ../common/shaders.c			   	\
  ../common/comm.h     ../common/lispext.h   ../common/space.h	   	\
  ../common/common.h   ../common/main.h      ../common/transform.h 	\
  ../common/drawer.h   ../common/motion.h    ../common/ui.h	   	\
  ../common/event.h    ../common/mouse.h     ../common/version.h	\
  ../common/lang.h     ../common/pickfunc.h  ../common/worldio.h	\
  ../common/lights.h   ../common/rman.h

if X11RENDER
GVX_X11 = gvx.x11
else
GVX_X11 = 
endif

bin_PROGRAMS = gvx $(GVX_X11)

COMMON_SRCS = \
    gvappear.c	gvcameras.c  gvcolor.c  gvcommands.c      \
    gvcredits.c gvevent.c    gvfiles.c   gvlights.c gvload.c          \
    gvmain.c	gvmaterial.c gvmnpanel.c gvsave.c   gvtoolui.c gvui.c \
    clang.h gvui.h


gvx_SOURCES = $(COMMON_SRCS) gvcamui-opengl.c

gvcamui-x11.o: gvcamui-x11.c
	$(COMPILE) -DMGOPENGL=1 gvcamui-x11.c

# gvx_CFLAGS = -DMGOPENGL=1


gvx_x11_SOURCES = $(COMMON_SRCS) gvcamui-x11.c

gvcamui-x11.o: gvcamui-x11.c
	$(COMPILE) -DMGX11=1 gvcamui-x11.c

# gvx_x11_CFLAGS = -DMGX11=1

.PHONY: _always

_always:

clang:	_always
	$(top_srcdir)/src/lib/oogl/lisp/lisp2c -cprefix "gv_" -o clang *.c ../common/*.c

buildinfo.c:	_always
	(echo 'char builddate[] = "'"`date +%y%m%d%H%M`"'";'; \
	 echo 'char buildinfo1[] = "'"  By `whoami`@`hostname`[`uname -r`]"'";'; \
	 echo 'char buildinfo2[] = "'"  On `date`"'";'; \
	) > buildinfo.c
