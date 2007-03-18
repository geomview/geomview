/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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


/* Authors: Silvio Levy, Charlie Gunn, Stuart Levy, Tamara Munzner, 
   Mark Phillips */

#include "math.h"
#include "bezierP.h"
#include "bsptreeP.h"

void bezier_interp();

Bezier *
BezierDice(Bezier *bezier, int nu, int nv)
{
    if(nu < 2) nu = BEZ_DEFAULT_MESH_SIZE;
    if(nv < 2) nv = BEZ_DEFAULT_MESH_SIZE;
    if(nu != bezier->nu || nv != bezier->nv) {
	bezier->geomflags |= BEZ_REMESH;
	bezier->nu = nu;
	bezier->nv = nv;
    }
    return bezier;
}

/* this gets called when it's time to remesh, so the current mesh is destroyed
   and a new one is allocated */
Mesh *BezierReDice(Bezier *bezier)
{
    int      u, v, d;
    int      dimn, nu, nv, degree_u, degree_v;
    float    *tmpp0, *tmpp1, *tmpp2, *p;
    float    *tmpdv, *tmpdv0, *tmpdv1, *tmpdv2, *dv;
    float    *tmpdu, *tmpdu0, *tmpdu1, *tmpdu2, *du;
    float norm;
    HPoint3   *bmp;
    Point3 *bmn;
    TxST *bmt;
    Mesh m;

    dimn = bezier->dimn;
    degree_u = bezier->degree_u;
    degree_v = bezier->degree_v;

    if (bezier->nu <= 1)         bezier->nu = BEZ_DEFAULT_MESH_SIZE;
    if (bezier->nv <= 1)         bezier->nv = BEZ_DEFAULT_MESH_SIZE;
    nu = bezier->nu;
    nv = bezier->nv;

    m.nu = nv;
    m.nv = nu;
    m.geomflags = MESH_N;
    if (bezier->geomflags & BEZ_C) {
	m.geomflags |= MESH_C;
    }
    
    m.p = NULL;
    m.c = NULL;
    m.n = NULL;
    m.u = NULL;

    m.p = OOGLNewNE(HPoint3, nu*nv, "BezierReDice: mesh points");
    m.n = OOGLNewNE(Point3, nu*nv, "BezierReDice: mesh normals");
    if(bezier->geomflags & BEZ_C)
      m.c = OOGLNewNE(ColorA, nu*nv,  "BezierReDice: mesh colors");
    if(bezier->geomflags & BEZ_ST)
      m.u = OOGLNewNE(TxST, nu*nv, "BezierReDice: mesh txcoords");


    /* compute first pass: interpolate mesh points in u direction */
    tmpdv = OOGLNewNE(float, dimn * degree_v * (degree_u+1),
		      "BezierReDice: tempdv");

    for (v=0; v<degree_v; ++v)
      for (u=0; u<degree_u+1; ++u) {
        for (d=0; d<dimn; ++d) {
          *(tmpdv+dimn*(v*(degree_u+1)+u)+d)=
            *(bezier->CtrlPnts + dimn * ((v+1)*(degree_u+1)+u)+d)-
            *(bezier->CtrlPnts + dimn * (v*(degree_u+1)+u)+d);
	
/*
          printf("%f ", *(tmpdv+dimn*(v*(degree_u+1)+u)+d));
*/
        }
/*
      printf("\n");
*/
      }
       
    tmpdu = OOGLNewNE(float,dimn*(degree_v+1)*degree_u,"BezierReDice:tmpdu");
/*
printf("this is du\n");
*/
    for (v=0; v<degree_v+1; ++v)
      for (u=0; u<degree_u; ++u) {
        for (d=0; d<dimn; ++d) {
          *(tmpdu+dimn*(v*degree_u+u)+d)=
            *(bezier->CtrlPnts + dimn * (v*(degree_u+1)+u+1)+d)-
            *(bezier->CtrlPnts + dimn * (v*(degree_u+1)+u)+d);
/*
          printf("%f ", *(tmpdv+dimn*(v*degree_u+u)+d));
*/
        }
/*
      printf("\n");
*/
      }
    
/*
printf("this is tmpp0\n");
*/
    tmpp0 = OOGLNewNE(float, dimn * (degree_v+1) * nu, "BezierReDice: tmpp0");
    for (v=0; v<degree_v+1; ++v)
      bezier_interp(bezier->CtrlPnts + v * dimn * (degree_u+1),
                    tmpp0 + v * dimn * nu, degree_u, nu, dimn);
    tmpdv0 = OOGLNewNE(float, dimn * degree_v * nu, "BezierReDice: tmpdv0");
/*
printf("this is tmpdv0\n");
*/
    for (v=0; v<degree_v; ++v)
      bezier_interp(tmpdv + v * dimn * (degree_u+1),
                    tmpdv0 + v * dimn * nu, degree_u, nu, dimn);
/*
printf("this is tmpdu0\n");
*/
    tmpdu0 = OOGLNewNE(float, dimn * (degree_v+1) * nu,"BezierReDice: tmpdu0");
    for (v=0; v<degree_v+1; ++v)
      bezier_interp(tmpdu + v * dimn * degree_u,
                    tmpdu0 + v * dimn * nu, degree_u-1, nu, dimn);

    /* now compute second pass, filling in columns */
    tmpp1 = OOGLNewNE(float, dimn * (degree_v+1), "BezierReDice: tmpp1");
    tmpdv1 = OOGLNewNE(float, dimn * degree_v, "BezierReDice: tmpdv1");
    tmpdu1 = OOGLNewNE(float, dimn * (degree_v+1), "BezierReDice: tmpdu1");
    tmpp2 = OOGLNewNE(float, dimn * nv, "BezierReDice: tmpp2");
    tmpdv2 = OOGLNewNE(float, dimn * nv, "BezierReDice: tmpdv2");
    tmpdu2 = OOGLNewNE(float, dimn * nv, "BezierReDice: tmpdu2");
    bmp = m.p;
    bmn = m.n;
    bmt = m.u;
    for (u=0; u<nu; ++u) {
      float txs0 = 0, txds = 0, txt0 = 0, txdt = 0, tu;

      for (v=0; v<degree_v+1; v++)
        memcpy(tmpp1+v*dimn, tmpp0 + (u + v*nu)*dimn, dimn*sizeof(float));
      for (v=0; v<degree_v; v++)
        memcpy(tmpdv1+v*dimn, tmpdv0 + (u + v*nu)*dimn, dimn*sizeof(float));
      for (v=0; v<degree_v+1; v++)
        memcpy(tmpdu1+v*dimn, tmpdu0 + (u + v*nu)*dimn, dimn*sizeof(float));

      bezier_interp(tmpp1, tmpp2, degree_v, nv, dimn);
      bezier_interp(tmpdv1, tmpdv2, degree_v-1, nv, dimn);
      bezier_interp(tmpdu1, tmpdu2, degree_v, nv, dimn);
      if(bmt) {
	  tu = (float)u/(nu-1);
	  txs0 = bezier->STCoords[0].s*(1-tu) + bezier->STCoords[1].s*tu;
	  txds = bezier->STCoords[2].s*(1-tu) + bezier->STCoords[3].s*tu - txs0;
	  txt0 = bezier->STCoords[0].t*(1-tu) + bezier->STCoords[1].t*tu;
	  txdt = bezier->STCoords[2].t*(1-tu) + bezier->STCoords[3].t*tu - txt0;
      }
      for (v=0, p=tmpp2, dv=tmpdv2, du=tmpdu2; 
           v<nv; ++v, p+=dimn, dv+=dimn, du+=dimn) {
        if(dimn == 4) {
          dv[0] = dv[0]*p[3]-dv[3]*p[0];
          dv[1] = dv[1]*p[3]-dv[3]*p[1];
          dv[2] = dv[2]*p[3]-dv[3]*p[2];
          du[0] = du[0]*p[3]-du[3]*p[0];
          du[1] = du[1]*p[3]-du[3]*p[1];
          du[2] = du[2]*p[3]-du[3]*p[2];
          p[0] /= p[3];
          p[1] /= p[3];
          p[2] /= p[3];
        }
        *bmp = *(HPoint3 *)p;   bmp->w = 1.; bmp++;
        bmn->x = -du[1]*dv[2]+du[2]*dv[1];
        bmn->y = -du[2]*dv[0]+du[0]*dv[2];
        bmn->z = -du[0]*dv[1]+du[1]*dv[0];
        norm=bmn->x *bmn->x +bmn->y *bmn->y +bmn->z *bmn->z;
        if (norm==0.0)
          {bmn->x=1.; norm=1.;}
        norm=sqrt(norm);
        bmn->x /= norm;
        bmn->y /= norm;
        bmn->z /= norm;
        bmn++;
	if(bmt) {
	    float tv = (float)v/(nv-1);
	    bmt->s = txs0 + txds*tv;
	    bmt->t = txt0 + txdt*tv;
	    bmt++;
	}
      }
    }
    GeomFree(tmpp0);
    GeomFree(tmpp1);
    GeomFree(tmpp2);
    GeomFree(tmpdu);
    GeomFree(tmpdu0);
    GeomFree(tmpdu1);
    GeomFree(tmpdu2);
    GeomFree(tmpdv);
    GeomFree(tmpdv0);
    GeomFree(tmpdv1);
    GeomFree(tmpdv2);
    bezier->geomflags &= ~BEZ_REMESH;        /* turn off this bit */

    if(bezier->geomflags & BEZ_C) {
        float fu, unu, fv, unv;
        ColorA u0, u1;
        ColorA *cp;

#define INTC(c0, c1, t, unt, dest)        /* Interpolate color */ \
                dest.r = c0.r * unt + c1.r * t; \
                dest.g = c0.g * unt + c1.g * t; \
                dest.b = c0.b * unt + c1.b * t; \
                dest.a = c0.a * unt + c1.a * t;

        cp = m.c;
        for(v = 0; v < nv; v++) {
           fv = (float) v / (nv - 1);
           unv = 1 - fv;
           INTC(bezier->c[0], bezier->c[1], fv, unv, u0);
           INTC(bezier->c[2], bezier->c[3], fv, unv, u1);
           for(u = 0; u < nu; u++) {
                fu = (float) u / (nu - 1);
                unu = 1 - fu;
                INTC(u0, u1, fu, unu, (*cp));
                cp++;
           }
        }
    }
    GeomDelete((Geom *)bezier->mesh);
    HandleSetObject(bezier->meshhandle, NULL);
    bezier->mesh = (Mesh *) GeomCreate("mesh",
				       CR_NOCOPY,
				       /* For now, assume these are
					  rational beziers hence
					  belong in 3-space CR_4D,
					  (dimn == 4) ? 1 : 0,
				       */
				       CR_FLAG, m.geomflags,
				       CR_NU, m.nu,
				       CR_NV, m.nv, 
				       CR_POINT4, m.p,
				       CR_NORMAL, m.n,
				       CR_COLOR, m.c,
				       CR_U, m.u,
				       CR_END);
    if (bezier->mesh  == NULL) {
	OOGLError(1, "BezierReDice: can't create Mesh");
	return NULL;
    }
    HandleSetObject(bezier->meshhandle, (Ref *)bezier->mesh);

    /*GeomCCreate(bezier->mesh, NULL, CR_COPY, CR_APPEAR, bezier->ap, CR_END);*/

#if 0
    /* should be handled by a call-back now */
    if (bezier->bsptree != NULL && bezier->bsptree->tree != NULL) {
	/* simply free the tree */
	BSPTreeFreeTree(bezier->bsptree);
    }
#endif

    return bezier->mesh;
}


