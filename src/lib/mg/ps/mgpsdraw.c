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

#include "mgP.h"
#include "mgpsP.h"
#include "polylistP.h"
#include "mgpswindows.h"


/* Author: Daeron Meyer */

void	mgps_polygon( int nv, HPoint3 *v, int nn, Point3 *n,
	      	          int nc, ColorA *c );
void	mgps_mesh(int wrap, int nu, int nv, HPoint3 *p, Point3 *n, Point3 *nq,
		  ColorA *c );
void	mgps_line( HPoint3 *p1, HPoint3 *p2 );
void	mgps_polyline( int nv, HPoint3 *verts, int nc, ColorA *colors,
			int wrap );
void	mgps_polylist(  int np, Poly *p, int nv, Vertex *v, 
			int pl_flags );
void	mgps_drawnormal(HPoint3 *p, Point3 *n);

void	mgps_closer();
void	mgps_farther();

static ColorA *C2;

/*-----------------------------------------------------------------------
 * Function:	mgps_polygon
 * Description:	draw a polygon
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void
mgps_polygon(int nv,  HPoint3 *V, 
	     int nn,  Point3 *N, 
	     int nc,  ColorA *C)
{
  int		count;
  HPoint3     *v;
  Point3      *n;
  int			flag, ninc, smooth;

/*  fprintf(stderr,"X11: draw a polygon\n"); */

  flag = _mgc->astk->ap.flag;
  if ((_mgc->astk->mat.override & MTF_DIFFUSE) && 
      !(_mgc->astk->flags & MGASTK_SHADER)) nc = 0;
  ninc = (nn > 1);
  smooth = IS_SMOOTH(_mgc->astk->ap.shading);
/*
  fprintf(stderr,"cinc = %d, nc = %d, nn = %d\n", cinc, nc, nn);
*/
  if (nc == 0)
    C = (ColorA*)&_mgc->astk->ap.mat->diffuse;

  if ((flag & APF_FACEDRAW) && (flag & APF_EDGEDRAW)) {
    /* put polygon in display list */

    if (smooth && (nc > 0))
      mgps_add(MGX_BGNSEPOLY, 0, NULL, NULL);
    else
      mgps_add(MGX_BGNEPOLY, 0, NULL, NULL);

    mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->edgecolor);
							    /* edge color */
    mgps_add(MGX_COLOR, 0, NULL, C);			    /* face color */

    if (smooth)
      mgps_add(MGX_CVERTEX, nv, V, C);
    else
      mgps_add(MGX_VERTEX, nv, V, NULL);

    mgps_add(MGX_END, 0, NULL, NULL);
  }
  else
    if (flag & APF_FACEDRAW) {
      /* put polygon in display list */
      if (smooth)
        mgps_add(MGX_BGNSPOLY, 0, NULL, NULL);
      else
        mgps_add(MGX_BGNPOLY, 0, NULL, NULL);

      mgps_add(MGX_COLOR, 0, NULL, C);

      if (smooth && (nc > 0))
        mgps_add(MGX_CVERTEX, nv, V, C);
      else
        mgps_add(MGX_VERTEX, nv, V, NULL);

      mgps_add(MGX_END, 0, NULL, NULL);
    }
    else
      if (flag & APF_EDGEDRAW) {
	mgps_add(MGX_BGNLINE, 0, NULL, NULL);
	mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->edgecolor);
	mgps_add(MGX_VERTEX, nv, V, NULL);
        mgps_add(MGX_VERTEX, 1, V, NULL);
	mgps_add(MGX_END, 0, NULL, NULL);
      }

  if (flag & APF_NORMALDRAW)
  {
    mgps_closer();
    mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->normalcolor);
    for (n = N, v = V, count = 0; count<nv; ++count, ++v, n += ninc)
      mgps_drawnormal(v, n);
    mgps_farther();
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgps_line
 * Description:	draw a line (one color)
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void mgps_line( HPoint3 *p1, HPoint3 *p2 )
{

/*  fprintf(stderr,"X11: draw a line\n"); */

  /* put a line in the display list */

  mgps_add(MGX_BGNLINE, 0, NULL, NULL);
  mgps_add(MGX_VERTEX, 1, p1, NULL);
  mgps_add(MGX_VERTEX, 1, p2, NULL);
  mgps_add(MGX_END, 0, NULL, NULL);

}

