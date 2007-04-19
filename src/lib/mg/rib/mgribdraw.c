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
#include "mgribP.h"
#include "mgribtoken.h"
#include "polylistP.h"

#define MAXCLINE 999999
void mgrib_drawPline(HPoint3 *p1, HPoint3 *p2);
void mgrib_drawCline(HPoint3 *p1, HPoint3 *p2);

void	mgrib_polygon( int nv, HPoint3 *v, int nn, Point3 *n,
		       int nc,ColorA *c );
void	mgrib_mesh(int wrap,int nu,int nv,HPoint3 *p, Point3 *n, Point3 *nq,
		   ColorA *c );
void	mgrib_line( HPoint3 *p1, HPoint3 *p2 );
void	mgrib_polyline( int nv, HPoint3 *verts, int nc, ColorA *colors,
			int wrapped );
void	mgrib_polylist(  int np, Poly *p, int nv, Vertex *v, 
			 int plflags );
void	mgrib_drawnormal(HPoint3 *p, Point3 *n);
				 


/*-----------------------------------------------------------------------
 * Function:	mgrib_polygon
 * Description:	draw a polygon
 * Author:	wisdom
 * Date:	Thu Aug 26 13:46:25 CDT 1993
 * Notes:	See mg.doc.
 *
 */
void
mgrib_polygon(int nv,  HPoint3 *V, 
	      int nn,  Point3 *N, 
	      int nc,  ColorA *C)
{
  int i;	
  HPoint3 *v;
  Point3 *n;
  Color *c;
  ColorA *c4;
  HPoint3 hpt;
  int flag;
  int shading;
  int matover;
  int ninc;

  flag = _mgc->astk->ap.flag;
  shading = _mgc->astk->ap.shading;
  matover = _mgc->astk->mat.override;
  ninc = (nn > 1);
  if ((matover & MTF_DIFFUSE) && !(_mgc->astk->flags & MGASTK_SHADER)) nc = 0;
    
  mrti(mr_polygon,mr_NULL);
    
  /* Points */
  if (nv>0 && (flag & APF_FACEDRAW)) {
    mrti(mr_P,mr_buildarray, nv*3, mr_NULL);
    for (i = 0; i < nv; i++) {
      /* we cannot descibe a polygon using Pw, so we normalize */
      HPt3Dehomogenize(&V[i], &hpt);
      mrti(mr_subarray3, &hpt, mr_NULL);
    }
  }

  /* Supply Color (explicit) */
  if (nc>0 && (flag & APF_FACEDRAW)) {
    /* note:color should already be set in case of APF_CONSTANT,no?  */
    mrti(mr_Cs, mr_buildarray, nv*3, mr_NULL);
    for (i = 0; i < nv; i++) {
      if (nc>1) c4 = &C[i]; else c4 = C;
      mrti(mr_subarray3, (float *)c4, mr_NULL);
    }
  }

  /* Supply Transparency */
  if (nc>0 && flag & APF_TRANSP && !(matover & MTF_ALPHA)) {
    for (i = 0; i < nv; i++) {
      float opacity[3];
      if (nc>1) c4 = &C[i]; else c4 = C;
      opacity[0]=opacity[1]=opacity[2]=c4->a;
      mrti(mr_subarray3, opacity, mr_NULL);
    }
  }
  /* Supply Normals */
  if (nn>0 && (flag & APF_FACEDRAW) && shading == APF_SMOOTH) {
    mrti(mr_N, mr_buildarray, nv*3, mr_NULL); 
    for (i = 0; i < nv; i++) {
      if (nn>1) n = &N[i]; else n = N;
      mrti(mr_subarray3, n, mr_NULL);
    }
  }
    
  /* Draw Edges */
  if (flag & APF_EDGEDRAW) {
    c = &_mgc->astk->ap.mat->edgecolor;
    mrti(mr_attributebegin,
	 mr_surface, mr_constant, 
	 mr_color, mr_parray, 3, c,
	 mr_opacity, mr_array, 3, 1., 1., 1., mr_NULL);
	
    for (i=0;i<nv-1;i++) mgrib_drawline(&V[i],&V[i+1]);
    mgrib_drawline(&V[i],&V[0]);
    mrti(mr_attributeend, mr_NULL);
  }
    
  /* Draw Normals */
  if (flag & APF_NORMALDRAW) {
    for (n = N, v = V, i = 0; i<nv; ++i, ++v, n += ninc)
      mgrib_drawnormal(v, n);
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_line
 * Description:	draws a line
 * Author:	wisdom
 * Date:	Fri Jan 17 14:31:06 CST 1992
 * Notes:	see mg.doc
 */
void mgrib_line( HPoint3 *p1, HPoint3 *p2 )
{
  mrti(mr_attributebegin, mr_surface, mr_constant, mr_NULL);
  mgrib_drawline(p1,p2);
  mrti(mr_attributeend, mr_NULL);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_drawline, mgrib_drawPline, mgrib_drawCline, bounded
 * Description: draws a line for rman.
 * Returns:	nothing
 * Author:	wisdom, gunn, slevy, munzner
 * Date:	Tue Feb 18 14:20:29 CST 1992
 * Notes:
 */
void
mgrib_drawline(HPoint3 *p1, HPoint3 *p2)
{
  if (_mgribc->line_mode==MG_RIBPOLYGON) mgrib_drawPline(p1,p2);
  if (_mgribc->line_mode==MG_RIBCYLINDER) mgrib_drawCline(p1,p2);
  if (_mgribc->line_mode==MG_RIBPRMANLINE)
    NotImplemented("MG_RIBPRMANLINE");
}

void
mgrib_drawPline(HPoint3 *p1, HPoint3 *p2)
{
  Transform V;
  Transform P2S,O2S, O2P, S2O;
  int xsize, ysize;
  HPoint3 pnts[4];
  Point3 s1, s2;
  int i;
  float dx,dy,k, len;

  /* This code will simulate line drawing in Photorman */
  /* create obj->proj transform */
  CamView(_mgc->cam, V);		/* world->proj */
  TmConcat(_mgc->xstk->T, V, O2P);	/* obj->proj */
    
  /* create obj->screen transform */
  WnGet(_mgc->win, WN_XSIZE, &xsize);
  WnGet(_mgc->win, WN_YSIZE, &ysize);
  TmScale(P2S, (float)xsize, (float)ysize, 1.0);
  TmConcat(O2P, P2S, O2S);
    
  /* translate & dehomogenize line endpoints from object to screen */
  HPt3TransPt3(O2S, p1, &s1);
  HPt3TransPt3(O2S, p2, &s2);

  dy = s2.y - s1.y;
  dx = s2.x - s1.x;
  len = hypot(dy,dx);
  k = _mgc->astk->ap.linewidth / len;
    
  pnts[0].x = s1.x -dy * k;
  pnts[0].y = s1.y +dx * k;
  pnts[1].x = s1.x +dy * k;
  pnts[1].y = s1.y -dx * k;
  pnts[2].x = s2.x +dy * k;
  pnts[2].y = s2.y -dx * k;
  pnts[3].x = s2.x -dy * k;
  pnts[3].y = s2.y +dx * k;
	    
  pnts[0].z = s1.z;
  pnts[1].z = s1.z;
  pnts[2].z = s2.z;
  pnts[3].z = s2.z;
    
  for (i=0; i<4; ++i) pnts[i].w = 1.0;
 	    
  /* now project back... */
  /* first, find S2O transform */
  TmInvert(O2S, S2O);
    
  /* now transform screen coords to object coords */
    
  /* DRAW HERE */
  mrti(mr_polygon, mr_P, mr_buildarray, 4*3, mr_NULL);
  for (i=0;i<4;i++) {
    HPoint3 pt;

    HPt3Transform(S2O, &pnts[i], &pt);
    HPt3Dehomogenize(&pt, &pt);
    mrti(mr_subarray3, &pt, mr_NULL);
  }
}

void
mgrib_drawCline(HPoint3 *p1, HPoint3 *p2)
{
  Pt3Coord angle,length;
  Point3 t,axis;
  Point3 start, end;
  HPoint3 Hstart, Hend;
  static float unitz[3] = {0.0, 0.0, 1.0};
  float radius = 0.004;
  float size;
  int bounded(Point3 *p);
  
  HPt3Dehomogenize(p1, &Hstart);
  HPt3Dehomogenize(p2, &Hend);
    
  start.x = Hstart.x;
  start.y = Hstart.y;
  start.z = Hstart.z;
    
  end.x = Hend.x;
  end.y = Hend.y;
  end.z = Hend.z;
    
  if (! Pt3Equal(&start,&end)) {
    size = radius*_mgc->astk->ap.linewidth;
    Pt3Sub(&end,&start,&t);
    length = Pt3Length(&t);
    Pt3Cross((Point3*)(void *)unitz,&t,&axis);
    Pt3Unit(&t);
    angle = Pt3Dot((Point3*)(void *)unitz,&t);
    angle = acos(angle);
    mrti(mr_transformbegin, mr_NULL);
    if (bounded(&start))
      mrti(mr_translate,
	   mr_float, start.x,
	   mr_float, start.y,
	   mr_float, start.z, mr_NULL);
    if ( (t.x == 0.0) && (t.y == 0.0) && (t.z < 0.0)) {
      /* if along negative z axis cross product is 0
	 but rotation by 180 degrees is necessary
	 (angle is computed correctly) */
      axis.y = 1.0;
    }
    if (bounded(&axis))
      mrti(mr_rotate, mr_float, DEGREES(angle),
	   mr_float, axis.x, mr_float, axis.y, mr_float, axis.z, mr_NULL);
    if (length < MAXCLINE)
      mrti(mr_cylinder, mr_float, size, mr_float, 0.,
	   mr_float, length, mr_float, 360., mr_NULL);
    mrti(mr_transformend, mr_NULL);
  }
}

int 
bounded(p)
     Point3 *p;
{
  if (! Pt3Equal(p,&Pt3Origin)) {
    if (p->x < MAXCLINE && p->y < MAXCLINE && p->z < MAXCLINE) 
      return 1;
  }
  return 0;
}



/*-----------------------------------------------------------------------
 * Function:	mgrib_polyline
 * Description:	draws a Polyline
 * Author:	wisdom
 * Date:	Fri Jan 17 14:31:06 CST 1992
 * Notes:	see mg.doc
 */
void mgrib_polyline( int nv, HPoint3 *v, int nc, ColorA *c, int wrapped )
{
  ColorA *color;
	
  mrti(mr_attributebegin, mr_surface, mr_constant, mr_NULL);
  if (nc==0) mrti(mr_color, mr_parray, 3,
		 &_mgc->astk->mat.edgecolor, mr_NULL);
  if (nc==1) {
    mrti(mr_color, mr_parray, 3, c, mr_NULL);
    if (_mgc->astk->ap.flag & APF_TRANSP && !(_mgc->astk->mat.override & MTF_ALPHA))
      mrti(mr_opacity, mr_array, 3, c->a, c->a, c->a, mr_NULL);
  }
  if (nv == 1) {
    mgrib_drawpoint(v);
  }
  else {
    if (wrapped & 1) {
      if (nc > 1) {
	color = c + nc - 1;
	mrti(mr_color, mr_parray, 3, color, mr_NULL);
      }
      mgrib_drawline(v + nv - 1,v);
    }
    
    while(--nv > 0) {
      if (nc > 1) {
	color = c++;
	mrti(mr_color, mr_parray, 3, color, mr_NULL);
      }
      mgrib_drawline(v,(v+1));
      v++;
    }
  }
	
  mrti(mr_attributeend, mr_NULL);

}

/*-----------------------------------------------------------------------
 * Function:	mgrib_drawpoint
 * Description: draws a point for rman.
 * Returns:	nothing
 * Author:	wisdom
 * Date:	Fri Mar 13 15:04:01 CST 1992	
 * Notes:
 */

void
mgrib_drawpoint(HPoint3 *p)
{
  float radius = 0.004;
  float size = radius*_mgc->astk->ap.linewidth;
#if 0
  /* Somehow the Points directive doesn't work well, the scaling is
   * quite strange
   */
  HPoint3 tmp;

  HPt3Dehomogenize(p, &tmp);

  size *= 20.0;
  mrti(mr_points,
       mr_P, mr_parray, 3, &tmp,
       mr_width, mr_parray, 1, &size, mr_NULL);
#else
    
  /* To have a chance to get a visible point we have to rescale the
   * radius by the distance from the camera. Same holds, in principle,
   * for lines.
   */
  if (_mgribc->persp && _mgc->space == TM_EUCLIDEAN) {
    HPoint3 p_cam;
    float len;

    HPt3Transform(_mgc->xstk->T, p, &p_cam);
    HPt3Transform(_mgc->W2C, &p_cam, &p_cam);
    len = sqrt(HPt3R30Dot(&p_cam, &p_cam));
    size *= len / _mgribc->focallen;
  }

  mrti(mr_transformbegin, mr_NULL);
  mrti(mr_translate, mr_float, p->x, mr_float, p->y, mr_float, p->z, mr_NULL);
  mrti(mr_sphere, mr_float, size, mr_float, size, mr_float, -size,
       mr_float, 360., mr_NULL);
  mrti(mr_transformend, mr_NULL);
#endif
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_polylist
 * Description:	draws a Polylist: linked list of Polys
 * Author:	wisdom
 * Date:	Wed Jan 22 16:07:03 CST 1992
 * Notes:	see mg.doc
 */
void mgrib_polylist( int np, Poly *P, int nv, Vertex *V, int plflags )
{
  Appearance *ap;
  int i,j;
  Poly *p;
  Vertex **v, *vp;
  HPoint3 hpt;
  int flag,shading,matover;
  Color *color;
  
  ap = &_mgc->astk->ap;
  flag = ap->flag;
  shading = ap->shading;
  matover = _mgc->astk->mat.override;

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

  if ((matover & MTF_DIFFUSE) && !(_mgc->astk->flags & MGASTK_SHADER)) {
    plflags &= ~(PL_HASVCOL | PL_HASPCOL);
  }
  
  if (flag & APF_FACEDRAW) {
    mrti(mr_attributebegin, mr_NULL);
    for (p = P, i = 0; i < np; i++, p++) {
      
      /* per polygon color */
      if (plflags & PL_HASPCOL) {
	mrti(mr_color, mr_parray, 3, &p->pcol, mr_NULL);
	/* then per-polygon transparency, if defined */
	if (flag & APF_TRANSP && !(matover & MTF_ALPHA)) {
	  mrti(mr_opacity, mr_array, 3, 
	       p->pcol.a, p->pcol.a, p->pcol.a, mr_NULL);
	}
      }
      switch (p->n_vertices) {
      case 1:
	v = p->v;
	mrti(mr_attributebegin, mr_NULL);
	if (plflags & PL_HASVCOL) 
	  mrti(mr_color, mr_parray, 3, &(*v)->vcol, mr_NULL);	      
        if (plflags & PL_HASST)
	  mrti(mr_st, mr_parray, 2, &(*v)->st, mr_NULL);
	mrti(mr_surface, mr_constant,
	     mr_opacity, mr_array, 3, 1., 1., 1., mr_NULL);
	mgrib_drawpoint((HPoint3 *)(*v));
	mrti(mr_attributeend, mr_NULL);
	break;
      case 2:
	v = p->v;
	mrti(mr_attributebegin, mr_NULL);
	if (plflags & PL_HASVCOL)
	  mrti(mr_color, mr_parray, 3, &(*v)->vcol, mr_NULL);	      
	mrti(mr_surface, mr_constant,
	     mr_opacity, mr_array, 3, 1., 1., 1., mr_NULL);
	mgrib_drawline((HPoint3 *)*v,(HPoint3*)*(v+1));
	mrti(mr_attributeend, mr_NULL);
	break;
      default:
	mrti(mr_polygon, mr_NULL);
	
	/* do points */
	mrti(mr_P, mr_buildarray, p->n_vertices*3, mr_NULL);
	for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	  HPt3Dehomogenize(&(*v)->pt, &hpt);
	  mrti(mr_subarray3, &hpt, mr_NULL);
	}
	
	/* colors, if supplied */
	if (plflags & PL_HASVCOL) {
	  mrti(mr_Cs, mr_buildarray, p->n_vertices*3, mr_NULL);
	  for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	    mrti(mr_subarray3, &(*v)->vcol, mr_NULL);
	  }
	  /* then per-vertex transparency, if defined */
	  if (flag & APF_TRANSP && !(matover & MTF_ALPHA)) {
	    mrti(mr_Os, mr_buildarray, p->n_vertices*3, mr_NULL);
	    for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	      float opacity[3];
	      opacity[0]=opacity[1]=opacity[2]=(*v)->vcol.a;
	      mrti(mr_subarray3, opacity, mr_NULL);
	    }
	  }
	}
	
	/* now normals, if supplied */
	if (plflags & PL_HASVN) {
	  mrti(mr_N, mr_buildarray, p->n_vertices*3, mr_NULL);
	  for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	    mrti(mr_subarray3, &((*v)->vn), mr_NULL);
	  }
	} else if (plflags & PL_HASPN) {
	  mrti(mr_N, mr_buildarray, p->n_vertices*3, mr_NULL);
	  for (j=0, v=p->v; j< p->n_vertices; j++, v++) {
	    mrti(mr_subarray3, &(p->pn), mr_NULL);
	  }
	}

	/* Texture support, to some extend. We dump all (different)
	 * texture images to files, enumerated by a sequence
	 * number. Then we insert for each image a line

	 ``MakeTexture "ourfile" "ribtxfile" "periodic" "clamp" "gaussian" 1.0 1.0''
	 
	 * in front of WorldBegin (unluckily all textures must be
	 * defined before the call to WorldBegin, meaning the
	 * RIB-command).
	 *
	 * We do all the clamping and transformation ourselves. FIXME:
	 * is there as texture transformation in the RenderMan shading
	 * language??
	 */
	if ((ap->flag & (APF_TEXTURE|APF_FACEDRAW))
	    == 
	    (APF_TEXTURE|APF_FACEDRAW)
	    && _mgc->astk->ap.tex != NULL && (plflags & PL_HASST)) {
	  Transform T;
	  Texture *tex = _mgc->astk->ap.tex;
	  TxST stT;

	  TmConcat(tex->tfm, _mgc->txstk->T, T);

	  mrti(mr_st, mr_buildarray, p->n_vertices*2, mr_NULL);
	  for (j = 0, v = p->v; j < p->n_vertices; j++, v++) {
	    TxSTTransform (T, &(*v)->st, &stT);
	    stT.t = 1.0 - stT.t;
	    mrti(mr_subarray2, (float *)&stT, mr_NULL);
	  }
	}

	break;

      }
    }
    mrti(mr_attributeend, mr_NULL);
  }    
  if (flag & APF_EDGEDRAW) {
    color = &_mgc->astk->ap.mat->edgecolor;
    mrti(mr_attributebegin, mr_surface, mr_constant,
	 mr_color, mr_parray, 3, color,
	 mr_opacity, mr_array, 3, 1., 1., 1., mr_NULL);
    
    for (p = P, i = 0; i < np; i++, p++) {	
      for (j=0, v=p->v; j < (p->n_vertices-1); j++, v++) {
	mgrib_drawline((HPoint3 *)*v,(HPoint3*)*(v+1));
      }
      mgrib_drawline((HPoint3 *)*v,(HPoint3 *)*(p->v));
    }
    mrti(mr_attributeend,mr_NULL);
  }
  
  
  if (flag & APF_NORMALDRAW) {
    /* since mg_drawnormal handles attributes and stacking, we do
     * nothing here
     */
    if (plflags & PL_HASPN) {
      for (p = P, i = 0; i < np; i++, p++) {
	for (j=0, v=p->v; j < p->n_vertices; j++, v++) {
	  mgrib_drawnormal(&(*v)->pt, &p->pn);
	}
      }
    } else if (plflags & PL_HASVN) {
      for (vp = V, i = 0; i < nv; i++, vp++) {
	mgrib_drawnormal(&vp->pt, &vp->vn);
      }
    }
  }
  
}

