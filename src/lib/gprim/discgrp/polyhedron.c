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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* 
 *	polyhedron.c:	routines which operate on the polyhedron data structure
 */
/*
*/

#include "geom.h"
#include "geomclass.h"
#include "polylistP.h"
#include "ooglutil.h"
#include "point.h"
#include "winged_edge.h"
#include "extern.h"

Geom *
WEPolyhedronToVect(WEpolyhedron *poly, HPoint3 origin)
{
    int i, ii, jj;
    Geom *orbit;
    HPoint3 gorigin;
    /*static HPoint3 origin2 = {0,0,0,1};*/
    WEface *fptr;
    Transform T;
    short *vnvert;
    short *vncolor;
    HPoint3 *p;
    ColorA *c;

    vnvert = OOGLNewN(short, poly->num_faces);
    vncolor = OOGLNewN(short, poly->num_faces);
    p = OOGLNewN(HPoint3, 2*poly->num_faces);
    c = OOGLNewN(ColorA, poly->num_faces);
	
    for (i=0, fptr=poly->face_list; i<poly->num_faces; 
	 ++i, fptr = fptr->next)
	{
	vnvert[i] = 2;	/* line segment! */
	vncolor[i] = 1;
	/* next two lines assume the indices in two sorts of groups match */
	c[i] =  GetCmapEntry( fptr->fill_tone);
	for (ii=0; ii<4; ++ii) for (jj=0; jj<4; ++jj)	
	    /* fptr's tforms are transposed! */
	    T[jj][ii] = fptr->group_element[ii][jj];
	HPt3Transform( T, &origin, &gorigin);
	p[2*i] = origin;
	p[2*i+1] = gorigin;
	}

    orbit = GeomCreate("vect", 
				CR_NOCOPY,
				CR_NVECT, poly->num_faces,
				CR_NVERT, 2*poly->num_faces,
				CR_NCOLR, poly->num_faces,
				CR_VECTC, vnvert,
				CR_COLRC, vncolor,
				CR_POINT4, p,
				CR_COLOR, c, 
				CR_4D, 1,
				CR_END);
    return(orbit);
}
/*static ColorA white = {1,1,1,1};*/

Geom *
  WEPolyhedronToPolyList(poly)
WEpolyhedron *poly;
{
  ColorA *colors = NULL;
  HPoint3 *points = NULL;
  int *nvert = NULL;
  int *vindex = NULL;
  
  WEvertex	*vptr;
  WEedge	*eptr;
  WEface	*fptr;
  int		cnt,  cnt2, total;
  
  Geom *plist;
  
  points = OOGLNewN(HPoint3, poly->num_vertices);
  colors = OOGLNewN(ColorA,poly->num_faces);
  nvert = OOGLNewN(int, poly->num_faces);

  vptr = poly->vertex_list;
  cnt = 0;
  do {
    points[cnt].x = vptr->x[0];
    points[cnt].y = vptr->x[1];
    points[cnt].z = vptr->x[2];
    points[cnt].w = vptr->x[3];
    vptr->ideal = cnt++;
    vptr = vptr->next;
  } while (vptr != NULL);
  
  cnt = 0;
  fptr = poly->face_list;
  total = 0;
  do {
    colors[cnt] = GetCmapEntry(fptr->fill_tone);
    nvert[cnt] = fptr->order;
    total += nvert[cnt++];
    fptr = fptr->next;
  } while (fptr != NULL);
  vindex = OOGLNewN(int, total);
  cnt = 0;
  fptr = poly->face_list;
  do {
    eptr = fptr->some_edge;
    cnt2 = 0;
    do {
      if (eptr->fL == fptr) {
	vindex[cnt+cnt2] = eptr->v0->ideal;
	cnt2++;
	eptr = eptr->e1L;
      } else {
	vindex[cnt+cnt2] = eptr->v1->ideal;
	cnt2++;
	eptr = eptr->e0R;
      }
    } while (eptr != fptr->some_edge);
    cnt+= fptr->order;
    fptr = fptr->next;  }
  while (fptr != NULL);
  
  plist = 
    GeomCreate("polylist", 
		/*CR_NOCOPY,		isn't supported for polylists! */
		CR_4D, 1,
		CR_NPOLY, poly->num_faces, 
	       	CR_NVERT, nvert, 
		CR_VERT, vindex, 
		CR_POINT4, points, 
		CR_POLYCOLOR, colors, 
		CR_FLAG, PL_HASPCOL, 	/* put this here or meet a bug libpolylist */
		CR_END);
  return(plist);
}

