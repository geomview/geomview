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
 * hingehelp.h: generated automatically from file "hingehelp"
 *
 *  to modify, edit "hingehelp" and type "make hingehelp.h"
 */
"\n\
Hinge lets you rotate copies of a\n\
polyhedron around edges of itself to build\n\
up constructions in space.  It is an\n\
example of a program that interacts with\n\
geomview, the Geometry Center's 3D viewing\n\
program.  Its main purpose is to demonstrate\n\
some capabilities of geomview.\n\
\n\
To move the construction around in the\n\
graphics window, hold the left mouse button\n\
down and drag the mouse.  If you let go of\n\
the button while moving the mouse, the motion\n\
will continue.  The type of motion is\n\
determined by the selected icon in the\n\
\"Tools\" panel; to change actions just click\n\
on one of these icons.\n\
\n\
Start by clicking the right mouse button on\n\
an edge to select it.  Then click the right\n\
mouse button on an adjacent face to rotate\n\
the object through that face.  You may select\n\
another edge at any time by clicking the\n\
right mouse button on it.\n\
\n\
You may load an alternate polyheron via the\n\
\"File\" button.  Nice objects to try are:\n\
\n\
  in Euclidean space      in hyperbolic space\n\
\n\
          HingeCube                HingeDodec\n\
          HingeTetra\n\
          HingeDiamond\n\
          HingeTriangle\n\
          HingeLongcube\n\
\n\
NOTES:\n\
\n\
Double-clicking the right mouse button sets\n\
geomview's notion of the current mouse\n\
target object.  In most cases with Hinge\n\
you want the current target to be the\n\
entire world.  If you accidentally move an\n\
individual object you may reset it by\n\
hitting the 'w' key, or 'W' to reset\n\
everything.  Then double-click on the\n\
background to set the mouse target back to\n\
the world.\n\
\n\
Hinge works with any geomview OFF object.\n\
You may create your own objects in the OFF\n\
format as described in oogl(5).  Type\n\
\"man 5 oogl\" for details.\n\
\n\
BUGS\n\
\n\
Because of some limitations in the current\n\
version of geomview, Hinge sometimes does\n\
not recognize that you are pointing at a\n\
certain edge or face.  This usually happens\n\
with polyhedra having faces which are\n\
nearly coplanar, or in hyperbolic space as\n\
you get far away from the origin.  It does\n\
not work at all with a polyhedron that has\n\
two or more exactly coplanar faces.  This\n\
limitation will disappear in a future\n\
version.\n\n"
