INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-transformer panel.fd  transformer.1

modules_PROGRAMS = transformer

modules_DATA = .geomview-transformer

transformer_SOURCES = transformer.main.c panel.c panel.h
