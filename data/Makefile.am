EXTRA_DIST = $(nobase_geomdata_DATA) $(nobase_geomdata_SCRIPTS)

# Current automake (version 1.4) will not create directory "b" if
# "a/b/foo" is listed in a _DATA variable.
#
#
# We can hack our way around this by taking advantage of the fact that
# automake will install the various *_DATA files in order of the
# variable name.

groups =					\
	groups/101a.dgp				\
	groups/101b.dgp				\
	groups/120.dgp				\
	groups/230data				\
	groups/230data.Conway			\
	groups/230data.lat			\
	groups/230data.names			\
	groups/3torus.dgp			\
	groups/3torus2.dgp			\
	groups/94a.dgp				\
	groups/94b.dgp				\
	groups/borrom.dgp			\
	groups/borrom2.dgp			\
	groups/borrom3.dgp			\
	groups/borrom4.dgp			\
	groups/borrom4.wa			\
	groups/br4.0.tlist			\
	groups/br4.1.tlist			\
	groups/br4.2.tlist			\
	groups/br4.3.tlist			\
	groups/dir.1.gv				\
	groups/dir.2.gv				\
	groups/dir.3.gv				\
	groups/dir.4.gv				\
	groups/dodec.vect			\
	groups/equi.1.gv			\
	groups/equi.2.gv			\
	groups/equi.3.gv			\
	groups/equi.4.gv			\
	groups/fig8.dgp				\
	groups/g233.prj				\
	groups/g234.prj				\
	groups/g235.prj				\
	groups/g236.prj				\
	groups/g237.prj				\
	groups/good_klein_quick.dgp		\
	groups/hook.off				\
	groups/klein.wa				\
	groups/klein_bricks.dgp			\
	groups/klein_whole.dgp			\
	groups/loop.1.gv			\
	groups/loop.2.gv			\
	groups/loop.3.gv			\
	groups/loop.4.gv			\
	groups/notknot.dgp			\
	groups/quart.1.gv			\
	groups/quart.2.gv			\
	groups/quart.3.gv			\
	groups/quart.4.gv			\
	groups/sample.proj.dgp			\
	groups/sample.uhs.dgp			\
	groups/spaceshipSpace.geom		\
	groups/tetra2.dgp			\
	groups/wholecube.geom

shaders = \
 shaders/hlight.sl \
 shaders/hlight.slo \
 shaders/hmatte.sl \
 shaders/hmatte.slo \
 shaders/hplastic.sl \
 shaders/hplastic.slo

ndview_scripts = NDview/scripts/.ndview NDview/scripts/4d.2bigwins     \
 NDview/scripts/4d.3wins NDview/scripts/4d.4wins NDview/scripts/4d.cplx \
 NDview/scripts/4d.earthcolor NDview/scripts/4d.wildcolor	       \
 NDview/scripts/4d.xyw NDview/scripts/4d.xyz NDview/scripts/4d.xzw      \
 NDview/scripts/4d.yzw NDview/scripts/4dcol.w.rb		       \
 NDview/scripts/4dcol.x.rb NDview/scripts/4dcol.y.rb		       \
 NDview/scripts/4dcol.z.rb NDview/scripts/5d.123		       \
 NDview/scripts/5d.5wins NDview/scripts/5dcol.1.rb		       \
 NDview/scripts/5dcol.2.rb NDview/scripts/5dcol.3.rb		       \
 NDview/scripts/5dcol.4.rb NDview/scripts/5dcol.5.rb		       \
 NDview/scripts/color.script NDview/scripts/delete_all.script	       \
 NDview/scripts/deletecams.script NDview/scripts/nocolor.script	       \
 NDview/scripts/reset.script NDview/scripts/startup.big.script	       \
 NDview/scripts/startup.small.script

ndview_geom = NDview/geom/4cube NDview/geom/4daxes NDview/geom/5daxes  \
 NDview/geom/6daxes NDview/geom/crosscap NDview/geom/dodecaplex.skel    \
 NDview/geom/flattorus NDview/geom/funcplane NDview/geom/hypercube.skel \
 NDview/geom/octaplex.skel NDview/geom/orthoplex.skel		       \
 NDview/geom/pluker23col.4d NDview/geom/simplex.skel		       \
 NDview/geom/tetraplex.skel NDview/geom/demo/axes		       \
 NDview/geom/demo/cubew NDview/geom/demo/cubew+ NDview/geom/demo/point  \
 NDview/geom/demo/points NDview/geom/demo/segmentw+x+z		       \
 NDview/geom/demo/segmentw+x+z+ NDview/geom/demo/squarew+x	       \
 NDview/geom/demo/squarew+x+ NDview/geom/demo/tesseract

ndview_text = NDview/text/axeshelp.txt NDview/text/cube.txt	       \
 NDview/text/cubed.txt NDview/text/demohelp.txt			       \
 NDview/text/introhelp.txt NDview/text/point.txt NDview/text/pointd.txt \
 NDview/text/prefabhelp.txt NDview/text/segment.txt		       \
 NDview/text/segmentd.txt NDview/text/square.txt			       \
 NDview/text/squared.txt NDview/text/tesseract.txt		       \
 NDview/text/toolkithelp.txt

