INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIB) 

EXTRA_DIST = .geomview-transformer panel.fd  transformer.1

man_MANS = transformer

module_PROGRAMS = transformer
transformer_SOURCES = transformer.main.c panel.c panel.h
transformer_DEPENDENCIES = $(OOGLLIB)
transformer_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-transformer

.geomview-transformer:
	echo '(emodule-define "Transformer" "transformer")' > .geomview-transformer

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Transformer" "$(subdir)/transformer")' > $(top_builddir)/modules/.geomview-transformer