/* Construct a prototype polygonal outline for creating fat points.
 * Curiously, we can do this independently of the position of the point,
 * if we operate in homogeneous space.
 */

/*-----------------------------------------------------------------------
 * Function:	mgps_fatpoint
 * Description:	draw a point (possible width > 1)
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void mgps_fatpoint(HPoint3 *v)
{
  HPoint3 a;
  HPoint3 *p, *q;
  float vw;

/*  fprintf(stderr,"X11: make a fat-point\n"); */

  if (!(_mgc->has & HAS_POINT))
    mg_makepoint();

  /* Compute w component of point after projection to screen */
  vw = v->x * _mgc->O2S[0][3] + v->y * _mgc->O2S[1][3]
      + v->z * _mgc->O2S[2][3] + v->w * _mgc->O2S[3][3];
  if (vw <= 0) return;

#define  PUT(p)                                         \
        a.x = v->x + p->x*vw; a.y = v->y + p->y*vw;     \
        a.z = v->z + p->z*vw; a.w = v->w + p->w*vw;     \
        mgps_add(MGX_VERTEX, 1, &a, NULL);

  p = VVEC(_mgc->point, HPoint3);
  q = p + VVCOUNT(_mgc->point);

  mgps_add(MGX_BGNPOLY, 0, NULL, NULL);
  do
  {
    PUT(p);
  } while (++p < q);
  mgps_add(MGX_END, 0, NULL, NULL);

}
  

/*-----------------------------------------------------------------------
 * Function:	mgps_polyline
 * Description:	draw a polyline (possibly more than 2 vertices)
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */

void mgps_polyline( int nv, HPoint3 *v, int nc, ColorA *c, int wrapped )
{
  int remain;
/*  fprintf(stderr,"X11: draw a polyline\n"); */

  if (!(wrapped & 2)) {
    if (_mgpsc->znudge) mgps_closer();
  }
  if (nv == 1)
  {
    if (nc > 0)
      mgps_add(MGX_ECOLOR, 0, NULL, c);

    if (_mgc->astk->ap.linewidth > 1)
    {
      mgps_add(MGX_COLOR, 0, NULL, c);
      mgps_fatpoint(v);
    }
    else
    {
      mgps_add(MGX_BGNSLINE, 0, NULL, NULL);
      mgps_add(MGX_CVERTEX, 1, v, c);
      mgps_add(MGX_END, 0, NULL, NULL);
    }
  }
  else
    if (nv > 0)
    {
        mgps_add(MGX_BGNSLINE, 0, NULL, NULL);

      if (wrapped & 1)
      {
	if (nc > 0)
        {
	  mgps_add(MGX_ECOLOR, 0, NULL, (c + nc - 1));
	  mgps_add(MGX_CVERTEX, 1, (v + nv - 1), (c + nc - 1));
        }
        else
	  mgps_add(MGX_CVERTEX, 1, (v + nv - 1), c);
      }

      for (;;)
      {
	remain = nv > 254 ? 254 : nv;
	nv -= remain;
	do
	{
	  if (--nc > 0)
          {
	    mgps_add(MGX_ECOLOR, 0, NULL, c);
	    mgps_add(MGX_CVERTEX, 1, v++, c++);
          }
          else
	    mgps_add(MGX_CVERTEX, 1, v++, c);

	} while (--remain > 0);
	if (nv == 0)
	  break;
	if (nc > 0)
	  mgps_add(MGX_ECOLOR, 0, NULL, c);
	mgps_add(MGX_CVERTEX, 1, v, c);
	mgps_add(MGX_END, 0, NULL, NULL);
	mgps_add(MGX_BGNSLINE, 0, NULL, NULL);
      }
      mgps_add(MGX_END, 0, NULL, NULL);
    }
  if (!(wrapped & 4) && _mgpsc->znudge)
    mgps_farther();
}


