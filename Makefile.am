SUBDIRS = doc src data
EXTRA_DIST = reconf geomview.sh INSTALL.Geomview mkmodulesdir \
             $(m4_files)

noinst_SCRIPTS = geomview

ACLOCAL_AMFLAGS = -I m4
m4_files = \
	m4/geom_local.m4 \
	m4/lf_local.m4

#	m4/smr_OPTIONAL_LIB.m4 \
#	m4/smr_REQUIRED_LIB.m4 \
#	m4/smr_WITH_BUILD_PATH.m4


geomview: geomview.sh Makefile
	echo top_builddir is $(top_builddir)
	sed -e 's|%GEOMROOT%|$(top_builddir)|g'                              \
	    -e 's|%GEOMVIEW_GVX%|$(top_builddir)/src/bin/geomview/x11/gvx|g' \
	    -e 's|%MACHTYPE%|$(MACHTYPE)|g'                                  \
	    -e 's|%GEOMDATA%|$(top_srcdir)/data|g'                           \
	    -e 's|%MODULESDIR%|$(top_builddir)/modules|g'                    \
	    -e "s|%LD_LIBRARY_PATH_SETTINGS%|$(LD_LIBRARY_PATH_SETTINGS)|"   \
	    < $(top_srcdir)/geomview.sh > geomview
	chmod +x geomview

DISTCLEANFILES = geomview

distclean-local:
	rm -rf include modules

