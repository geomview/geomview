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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * NPolyList creation, editing, and deletion.
 */
#include "npolylistP.h"


 /*
  * Free a NPolyList. 
  */

#include "npolylistP.h"

NPolyList *
NPolyListDelete( register NPolyList *np )
{
    if (np == NULL)
	return NULL;

    OOGLFree(np->vi);
    OOGLFree(np->p);
    OOGLFree(np->v);
    if(np->vcol) OOGLFree(np->vcol);
    return NULL;
}

NPolyList *
NPolyListCreate(NPolyList *exist, GeomClass *classp, va_list *a_list)
{
   register NPolyList *pl;
   int *nvertperpol=NULL, *verts=NULL;
   int npolyflag=0, nvertflag=0, vertflag=0, pointflag=0, pointhomog = 0, colorflag = 0;
   float *v = NULL;
   ColorA *vc = NULL, *pc = NULL;
   int attr, copy=1;
   int numentries=0, numvertices=0;
   register int i;
   int j,k=0, pdim = 4;

   if (exist == NULL) {
	pl = OOGLNewE(NPolyList,"NPolyListCreate npolylist");
	GGeomInit(pl, classp, NPLMAGIC, NULL);
	pl->flags = pl->n_polys = pl->n_verts = 0;
	pl->p = NULL;
	pl->v = NULL;
	pl->vi = NULL;
	pl->vcol = NULL;
	pl->pdim = 4;	/* 3-D plus homogeneous component */
   } else {
	pl = exist;
	pdim = pl->pdim;
   }


   while ((attr = va_arg(*a_list, int))) 
     switch (attr) {

       case CR_NOCOPY:
          OOGLError(0,"Note: NOCOPY option not used by NPolyListCreate()");
          break;

       case CR_FLAG:
          pl->flags = va_arg(*a_list, int);
          break;

       case CR_NPOLY:
          pl->n_polys = va_arg(*a_list, int);
          npolyflag = 1;
          break;

       case CR_NVERT:	/* number of verts of each polygon */
          nvertperpol = va_arg(*a_list, int*);
          nvertflag = 1;
          break;

       case CR_VERT:	/* indices of all verts of all polygons, concatenated */
			/* verts[] contains sum(nvertperpol[]) elements */
          verts = va_arg(*a_list, int*);
          vertflag = 1;
          break;

	case CR_DIM:
	  pl->pdim = va_arg(*a_list, int) + 1;
	  break;

	case CR_POINT4:	   /* CR_POINT4, <points including homog. components> */
	  pointhomog = 1;
	  /* Fall into CR_POINT */

	case CR_POINT:	   /* CR_POINT, <points without homog. components> */
	
          v = va_arg(*a_list, float *);
          pointflag = 1;
          break;

       case CR_COLOR:	   /* CR_COLOR, <ColorA's, one per vertex */
	  pl->flags &= ~PL_HASVCOL;
          vc = va_arg(*a_list, ColorA *);
          if ( vc && (pl->flags & (PL_HASVCOL | PL_HASPCOL)) == 0)
          	pl->flags |= PL_HASVCOL;
	  colorflag = 1;
          break;

       case CR_POLYCOLOR:
	  pl->flags &= ~PL_HASPCOL;
          pc = va_arg(*a_list, ColorA *);
          if(pc) pl->flags |= PL_HASPCOL;
          break;

       default:
          if (GeomDecorate(pl, &copy, attr, a_list)) {
             OOGLError(0,"Undefined PolyList option: %d", attr);
             if (!exist) GeomDelete((Geom *)pl);
             return NULL;
	}
    }

    if (!exist && (!npolyflag || !nvertflag || !vertflag || !pointflag)) {
	if (!npolyflag) OOGLError(0,"Must specify number of polygons");
	if (!nvertflag) OOGLError(0,"Must specify NVERT array");
	if (!vertflag) OOGLError(0,"Must specify VERT array");
	if (!pointflag) OOGLError(0,"Must specify vertices");
	GeomDelete((Geom *)pl);
	return NULL;
    }


    if(nvertflag && vertflag) {
	numentries = 0;
	for (i=0; i<pl->n_polys; i++)
	    numentries += nvertperpol[i];
	for (i=0; i<numentries; i++) 
	    if (verts[i] > numvertices)
		numvertices = verts[i];
	pl->n_verts = numvertices + 1;
	if(pl->v)
	    OOGLFree(pl->v);
	pl->v = OOGLNewNE(float, pl->n_verts * pl->pdim, "polylist vertices");
    }
    if(pointflag) {
	/* if dimn == 3, copy over into 4-vector area and add a '1' in
	 * the 4th coordinate */
	if (pointhomog) {
	    memcpy(pl->v, v, pl->pdim*pl->n_verts*sizeof(float));
	} else {
	    float *pv;
	    for(i = pl->n_verts, pv = pl->v; --i >= 0; ) {
		for(j = pl->pdim; --j > 0; )
		    *pv++ = *v++;
		*pv++ = 1.0;	/* Add homogenous '1' */
	    }
	}
    }
    if(npolyflag) {
	if(pl->p)
	    OOGLFree(pl->p);
	pl->p = OOGLNewNE(NPoly, pl->n_polys, "nPolyListCreate polygons");
    }
    if(nvertflag && vertflag) {
	k = 0;
	for (i=0; i<pl->n_polys; i++) {
	    pl->p[i].vi0 = k;
	    k += (pl->p[i].n_vertices = nvertperpol[i]);
	}
	if(pl->vi)
	    OOGLFree(pl->vi);
	pl->vi = OOGLNewNE(int, numentries, "nPolyListCreate vertex indices");
	pl->nvi = k;
	memcpy(pl->vi, verts, k*sizeof(int));
    }

    if (colorflag) {
	if(pl->vcol)
	    OOGLFree(pl->vcol);
	pl->vcol = NULL;
	if(vc) {
	    pl->vcol = OOGLNewNE(ColorA, pl->n_verts, "NPolyList vert colors");
	    memcpy(pl->vcol, vc, pl->n_verts * sizeof(ColorA));
	}
    }

    if (pc)
        for (i = 0; i < pl->n_polys; i++)
	    pl->p[i].pcol = pc[i];

    return pl;
}
