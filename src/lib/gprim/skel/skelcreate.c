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

#include "skelP.h"

/*
 * Vect object creation, editing, copying, deletion.
 */

Skel *SkelCopy(Skel *os)
{
  Skel  *s;

  if (os == NULL) return (NULL);

  s = OOGLNewE(Skel, "new SKEL");

  *s = *os;
  s->p = OOGLNewNE(float, os->nvert * os->pdim, "Skel vertices");
  s->c = os->nc > 0 ? OOGLNewNE(ColorA, os->nc, "Skel colors") : NULL;
  s->l = OOGLNewNE(Skline, os->nlines, "Skel lines");
  s->vi = OOGLNewNE(int, os->nvi, "Skel nverts");
  if (s->vc) {
    s->vc = OOGLNewNE(ColorA, os->nvert, "Skel vertex colors");
  }

  memcpy(s->p, os->p, os->nvert * os->pdim * sizeof(float));
  memcpy(s->l, os->l, os->nlines * sizeof(Skline));
  if(os->nc > 0) memcpy(s->c, os->c, os->nc * sizeof(ColorA));
  memcpy(s->vi, os->vi, os->nvi * sizeof(int));
  if (os->vc) memcpy(s->vc, os->vc, os->nvert * sizeof(ColorA));
  return s;
}

void SkelDelete(Skel *s)
{
  if (s) {
    if (s->p != NULL) OOGLFree(s->p);
    if (s->c != NULL) OOGLFree(s->c);
    if (s->vi != NULL) OOGLFree(s->vi);
    if (s->vc != NULL) OOGLFree(s->vc);
  }
}

Skel *SkelCreate(Skel *exist, GeomClass *classp, va_list *a_list)
{
  Skel *s;

  (void)a_list;

  if (exist == NULL) {
    s = OOGLNewE(Skel, "new skel");
    GGeomInit (s, classp, SKELMAGIC, NULL);
    s->nlines = 0;
    s->nvert = 0;
    s->nvi = 0;
    s->p = NULL;
    s->c = NULL;
    s->vi = NULL;
    s->vc = NULL;
  } else {
    /* Should check that exist is a vect */
    s = exist;
  }

#ifdef notyet
  while (attr = va_arg (*a_list, int)) 
    switch (attr) {
    case CR_FLAG:
      vect->flag = va_arg (*a_list, int);
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
      if (v3 == NULL) {
	vect->p = NULL;
	vect->nvert = 0;
      } else if (copy) {
	vect->p = OOGLNewNE(HPoint3, vect->nvert, "vect points");
	Pt3ToPt4(v3, vect->p, vect->nvert);
      } else {
	Pt3ToPt4(v3, vect->p, vect->nvert);
      }
      break;

    case CR_POINT4:
      v4 = va_arg (*a_list, HPoint3 *);
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
      if (GeomDecorate (s, &copy, attr, a_list)) {
	OOGLError (0, "VectCreate: Undefined option: %d\n", attr);
	OOGLFree (vect);
	return NULL;
      }
    }


  if (!SkelSane(s)) {
    OOGLError (0, "SkelCreate: Bogus data supplied");
    GeomDelete((Geom *)s);
    return NULL;
  }	/* end police work */

  if (exist != NULL) return exist;
#endif

  return s;
}

int SkelSane(Skel *s)
{
  int i;
  Skline *l;

  if (s == NULL || s->vi==NULL || s->p==NULL || s->nlines < 0 ||
     s->nvert < 0 || s->nc < 0 || (s->nc>0 && s->c==NULL))
    return 0;
  for (i = s->nlines, l = s->l; --i >= 0; ) {
    if (l->v0 < 0 || l->nv < 0 || l->nv + l->v0 > s->nvi)
      return 0;
    if (l->nc < 0 || l->c0 < 0 || l->c0 + l->nc > s->nc)
      return 0;
  }
  for (i = 0; i < s->nvi; i++)
    if ((unsigned)s->vi[i] >= (unsigned)s->nvi)
      return 0;
  return 1;
}
