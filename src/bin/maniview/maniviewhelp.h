/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */
/*
 * maniviewhelp.h: generated automatically from file "maniviewhelp"
 *
 *  to modify, edit "maniviewhelp" and type "make maniviewhelp.h"
 */
"\n\
Maniview explores three-dimensional manifolds and orbifolds \n\
from \"within\".  You navigate as if you lived inside the\n\
space. This means, in practice, that you will be within\n\
a tessellation of hyperbolic, euclidean, or spherical\n\
space by a discrete groups of motions.  Maniview works\n\
in cooperation with Geomview.\n\
\n\
The application is structured around a main menu\n\
which accesses several panels.  To begin with,\n\
use the 'Load' panel to load a discrete group,\n\
which is the basic data structure used by Maniview.\n\
\n\
LOAD\n\
\n\
The format for discrete groups is described in\n\
the man page discgrp(5).  The default is the three\n\
dimensional torus.  Other files that are provided with\n\
the release include borrom4.dgp (a hyperbolic orbifold)\n\
and 120.dgp (the Poincare homology sphere, a spherical\n\
manifold).\n\
\n\
Maniview looks for the entered file first in the\n\
current directory and then in the $GEOMDATA/groups\n\
directory.  The 'geomview' script usually sets $GEOMDATA,\n\
but you can set it by hand if you wish using 'setenv'.\n\
\n\
By activating the buttons on the Load panel you can\n\
choose instead to load geometry to be tessellated by the group\n\
(depress the 'Load Geom' button); to load a geometry\n\
file to represent the camera, depress the 'Load Camgeom'\n\
button.\n\
\n\
SAVE \n\
\n\
Use the 'Save' menu option to save the current discrete\n\
group to a file. Activate the 'Save Geom' button to \n\
explicitly save the currently displayed geometry with\n\
the group; use the 'Save Group' button to save the\n\
currently displayed list of group elements with the\n\
group.  \n\
\n\
DISPLAY \n\
\n\
You can control display characteristics through the\n\
Display panel.  The toggles control how the\n\
discrete group draws itself. To activate a property,\n\
depress its button by clicking on it.  Deactivation\n\
works in the inverse manner.\n\
\n\
centercam:  toggle controls whether to keep the observer \n\
    centered in the central fundamental domain. \n\
draw dirdom: will cause the Dirichlet domain geometry to \n\
    be displayed (see BASIC TILE panel below for more).  \n\
drawgeom: will cause any other geometry you have supplied \n\
    to be displayed.  (see BASIC TILE panel below for more).\n\
showcam: will tessellate an image of the camera or observer.\n\
    The geometry for the camera can be loaded using the LOAD\n\
    panel (see above).  \n\
cull: will attempt to ignore group elements which are not \n\
    visible.  It is provided as an option as long as it \n\
    hasn't been fully debugged. \n\
software shading: allows you to toggle between hardware and \n\
   software shading.  Hardware shading is correct only \n\
   for Euclidean groups. \n\
\n\
Finally, three sliders control the lighting model.\n\
The first 'atten1' influences the attenuation of light \n\
intensity as a function of the distance d of the light source \n\
to the object.  For the hyperbolic and spherical case this value\n\
appears in an expression of the form exp(-atten1 d).\n\
This is reasonable for hyperbolic geometry and used in\n\
spherical geometry to provide depth cueing.  In the\n\
Euclidean case, atten1 appears in an expression of the\n\
form 1/(d^atten1).  \n\
\n\
The other two sliders, 'fogfree' and 'fog', control the presence\n\
of fog in the lighting model.  'fogfree' specifies a radius around\n\
the observer which is free of fog.  'fog' is a constant which \n\
appears in an expression of the form exp( -fog d') where \n\
d' = d - fogfree, where d is the distance from the eye to the\n\
surface and d' is clamped to zero if it's negative.\n\
\n\
ENUMERATE\n\
\n\
The 'Enumerate' panel controls how many and which group\n\
elements are displayed.  'worddepth' is an integer value\n\
which specifies the maximum length of a string, in the\n\
generators of the group, which is to be used. (Use the double\n\
arrows to control this counter, since it must have an \n\
integer value.) 'tesselation radius' controls how far away \n\
the group element may move the center point (this is controlled\n\
from the BASIC TILE panel).  These two contraints work as a\n\
Boolean 'and':  both must be satisfied for a group element to\n\
be displayed.\n\
\n\
BASIC TILE\n\
\n\
This panel controls the geometry that is tessellated by\n\
the discrete group.  There are basically two modes: \n\
Dirichlet domain, and user-supplied geometry.  When the\n\
former button is depressed, the widgets control the computation \n\
of a Dirichlet domain.  There are positioning widgets to specify \n\
an (x,y,z) point, and a 'Reset' button to return it to the \n\
default (0,0,0).  In the current version, a full copy of \n\
the Dirichlet domain is displayed in wireframe and a smaller copy \n\
with cusps truncated (in the hyperbolic case) is displayed with \n\
face-shading.  The amount of scaling of this second copy is \n\
controlled by the slider labeled 'scale factor'.  A maximum value of \n\
1.0 results in images which completely fill space and hence \n\
cannot be seen through or around.  NOTE: The Dirichlet domain\n\
will be displayed if and only if the 'drawdirdom' button \n\
on the DISPLAY panel is depressed.\n\
\n\
Operation in 'user geometry' mode is as follows: \n\
If other geometry has been provided for display via the LOAD\n\
panel or in the original data file, the widgets in this panel \n\
are interpreted to give a translation and a scale for \n\
the supplied geometry, in the obvious way.\n\
NOTE: This geometry will be displayed if and only if \n\
the 'drawgeom' button in the DISPLAY panel is depressed, \n\
\n\
INFO\n\
\n\
Brings up a panel saying who wrote this module and how\n\
you can acquire it.\n\
\n\
HELP\n\
\n\
shows this panel.\n\
\n\
SEE ALSO\n\
\n\
There are connections from snappea and eucsyms to maniview.\n\
See eucsyms(1).\n\
\n\
BUGS and NOTES\n\
\n\
Sometimes you might need to toggle the 'software shading'\n\
button on the Display menu, to get geomview synchronized \n\
with maniview.  This may be needed particularly after loading\n\
a discrete group that changes the metric.\n\
\n\
Maniview forces the camera to be the target.  This allows\n\
the user to use 'fly' mode in the desired way when 'centercam'\n\
mode is active.  In other cases this setting might not be\n\
expected and you'll need to select the appropriate target\n\
in geomview to override this.\n\
\n\
The use of an (x,y,z) locator for Dirichlet domain is\n\
appropriate for euclidean and hyperbolic space but really\n\
ought to be fully 4D for the spherical case.\n\
\n\
Because geomview currently doesn't allow the use of finite\n\
light sources, maniview turns off Geomview lights and creates\n\
its own fixed lights.  Finite light sources are important \n\
since they give good depth cueing.\n\n"
