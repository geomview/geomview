INCLUDES = $(default_includes) -I..

EXTRA_DIST = Headers

noinst_LIBRARIES = libgvcommon.a

libgvcommon_a_SOURCES = \
   comm.c drawer.c event.c lang.c lights.c lispext.c main.c motion.c     \
   mouse.c ndshade.c rman.c shaders.c space.c transform.c ui.c worldio.c \
   comm.h common.h drawer.h event.h lang.h lights.h lispext.h main.h     \
   motion.h mouse.h pickfunc.h rman.h space.h transform.h ui.h version.h worldio.h

../clang.c ../clang.h:
	cd .. && $(MAKE) $(AM_MAKEFLAGS) clang