/* Takes a list of control points describing a spline */
/* and produces values for the spline */
/* at  n  equally spaced points.   */
#if 0
float *in; /* input array of control points */
float *out; /* output array */
int deg; /* degree of spline */
int n; /* number of points to interpolate */
int dimn; /* dimension of range */
#endif
void
bezier_interp(float *in, float *out, int deg, int n, int dimn)
{
  int    m, offset, k, j;
  float  p[(MAX_BEZ_DEGREE+1)*MAX_BEZ_DIMN];
  float  t;

/*
for (j=0; j<deg+1; ++j) 
  printf("%f %f %f    ",*(in+j*dimn),*(in+j*dimn+1),*(in+j*dimn+2));
  printf("\n");
*/

  for (j=0; j<n; ++j) {
    t = ((float) j) / (n - 1);
    memcpy(p, in, dimn*sizeof(float)*(deg+1));
    for (k=0; k<deg; ++k)
      for (offset=0, m=0; m<deg; ++m, offset += dimn) {
        p[offset] += t * (p[offset+dimn] - p[offset]);
        p[offset+1] += t * (p[offset+dimn+1] - p[offset+1]);
        p[offset+2] += t * (p[offset+dimn+2] - p[offset+2]);
        if(dimn == 4)
           p[offset+3] += t * (p[offset+dimn+3] - p[offset+3]);
    }
    memcpy(out, p, sizeof(float)*dimn);
    out+=dimn;
  }
/*
out -= dimn*n;
for (j=0; j<n; ++j) 
  printf("%f %f %f    ",*(out+j*dimn),*(out+j*dimn+1),*(out+j*dimn+2));
  printf("\n");
*/
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
