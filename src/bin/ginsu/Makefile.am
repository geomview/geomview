INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(top_builddir)/src/lib/fexpr/libfexpr.a                    \
	$(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-ginsu ginsu.1 ginsuF.fd

bin_PROGRAMS = ginsu

bin_SCRIPTS = .geomview-ginsu

ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: ginsu
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Ginsu" "$(subdir)/ginsu")' > $(top_builddir)/modules/.geomview-ginsu


.geomview-ginsu:
	echo '(emodule-define "Ginsu" "ginsu")' > .geomview-ginsu
