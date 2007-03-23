NDviewdatadir = $(geomdatadir)/NDview

ndview_scripts = scripts/.ndview scripts/4d.2bigwins     \
 scripts/4d.3wins scripts/4d.4wins scripts/4d.cplx \
 scripts/4d.earthcolor scripts/4d.wildcolor	       \
 scripts/4d.xyw scripts/4d.xyz scripts/4d.xzw      \
 scripts/4d.yzw scripts/4dcol.w.rb		       \
 scripts/4dcol.x.rb scripts/4dcol.y.rb		       \
 scripts/4dcol.z.rb scripts/5d.123		       \
 scripts/5d.5wins scripts/5dcol.1.rb		       \
 scripts/5dcol.2.rb scripts/5dcol.3.rb		       \
 scripts/5dcol.4.rb scripts/5dcol.5.rb		       \
 scripts/color.script scripts/delete_all.script	       \
 scripts/deletecams.script scripts/nocolor.script	       \
 scripts/reset.script scripts/startup.big.script	       \
 scripts/startup.small.script

ndview_geom = geom/4cube geom/4daxes geom/5daxes  \
 geom/6daxes geom/crosscap geom/dodecaplex.skel    \
 geom/flattorus geom/funcplane geom/hypercube.skel \
 geom/octaplex.skel geom/orthoplex.skel		       \
 geom/pluker23col.4d geom/simplex.skel		       \
 geom/tetraplex.skel geom/demo/axes		       \
 geom/demo/cubew geom/demo/cubew+ geom/demo/point  \
 geom/demo/points geom/demo/segmentw+x+z		       \
 geom/demo/segmentw+x+z+ geom/demo/squarew+x	       \
 geom/demo/squarew+x+ geom/demo/tesseract

ndview_text = text/axeshelp.txt text/cube.txt	       \
 text/cubed.txt text/demohelp.txt			       \
 text/introhelp.txt text/point.txt text/pointd.txt \
 text/prefabhelp.txt text/segment.txt		       \
 text/segmentd.txt text/square.txt			       \
 text/squared.txt text/tesseract.txt		       \
 text/toolkithelp.txt

nobase_NDviewdata_DATA = $(ndview_scripts) $(ndview_geom) $(ndview_text)

nobase_NDviewdata_SCRIPTS = scripts/eitherwindow

EXTRA_DIST = $(nobase_NDviewdata_SCRIPTS) $(nobase_NDviewdata_DATA)
