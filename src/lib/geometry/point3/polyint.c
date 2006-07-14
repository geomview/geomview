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

#include <math.h>
#include "point3.h"
#include "transform.h"
#include "ooglutil.h"
#include "polyint.h"

#define FUDGE2 1.e-12

#define PT3SUB_XY(a, b, dst) { (dst).x = (a).x - (b).x; \
			       (dst).y = (a).y - (b).y; \
			       (dst).z = (a).z - (b).z; }
#define PT3LENGTH_XY(a) sqrt((a).x*(a).x + (a).y*(a).y)
#define PT3DOT_XY(a, b) ((a).x*(b).x + (a).y*(b).y)

struct PolyHit {
  Point3 hitpt;
  int vno;
  int edgevno;
};

/*
 * Internal routines.  These should not need to be called directly.
 */
static int PolyInt_InBBox(int n_verts, Point3 *verts, float tol);
static int PolyInt_Origin(int n_verts, Point3 *verts, Point3 *origin);

int PolyZInt(int n_verts, Point3 *verts, float tol, int wanted, vvec *hits)
{
  int i;
  int count = 0;
  struct PolyHit *ph;
  Point3 pt, bma, *thisp, *nextp;
  float thisd2, nextd2, od2, tol2, len2, f;

  float angsum = 0.0;

  /* Do test for trivial rejection */
  if (!PolyInt_InBBox(n_verts, verts, tol)) return 0; 

  /* If we have just one vertex, and passed the above test, it's a hit. */
  if(n_verts == 1 && wanted&PW_VERT) {
    ph = VVAPPEND(*hits, struct PolyHit);
    ph->vno = 0;
    ph->edgevno = -1;
    ph->hitpt = verts[0];
    return 1;
  }

  tol2 = tol*tol;

  /* Go through the edges of the polygon looking for edge and vertex hits. */
  thisp = &verts[n_verts-1];
  thisd2 = PT3DOT_XY(*thisp, *thisp);
  nextp = &verts[0];
  for (i = 0; i < n_verts; i++, thisp = nextp++, thisd2 = nextd2) {

    /* Line equation: this + t*(next-this)/|next-this|, for 0<=t<=1 */
    /* Find closest point on line to origin. */
    nextd2 = PT3DOT_XY(*nextp, *nextp);
    PT3SUB_XY(*thisp, *nextp, bma);
    len2 = PT3DOT_XY(bma, bma);
    if(nextd2 < tol2 && wanted&PW_VERT) {
	count++;
	ph = VVAPPEND(*hits, struct PolyHit);
	ph->hitpt = *nextp;
	ph->vno = i;
	ph->edgevno = -1;
    } else if(len2 > 0) {
	f = -PT3DOT_XY(*thisp, bma) / len2;
	pt.x = thisp->x + f*bma.x;
	pt.y = thisp->y + f*bma.y;
	od2 = PT3DOT_XY(pt, pt);
	if(od2 < tol2 && wanted&PW_EDGE && (thisd2 > tol2 || !(wanted&PW_VERT))) {
	    count++;
	    ph = VVAPPEND(*hits, struct PolyHit);
	    pt.z = thisp->z + f*bma.z;
	    ph->hitpt = pt;
	    ph->vno = -1;
	    ph->edgevno = (i==0) ? n_verts-1 : i-1;
	}
	if(len2 > FUDGE2) {
	   /* Add in the (signed) angle subtended by this edge as seen from
	    * the origin: atan2( this cross next, this dot next )
	    */
	   angsum += atan2( thisp->x * nextp->y - thisp->y * nextp->x,
				PT3DOT_XY(*thisp, *nextp) );
	}
    }


  }

  /* Look for the face hits */
  if (wanted&PW_FACE && count == 0 && n_verts > 2 && fabs(angsum) > M_PI
	&& PolyInt_Origin(n_verts, verts, &pt)) {
    count++;
    ph = VVAPPEND(*hits, struct PolyHit);
    ph->hitpt = pt;
    ph->vno = -1;
    ph->edgevno = -1;
  }

  return(count);

}


