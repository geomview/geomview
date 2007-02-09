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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "vectP.h"

/*
 * Vect object creation, editing, copying, deletion.
 */

Vect *
VectCopy(Vect *ov)
{
	Vect  *v;

	if (ov == NULL) return (NULL);

	v = OOGLNewE(Vect, "new Vect");

	*v = *ov;
	v->p = OOGLNewNE(HPoint3, ov->nvert, "Vect vertices");
	v->c = ov->ncolor ? OOGLNewNE(ColorA, ov->ncolor, "Vect colors")
			  : NULL;
	v->vnvert = OOGLNewNE(short, ov->nvec, "Vect nverts");
	v->vncolor = OOGLNewNE(short, ov->nvec, "Vect nverts");

	memcpy(v->p, ov->p, ov->nvert * sizeof(HPoint3));
	memcpy(v->c, ov->c, ov->ncolor * sizeof(ColorA));
	memcpy(v->vnvert, ov->vnvert, ov->nvec * sizeof(short));
	memcpy(v->vncolor, ov->vncolor, ov->nvec * sizeof(short));
	return(v);
}

void
VectDelete( Vect *v )
{
    if (v) {
	if(v->p != NULL) OOGLFree(v->p);
	if(v->c != NULL) OOGLFree(v->c);
	if(v->vnvert != NULL) OOGLFree(v->vnvert);
    }
}

Vect *
VectCreate ( Vect *exist, GeomClass *classp, va_list *a_list )
{
    Vect *vect;
    int attr, copy = 1;
    int police = 1;	/* by default, check the input to see it's good */

    short *vectcounts;	/* vectcounts[0..nvec-1] */
    short *colorcounts;	/* colorcounts[0..nvec-1] */
    Point3	*v3;
    HPoint3	*v4;			/* vertices[0..nvert-1] */
    ColorA	*colors;		/* colors[0..ncolor-1] */

    if (exist == NULL) {
	vect = OOGLNewE(Vect, "new vect");
        GGeomInit (vect, classp, VECTMAGIC, NULL);
	vect->nvec = 0;
	vect->nvert = 0;
	vect->ncolor = 0;
	vect->p = NULL;
	vect->c = NULL;
	vect->vnvert = NULL;
	vect->vncolor = NULL;
    } else {
	/* Should check that exist is a vect */
	vect = exist;
    }

    while ((attr = va_arg (*a_list, int))) 
      switch (attr) {
 	case CR_POLICE:
	    police = 1;
	    break;

 	case CR_NOPOLICE:
	    police = 0;
	    break;

	case CR_FLAG:
	    vect->geomflags = va_arg (*a_list, int);
	    break;

	case CR_NVECT:
	    vect->nvec = va_arg (*a_list, int);
	    break;

	case CR_NVERT:
	    vect->nvert = va_arg (*a_list, int);
	    break;

	case CR_NCOLR:
	    vect->ncolor = va_arg (*a_list, int);
	    break;

	case CR_VECTC:
	    vectcounts = va_arg (*a_list, short *);
	    if(vect->vnvert) OOGLFree(vect->vnvert);
            if (vectcounts == NULL) {
               vect->vnvert = NULL;
               vect->nvert = 0;
            } else if (copy) {
    		vect->vnvert = OOGLNewNE(short, vect->nvec, "vect vert counts");
    		memcpy(vect->vnvert, vectcounts, vect->nvec*sizeof(*vect->vnvert));
            } else {
		vect->vnvert = vectcounts;
	    }
	    break;

	case CR_COLRC:
	    colorcounts = va_arg (*a_list, short *);
	    if(vect->vncolor) OOGLFree(vect->vncolor);
            if (colorcounts == NULL) {
		vect->vncolor = NULL;
		vect->nvert = 0;
            } else if (copy) {
    		vect->vncolor = OOGLNewNE(short, vect->nvec, "vect vert counts");
    		memcpy(vect->vncolor, colorcounts, vect->nvec*sizeof(*vect->vncolor));
            } else {
		vect->vncolor = colorcounts;
	    }
	    break;

	case CR_POINT:
	    v3 = va_arg (*a_list, Point3 *);
	    if(vect->p) OOGLFree(vect->p);
            if (v3 == NULL) {
               vect->p = NULL;
               vect->nvert = 0;
            } else {
    		vect->p = OOGLNewNE(HPoint3, vect->nvert, "vect points");
		Pt3ToHPt3(v3, vect->p, vect->nvert);
		if(!copy) OOGLFree(v3);
	    }
	    break;

	case CR_POINT4:
	    v4 = va_arg (*a_list, HPoint3 *);
	    if(vect->p) OOGLFree(vect->p);
            if (v4 == NULL) {
               vect->p = NULL;
               vect->nvert = 0;
            } else if (copy) {
    		vect->p = OOGLNewNE(HPoint3, vect->nvert, "vect points");
        	memcpy(vect->p, v4, vect->nvert*sizeof(HPoint3));
            } else {
		vect->p = v4;
	    }
	    break;

	case CR_COLOR:
	    colors = va_arg (*a_list, ColorA *);
	    if(vect->c) OOGLFree(vect->c);
            if (colors == NULL) {
               vect->c = NULL;
               vect->ncolor = 0;
            } else if (copy) {
    		vect->c = OOGLNewNE(ColorA, vect->ncolor, "vect colors");
    		memcpy(vect->c, colors, vect->ncolor*sizeof(ColorA));
            } else {
		vect->c = colors;
	    }
	    break;

	default:
	    if (GeomDecorate (vect, &copy, attr, a_list)) {
		OOGLError (0, "VectCreate: Undefined option: %d\n", attr);
		OOGLFree (vect);
		return NULL;
	    }
    }


    if (police)	{
      if(0 > vect->ncolor || 0 > vect->nvec )
	OOGLError(0, "VectCreate: ncol=%d nvert=%d; need 0<ncol<nvert",vect->ncolor, vect->nvert);
      if (	0 > vect->nvec ||
		vect->nvec > vect->nvert || 
		vect->nvert > 99999999) {
	OOGLError (0, "VectCreate: nvec=%d, nvert=%d; need 0<=nvec<=nvert<=1e8", vect->nvec, vect->nvert);
	return(NULL);
      }

      if (!VectSane(vect)) {
	  OOGLError (0, "VectCreate: Bogus data supplied");
	  GeomDelete((Geom *)vect);
	  return NULL;
        }
      }	/* end police work */

    if (exist != NULL) return exist;

    return vect;
}

int
VectSane(vect)
	Vect *vect;
{
	int i;
	int vleft, cleft;
	short *p, *c;

	if(!vSane(vect))
		return(0);
	vleft = vect->nvert;
	cleft = vect->ncolor;
	for(i = vect->nvec, p = vect->vnvert, c = vect->vncolor; --i >= 0; p++, c++) {
		if(*p == 0 || (vleft -= vcount(*p)) < 0)
			return(0);
		if(*c < 0 || (cleft -= *c) < 0)
			return(0);
	}
	if(vleft || cleft)
		return(0);
	return(1);
}