/*-----------------------------------------------------------------------
 * Function:	mgps_polylist
 * Description:	draws a Polylist: collection of Polys
 * Author:	Daeron Meyer
 * Notes:	see mg.doc
 */
void mgps_polylist( int np, Poly *_p, int nv, Vertex *V, int pl_flags )
{
  int		i, j;
  Poly		*p;
  Vertex 	**v, *vp, **vh;
  struct mgastk *ma = _mgc->astk;
  int plflags = pl_flags;
  int flag, shading;
  int nonsurf = -1;

  flag = ma->ap.flag;
  shading = ma->ap.shading;

/*  fprintf(stderr,"X11: draw a polylist %d\n",np); */

  switch(shading) {
  case APF_FLAT:
    plflags &= ~PL_HASVN;
    if (plflags & PL_HASPCOL) {
      plflags &= ~PL_HASVCOL;
    }
    break;
  case APF_SMOOTH: plflags &= ~PL_HASPN; break;
  case APF_VCFLAT: plflags &= ~PL_HASVN; break;
  default: plflags &= ~(PL_HASVN|PL_HASPN); break;
  }

  if ((_mgc->astk->mat.override & MTF_DIFFUSE) &&
      !(_mgc->astk->flags & MGASTK_SHADER))
    plflags &= ~(PL_HASVCOL | PL_HASPCOL);

  if (flag & APF_FACEDRAW)
  {
    for (p = _p, i = 0; i < np; i++, p++)
    {

      v = p->v;
      if ((j = p->n_vertices) <= 2)
	nonsurf = i;
      else
      {
        if (flag & APF_EDGEDRAW)
	{
	  if (shading == APF_FLAT || shading == APF_CONSTANT) {
	    mgps_add(MGX_BGNEPOLY, 0, NULL, NULL);
	  } else
          if (plflags & PL_HASVCOL)
	    mgps_add(MGX_BGNSEPOLY, 0, NULL, NULL);
          else
	    mgps_add(MGX_BGNEPOLY, 0, NULL, NULL);
          mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->edgecolor);
	}
	else
          {
	    if (shading == APF_FLAT || shading == APF_CONSTANT) {
	      mgps_add(MGX_BGNPOLY, 0, NULL, NULL);
	    } else
            if (plflags & PL_HASVCOL)
	      mgps_add(MGX_BGNSPOLY, 0, NULL, NULL);
	    else
	      mgps_add(MGX_BGNPOLY, 0, NULL, NULL);
	  }

	if (plflags & PL_HASPCOL)
	  mgps_add(MGX_COLOR, 0, NULL, &p->pcol);
	else
	  if (plflags & PL_HASVCOL)	        /* if we have per vertex */
	    mgps_add(MGX_COLOR, 0, NULL, &(*v)->vcol);
	  else
	    mgps_add(MGX_COLOR, 0, NULL, &(ma->ap.mat->diffuse));
	vh = v;
	do
	{
	  if (plflags & PL_HASVCOL) mgps_add(MGX_CVERTEX, 1, &(*v)->pt, &(*v)->vcol);
	    else
	      mgps_add(MGX_CVERTEX, 1, &(*v)->pt, &(*vh)->vcol);

	  v++;
	} while (--j > 0);

        mgps_add(MGX_END, 0, NULL, NULL);
      }
    }
  }
  if (flag & (APF_EDGEDRAW | APF_NORMALDRAW) || nonsurf > 0)
  {
    if (_mgpsc->znudge) mgps_closer();

    if (flag & APF_EDGEDRAW && !(flag & APF_FACEDRAW))
    {
      mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->edgecolor);
      for (p = _p, i = 0; i < np; i++, p++)
      {
	mgps_add(MGX_BGNLINE, 0, NULL, NULL);
	for (j = 0, v = p->v; j < p->n_vertices; j++, v++)
	  mgps_add(MGX_VERTEX, 1, &(*v)->pt, NULL);
	mgps_add(MGX_VERTEX, 1, &(*(p->v))->pt, NULL);
	mgps_add(MGX_END, 0, NULL, NULL);
      }

    }

    if (flag & APF_NORMALDRAW)
    {
      mgps_add(MGX_ECOLOR, 0, NULL, &_mgc->astk->ap.mat->normalcolor);
      if (pl_flags & PL_HASPN)
      {
	for (p = _p, i = 0; i < np; i++, p++)
	{
	  for (j = 0, v = p->v; j < p->n_vertices; j++, v++)
	    mgps_drawnormal(&(*v)->pt, &p->pn);
	}
      }
      else
      if (pl_flags & PL_HASVN)
      {
	for (vp = V, i = 0; i < nv; i++, vp++)
	  mgps_drawnormal(&vp->pt, &vp->vn);
      }
    }

    for (p = _p, i = 0; i <= nonsurf; p++, i++)
    {
      v = p->v;
      switch (j = p->n_vertices)
      {
	case 1:
	  mgps_add(MGX_BGNLINE, 0, NULL, NULL);
	  if (pl_flags & PL_HASVCOL)
	    mgps_add(MGX_ECOLOR, 0, NULL, &(*v)->vcol);
	  mgps_add(MGX_VERTEX, 1, &(*v)->pt, NULL);
	  mgps_add(MGX_END, 0, NULL, NULL);
	  break;
	case 2:
	  mgps_add(MGX_BGNLINE, 0, NULL, NULL);
	  do
	  {
	    if (pl_flags & PL_HASVCOL)
	      mgps_add(MGX_ECOLOR, 0, NULL, &(*v)->vcol);
	    mgps_add(MGX_VERTEX, 1, &(*v)->pt, NULL);
	    v++;
	  } while (--j > 0);
	  mgps_add(MGX_END, 0, NULL, NULL);
	  break;
      }
    }
    if (_mgpsc->znudge) mgps_farther();
  }
}

