INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS) 	\
	$(FORMSLIBS)	\
	$(XLIBS)	\
	$(SOCKETLIBS) \
	$(MATHLIB)

EXTRA_DIST = .geomview-stereo help.txt sterui.fd

module_PROGRAMS = stereo
stereo_SOURCES = stereo.c sterui.c sterui.h help.h

module_DATA = .geomview-stereo

.geomview-stereo:
	echo '(emodule-define "Stereo" "stereo")' > .geomview-stereo

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Stereo" "$(subdir)/stereo")' > $(top_builddir)/modules/.geomview-stereo

