INCLUDES = $(default_includes) -I../x11

# NOTE: we don't really create the library 'libgvcommon.a'; the result
# of compilation in this directory is just a collection of .o files.
# The use of "libgvcommon.a" in this Makefile.am is just because I
# don't know how to use automake to generate a bunch of .o files
# without actually putting them into either a program or a library.
# The "LINK = :" setting below prevents linking from actually
# happening, so libgvcommon.a is never really created.

# mbp Mon May  8 16:41:26 2000

noinst_LIBRARIES = libgvcommon.a

LINK = :

libgvcommon_a_SOURCES = \
   comm.c drawer.c event.c lang.c lights.c lispext.c main.c motion.c     \
   mouse.c ndshade.c rman.c shaders.c space.c transform.c ui.c worldio.c \
   comm.h common.h drawer.h event.h lang.h lights.h lispext.h main.h     \
   motion.h mouse.h pickfunc.h rman.h space.h transform.h ui.h version.h worldio.h

libgvcommon_a_DEPENDENCIES = clang

lang.o: clang

clang:
	cd ../x11 ; $(MAKE) clang

../x11/clang.c: clang
