SUBDIRS = geometry camera color oogl shade window gprim fexpr geomutil \
 pointlist mg mib forms aclib

if INSTALL_LIBGEOMVIEW
lib_LTLIBRARIES = libgeomview.la
else
noinst_LTLIBRARIES = libgeomview.la
endif

libgeomview_la_SOURCES =
libgeomview_la_LIBADD = $(OOGLLIBS) $(OPENGLLIBS) $(SOCKETLIBS) $(XLIBS)
libgeomview_la_DEPENDENCIES = $(OOGLLIBS)

