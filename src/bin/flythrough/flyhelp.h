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
 * flyhelp.h: generated automatically from file "flyhelp"
 *
 *  to modify, edit "flyhelp" and type "make flyhelp.h"
 */
"\n\
This geomview external module lets you fly through the tesselation of\n\
hyperbolic space by a right-angled regular dodecahedron which appeared\n\
in the mathematical animation \"Not Knot\" produced by the Geometry\n\
Center. You can either pick a pre-computed flight path or fly around\n\
interactively.\n\
\n\
All 30 edges of the dodecahedron are white except the three pairs of\n\
edges colored green, blue and red corresponding to the three loops of\n\
the Borromean rings. Every face of the dodecahedron has exactly one\n\
non-white edge, so we can color the face by this color.\n\
\n\
All flight paths begin and end at the center of a green face.  There\n\
are three other green faces: one adjacent to this one, at right\n\
angles along the green beam; and a pair which border the other green\n\
beam, on the other side of the dodecahedron.\n\
\n\
The light blue \"Direct\" path is the simplest to understand:\n\
we go straight through to the green face directly opposite from the\n\
original face.\n\
\n\
The yellow \"Quarter Turn\" path, which goes to the adjacent green face,\n\
simply circles around the green axis which the two faces share.\n\
\n\
The \"Full Loop\" path is also yellow: it repeats this quarter turn four\n\
times so that we start and finish in the same place. The three other\n\
paths just jump back to the starting place when they reach the end.\n\
\n\
The magenta \"Equidistant\" path, which goes to the other green face\n\
which doesn't border the original face, is the most interesting.  It\n\
follows a so-called equidistant curve: in this case, one that is\n\
equidistant to the red axis that connects the two green faces in\n\
question. This curve is like a parallel line in Euclidean space: it\n\
stays a constant distant from the red axis, but it's not a geodesic in\n\
hyperbolic space.\n\
\n\
In the small 3D diagram window, you can use the left mouse button to\n\
spin around a dodecahedron with colored coded flight paths as\n\
mentioned above. It's easier to see what's going on in the Euclidean\n\
diagram, while the hyperbolic version is more similar to what you see\n\
in the flythrough.\n\
\n\
You can either choose one of four flight paths through the tesselation\n\
or stop the automatic flight by hitting the \"Stop\" button and fly\n\
around yourself.  For interactive flight, hit the \"Cam Fly\" button on\n\
the geomview Tools panel: then dragging the mouse with the middle\n\
button down moves you forwards or backwards, and dragging with the\n\
left button down is like turning your head. When you hit \"Go\", the\n\
automatic flight will continue.\n\
\n\
You can choose one of four tesselation levels: level 0 is a single\n\
dodecahedron, level 1 adds a layer of 12 dodecahedra (one for each\n\
face of the original dodecahedron), level 2 tesselates two layers\n\
deep, and level 3 has three layers. The more layers you have the\n\
slower the update rate: level 3 is glacially slow, but each frame\n\
looks pretty impressive. You can change the size of the dodecahedra\n\
with the \"Scale Dodecahedra\" slider: at 1.0 they fit together exactly.\n\
The \"Steps\" buttons control the smoothness of the flight path: you can\n\
set the number of steps to 10 (jerky but fast), 20, 40, or 80 (smooth\n\
but slow).\n\
\n\
\n\
Authors:	\n\
Charlie Gunn  	(geometry and flight paths)	gunn@geomview.org\n\
Tamara Munzner	(interactive interface)		munzner@geomview.org\n\
Stuart Levy	(3D diagram)				levy@geomview.org\n\
\n\
Copyright (c) 1992\n\
The Geometry Center\n\
\n\
Available free from www.geomview.org\n\
\n\
You can redistribute and/or modify this program according to the terms\n\
of the the GNU General Public License.\n\n"
