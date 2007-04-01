DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = flythrough.1gv

EXTRA_DIST = flyhelp panel.fd $(man_MANS)

module_PROGRAMS = flythrough
flythrough_SOURCES = main.c flyhelp.h panel.c panel.h flythrough.h

EMODULES = .geomview-flythrough

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

.geomview-flythrough: Makefile
	echo '(emodule-define "Flythrough" "$(modulepath)flythrough")' > $@

.geomview: Makefile
	echo '(emodule-define "Flythrough" "$(abs_builddir)/flythrough")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
