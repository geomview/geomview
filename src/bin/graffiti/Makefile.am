INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)

EXTRA_DIST = .geomview-graffiti graffiti.1

modules_PROGRAMS = graffiti

modules_DATA = .geomview-graffiti

graffiti_SOURCES = graffiti.c gui.c lang.h panel.h graffiti.h lang.c panel.c
