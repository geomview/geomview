DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = flythrough.1

EXTRA_DIST = flyhelp panel.fd $(man_MANS)

module_PROGRAMS = flythrough
flythrough_SOURCES = main.c flyhelp.h panel.c panel.h flythrough.h
module_DATA = .geomview-flythrough

.geomview-flythrough:
	echo '(emodule-define "Flythrough" "flythrough")' > .geomview-flythrough

CLEANFILES = $(module_DATA)
