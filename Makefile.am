SUBDIRS = m4 doc src data
EXTRA_DIST = reconf geomview.sh INSTALL.Geomview mkinstalldirs mkmodulesdir \
             build build.in packbuild packbuild.in

ACLOCAL_AMFLAGS  = -I ./m4

noinst_SCRIPTS = geomview

ACLOCAL_AMFLAGS = -I m4

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

build: build.in
	m4 build.in > build
	chmod +x build

packbuild: packbuild.in
	m4 packbuild.in > packbuild
	chmod +x packbuild

buildscripts: build packbuild

#dist-hook:
#	cp -r $(srcdir)/xforms $(distdir)