ndview = $(ndview_scripts) $(ndview_geom) $(ndview_text)

nobase_geomdata_SCRIPTS = NDview/scripts/eitherwindow

cplxview = \
 NDview/cplxview/text/cplxhelp.txt \
 NDview/cplxview/geom/funcplane \
 NDview/cplxview/scripts/reset.script \
 NDview/cplxview/scripts/startup.script \
 NDview/cplxview/scripts/del_c0 \
 NDview/cplxview/scripts/4d.colorcplx \
 NDview/cplxview/scripts/4d.cplx

nobase_geomdata_DATA = \
	.geomview \
	cmap.fmap \
	geom/HingeCube \
	geom/HingeDiamond \
	geom/HingeDodec \
	geom/HingeLongcube \
	geom/HingeTetra \
	geom/HingeTriangle \
	geom/abstr.off \
	geom/alphabet.vect \
	geom/antoine0.oogl \
	geom/antoine1.oogl \
	geom/antoine2.oogl \
	geom/antoine3.oogl \
	geom/axes.list \
	geom/br2.grp \
	geom/br2.off \
	geom/br4.2 \
	geom/br4fd.grp \
	geom/br4fd.list \
	geom/cam.off \
	geom/camera-frustum \
	geom/catenoid \
	geom/cone.off \
	geom/csquare.quad \
	geom/cube \
	geom/cube.off \
	geom/diamond \
	geom/dodec \
	geom/dodec.off \
	geom/dodec.quad \
	geom/dodec.vect \
	geom/dodec2.off \
	geom/facecube.off \
	geom/flythrough_diagram.gv \
	geom/hdodec.off \
	geom/helicoid \
	geom/hypercube.off \
	geom/ico.grp \
	geom/icosa.off \
	geom/icosa.quad \
	geom/jungle.list \
	geom/klein8 \
	geom/link2.grp \
	geom/link8.grp \
	geom/moebius.4d.mesh \
	geom/mushroom.off \
	geom/nsquare.quad \
	geom/octa.off \
	geom/octant.bez \
	geom/office.oogl \
	geom/pieces/cube1.off \
	geom/pieces/linkA.bez \
	geom/pieces/linkB.bez \
	geom/polyhedra/cube \
	geom/polyhedra/cuboctahedron \
	geom/polyhedra/dodecahedron \
	geom/polyhedra/icosahedron \
	geom/polyhedra/icosidodecahedron \
	geom/polyhedra/octahedron \
	geom/polyhedra/rhombicosidodecahedron \
	geom/polyhedra/rhombicuboctahedron \
	geom/polyhedra/rhombitruncated_cubeoctahedron \
	geom/polyhedra/rhombitruncated_icosidodecahedron \
	geom/polyhedra/snub_cube \
	geom/polyhedra/snub_dodecahedron \
	geom/polyhedra/tetrahedron \
	geom/polyhedra/truncated_cube \
	geom/polyhedra/truncated_dodecahedron \
	geom/polyhedra/truncated_icosahedron \
	geom/polyhedra/truncated_octahedron \
	geom/polyhedra/truncated_tetrahedron \
	geom/ref.inst \
	geom/ref.list \
	geom/ref1.grp \
	geom/ref2.list \
	geom/sample.mesh \
	geom/sph12.mesh \
	geom/sphere.mesh \
	geom/spherical/dodecaplex.off \
	geom/spherical/flattorus.mesh \
	geom/spherical/hypercube.off \
	geom/spherical/octaplex.off \
	geom/spherical/orthoplex.off \
	geom/spherical/ruledtorus.mesh \
	geom/spherical/simplex.off \
	geom/spherical/tetraplex.off \
	geom/spherical/twosided.flattorus \
	geom/square.quad \
	geom/teapot.bez \
	geom/tent.mesh \
	geom/tetra \
	geom/tetra.off \
	geom/textured/Antenna.geom \
	geom/textured/GCGball \
	geom/textured/dodec.off \
	geom/textured/lunar-t.oogl \
	geom/textured/rabbitball \
	geom/textured/rabbits.ppm.gz \
	geom/textured/square.off \
	geom/textured/themi.bez \
	geom/torus.bez \
	geom/torus3.oogl \
	geom/torus8.oogl \
	geom/trapezoid.4d.off \
	geom/tref.mesh \
	geom/tref.off \
	geom/tri.4d.vect \
	geom/tri.vect \
	geom/triangle \
	geom/twosidedsquare.quad \
	geom/unit.sphere \
	geom/unitcube.off \
	geom/vertcube.off \
	geom/warp.inst \
	geom/wraptent.mesh \
	geom/xwyezed.vect \
	geom/xyz.grp \
	geom/xyz.vect \
	geom/ztent.mesh

nobase_geomdata_DATA += $(groups)
nobase_geomdata_DATA += $(shaders)
nobase_geomdata_DATA += $(ndview)
nobase_geomdata_DATA += $(cplxview)