int PolyNearPosZInt(int n_verts, Point3 *verts, float tol,
		    Point3 *ip, int *vertex, int *edge, Point3 *ep,
		    int wanted, float zclose) {
  int i;
  vvec hits;
  struct PolyHit *ph, *closest, phits[32];

  VVINIT(hits, struct PolyHit, 32);
  vvuse(&hits, phits, 32);

  if (!PolyZInt(n_verts, verts, tol, wanted, &hits))
	return 0;

  closest = NULL;


  for (i = 0, ph = VVEC(hits, struct PolyHit); i < VVCOUNT(hits); i++, ph++) {
    if (ph->hitpt.z > -1.0 && ph->hitpt.z < zclose) {
      closest = ph;
      zclose = ph->hitpt.z;
    }
  }

  if (closest) {
    *ip = closest->hitpt;
    *vertex = closest->vno;
    *edge = closest->edgevno;
    *ep = closest->hitpt;
  }

  vvfree(&hits);
  return (closest ? 1 : 0);
}


#ifdef UNUSED

int PolyLineInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *verts, 
		float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep) 
{
  int i;
  int n_hits, old_n_hits;
  Point3 *vertcopy;
  Transform T, Tinv;

  PolyInt_Align(pt1, pt2, T);
  TmInvert(T, Tinv);

  vertcopy = OOGLNewNE(Point3, n_verts, msg);
  for (i = 0; i < n_verts; i++) Pt3Transform(T, &verts[i], &vertcopy[i]);

  old_n_hits = VVCOUNT(*ip);
  n_hits = PolyZInt(n_verts, vertcopy, tol, ip, vertices, edges, ep, PW_EDGE|PW_VERT);
  for (i = 0; i < n_hits; i++) {
    Pt3Transform(Tinv, VVINDEX(*ip, Point3, old_n_hits + i), 
		 VVINDEX(*ip, Point3, old_n_hits + i));
    Pt3Transform(Tinv, VVINDEX(*ep, Point3, old_n_hits + i),
		 VVINDEX(*ep, Point3, old_n_hits + i));
  }

  OOGLFree(vertcopy);

  return n_hits;

}


int PolyRayInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *verts, 
	       float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep)
{
  int i, j;
  int n_hits, old_n_hits;
  Point3 *vertcopy;
  Transform T, Tinv;

  PolyInt_Align(pt1, pt2, T);
  TmInvert(T, Tinv);

  vertcopy = OOGLNewNE(Point3, n_verts, msg);
  for (i = 0; i < n_verts; i++) Pt3Transform(T, &verts[i], &vertcopy[i]);

  old_n_hits = VVCOUNT(*ip);
  n_hits = PolyZInt(n_verts, vertcopy, tol, ip, vertices, edges, ep, PW_EDGE|PW_VERT);
  for (i = j = 0; i < n_hits; i++)
    if (VVINDEX(*ip, Point3, old_n_hits + i)->z <= 0) {
      Pt3Transform(Tinv, VVINDEX(*ip, Point3, old_n_hits + i),
		   VVINDEX(*ip, Point3, old_n_hits + j));
      Pt3Transform(Tinv, VVINDEX(*ep, Point3, old_n_hits + i),
		   VVINDEX(*ep, Point3, old_n_hits + j));
      j++;
    }
 
  OOGLFree(vertcopy);

  return j;

}


