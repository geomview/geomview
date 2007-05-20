DEFS = -DHAVE_CONFIG_H

INCLUDES =\
 -I$(geomviewincludes)\
 -I$(geomviewincludes:=/private)\
 $(default_includes)\
 -I$(XFORMS_ALL_INCLUDES)

LDADD = $(OOGLLIB) $(XFORMS_ALL_LIB) $(XLIBS)

man_MANS = example.1gv

EXTRA_DIST = README panel.fd tutorial.ps tutorial.tex $(man_MANS)

doc_DATA = tutorial.ps tutorial.tex

module_PROGRAMS = example
example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h

EMODULES = .geomview-example

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

.geomview-example: Makefile
	echo '(emodule-define "Example" "$(modulepath)example")' > $@

.geomview: Makefile
	echo '(emodule-define "Example" "$(abs_builddir)/example")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview
