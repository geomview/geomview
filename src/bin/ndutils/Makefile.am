DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes)\
 -I$(geomviewincludes:=/private)\
 $(default_includes)\
 $(X_CFLAGS)\
 -I$(TCL_INCLUDE_PATH)\
 -I$(TK_INCLUDE_PATH)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

module_tcl_DATA = 3D-Snapshot Slicer Colormap
module_PROGRAMS = ndutils
ndutils_SOURCES = BasicFns.h Clip.h ColorMapDefs.h ColorPoint.h\
 GeExDir.h GeomExtentDirection.h GeomtoNoff.h LoadMap.h\
 MakeGoodTransforms.h Random.h TransformGeom.h WithColor.h\
 init.h slicer.h snapshot.h wrappers.h BasicFns.c ColorPoint.c\
 GeExDir.c GeomExtentDirection.c GeomtoNoff.c LoadMap.c\
 MakeGoodTransforms.c Random.c TransformGeom.c WithColor.c clip.c\
 clipoogl.c init.c slicer.c snapshot.c testwish.c wrappers.c

EMODULES  = .geomview-3d-snapshot .geomview-slicer .geomview-colormap

if LOCAL_EMODULE
  homedir = $(HOME)
  home_DATA = $(EMODULES)
  tclpath = $(module_tcldir)
  modulepath = $(moduledir)/
else
  module_DATA = $(EMODULES)
  tclpath = $$GEOMVIEW_EMODULE_DIR/tcl
if !GEOMVIEW_QUERY
    modulepath = $(moduledir)/
else
    modulepath =
endif
endif

.geomview-3d-snapshot: Makefile
	echo '(emodule-define "3D-Snapshot" "$(modulepath)ndutils $(tclpath)/3D-Snapshot")' > $@

.geomview-slicer: Makefile
	echo '(emodule-define "Slicer" "$(modulepath)ndutils $(tclpath)/Slicer")' > $@

.geomview-colormap: Makefile
	echo '(emodule-define "Colormap" "$(modulepath)ndutils $(tclpath)/Colormap")' > $@

.geomview: Makefile
	> $@
	for emod in 3D-Snapshot Slicer Colormap; do\
	  echo "(emodule-define \"$$emod\" \"$(abs_builddir)/ndutils $(abs_srcdir)/$$emod\")" >> $@;\
	done

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

EXTRA_DIST = $(module_tcl_DATA) $(man_MANS)