Geom *
WEPolyhedronToBeams( WEpolyhedron *poly, float alpha)
{
    WEedge *eptr;
    WEvertex *vptr;
    Geom *beams;
    HPoint3 *points, p0, p1, v0, v1;
    ColorA *colors;
    int *nvert, *vindex, vcnt, fcnt;
    float omega; 
    static ColorA white = {1,1,1,1};

    points = OOGLNewN(HPoint3, 4 * poly->num_edges);
    colors = OOGLNewN(ColorA,  poly->num_edges);
    nvert = OOGLNewN(int, poly->num_edges);
    vindex = OOGLNewN(int, 4*poly->num_edges);

    omega = 1.0 - alpha;

    vcnt = fcnt = 0;
    eptr = poly->edge_list;
#define CCOPY( d4, hpt3)	\
	(hpt3)->x = d4[0];	\
	(hpt3)->y = d4[1];	\
	(hpt3)->z = d4[2];	\
	(hpt3)->w = d4[3];
    do {
	CCOPY( eptr->v0->x, &v0);
      	if ((eptr->e0L->v0 = eptr->v0))	vptr = eptr->e0L->v1;
      	else vptr = eptr->e0L->v0;
	CCOPY( vptr->x, &v1);
	HPt3Scale(omega, &v0, &p0);
	HPt3Scale(alpha, &v1, &p1);
	HPt3Add(&p0, &p1, &points[vcnt]);
	vindex[vcnt] = vcnt;
	vcnt++;

    if (eptr->fR == eptr->e0R->fR)	
      	if ((eptr->e0R->v0 = eptr->v0))	vptr = eptr->e0R->v1;
      	else vptr = eptr->e0R->v0;
    else 
      	if ((eptr->e0L->v0 = eptr->v0))	vptr = eptr->e0L->v1;
      	else vptr = eptr->e0L->v0;
	CCOPY( vptr->x, &v1);
	HPt3Scale(alpha, &v1, &p1);
	HPt3Add(&p0, &p1, &points[vcnt]);
	vindex[vcnt] = vcnt;
	vcnt++;

	CCOPY( eptr->v1->x, &v0);
      	if ((eptr->e1R->v0 = eptr->v1))	vptr = eptr->e1R->v1;
      	else vptr = eptr->e1R->v0;
	CCOPY( vptr->x, &v1);
	HPt3Scale(omega, &v0, &p0);
	HPt3Scale(alpha, &v1, &p1);
	HPt3Add(&p0, &p1, &points[vcnt]);
	vindex[vcnt] = vcnt;
	vcnt++;

      	if ((eptr->e1L->v0 = eptr->v1))	vptr = eptr->e1L->v1;
      	else vptr = eptr->e1L->v0;
	CCOPY( vptr->x, &v1);
	HPt3Scale(alpha, &v1, &p1);
	HPt3Add(&p0, &p1, &points[vcnt]);
	vindex[vcnt] = vcnt;
	vcnt++;

    	colors[fcnt] = white;
    	nvert[fcnt] = 4;
	fcnt++;

      	eptr = eptr->next;
    }
    while (eptr != NULL);

    beams = GeomCreate("polylist",
                CR_NPOLY, poly->num_edges,
	       	CR_NVERT, nvert, 
                CR_VERT, vindex,
                CR_POINT4, points,
                CR_POLYCOLOR, colors,
                CR_FLAG, PL_HASPCOL,    /* put this here or meet a bug libpolyli
st */
                CR_END);
    return(beams);
}

