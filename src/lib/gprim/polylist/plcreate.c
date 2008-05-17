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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * PolyList creation, editing, and deletion.
 */
#include "polylistP.h"
#include "bsptree.h"

/*
 * Free a PolyList. 
 */

#include "polylistP.h"

PolyList *
PolyListDelete(PolyList *pl)
{
  int i;
  Poly *p;
    
  if (pl == NULL) {
    return NULL;
  }

  if((p = pl->p) != NULL) {
    for(i = pl->n_polys; --i >= 0; p++) {
      if(p->v != NULL) OOGLFree(p->v);
    }
    OOGLFree(pl->p);
  }
  if(pl->vl != NULL) {
    OOGLFree(pl->vl);
  }

  PolyListDelete(pl->plproj);

  return NULL;
}

PolyList *
PolyListCreate(PolyList *exist, GeomClass *classp, va_list *a_list)
{
  PolyList *pl;
  Vertex *v;
  int *nvertperpol=NULL, *verts=NULL;
  int npolyflag=0, nvertflag=0, vertflag=0, pointflag=0;
  HPoint3 *v4 = NULL;
  Point3 *v3 = NULL, *vn = NULL, *pn = NULL;
  ColorA *vc = NULL, *pc = NULL;
  int *pf = NULL;
  int attr, copy=1;
  int numentries=0, numvertices=0;
  int i;
  int j, k=0, dimn = 3;

  if (exist == NULL) {
    pl = OOGLNewE(PolyList,"PolyListCreate polylist");
    GGeomInit(pl, classp, PLMAGIC, NULL);
    pl->geomflags = pl->n_polys = pl->n_verts = 0;
    pl->pdim = 4;
    pl->p = (Poly *)NULL; pl->vl = (Vertex *)NULL;
    pl->plproj = NULL;
  } else {
    pl = exist;
    PolyListDelete(pl->plproj); /* better delete it. */
    pl->plproj = NULL;
  }

  while ((attr = va_arg(*a_list, int))) 
    switch (attr) {

    case CR_NOCOPY:
      OOGLError(0,"Note: NOCOPY option not used by PolyListCreate()");
      break;

    case CR_FLAG:
      pl->geomflags = va_arg(*a_list, int);
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

    case CR_POINT:
      v3 = va_arg(*a_list, Point3 *);
      pointflag = 1;
      dimn = 3;
      pl->geomflags &= ~(PL_HASVN | PL_HASPN | PL_HASPFL);
      break;

    case CR_POINT4:
      v4 = va_arg(*a_list, HPoint3 *);
      pointflag = 1;
      dimn = 4;
      pl->geomflags &= ~(PL_HASVN | PL_HASPN | PL_HASPFL);
      break;

    case CR_NORMAL:
      vn = va_arg(*a_list, Point3 *);
      /* if no normal bit has been set... */
      if (vn)
	pl->geomflags |= PL_HASVN;
      break;

    case CR_COLOR:
      pl->geomflags &= ~COLOR_ALPHA;
      vc = va_arg(*a_list, ColorA *);
      if (vc) {
	pl->geomflags |= PL_HASVCOL;
      }
      break;

    case CR_POLYNORMAL:
      pn = va_arg(*a_list, Point3 *);
      if (pn)
	pl->geomflags |= PL_HASPN;
      break;

    case CR_POLYFLAGS:
      pf = va_arg(*a_list, int *);
      if (pf)
	pl->geomflags |= PL_HASPFL;
      break;

    case CR_POLYCOLOR:
      pl->geomflags &= ~COLOR_ALPHA;
      pc = va_arg(*a_list, ColorA *);
      if (pc) {
	pl->geomflags |= PL_HASPCOL;
      }
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

  if(nvertflag) {
    for (i=0; i<pl->n_polys; i++)
      numentries += nvertperpol[i];
    for (i=0; i<numentries; i++) 
      if (verts[i] > numvertices)
	numvertices = verts[i];
    pl->n_verts = numvertices + 1;
    if(pl->vl)
      OOGLFree(pl->vl);
    pl->vl = OOGLNewNE(Vertex, pl->n_verts, "polylist vertices");
  }
  if(pointflag) {
    /* if dimn == 3, copy over into 4-vector area and add a '1' in
     * the 4th coordinate */
    int needhomog = 0;
    v = pl->vl;
    i = pl->n_verts;
    if (dimn == 3) {
      while(--i >= 0) {
	v->pt.x = v3->x;
	v->pt.y = v3->y;
	v->pt.z = v3->z;
	v->pt.w = 1.0;
	v++; v3++;
      }
    } else {
      while(--i >= 0) {
	v->pt = *v4++;
	if(v->pt.w != 1) needhomog = VERT_4D;
	v++;
      }
    }
    pl->geomflags = (pl->geomflags & ~VERT_4D) | needhomog;
  }
  if(nvertflag) {
	
    pl->p = OOGLNewNE(Poly, pl->n_polys, "PolyListCreate polygons");
    k = 0;
    for (i=0; i < pl->n_polys; i++) {
      int nv = nvertperpol[i];

      pl->p[i].flags = 0;
      pl->p[i].n_vertices = nv;
      pl->p[i].v = OOGLNewNE(Vertex *, nvertperpol[i],
			     "PolyListCreate poly vert pointers");
      for (j = 0; j < nv; j++)
	pl->p[i].v[j] = &pl->vl[verts[k++]];
    }
  }

  if (vn)
    for (i = 0, v = pl->vl; i < pl->n_verts; i++, v++)
      v->vn = vn[i];

  if (pn)
    for (i = 0; i < pl->n_polys; i++)
      pl->p[i].pn = pn[i];

  if (pf)
    for (i = 0; i < pl->n_polys; i++)
      pl->p[i].flags = pf[i];

  if (vc) {
    for (i = 0, v = pl->vl; i < pl->n_verts; i++, v++) {
      v->vcol = vc[i];
      if (vc[i].a != 1.0) {
	pl->geomflags |= COLOR_ALPHA;
      }
    }
  }

  if (pc) {
    for (i = 0; i < pl->n_polys; i++) {
      pl->p[i].pcol = pc[i];
      if (pc[i].a != 1.0) {
	pl->geomflags |= COLOR_ALPHA;
      }
    }
  }

  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
