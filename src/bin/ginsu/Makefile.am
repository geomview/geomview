DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = ginsu.1gv

EXTRA_DIST = ginsuF.fd $(man_MANS)

module_PROGRAMS = ginsu
ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h

EMODULES = .geomview-ginsu

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

.geomview-ginsu: Makefile
	echo '(emodule-define "Ginsu" "$(modulepath)ginsu")' > $@

.geomview: Makefile
	echo '(emodule-define "Ginsu" "$(abs_builddir)/ginsu")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
