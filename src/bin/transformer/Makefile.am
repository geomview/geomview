INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	$(MATHLIB)

EXTRA_DIST = .geomview-transformer panel.fd  transformer.1

bin_PROGRAMS = transformer

bin_SCRIPTS = .geomview-transformer

transformer_SOURCES = transformer.main.c panel.c panel.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: transformer
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Transformer" "$(subdir)/transformer")' > $(top_builddir)/modules/.geomview-transformer


.geomview-transformer:
	echo '(emodule-define "Transformer" "transformer")' > .geomview-transformer
