SUBDIRS = lib bin

bin_SCRIPTS = geomview

DISTCLEANFILES = geomview

geomview: ../geomview.sh Makefile
	sed -e 's|%GEOMROOT%|$(prefix)|g'                                    \
	    -e 's|%GEOMVIEW_GVX%|$(exec_prefix)/bin/gvx|g'                   \
	    -e 's|%MACHTYPE%|$(MACHTYPE)|g'                                  \
	    -e 's|%MODULESDIR%|bin|g'                                        \
	    -e "s|%LD_LIBRARY_PATH_SETTINGS%|$(LD_LIBRARY_PATH_SETTINGS)|"   \
	    < ../geomview.sh > geomview
	chmod +x geomview
