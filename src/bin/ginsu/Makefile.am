DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes)/private\
 $(default_includes)\
 -I$(XFORMS_INCLUDE_PATH)

LDADD =	$(XFORMS_ALL_LIB) $(XLIBS) $(OOGLLIB) 

man_MANS = ginsu.1gv

EXTRA_DIST = ginsuF.fd $(man_MANS)

module_PROGRAMS = ginsu
ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h
module_DATA = .geomview-ginsu

.geomview-ginsu:
	echo '(emodule-define "Ginsu" "ginsu")' > .geomview-ginsu

CLEANFILES = $(module_DATA)