int PolySegmentInt(Point3 *pt1, Point3 *pt2, int n_verts, Point3 *verts,
		   float tol, vvec *ip, vvec *vertices, vvec *edges, vvec *ep)
{
  int i, j;
  int n_hits, old_n_hits;
  Point3 *vertcopy;
  Transform T, Tinv;

  PolyInt_Align(pt1, pt2, T);
  TmInvert(T, Tinv);

  vertcopy = OOGLNewNE(Point3, n_verts, msg);
  for (i = 0; i < n_verts; i++) Pt3Transform(T, &verts[i], &vertcopy[i]);

  old_n_hits = VVCOUNT(*ip);
  n_hits = PolyZInt(n_verts, vertcopy, tol, ip, vertices, edges, ep, PW_EDGE|PW_VERT);
  for (i = j = 0; i < n_hits; i++) 
    if ((VVINDEX(*ip, Point3, old_n_hits + i)->z <= 0) &&
	(VVINDEX(*ip, Point3, old_n_hits + i)->z >= -1.0)) {
      Pt3Transform(Tinv, VVINDEX(*ip, Point3, old_n_hits + i),
		   VVINDEX(*ip, Point3, old_n_hits + j));
      Pt3Transform(Tinv, VVINDEX(*ep, Point3, old_n_hits + i),
		   VVINDEX(*ep, Point3, old_n_hits + j));
      j++;
    }

  OOGLFree(vertcopy);

  VVCOUNT(*ip) = old_n_hits + j;
  vvtrim(ip);

  return j;

}

#endif /*UNUSED*/


void PolyInt_Align(Point3 *pt1, Point3 *pt2, Transform T) {
  Point3 newpt2;
  Transform Ttmp;

  if (!memcmp(pt1, pt2, sizeof(Point3))) {
    OOGLError(1, "PolyInt_Align called with identical points.");
    TmIdentity(T);
    return;
  }

  TmTranslate(T, -pt1->x, -pt1->y, -pt1->z);
  Pt3Transform(T, pt2, &newpt2);

  TmRotateY(Ttmp, -atan2(newpt2.x, -newpt2.z));
  TmConcat(T, Ttmp, T);
  Pt3Transform(T, pt2, &newpt2);

  TmRotateX(Ttmp, -atan2(newpt2.y, -newpt2.z));
  TmConcat(T, Ttmp, T);
  Pt3Transform(T, pt2, &newpt2);

  if (newpt2.z == 0.0) {
    OOGLError(1, "Second point too close to first point in PolyInt_Align");
    TmIdentity(T);
    return;
  }
  TmScale(Ttmp, -1.0 / newpt2.z, -1.0 / newpt2.z, -1.0 / newpt2.z);
  TmConcat(T, Ttmp, T);

}

/*
 * PolyInt_InBBox
 * Returns non-zero if the origin is inside the polygon described by
 * verts or within tol of begin so; returns 0 otherwise.
 */
static int PolyInt_InBBox(int n_verts, Point3 *verts, float tol) {
  int i;
  int posx = 0, negx = 0, posy = 0, negy = 0;

  for (i = 0; i < n_verts; i++, verts++) {
    if (verts->x < tol) negx |= 1;
    if (verts->x > -tol) posx |= 1;
    if (verts->y < tol) negy |= 1;
    if (verts->y > -tol) posy |= 1;
  }

  return (posx & negx & posy & negy);

}


static int PolyInt_Origin(int n_verts, Point3 *verts, Point3 *origin) {
  int bi, ci;
  float pz = 0, pw;

  /* Find the first vertex different from the 0th vertex */
  for (bi = 0; bi < n_verts && !memcmp(&verts[0], &verts[bi], sizeof(Point3));
       bi++);
  if (bi >= n_verts) {
    Pt3Copy(&verts[0], origin);
    return 0;
  }

  /* Find the first vertex not collinear with the other two vertices */
  for (ci = bi+1; ci < n_verts; ci++) {
    pz = (verts[0].x * (verts[bi].y - verts[ci].y) 
	  - verts[0].y * (verts[bi].x - verts[ci].x) 
	  + (verts[bi].x * verts[ci].y - verts[bi].y * verts[ci].x));
    if (pz*pz > FUDGE2) break;
  }
  if (ci >= n_verts) {
    Pt3Copy(&verts[0], origin);
    return 0;
  }

  pw = (verts[0].x * (verts[bi].z*verts[ci].y - verts[bi].y*verts[ci].z)
	- verts[0].y * (verts[bi].z*verts[ci].x - verts[bi].x*verts[ci].z)
	+ verts[0].z * (verts[bi].y*verts[ci].x - verts[bi].x*verts[ci].y));

  origin->x = origin->y = 0.0;
  origin->z = -pw / pz;

  return 1;
}





