DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = cplxview.1gv

EXTRA_DIST = cplxpanels2.fd $(man_MANS)

module_PROGRAMS = cplxview
cplxview_SOURCES = computegraph.c cplxpanels2.c cplxpanels2.h cplxview.c
module_DATA = .geomview-cplxview

.geomview-cplxview: Makefile
	echo '(emodule-define "Cplxview" "cplxview")' > .geomview-cplxview

CLEANFILES = $(module_DATA)
