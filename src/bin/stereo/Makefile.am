INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(XFORMSLIBS)	\
	$(XLIBS)	\
	$(OOGLLIB)

EXTRA_DIST = .geomview-stereo help.txt sterui.fd

module_PROGRAMS = stereo
stereo_SOURCES = stereo.c sterui.c sterui.h help.h
stereo_DEPENDENCIES = $(OOGLLIB)
stereo_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-stereo

.geomview-stereo:
	echo '(emodule-define "Stereo" "stereo")' > .geomview-stereo

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Stereo" "$(subdir)/stereo")' > $(top_builddir)/modules/.geomview-stereo