/* There is a basic problem now with 4-d points and 3-d normal vectors.
   For now, we'll just ignore the 4-th coordinate of the point when 
   computing the tip of the normal vector.  This will work OK with all
   existing models, but for genuine 4-d points it won't work.  But,
   come to think of it, what is the correct interpretation of the
   normal vector when the points live in 4-d?
*/
void
mgrib_drawnormal(HPoint3 *p, Point3 *n) {	
  HPoint3 end, tp;
  float scale;
  Color *color;
    
  if (p->w <= 0.0) return;
  scale = p->w * _mgc->astk->ap.nscale;
  end.x = p->x + scale*n->x;
  end.y = p->y + scale*n->y;
  end.z = p->z + scale*n->z;
  end.w = p->w;

  color = &_mgc->astk->mat.normalcolor;
  mrti(mr_attributebegin, mr_surface, mr_constant,
       mr_color, mr_parray, 3, color,
       mr_opacity, mr_array, 3, 1., 1., 1., mr_NULL);
  mgrib_drawline(&tp,&end);
  mrti(mr_attributeend, mr_NULL);
    
}

void
mgrib_bezier(int du, int dv, int dimn, float *CtrlPnts,
	     TxST *txmapst, ColorA *c)
{
  int i, ip, nu, nv;
  static float *uknot=NULL, *vknot=NULL;
  static size_t ulen=0,vlen=0;
  size_t nulen=0, nvlen=0;
  int  flag = _mgc->astk->ap.flag;
  int  matover = _mgc->astk->mat.override;

  du += 1;
  dv += 1;
  nu = du;
  nv = dv;
  ip = nu * nv * dimn; 

  if (!uknot) {
    ulen=nu+du;
    uknot=(float *)malloc(ulen*sizeof(float));
  }
  if (!vknot) {
    vlen=nv+dv;
    vknot=(float *)malloc(vlen*sizeof(float));
  }
  nulen=nu+du;
  nvlen=nv+dv;
  if (nulen>ulen) uknot=(float *)realloc(uknot,(ulen=nulen)*sizeof(float));
  if (nvlen>vlen) vknot=(float *)realloc(vknot,(vlen=nvlen)*sizeof(float));
    
  /* uknot = (float *)malloc((nu+du)*sizeof(float)); */
  for (i=0;i<nu;i++) uknot[i] = 0;
  for (i=nu;i<(nu+du);i++) uknot[i] = 1;
    
  /* vknot = (float *)malloc((nv+dv)*sizeof(float)); */
  for (i=0;i<nv;i++) vknot[i] = 0;
  for (i=nv;i<(nv+dv);i++) vknot[i] = 1;

  mrti(mr_nupatch, mr_int, nu, mr_int, du, mr_NULL);
  mrti(mr_parray, (nu+du), uknot, mr_NULL);
  mrti(mr_int, 0, mr_int, (nu-1), mr_int, nv, mr_int, dv, mr_NULL);
  mrti(mr_parray, (nv+dv), vknot, mr_NULL);
  mrti(mr_int, 0, mr_int, nv -1, mr_NULL);
    
  mrti(dimn == 3 ? mr_P : mr_Pw,
       mr_parray, ip, CtrlPnts, mr_NULL);
    
  /* free(uknot); */
  /* free(vknot); */
    
  if (c &&
      !((matover & MTF_DIFFUSE) && !(_mgc->astk->flags & MGASTK_SHADER)) ) {
    mrti(mr_Cs, mr_buildarray,  12, mr_NULL);
    for (i = 0; i < 4; i++) {
      mrti(mr_subarray3, (float *)&c[i], mr_NULL);
    }
    if (flag & APF_TRANSP && !(matover & MTF_ALPHA)) {
      float opacity[3];
      opacity[0]=opacity[1]=opacity[2]=c[i].a;
      mrti(mr_Os, mr_buildarray, 12, mr_NULL);
      for (i = 0; i < 4; i++) {
	mrti(mr_subarray3, opacity, mr_NULL);
      }
    }
  }
    
  if ((_mgc->astk->ap.flag & (APF_TEXTURE|APF_FACEDRAW))
      == 
      (APF_TEXTURE|APF_FACEDRAW)
      && _mgc->astk->ap.tex != NULL && txmapst) {
    Transform T;
    Texture *tex = _mgc->astk->ap.tex;
    TxST stT;
    int j;

    TmConcat(tex->tfm, _mgc->txstk->T, T);

    mrti(mr_nl, mr_st, mr_buildarray, 8, mr_NULL);
    for (j = 0; j < 4; j++) {
      TxSTTransform(T, &txmapst[j], &stT);
      stT.t = 1.0 - stT.t;
      mrti(mr_subarray2, &stT, mr_NULL);
    }
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
