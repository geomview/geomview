INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS) 	\
	$(FORMSLIBS)	\
	$(XLIBS)	\
	$(SOCKETLIBS) \
	$(MATHLIB)

EXTRA_DIST = .geomview-stereo help.txt sterui.fd

bin_PROGRAMS = stereo

bin_SCRIPTS = .geomview-stereo

stereo_SOURCES = stereo.c sterui.c sterui.h help.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: stereo
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Stereo" "$(subdir)/stereo")' > $(top_builddir)/modules/.geomview-stereo


.geomview-stereo:
	echo '(emodule-define "Stereo" "stereo")' > .geomview-stereo
