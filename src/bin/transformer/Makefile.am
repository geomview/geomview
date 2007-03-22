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
module_DATA = .geomview-transformer

.geomview-transformer:
	echo '(emodule-define "Transformer" "transformer")' > .geomview-transformer

CLEANFILES = $(module_DATA)
