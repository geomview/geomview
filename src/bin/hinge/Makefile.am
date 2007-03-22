DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = hinge.1gv

EXTRA_DIST =\
 HingeCube HingeTetra HingeDiamond HingeTriangle HingeDodec HingeLongcube\
 hingedata hingehelp panel.fd $(man_MANS)

module_PROGRAMS = hinge
hinge_SOURCES = hui.c panel.c hingedata.h hui.h hinge.c lang.c panel.h\
 hinge.h hingehelp.h lang.h version.h
module_DATA = .geomview-hinge

.geomview-hinge: Makefile
	echo '(emodule-define "Hinge" "hinge")' > .geomview-hinge

CLEANFILES = $(module_DATA)

