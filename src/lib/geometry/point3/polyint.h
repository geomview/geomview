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
#ifndef POLYINT_H
#define POLYINT_H


#define PW_VERT 1	/* What kinds of polygon intersections do we want? */
#define PW_EDGE 2
#define PW_FACE 4
#define PW_VISIBLE 8


/*
 * Tolerance - a good value to pass to PolyZInt and its subsidiaries.
 */
#define POLYINT_TOL 1.e-6

/*
 * PolyZInt
 * Finds intersection of z-axis and arbitrary flat polygon.
 *
 * Input:
 * n_verts          # of vertices in polygon (the length of the verts array)
 * verts            vertices of polygon
 * tol              Tolerance.  Any point whose distance from the z-axis 
 *                  is less than tol will be considered hit.
 *
 * Output:
 * All output values are vvecs (See ooglutil.h).  They must already be 
 * initialized when passed to PolyZInt.  Any new intersection points found
 * by PolyZInt will be appended onto the vvec.  The count field of all the 
 * vvecs must be up to date.  When PolyZInt returns, this field will reflect
 * any new elements that have been added to the vvec (ie, the new count 
 * field will be equal to the old count field plus the number of intersection
 * points found by PolyZInt).  Note that all the vvecs will be the same 
 * length.  PolyZInt will return the number of intersection points found.
 * ip               Intersection points (Point3).  These are the points where
 *                  the Z-axis intersects the plane containing the polygon.
 *                  Note that if tol > 0.0, these points may not actually
 *                  be inside the polygon.
 * vertices         Vertex intersections (ints - indices into verts).  If no
 *                  vertex intersection was found, the corresponding element
 *                  in this vvec will be set to -1.
 * edges            Edge intersections (ints - indices of first point in
 *                  verts list).  If no edge intersection was found, the 
 *                  corresponding element in this vvec will be set to -1.
 * ep               Edge intersection locations, if any were found (Point3).
 *                  An element of this array will contain valid data iff
 *                  the corresponding element of the edges vvec is not equal
 *                  to -1.  The elements of this are guarenteed to be points
 *                  on the edge, but, because of the tolerance, may not be
 *                  equal to the corresponding elements of the ip array.
 * For every intersection point found, ip will be filled in.  vertices will
 * be filled in only if a vertices was within tol of the intersection point.
 * If a vertices was within tol of the intersection point, the edges to which
 * the point belongs will not be considered to have been hit.
 */
int PolyZInt(int n_verts, Point3 *verts, float tol, int wanted, vvec *hits);

/* 
 * PolyNearNegZInt
 * Wrapper for PolyZInt designed for mouse picking.  Finds the 
 * intersection closest to the origin on the positive Z axis.
 * Returns non-zero if there was a hit.
 *
 * Input:
 * 		(See above)
 * Output:
 * ip		Intersection point.
 * vertex	Index of vertex or -1 if there was no vertex hit.
 * edge		Index of first vertex of edge or -1 if there was no 
 * 		edge hit.
 * ep		Edge point.
 * wanted	Bit-mask of PW_{FACE|VERT|EDGE}: want which kinds of picks?
 * zmin		Only accept picks where z<zmax (closer than zmax; -1=near, 1=far clip plane)
 */
int PolyNearPosZInt(int n_verts, Point3 *verts, float tol,
		    Point3 *ip, int *vertex, int *edge, Point3 *ep,
		    int wanted, float zmax);

/* 
 * PolyLineInt
 * Finds intersection of arbitrary line and arbitrary flat polygon.
 * pt1              One point on the line
 * pt2              A different point on the line
 * Other arguments as above
 */
int PolyLineInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *vert, 
		float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep);

/* 
 * PolyRayInt
 * Finds intersection of an arbitrary ray and an arbitrary flat polygon.
 * The endpoint is included.
 * pt1              Endpoint of the ray
 * pt2              A different point on the ray
 * Other arguments as above
 */
int PolyRayInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *verts, 
	       float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep);

/*
 * PolySegmentInt
 * Finds the intersection of an arbitrary segment and an arbitrary flat
 * polygon.  The endpoints are included.
 * pt1              One endpoint of the segment
 * pt2              The other endpoint of the segment
 * Other arguments as above
 */
int PolySegmentInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *verts, 
		   float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep);


/*
 * PolyInt_Align
 * Creates a coordinate system such that the first specified point becomes
 * the origin and the second becomes (0, 0, -1)
 * pt1              Point which will map to the origin
 * pt2              Point which will map to (0, 0, -1)
 * T                Matrex in which the transformation will be placed
 */
void PolyInt_Align(Point3 *pt1, Point3 *pt2, Transform T);

#endif /* POLYINT_H */

