DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = transformer.1gv

EXTRA_DIST = panel.fd $(man_MANS)

module_PROGRAMS = transformer
transformer_SOURCES = transformer.main.c panel.c panel.h

EMODULES = .geomview-transformer

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

.geomview-transformer: Makefile
	echo '(emodule-define "Transformer" "$(modulepath)transformer")' > $@

.geomview: Makefile
	echo '(emodule-define "Transformer" "$(abs_builddir)/transformer")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
