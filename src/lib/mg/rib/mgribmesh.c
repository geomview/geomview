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

#define	HAS_N	0x1
#define	HAS_C	0x2
#define	HAS_SMOOTH 0x4

#define VI_TUPLET_LIMIT 2 /* for sgi vi formatting (so we can use it!) */

static void mgrib_submesh( int wrap, int nu, int nv,
			   HPoint3 *P, Point3 *N, Point3 *NQ,
			   ColorA *C, TxST *ST, int mflags );
static void mgrib_prmanmesh( int wrap, int nu, int nv, HPoint3 *P );

int
mgrib_mesh(int wrap, int nu, int nv,
	   HPoint3 *P, Point3 *N, Point3 *NQ, ColorA *C, TxST *ST,
	   int mflags)
{
    Appearance *ap;
    Color   *c3;
    Point3  *n;
    HPoint3 *p;
    int u;

    ap = &_mgc->astk->ap;

    if (ap->flag & APF_FACEDRAW) {
	mgrib_submesh( wrap, nu, nv, P, N, NQ, C, ST, mflags);
    }
    
    if (ap->flag & APF_EDGEDRAW) {
	/* must draw edges manually */
	c3 = &ap->mat->edgecolor;
	mrti(mr_attributebegin, mr_color, mr_parray, 3, c3,
	    mr_opacity, mr_array, 3, 1., 1., 1.,
	    mr_surface, mr_constant, mr_NULL);
	mgrib_prmanmesh( wrap, nu, nv, P);
	mrti(mr_attributeend,mr_NULL);
    }
    
    if ((ap->flag & APF_NORMALDRAW) && N!=NULL) {
	p = P;
	n = N;
	for (u = nu*nv; --u >= 0; )
	    mgrib_drawnormal(p++, n++);
    }
    
    return 1;
}

static void
mgrib_submesh( int wrap, int nu, int nv,
	       HPoint3 *P, Point3 *N, Point3 *meshNQ,
	       ColorA *C, TxST *ST, int mflags )
{
    Appearance *ap;
    char    *uwrap,*vwrap;
    int     i;
    HPoint3 *p;
    Point3  *n;
    TxST    *st;
    ColorA  *c;
    int     nunv;
    int     viflag = 0; /* used to insert \n into RIB file so lines */
		    /* won't be too long for 'vi' to work well   */

    nunv = nu * nv;
    p = P;
    n = N;
    c = C;
    st = ST;
	    
    ap = &_mgc->astk->ap;
    
    uwrap = (wrap & MM_UWRAP) ? "periodic" : "nonperiodic";
    vwrap = (wrap & MM_VWRAP) ? "periodic" : "nonperiodic";
    
    mrti(mr_attributebegin, mr_NULL);

/* THIS SHOULD BE AT A HIGHER LEVEL
    mgrib_print("ShadingInterpolation ");
    if (ap->shading & APF_SMOOTH) mgrib_print("\"smooth\"\n");
    else mgrib_print("\"constant\"\n");
*/
    
    mrti(mr_patchmesh, mr_string, "bilinear",
    	mr_int, nu, mr_string, uwrap,
    	mr_int, nv, mr_string, vwrap,
	mr_P, mr_buildarray, 3*nunv, mr_NULL);
	
    for (i=0; i<nunv; i++, p++, viflag++) {
	mrti(mr_subarray3, p, mr_NULL);
	if (viflag>=VI_TUPLET_LIMIT) {
	    viflag=0;
	    /* wrap lines so they won't be too long for vi */
	    mrti(mr_nl, mr_NULL);
	}
    }
    
    /* use normals if supplied; flat shading, constant shading and
     * csmooth shading do not need normals
     */
    if (N != NULL && ap->shading == APF_SMOOTH) {
	viflag = 0;
	mrti(mr_N, mr_buildarray, 3*nunv, mr_NULL);
	for (i=0; i<nunv; i++, n++, viflag++) {
	  mrti(mr_subarray3, n, mr_NULL);
	  if (viflag>=VI_TUPLET_LIMIT) {
	    viflag = 0;
	    mrti(mr_nl, mr_NULL);
	  }
	}
    }
    
    /* use colors if supplied and not overridden */
    if (C &&
       !((ap->mat->override & MTF_DIFFUSE) &&
	 !(_mgc->astk->flags & MGASTK_SHADER))) {
	viflag = 0;
	mrti(mr_Cs, mr_buildarray, 3*nunv, mr_NULL);
	for (i=0; i<nunv; i++, c++, viflag++) {
	    mrti(mr_subarray3, c, mr_NULL);
	    if (viflag>=VI_TUPLET_LIMIT) {
		viflag = 0;
		mrti(mr_nl, mr_NULL);
	    }
	}
	
	/* transparancy */
	if (ap->flag & APF_TRANSP) {
	    c = C;
	    mrti(mr_Os, mr_buildarray, 3*nunv, mr_NULL);
	    for (i=0; i<nunv; i++, c++) {
		mrti(mr_subarray3, c, mr_NULL);
		if (viflag>=VI_TUPLET_LIMIT)
		{
		    viflag = 0;
		    mrti(mr_nl, mr_NULL);
		}
	    }
	}			
    }

    /* Texture support, to some extend. We dump all (different)
     * texture images to files, enumerated by a sequence number. Then
     * we insert for each image a line
     *
     * ``MakeTexture "ourfile" "ribtxfile" "none" "none" "gaussian" 1.0 1.0''
     *
     * in front of WorldBegin (unluckily all texture must be defined
     * before the call to WorldBegin, meaning the RIB-command).
     *
     * We do all the clamping and transformation ourselves. FIXME: is
     * there as texture transformation in the RenderMan shading
     * language??
     */
    if ((ap->flag & (APF_TEXTURE|APF_FACEDRAW)) == (APF_TEXTURE|APF_FACEDRAW)
        && _mgc->astk->ap.tex != NULL && ST) {
	Transform T;
	Texture *tex = _mgc->astk->ap.tex;
	TxST stT;

	TmConcat(tex->tfm, _mgc->txstk->T, T);
      
	viflag = 0;
	mrti(mr_st, mr_buildarray, 2*nunv, mr_NULL);
	for (i=0; i<nunv; i++, st++, viflag++) {
	    TxSTTransform (T, st, &stT);
	    stT.t = 1.0 - stT.t;
	    mrti(mr_subarray2, (float *)&stT, mr_NULL);
	    if (viflag>=VI_TUPLET_LIMIT) {
		viflag=0;
		mrti(mr_nl, mr_NULL);
	    }
	}
    }

    mrti(mr_attributeend, mr_NULL);
}

static void
mgrib_prmanmesh( int wrap, int nu, int nv, HPoint3 *P )
{
    int u, v, prevu, prevv;

    for (v=0; v<nv; v++) {
	if (wrap & MM_UWRAP) u = 0, prevu = nu-1;
	else		    u = 1, prevu = 0;
	for ( ; u<nu; u++) {
	    mgrib_drawline(&P[prevu + v * nu], &P[u + v * nu]);
	    prevu = u;
	}
    }

    for (u=0; u<nu; u++) {
	if (wrap & MM_VWRAP) v = 0, prevv = nv-1;
	else		    v = 1, prevv = 0;
	for ( ; v<nv; v++) {
	    mgrib_drawline(&P[u + prevv * nu], &P[u + v * nu]);
	    prevv = v;
	}
    }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
