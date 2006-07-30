INCLUDES = $(default_includes)

EXTRA_DIST = $(module_tcl_DATA)

LDADD = \
 $(TK_LIB) \
 $(TCL_LIB) \
 $(XLIBS) \
 $(OOGLLIB)

module_PROGRAMS = ndutils
ndutils_SOURCES = BasicFns.h Clip.h ColorMapDefs.h ColorPoint.h	  \
 GeExDir.h GeomExtentDirection.h GeomtoNoff.h LoadMap.h		  \
 MakeGoodTransforms.h Random.h TransformGeom.h WithColor.h	  \
 init.h slicer.h snapshot.h wrappers.h BasicFns.c ColorPoint.c	  \
 GeExDir.c GeomExtentDirection.c GeomtoNoff.c LoadMap.c		  \
 MakeGoodTransforms.c Random.c TransformGeom.c WithColor.c clip.c \
 clipoogl.c init.c slicer.c snapshot.c testwish.c wrappers.c
ndutils_DEPENDENCIES = $(OOGLLIB)
ndutils_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = 3D-Snapshot Slicer Colormap
module_DATA = .geomview-3d-snapshot .geomview-slicer .geomview-colormap

.geomview-3d-snapshot: Makefile
	echo '(emodule-define "3D-Snapshot" "ndutils $$GEOMVIEW_EMODULE_PATH/tcl/3D-Snapshot")' > $@

.geomview-slicer: Makefile
	echo '(emodule-define "Slicer" "ndutils $$GEOMVIEW_EMODULE_PATH/tcl/Slicer")' > $@

.geomview-colormap: Makefile
	echo '(emodule-define "Colormap" "ndutils $$GEOMVIEW_EMODULE_PATH/tcl/Colormap")' > $@

$(top_builddir)/modules/.geomview-3d-snapshot: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "3D-Snapshot" "$(subdir)/ndutils $(subdir)/$(srcdir)/3D-Snapshot")' > $@

$(top_builddir)/modules/.geomview-slicer: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Slicer" "$(subdir)/ndutils $(subdir)/$(srcdir)/Slicer")' > $@

$(top_builddir)/modules/.geomview-colormap: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Colormap" "$(subdir)/ndutils $(subdir)/$(srcdir)/Colormap")' > $@

all-local: \
 $(top_builddir)/modules/.geomview-3d-snapshot \
 $(top_builddir)/modules/.geomview-slicer \
 $(top_builddir)/modules/.geomview-colormap

CLEANFILES = \
 $(top_builddir)/modules/.geomview-3d-snapshot \
 $(top_builddir)/modules/.geomview-slicer \
 $(top_builddir)/modules/.geomview-colormap \
 $(module_DATA)

