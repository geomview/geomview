INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(MATHLIB)

EXTRA_DIST = .geomview-transformer panel.fd  transformer.1

module_PROGRAMS = transformer
transformer_SOURCES = transformer.main.c panel.c panel.h

module_DATA = .geomview-transformer

.geomview-transformer:
	echo '(emodule-define "Transformer" "transformer")' > .geomview-transformer

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Transformer" "$(subdir)/transformer")' > $(top_builddir)/modules/.geomview-transformer
