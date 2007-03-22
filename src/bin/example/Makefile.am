DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = example.1

EXTRA_DIST = README panel.fd tutorial.ps tutorial.tex $(man_MANS)

module_PROGRAMS = example
example_SOURCES = main.c oogl.c callbacks.c panel.c panel.h
module_DATA = .geomview-example

.geomview-example:
	echo '(emodule-define "Example" "example")' > .geomview-example

CLEANFILES = $(module_DATA) 