#define HAS_N   1
#define HAS_C   2
#define HAS_SMOOTH 4

/*-----------------------------------------------------------------------
 * Function:	mgpspolymeshrow
 * Description:	draw one row of a mesh
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void mgpspolymeshrow(int wrap, int has, int off, int count, HPoint3 *P,
			Point3 *N, ColorA *C, int flag, float *CE, int first)
{
  int k;
  int edges = flag & APF_EDGEDRAW;
  int faces = flag & APF_FACEDRAW;

  if (wrap & MM_UWRAP)
  {
    k = count - 1;
    if (edges && faces)
    {
      if (has & HAS_SMOOTH)
        mgps_add(MGX_BGNSEPOLY, 0, NULL, NULL);
      else
        mgps_add(MGX_BGNEPOLY, 0, NULL, NULL);

      mgps_add(MGX_ECOLOR, 0, NULL, CE);
    }
    else
    if (faces)
    {
      if (has & HAS_SMOOTH)
        mgps_add(MGX_BGNSPOLY, 0, NULL, NULL);
      else
        mgps_add(MGX_BGNPOLY, 0, NULL, NULL);
    }
    else
    {
       mgps_add(MGX_BGNLINE, 0, NULL, NULL);
       mgps_add(MGX_ECOLOR, 0, NULL, CE);
    }
    if (C)
    {
      mgps_add(MGX_COLOR, 0, NULL, C+off+k);
      C2 = C+off+k;
    }
    else
      mgps_add(MGX_COLOR, 0, NULL, C2);

    if (has & HAS_SMOOTH)
    {
      if (C)
      {
	mgps_add(MGX_CVERTEX, 1, P+off+k, C+off+k);
	mgps_add(MGX_CVERTEX, 1, P+k, C+k);
	mgps_add(MGX_CVERTEX, 1, P, C);
	mgps_add(MGX_CVERTEX, 1, P+off, C+off);
        C2 = C+off;
      }
      else
      {
	mgps_add(MGX_CVERTEX, 1, P+off+k, C2);
	mgps_add(MGX_CVERTEX, 1, P+k, C2);
	mgps_add(MGX_CVERTEX, 1, P, C2);
	mgps_add(MGX_CVERTEX, 1, P+off, C2);
      }
    }
    else
    {
      mgps_add(MGX_VERTEX, 1, P+off+k, NULL);
      mgps_add(MGX_VERTEX, 1, P+k, NULL);
      mgps_add(MGX_VERTEX, 1, P, NULL);
      mgps_add(MGX_VERTEX, 1, P+off, NULL);
    }
    mgps_add(MGX_END, 0, NULL, NULL);
  }
  k = count;
  do
  {
    if (edges && faces)
    {
      if (has & HAS_SMOOTH)
        mgps_add(MGX_BGNSEPOLY, 0, NULL, NULL);
      else
        mgps_add(MGX_BGNEPOLY, 0, NULL, NULL);

      mgps_add(MGX_ECOLOR, 0, NULL, CE);
    }
    else
    if (faces)
    {
      if (has & HAS_SMOOTH)
        mgps_add(MGX_BGNSPOLY, 0, NULL, NULL);
      else
        mgps_add(MGX_BGNPOLY, 0, NULL, NULL);
    }
    else
    {
      mgps_add(MGX_BGNLINE, 0, NULL, NULL);
      mgps_add(MGX_ECOLOR, 0, NULL, CE);
/* ADDED */
      if (first)
        mgps_add(MGX_VERTEX, 1, P+1+off, NULL);
