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

/* do the actual geometric computation parts of the "cmodel" program */

#include <math.h>
#include "cmodelP.h"

void projective_to_conformal(int curv, HPoint3 *proj, Transform T, Point3 *conf)
{
   double norm, scale;
   HPoint3 pt;

   HPt3Transform(T, proj, &pt);
   norm = pt.x*pt.x + pt.y*pt.y + pt.z*pt.z;
   if (curv)
   {
       norm = curv*norm + pt.w*pt.w;
       norm = (norm < 0)? 0 : sqrt(norm);
       scale = pt.w-curv*norm;
   }
   else
       scale = -norm/pt.w;
   Pt3Mul(1 / scale, HPoint3Point3(&pt), conf);
   return;
   }

void TgtTransform(Transform T, HPoint3 *p, Point3 *v, HPoint3 *tp, Point3 *tv)
{
   HPoint3 hv, thv;
   
   Pt3Copy(v, HPoint3Point3(&hv));
   hv.w = 0;
   
   HPt3Transform(T, p, tp);
   HPt3Transform(T, &hv, &thv);
   
   Pt3Comb(1/tp->w, HPoint3Point3(&thv),
	   -thv.w/tp->w/tp->w, (Point3 *)(void *)tp, tv);
   
   return;
   }
   

/* map a tangent vector from Projective model to Conformal model */
void projective_vector_to_conformal(int curv, HPoint3 *pt,  Point3 *v,
				    Transform T, Point3 *ppt, Point3 *pv)
{
   HPoint3 tp;
   Point3 tv;
   double norm,scale;

   /* transform point */
   TgtTransform(T, pt, v, &tp, &tv);

   norm = tp.x*tp.x + tp.y*tp.y + tp.z*tp.z;
   if (curv)
   {
       norm = curv*norm + tp.w*tp.w;
       norm = (norm < 0)? 0 : sqrt(norm);
       scale = tp.w-curv*norm;
   }
   else
       scale = -norm/tp.w;

   Pt3Mul(1 / scale, HPoint3Point3(&tp), ppt);
   if (curv)
       Pt3Comb(norm/scale, &tv, Pt3Dot(ppt, &tv), ppt, pv);
   else
       Pt3Comb(tp.w/scale, &tv, 2*Pt3Dot(ppt, &tv), ppt, pv);

   Pt3Unit(pv);
   return; 
   }

/* given three vertices of a triangle in the conformal model this 
   computes the center of the sphere on which the triangle lies. */

void triangle_polar_point(int curv,
			  const Point3 *a, const Point3 *b, const Point3 *c, 
			  HPoint3 *p)
{
   Point3 ab, bc, ca;

   Pt3Cross(a, b, &ab);
   Pt3Cross(b, c, &bc);
   Pt3Cross(c, a, &ca);
   p->w = 2*Pt3Dot(a, &bc);
   
   Pt3Mul(Pt3Dot(a, a) - curv, &bc, &bc);
   Pt3Mul(Pt3Dot(b, b) - curv, &ca, &ca);
   Pt3Mul(Pt3Dot(c, c) - curv, &ab, &ab);

   Pt3Add(&ab, &bc, (Point3 *)p);
   Pt3Add(&ca, (Point3 *)p, (Point3 *)p);
 /*fprintf(stderr,"In triangle_polar got %f %f %f %f\n",p->x,p->y,p->z,p->w);*/

   return;
   }

/* given two points on a geodesic in the conformal model this 
   computes the centre of the euclidean circle formed by the geodesic */

void edge_polar_point(int curv, const Point3 *a, const Point3 *b, HPoint3 *p)
{
   double aa, ab, bb, ca, cb;
   
   aa = Pt3Dot(a, a);
   ab = Pt3Dot(a, b);
   bb = Pt3Dot(b, b);

   ca = (aa-ab)*bb + curv*(ab - bb);
   cb = (bb-ab)*aa + curv*(ab - aa);
   
   Pt3Comb(ca, a, cb, b, (Point3 *)p);
   p->w = 2 * (aa * bb - ab * ab);

   return;
   }

struct vertex *edge_split(struct edge *e, double cosmaxbend)
{
   double cosbend, w;
   Point3 m, mp, x, y, *a, *b, p;
   double aa,ab,bb,ma,mb;

   a = (Point3 *)(void *)&e->v1->V.pt;
   b = (Point3 *)(void *)&e->v2->V.pt;
   w = e->polar.w;

 /*fprintf(stderr,"In edge_split\n");*/
   if (w < .001) return NULL;
   
   Pt3Mul(1./w, (Point3 *)(void *)&e->polar, &p);
   
   Pt3Sub(a, &p, &x);
   Pt3Sub(b, &p, &y);

   cosbend = Pt3Dot(&x,&y)/sqrt(Pt3Dot(&x,&x) * Pt3Dot(&y,&y));
   if (cosmaxbend < cosbend)
      return NULL;

 /*fprintf(stderr,"end pts %f %f %f, %f %f %f, polar %f %f %f %f\n",
		     a->x,a->y,a->z, b->x,b->y,b->z,
		     e->polar.x,e->polar.y,e->polar.z,e->polar.w);*/
   Pt3Add(&x, &y, &m);
   Pt3Mul(sqrt(Pt3Dot(&x, &x) / Pt3Dot(&m, &m)), 
	  &m, &m);
   Pt3Add(&p, &m, &mp);
   aa = Pt3Dot(a,a); ab = Pt3Dot(a,b); bb = Pt3Dot(b,b);
   ma = Pt3Dot(a,&mp); mb = Pt3Dot(b,&mp);
   if (aa*mb < ab*ma  ||  bb*ma < ab*mb)
      Pt3Sub(&p,&m,&mp);

   /* to check we're doing the right thing we could do:
   ma = Pt3Dot(a,&mp); mb = Pt3Dot(b,&mp);
   if (aa*mb < ab*ma  ||  bb*ma < ab*mb)
      fprintf(stderr,"Can't find right subdivision\n");
   */

   return new_vertex(&mp, e->v1, e->v2);
   }
