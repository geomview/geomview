INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS) 	\
	$(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-stereo help.txt sterui.fd

modules_PROGRAMS = stereo

modules_DATA = .geomview-stereo

stereo_SOURCES = stereo.c sterui.c sterui.h help.h
