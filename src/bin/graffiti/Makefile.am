DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes:=/private)\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = graffiti.1gv

EXTRA_DIST = $(man_MANS)

module_PROGRAMS = graffiti
graffiti_SOURCES = graffiti.c gui.c lang.h panel.h graffiti.h lang.c panel.c

EMODULES = .geomview-graffiti

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

.geomview-graffiti: Makefile
	echo '(emodule-define "Graffiti" "$(modulepath)graffiti")' > $@

.geomview: Makefile
	echo '(emodule-define "Graffiti" "$(abs_builddir)/graffiti")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
