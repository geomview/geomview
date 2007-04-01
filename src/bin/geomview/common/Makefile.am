INCLUDES = $(default_includes) -I..

EXTRA_DIST = Headers

noinst_LIBRARIES = libgvcommon.a

libgvcommon_a_SOURCES =\
 comm.c comm.h common.h drawer.c drawer.h event.c event.h help.c lang.c lang.h\
 lights.c lights.h lispext.c lispext.h main.c main.h motion.c motion.h mouse.c\
 mouse.h ndshade.c pickfunc.h rman.c rman.h shaders.c space.c space.h\
 transform.c transform.h ui.c ui.h version.h worldio.c worldio.h

../clang.c ../clang.h:
	cd .. && $(MAKE) $(AM_MAKEFLAGS) clang
