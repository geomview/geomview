INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD = $(top_builddir)/src/lib/fexpr/libfexpr.a                    \
	$(XFORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-cplxview cplxhelp.txt cplxpanels2.fd cplxview.1

module_PROGRAMS = cplxview
cplxview_SOURCES = computegraph.c cplxpanels2.c cplxpanels2.h cplxview.c


module_DATA = .geomview-cplxview


.geomview-cplxview:
	echo '(emodule-define "Cplxview" "cplxview")' > .geomview-cplxview

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Cplxview" "$(subdir)/cplxview")' > $(top_builddir)/modules/.geomview-cplxview

# noinst_SCRIPTS = install-geomview-dotfile
# 
# install-geomview-dotfile: cplxview
# 	$(top_builddir)/mkmodulesdir $(top_builddir)
# 	echo '(emodule-define "Cplxview" "$(subdir)/cplxview")' > $(top_builddir)/modules/.geomview-cplxview
# 
# 
# .geomview-cplxview:
# 	echo '(emodule-define "Cplxview" "cplxview")' > .geomview-cplxview
