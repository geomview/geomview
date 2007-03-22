DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = tackdown.1

EXTRA_DIST = panel.fd origin.panel.fd $(man_MANS)

module_PROGRAMS = tackdown origin

tackdown_SOURCES = tackdown.main.c panel.h panel.c
origin_SOURCES = \
 origin.main.c origin.common.c origin.common.h origin.panel.c origin.panel.h

module_DATA = .geomview-tackdown .geomview-origin

.geomview-tackdown: Makefile
	echo '(emodule-define "Tackdown" "tackdown")' > .geomview-tackdown
.geomview-origin: Makefile
	echo '(emodule-define "Origin" "origin")' > .geomview-origin

CLEANFILES = $(module_DATA)


