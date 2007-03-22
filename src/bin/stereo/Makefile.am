DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

EXTRA_DIST = help.txt sterui.fd

module_PROGRAMS = stereo
stereo_SOURCES = stereo.c sterui.c sterui.h help.h
module_DATA = .geomview-stereo

.geomview-stereo: Makefile
	echo '(emodule-define "Stereo" "stereo")' > .geomview-stereo

CLEANFILES = $(module_DATA)


