INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(top_builddir)/src/lib/fexpr/libfexpr.a                    \
	$(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-ginsu ginsu.1 ginsuF.fd

module_PROGRAMS = ginsu
ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h

module_DATA = .geomview-ginsu

.geomview-ginsu:
	echo '(emodule-define "Ginsu" "ginsu")' > .geomview-ginsu

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Ginsu" "$(subdir)/ginsu")' > $(top_builddir)/modules/.geomview-ginsu
