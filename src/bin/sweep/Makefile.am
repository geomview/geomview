DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes:=/private)\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = sweep.1gv sweep.3gv

EXTRA_DIST = panel.fd $(man_MANS)

module_PROGRAMS = sweep
sweep_SOURCES = sweep.c sweep.main.c panel.c panel.h sweep.h

EMODULES = .geomview-sweep

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

.geomview-sweep: Makefile
	echo '(emodule-define "Sweep" "$(modulepath)sweep")' > $@

.geomview: Makefile
	echo '(emodule-define "Sweep" "$(abs_builddir)/sweep")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
