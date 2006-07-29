INCLUDES = $(default_includes)

LDADD = \
 $(TK_LIB) \
 $(TCL_LIB) \
 $(XLIBS) \
 $(OOGLLIB)

module_PROGRAMS = ndview
ndview_SOURCES = ndview.c testwish.c
ndview_DEPENDENCIES = $(OOGLLIB)
ndview_LDFLAGS= -rpath $(libdir)

module_tcldir = $(moduledir)/tcl
module_tcl_DATA = NDview NDdemo
module_DATA = .geomview-ndview .geomview-nddemo

ndviewdir          = $(geomdatadir)/NDview
ndview_scriptsdir  = $(ndviewdir)/scripts
ndview_geomdir     = $(ndviewdir)/geom
ndview_geomdemodir = $(ndview_geomdir)/demo
ndview_textdir     = $(ndviewdir)/text

ndview_scripts_SCRIPTS = scripts/eitherwindow

ndview_scripts = scripts/.ndview scripts/4d.2bigwins	      \
 scripts/4d.3wins scripts/4d.4wins scripts/4d.cplx		      \
 scripts/4d.earthcolor scripts/4d.wildcolor scripts/4d.xyw	      \
 scripts/4d.xyz scripts/4d.xzw scripts/4d.yzw scripts/4dcol.w.rb	      \
 scripts/4dcol.x.rb scripts/4dcol.y.rb scripts/4dcol.z.rb	      \
 scripts/5d.123 scripts/5d.5wins scripts/5dcol.1.rb scripts/5dcol.2.rb \
 scripts/5dcol.3.rb scripts/5dcol.4.rb scripts/5dcol.5.rb	      \
 scripts/color.script scripts/delete_all.script			      \
 scripts/deletecams.script scripts/nocolor.script \
 scripts/reset.script scripts/startup.big.script			      \
 scripts/startup.small.script

ndview_geom = geom/4cube geom/4daxes geom/5daxes geom/6daxes \
 geom/crosscap geom/dodecaplex.skel geom/flattorus geom/funcplane  \
 geom/hypercube.skel geom/octaplex.skel geom/orthoplex.skel	  \
 geom/pluker23col.4d geom/simplex.skel geom/tetraplex.skel	  \
 geom/demo/axes geom/demo/cubew geom/demo/cubew+ geom/demo/point	  \
 geom/demo/points geom/demo/segmentw+x+z geom/demo/segmentw+x+z+	  \
 geom/demo/squarew+x geom/demo/squarew+x+ geom/demo/tesseract

ndview_text = text/axeshelp.txt text/cube.txt text/cubed.txt      \
 text/demohelp.txt text/introhelp.txt text/point.txt text/pointd.txt    \
 text/prefabhelp.txt text/segment.txt text/segmentd.txt text/square.txt \
 text/squared.txt text/tesseract.txt text/toolkithelp.txt

nobase_ndview_DATA = $(ndview_scripts) $(ndview_geom) $(ndview_text)

.geomview-ndview: Makefile
	echo '(emodule-define "NDview" "ndview $$GEOMVIEW_EMODULE_PATH/tcl/NDview")' > .geomview-ndview

.geomview-nddemo: Makefile
	echo '(emodule-define "NDdemo" "ndview $$GEOMVIEW_EMODULE_PATH/tcl/NDdemo")' > .geomview-nddemo

all-local: $(top_builddir)/modules/.geomview-ndview  $(top_builddir)/modules/.geomview-nddemo

$(top_builddir)/modules/.geomview-ndview \
 $(top_builddir)/modules/.geomview-nddemo: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "NDview" "env NDVIEWROOT=$$GEOMROOT/$(subdir)/.. $(subdir)/ndview $(subdir)/$(srcdir)/NDview")' > $(top_builddir)/modules/.geomview-ndview
	echo '(emodule-define "NDdemo" "env NDVIEWROOT=$$GEOMROOT/$(subdir)/.. $(subdir)/ndview $(subdir)/$(srcdir)/NDdemo")' > $(top_builddir)/modules/.geomview-nddemo

clean-local:
	rm -f $(top_builddir)/modules/.geomview-ndview
	rm -f $(top_builddir)/modules/.geomview-nddemo

