SUBDIRS = lib bin

bin_SCRIPTS = geomview

CLEANFILES = geomview

geomview: $(top_srcdir)/geomview.sh Makefile
	sed -e 's|%GEOMROOT%|$(prefix)|g'                                    \
	    -e 's|%GEOMVIEW_GVX%|$(moduledir)/gvx|g'                   \
	    -e 's|%MACHTYPE%|$(MACHTYPE)|g'                                  \
	    -e 's|%GEOMDATA%|$(geomdatadir)|g'                               \
	    -e 's|%MODULESDIR%|$(moduledir)|g'                               \
	    -e "s|%LD_LIBRARY_PATH_SETTINGS%|$(LD_LIBRARY_PATH_SETTINGS)|"   \
	    -e "s|%GEOMVIEW_LIB%|$(libdir)|g" \
	    -e "s|%GEOMVIEW_INCLUDE%|$(pkgincludedir)|g" \
	    -e "s|%PACKAGE_VERSION%|$(PACKAGE_VERSION)|g" \
	    -e "s|%GEOMDOCDIR%|$(docdir)|g" \
	    < $(top_srcdir)/geomview.sh > geomview
	chmod +x geomview
