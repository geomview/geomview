DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = tackdown.1gv

EXTRA_DIST = panel.fd origin.panel.fd $(man_MANS)

module_PROGRAMS = tackdown origin

tackdown_SOURCES = tackdown.main.c panel.h panel.c
origin_SOURCES = \
 origin.main.c origin.common.c origin.common.h origin.panel.c origin.panel.h

EMODULES = .geomview-tackdown .geomview-origin

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

.geomview-tackdown: Makefile
	echo '(emodule-define "Tackdown" "$(modulepath)tackdown")' > $@

.geomview-origin: Makefile
	echo '(emodule-define "Origin" "$(modulepath)origin")' > $@

.geomview: Makefile
	echo '(emodule-define "Tackdown" "$(abs_builddir)/tackdown")' > $@
	echo '(emodule-define "Origin" "$(abs_builddir)/origin")' >> $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

