INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-flythrough flyhelp flythrough.1 panel.fd

modules_PROGRAMS = flythrough

modules_DATA = .geomview-flythrough

flythrough_SOURCES = main.c flyhelp.h panel.c panel.h
