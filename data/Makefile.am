SUBDIRS = shaders

# Current automake (version 1.4) will not create directory "b" if
# "a/b/foo" is listed in a _DATA variable.
#
#
# We can hack our way around this by taking advantage of the fact that
# automake will install the various *_DATA files in order of the
# variable name.

groups =\
 groups/101a.dgp\
 groups/101b.dgp\
 groups/120.dgp\
 groups/230data\
 groups/230data.Conway\
 groups/230data.lat\
 groups/230data.names\
 groups/3torus.dgp\
 groups/3torus2.dgp\
 groups/94a.dgp\
 groups/94b.dgp\
 groups/borrom.dgp\
 groups/borrom2.dgp\
 groups/borrom3.dgp\
 groups/borrom4.dgp\
 groups/borrom4.wa\
 groups/fig8.dgp\
 groups/g233.prj\
 groups/g234.prj\
 groups/g235.prj\
 groups/g236.prj\
 groups/g237.prj\
 groups/good_klein_quick.dgp\
 groups/hook.off\
 groups/klein.wa\
 groups/klein_bricks.dgp\
 groups/klein_whole.dgp\
 groups/notknot.dgp\
 groups/sample.proj.dgp\
 groups/sample.uhs.dgp\
 groups/spaceshipSpace.geom\
 groups/tetra2.dgp\
 groups/wholecube.geom

gcl =\
 gcl/arithmetic.gcl\
 gcl/events.gcl

nobase_dist_geomdata_DATA =\
 .geomview\
 cmap.fmap\
 geom/abstr.off\
 geom/alphabet.vect\
 geom/antoine0.oogl\
 geom/antoine1.oogl\
 geom/antoine2.oogl\
 geom/antoine3.oogl\
 geom/axes.list\
 geom/br2.grp\
 geom/br2.off\
 geom/br4.2\
 geom/br4fd.grp\
 geom/br4fd.list\
 geom/cam.off\
 geom/camera-frustum\
 geom/catenoid\
 geom/cone.off\
 geom/csquare.quad\
 geom/cube\
 geom/cube.off\
 geom/diamond\
 geom/dodec\
 geom/dodec.off\
 geom/dodec.quad\
 geom/dodec2.off\
 geom/facecube.off\
 geom/hdodec.off\
 geom/helicoid\
 geom/hypercube.off\
 geom/ico.grp\
 geom/icosa.off\
 geom/icosa.quad\
 geom/jungle.list\
 geom/klein8\
 geom/link2.grp\
 geom/link8.grp\
 geom/moebius.4d.mesh\
 geom/moebius.skel\
 geom/mushroom.off\
 geom/nsquare.quad\
 geom/octa.off\
 geom/octant.bez\
 geom/office.oogl\
 geom/pieces/cube1.off\
 geom/pieces/linkA.bez\
 geom/pieces/linkB.bez\
 geom/polyhedra/cube\
 geom/polyhedra/cuboctahedron\
 geom/polyhedra/dodecahedron\
 geom/polyhedra/icosahedron\
 geom/polyhedra/icosidodecahedron\
 geom/polyhedra/octahedron\
 geom/polyhedra/rhombicosidodecahedron\
 geom/polyhedra/rhombicuboctahedron\
 geom/polyhedra/rhombitruncated_cubeoctahedron\
 geom/polyhedra/rhombitruncated_icosidodecahedron\
 geom/polyhedra/snub_cube\
 geom/polyhedra/snub_dodecahedron\
 geom/polyhedra/tetrahedron\
 geom/polyhedra/truncated_cube\
 geom/polyhedra/truncated_dodecahedron\
 geom/polyhedra/truncated_icosahedron\
 geom/polyhedra/truncated_octahedron\
 geom/polyhedra/truncated_tetrahedron\
 geom/ref.inst\
 geom/ref.list\
 geom/ref1.grp\
 geom/ref2.list\
 geom/sample.mesh\
 geom/sph12.mesh\
 geom/sphere.mesh\
 geom/spherical/dodecaplex.off\
 geom/spherical/flattorus.mesh\
 geom/spherical/hypercube.off\
 geom/spherical/octaplex.off\
 geom/spherical/orthoplex.off\
 geom/spherical/ruledtorus.mesh\
 geom/spherical/simplex.off\
 geom/spherical/tetraplex.off\
 geom/spherical/twosided.flattorus\
 geom/square.quad\
 geom/teapot.bez\
 geom/tent.mesh\
 geom/tetra\
 geom/tetra.off\
 geom/textured/Antenna.geom\
 geom/textured/GCGball\
 geom/textured/dodec.off\
 geom/textured/lunar-t.oogl\
 geom/textured/rabbitball\
 geom/textured/square.off\
 geom/textured/themi.bez\
 geom/textured/sphere.gcl\
 geom/textured/Earth.pam.bz2\
 geom/textured/NASA.pgm.gz\
 geom/textured/geologo.pgm.gz\
 geom/textured/grid.pgm.gz\
 geom/textured/rabbits.ppm.gz\
 geom/textured/white.pgm.gz\
 geom/torus.bez\
 geom/torus3.oogl\
 geom/torus8.oogl\
 geom/trapezoid.4d.off\
 geom/tref.mesh\
 geom/tref.off\
 geom/tri.4d.vect\
 geom/tri.vect\
 geom/triangle\
 geom/twosidedsquare.quad\
 geom/unit.sphere\
 geom/unitcube.off\
 geom/vertcube.off\
 geom/warp.inst\
 geom/wraptent.mesh\
 geom/xwyezed.vect\
 geom/xyz.grp\
 geom/xyz.vect\
 geom/ztent.mesh

nobase_dist_geomdata_DATA += $(groups) $(gcl)


