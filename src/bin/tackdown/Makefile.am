INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-tackdown tackdown.1 panel.fd origin.panel.fd

modules_PROGRAMS = tackdown origin

modules_DATA = .geomview-tackdown

tackdown_SOURCES = tackdown.main.c \
                   panel.h panel.c


origin_SOURCES = origin.main.c origin.common.c origin.common.h \
                   origin.panel.c origin.panel.h
