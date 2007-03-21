DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes) \
 -I$(geomviewincludes)/private \
 $(default_includes) \
 -I$(TCL_INCLUDE_PATH) \
 -I$(TK_INCLUDE_PATH)

EXTRA_DIST = $(module_tcl_DATA)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

module_PROGRAMS = ndutils
ndutils_SOURCES = BasicFns.h Clip.h ColorMapDefs.h ColorPoint.h	  \
 GeExDir.h GeomExtentDirection.h GeomtoNoff.h LoadMap.h		  \
 MakeGoodTransforms.h Random.h TransformGeom.h WithColor.h	  \
 init.h slicer.h snapshot.h wrappers.h BasicFns.c ColorPoint.c	  \
 GeExDir.c GeomExtentDirection.c GeomtoNoff.c LoadMap.c		  \
 MakeGoodTransforms.c Random.c TransformGeom.c WithColor.c clip.c \
 clipoogl.c init.c slicer.c snapshot.c testwish.c wrappers.c
#ndutils_DEPENDENCIES = $(OOGLLIB)
#ndutils_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = 3D-Snapshot Slicer Colormap
module_DATA = .geomview-3d-snapshot .geomview-slicer .geomview-colormap

.geomview-3d-snapshot: Makefile
	echo '(emodule-define "3D-Snapshot" "ndutils $$GEOMVIEW_EMODULE_DIR/tcl/3D-Snapshot")' > $@

.geomview-slicer: Makefile
	echo '(emodule-define "Slicer" "ndutils $$GEOMVIEW_EMODULE_DIR/tcl/Slicer")' > $@

.geomview-colormap: Makefile
	echo '(emodule-define "Colormap" "ndutils $$GEOMVIEW_EMODULE_DIR/tcl/Colormap")' > $@

CLEANFILES = $(module_DATA)

