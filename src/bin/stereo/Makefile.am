DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

EXTRA_DIST = help.txt sterui.fd

module_PROGRAMS = stereo
stereo_SOURCES = stereo.c sterui.c sterui.h help.h

EMODULES = .geomview-stereo

if LOCAL_EMODULE
homedir = $(HOME)
home_DATA = $(EMODULES)
modulepath = $(moduledir)/
else
if !GEOMVIEW_QUERY
modulepath = $(moduledir)/
module_DATA = $(EMODULES)
else
modulepath =
module_DATA = $(EMODULES)
endif
endif

.geomview-stereo: Makefile
	echo '(emodule-define "Stereo" "$(modulepath)stereo")' > $@

.geomview: Makefile
	echo '(emodule-define "Stereo" "$(abs_builddir)/stereo")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
