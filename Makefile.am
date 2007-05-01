ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc
EXTRA_DIST = reconf geomview.sh INSTALL.Geomview mkmodulesdir

noinst_SCRIPTS = geomview

nodist_pkginclude_HEADERS = @GEOMVIEWHDRFILES@

pkgincludeprivdir = ${pkgincludedir}/private
nodist_pkgincludepriv_HEADERS = @GEOMVIEWPRIVHDRFILES@

EXTRA_HEADERS =

geomview: geomview.sh Makefile
	echo top_builddir is $(abs_top_builddir)
	sed -e 's|%GEOMROOT%|$(abs_top_srcdir)|g' \
	    -e 's|%GEOMVIEW_GVX%|$(abs_top_builddir)/src/bin/geomview/x11/gvx|g' \
	    -e 's|%GEOMVIEW_LTGVX%|$(abs_top_builddir)/src/bin/geomview/x11/.libs/lt-gvx|g' \
	    -e 's|%MACHTYPE%|$(MACHTYPE)|g' \
	    -e 's|%GEOMDATA%|$(abs_top_srcdir)/data|g' \
	    -e 's|%MODULESDIR%|$(abs_top_builddir)/modules|g' \
	    -e "s|%LD_LIBRARY_PATH_SETTINGS%|$(LD_LIBRARY_PATH_SETTINGS)|g" \
	    -e "s|%GEOMVIEW_LIB%|$(libdir)|g" \
	    -e "s|%GEOMVIEW_INCLUDE%|$(pkgincludedir)|g" \
	    -e "s|%PACKAGE_VERSION%|$(PACKAGE_VERSION)|g" \
	    -e "s|%GEOMDOCDIR%|$(abs_top_builddir)/doc|g" \
	    < $(top_srcdir)/geomview.sh > geomview
	chmod +x geomview

DISTCLEANFILES = geomview

distclean-local:
	rm -rf include modules
	find . -name "*~" -exec rm -f \{\} \;

MAINTAINERCLEANFILES = \
 configure config.guess config.h.in config.sub depcomp geomview*.tar.* \
 install-sh missing INSTALL aclocal.m4
