INCLUDES = $(default_includes)
EXTRA_DIST = Headers

noinst_LIBRARIES = libooglutil.a

if LINUX
GLIBGLUE = glibglue.C
else
GLUE =
endif

libooglutil_a_SOURCES = \
    error.c findfile.c fsa.c futil.c glob.c mallocp.c porting.c ppopen.c vvec.c \
    create.h fsa.h ooglutil.h porting.h ray.h surf.h $(GLIBGLUE)

EXTRA_libooglutil_a_SOURCES = glibglue.C
