SUBDIRS = m4 doc src data
EXTRA_DIST = reconf geomview.sh build INSTALL.Geomview mkinstalldirs mkmodulesdir

noinst_SCRIPTS = geomview

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

#dist-hook:
#	cp -r $(srcdir)/xforms $(distdir)