/* END */
    }

    if (C)
    {
      mgps_add(MGX_COLOR, 0, NULL, C+off);
      C2 = C+off;
    }

    if (has & HAS_SMOOTH)
    {
      if (C) { mgps_add(MGX_CVERTEX, 1, P+off, C+off); C2 = C; }
      else mgps_add(MGX_CVERTEX, 1, P+off, C2);
      if (C) { mgps_add(MGX_CVERTEX, 1, P++, C++); C2 = C; }
      else mgps_add(MGX_CVERTEX, 1, P++, C2);
      if (C) { mgps_add(MGX_CVERTEX, 1, P, C); C2 = C; }
      else mgps_add(MGX_CVERTEX, 1, P, C2);
      if (C) { mgps_add(MGX_CVERTEX, 1, P+off, C+off); C2 = C; }
      else mgps_add(MGX_CVERTEX, 1, P+off, C2);
    }
    else
    {
      mgps_add(MGX_VERTEX, 1, P+off, NULL);
      if (C) C++;
      if (N) N++;
      mgps_add(MGX_VERTEX, 1, P++, NULL);
      mgps_add(MGX_VERTEX, 1, P, NULL);
      mgps_add(MGX_VERTEX, 1, P+off, NULL);
    }

    mgps_add(MGX_END, 0, NULL, NULL);

  } while (--k > 1);
    
}

/*-----------------------------------------------------------------------
 * Function:	mgpssubmesh
 * Description:	divide up mesh and draw by rows
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void mgpssubmesh(int wrap, int nu, int nv, int umin, int umax, int vmin,
		int vmax, HPoint3 *meshP, Point3 *meshN, ColorA *meshC)
{
  int v;
  int ucnt;
  HPoint3 *P;
  Point3 *N;
  ColorA *C;
  int prev;
  int du;
  int i;
  int has;
  Appearance *ap;


  if (nu <= 0 || nv <= 0)
    return;

  ap = &_mgc->astk->ap;
  if ((_mgc->astk->mat.override & MTF_DIFFUSE) &&
      !(_mgc->astk->flags & MGASTK_SHADER))
    meshC = 0;

  has = 0;
  if (meshN && !(_mgc->astk->flags & MGASTK_SHADER))
    has = HAS_N;
  if (meshC)
    has |= HAS_C;
  if (IS_SMOOTH(ap->shading))
    has |= HAS_SMOOTH;

  if ( ap->flag & (APF_FACEDRAW | APF_EDGEDRAW) ) /* BUG? */
  {
    if (!(has & HAS_C))
      mgps_add(MGX_COLOR, 0, NULL, &ap->mat->diffuse);

    C2 = (ColorA *) &ap->mat->diffuse;

    v = vmax - vmin + 1;
    du = umin + vmin * nu;

    if (wrap & MM_VWRAP)
    {
      prev = nu * (v - 1);
    }
    else
    {
      du += nu;
      prev = -nu;
      v--;
    }

    do
    {
      P = meshP + du;
      N = meshN + du;
      C = meshC + du;
      ucnt = umax - umin + 1;
      mgpspolymeshrow(wrap, has, prev, ucnt, P,
			has & HAS_N ? N : NULL,
			has & HAS_C ? C : NULL,
			ap->flag, (float *)&ap->mat->edgecolor, (int)(v!=1));

      prev = -nu;
      du += nu;
    } while (--v > 0);
  }

  if (ap->flag & APF_NORMALDRAW && meshN != NULL)
  {
    mgps_add(MGX_ECOLOR, 0, NULL, &ap->mat->normalcolor);

    if (_mgpsc->znudge)
      mgps_closer();
    for (i = nu*nv, P = meshP, N = meshN; --i >= 0; P++, N++)
      mgps_drawnormal(P, N);
    if (_mgpsc->znudge)
      mgps_farther();
  }
}
	
