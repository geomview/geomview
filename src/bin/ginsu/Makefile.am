INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD = $(XFORMSLIBS)	\
	$(XLIBS)	\
	$(OOGLLIB)

man_MANS = ginsu.1

EXTRA_DIST = .geomview-ginsu ginsu.1 ginsuF.fd

module_PROGRAMS = ginsu
ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h
ginsu_DEPENDENCIES = $(OOGLLIB)
ginsu_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-ginsu

.geomview-ginsu:
	echo '(emodule-define "Ginsu" "ginsu")' > .geomview-ginsu

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Ginsu" "$(subdir)/ginsu")' > $(top_builddir)/modules/.geomview-ginsu
