DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = graffiti.1gv

EXTRA_DIST = $(man_MANS)

module_PROGRAMS = graffiti
graffiti_SOURCES = graffiti.c gui.c lang.h panel.h graffiti.h lang.c panel.c
module_DATA = .geomview-graffiti

.geomview-graffiti:
	echo '(emodule-define "Graffiti" "graffiti")' > .geomview-graffiti

CLEANFILES = $(module_DATA)
