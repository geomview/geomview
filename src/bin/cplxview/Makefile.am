DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes:=/private)\
 $(default_includes)\
 -I$(XFORMS_ALL_INCLUDES)

LDADD =	$(OOGLLIB) $(XFORMS_ALL_LIB) $(XLIBS)

man_MANS = cplxview.1gv

EXTRA_DIST = cplxpanels2.fd $(man_MANS)

module_PROGRAMS = cplxview
cplxview_SOURCES = computegraph.c cplxpanels2.c cplxpanels2.h cplxview.c

EMODULES = .geomview-cplxview

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

.geomview-cplxview: Makefile
	echo '(emodule-define "Cplxview" "$(modulepath)cplxview")' > $@

.geomview: Makefile
	echo '(emodule-define "Cplxview" "$(abs_builddir)/cplxview")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
