SUBDIRS = geometry camera color oogl shade window gprim fexpr geomutil \
 pointlist mg mib forms aclib

lib_LTLIBRARIES = libgeomview.la

libgeomview_la_SOURCES =
libgeomview_la_LIBADD = \
 $(OOGLLIBS) $(OPENGLLIBS) $(SOCKETLIBS) $(XLIBS) $(MATHLIB)
libgeomview_la_DEPENDENCIES = $(OOGLLIBS)

