INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(top_builddir)/src/lib/fexpr/libfexpr.a                    \
	$(FORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-ginsu ginsu.1 ginsuF.fd

modules_PROGRAMS = ginsu

modules_DATA = .geomview-ginsu

ginsu_SOURCES = ginsu.c ginsuF.c ginsuF.h
