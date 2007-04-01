DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = hinge.1gv

module_PROGRAMS = hinge
hinge_SOURCES = hui.c panel.c hingedata.h hui.h hinge.c lang.c panel.h\
 hinge.h hingehelp.h lang.h version.h

geomdatageomdir = $(geomdatadir)/geom
geomdatageom_DATA =\
 HingeCube HingeTetra HingeDiamond HingeTriangle HingeDodec HingeLongcube

EMODULES = .geomview-hinge

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

.geomview-hinge: Makefile
	echo '(emodule-define "Hinge" "$(modulepath)hinge")' > $@

.geomview: Makefile
	echo '(emodule-define "Hinge" "$(abs_builddir)/hinge")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

EXTRA_DIST = hingedata hingehelp panel.fd $(man_MANS) $(geomdatageom_DATA)