/*-----------------------------------------------------------------------
 * Function:	mgps_mesh
 * Description:	draw a mesh
 * Author:	Daeron Meyer
 * Notes:	See mg.doc.
 *
 */
void mgps_mesh(int wrap, int nu, int nv, HPoint3 *P, Point3 *N, Point3 *NQ,
	       ColorA *C )
{
  mgpssubmesh( wrap, nu, nv, 0, nu-1, 0, nv-1, P, N, C);
}

/*-----------------------------------------------------------------------*
 * end of mesh drawing functions:					 *
 *-----------------------------------------------------------------------*/

/*
 * Z-shift routines: for moving edges closer than faces, etc.
 */
void
mgps_init_zrange()
{
  _mgpsc->znudge = 1;
  _mgpsc->znudgeby = 0.0;
}

void
mgps_closer()
{
/*  _mgpsc->znudgeby = 0.001; */
}

void
mgps_farther()
{
  _mgpsc->znudgeby =  0.0;
}

void
mgps_findcam()
{
}

/* There is a basic problem now with 4-d points and 3-d normal vectors.
For now, we'll just ignore the 4-th coordinate of the point when 
computing the tip of the normal vector.  This will work OK with all
existing models, but for genuine 4-d points it won't work.  But,
come to think of it, what is the correct interpretation of the
normal vector when the points live in 4-d?
*/
void
mgps_drawnormal(HPoint3 *p, Point3 *n)
{
  Point3 tp;
  HPoint3 end;
  Pt3Coord scale, w, s;

/*  fprintf(stderr,"X11: draw a normal\n"); */
  if (p->w <= 0.0) return;
  if (p->w != 1) {
    HPt3ToPt3(p, &tp);
    p = (HPoint3 *)(void *)&tp;
  }

  scale = _mgc->astk->ap.nscale;
  if (_mgc->astk->ap.flag & APF_EVERT) {
    HPoint3 *cp = &_mgc->cpos;
    if (!(_mgc->has & HAS_CPOS)) {
      mg_findcam();
    }
    if ((w = cp->w) != 1.0 && w != 0.0) {
      s = (p->x*w-cp->x)*n->x + (p->y*w-cp->y)*n->y + (p->z*w-cp->z)*n->z;
    } else {
      s = (p->x-cp->x)*n->x + (p->y-cp->y)*n->y + (p->z-cp->z)*n->z;
    }
    if (s > 0) {
      scale = -scale;
    }
  }
  end.x = p->x + scale*n->x;
  end.y = p->y + scale*n->y;
  end.z = p->z + scale*n->z;
  end.w = 1.0;
  mgps_add(MGX_BGNLINE, 0, NULL, NULL);
  mgps_add(MGX_VERTEX, 1, p, NULL);
  mgps_add(MGX_VERTEX, 1, &end, NULL);
  mgps_add(MGX_END, 0, NULL, NULL);

}
