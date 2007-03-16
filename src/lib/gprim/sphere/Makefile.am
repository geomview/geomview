INCLUDES = $(default_includes)
EXTRA_DIST = Headers

noinst_LTLIBRARIES = libsphere.la

libsphere_la_SOURCES = \
 sphereclass.c spherecreate.c sphereload.c spheremisc.c spheresave.c \
 sphere.h sphereP.h

if !BEZIER_SPHERES
libsphere_la_SOURCES += spheredice.c spheredraw.c
endif
