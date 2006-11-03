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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef PICKDEF
#define	PICKDEF

/*
 * Public definitions for Geom Picking.
 */

typedef struct Pick Pick;

				/* Things to look for while picking
				 * Specify PW_VERT|PW_EDGE|PW_FACE for all,
				 * or a subset for more specific picks.
				 * PW_VISIBLE checks Appearance to see what's
				 * actually visible.
				 */
#define	PW_VERT	1
#define	PW_EDGE	2
#define	PW_FACE	4
#define	PW_VISIBLE 8

#define	PA_END		100
#define	PA_THRESH	101	/* float; x/y threshold (in projected space) */
#define	PA_POINT	102	/* Point3 *; Picked 3D point (object space) */
			    /* Note PA_POINT gives a Point3, not an HPoint3! */
#define	PA_DEPTH	103	/* float; depth (== PA_POINT's screen z component) */

#define	PA_GPRIM	104	/* Geom *; geometric primitive picked if any */
#define	PA_TPRIM	105	/* Transform; from PA_GPRIM to
				   PA_POINT, PA_VERT, etc. */
#define	PA_TPRIMN	128	/* TransformN; from PA_GPRIM to
				   PA_POINT, PA_VERT, etc. */
#define	PA_TWORLD	111	/* Transform; from PA_POINT, PA_VERT, etc. to
				 *  world coords */
#define	PA_TWORLDN	129	/* Transform; from PA_POINT, PA_VERT, etc. to
				 *  world coords */

#define	PA_WANT		106	/* int; bitmask of desirable values */
#define PA_VERT		107	/* HPoint3 v; picked vertex, if any */
#define PA_EDGE		108	/* HPoint3 e[2]; endpoints of picked
				   edge, if any */
#define PA_FACE		109	/* HPoint3 *f; pointer to array of vertices of
				   picked face, if any */
#define PA_FACEN	110	/* number of elements in PA_FACE array; NOTE:
				   when setting PA_FACE you must also set
				   PA_FACEN. */

	/* Stuff to specify via PickSet() to allow
	 * INSTs with "location" fields to be pickable.
	 */
#define	PA_TC2N		112	/* Tfm camera coords to NDC (-1..+1) */
#define	PA_TW2N		113	/* Tfm global coords to NDC */
#define	PA_TS2N		114	/* Tfm screen (pixels; 0,0=lower left) to NDC */

extern Pick *PickSet( Pick *, int attr, ... );
extern int   PickGet( Pick *, int attr, void *attrp );
extern void  PickDelete( Pick * );

/*
 * Geometric Picking routines.
 * GeomMousePick() is intended for normal use by application programs.
 *   g is the root of the Geom tree
 *   p is the Pick structure created by PickSet(), etc., or NULL for default;
 *	If p!=NULL, it is modified in place, so results of the pick can be
 *	retrieved by later PickGet() calls.
 *   ap is the Appearance in effect when g is drawn, used to determine
 *	visibility of features if PW_VISIBLE is set;
 *   Tproj is the complete object-to-screen projection matrix;
 *	it normally maps world coordinates in g's space to
 *	the unit cube -1 <= {x, y, z} <= 1,
 *	with z = -1 at the near and z = +1 at the far clipping plane,
 *	x = y = 0 at the center of the screen;
 *   xpick and ypick are the relative (-1 <= {x,y} <= 1) window
 *	coordinates of the pick.
 *
 * Typical use:
 *   Point3 rawpt, pickworld, pickprim;
 *   double mousex, mousey;	( each reduced to range -1 .. +1; 0=center )
 *   Geom *world, *g;
 *   Pick *p = PickSet(NULL, PA_END);
 *   Transform Tworld2screen, Tpick2world, Tpick2prim;
 *   CamView( camera, Tworld2screen );	( camera's world to screen )
 *
 *   g = GeomMousePick( world, p, ap, Tworld2screen, mousex, mousey );
 *   if(g != NULL) {			(then g == world)
 *	PickGet(p, PA_POINT, &rawpt);
 *	PickGet(p, PA_TWORLD, Tpick2world);
 *      Pt3Transform(Tpick2world, &rawpt, &pickworld);  (in "world"'s coords)
 *
 *	PickGet(p, PA_TPRIM, Tpick2prim);
 *	Pt3Transform(Tpick2prim, &rawpt, &pickprim);	(in leaf-level gprim's coords)
 *    }
 */
extern Geom *GeomMousePick(Geom *g, Pick *p, Appearance *ap,
			   Transform Tproj, TransformN *TNproj, int *axes,
			   double xpick, double ypick );

/*
 * GeomPick() is an internal routine:
 *	g, p, ap are as above for GeomMousePick.
 *	T is adapted for convenience in pick calculations;
 *	it is translated such that the pick goes through (x,y) = (0,0).
 */
extern Geom *GeomPick(Geom *g, Pick *p, Appearance *ap,
		      Transform T, TransformN *TN, int *axes);

#endif /*PICKDEF*/
