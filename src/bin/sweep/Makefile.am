DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = sweep.1 sweep.3

EXTRA_DIST = panel.fd $(man_MANS)

module_PROGRAMS = sweep
sweep_SOURCES = sweep.c sweep.main.c panel.c panel.h sweep.h
module_DATA = .geomview-sweep

.geomview-sweep: Makefile
	echo '(emodule-define "Sweep" "sweep")' > .geomview-sweep

CLEANFILES = $(module_DATA)
