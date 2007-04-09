/* Copyright (C) 2007 Claus-Justus Heine
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

#include <stdio.h>
#include <stdlib.h> /* for abs() */

#include "geom.h"
#include "skelP.h"
#include "crayolaP.h"

static char msg[] = "craySkel.c";

void *cray_skel_HasColor(int sel, Geom *geom, va_list *args);
void *cray_skel_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_skel_HasFColor(int sel, Geom *geom, va_list *args);

void *cray_skel_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_skel_UseFColor(int sel, Geom *geom, va_list *args);

void *cray_skel_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_skel_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_skel_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_skel_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_skel_SetColorAtF(int sel, Geom *geom, va_list *args);

void *cray_skel_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_skel_GetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_skel_GetColorAtF(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 15

static craySpecFunc methods[] = {
  {"crayHasColor", cray_skel_HasColor},
  {"crayHasVColor", cray_skel_HasVColor},
  {"crayHasFColor", cray_skel_HasFColor},
  
  {"crayCanUseVColor", crayTrue},
  {"crayCanUseFColor", crayTrue},
  
  {"crayUseVColor", cray_skel_UseVColor},
  {"crayUseFColor", cray_skel_UseFColor},
  
  {"crayEliminateColor", cray_skel_EliminateColor},
  
  {"craySetColorAll", cray_skel_SetColorAll},
  {"craySetColorAt", cray_skel_SetColorAt},
  {"craySetColorAtV", cray_skel_SetColorAtV},
  {"craySetColorAtF", cray_skel_SetColorAtF},
  
  {"crayGetColorAt", cray_skel_GetColorAt},
  {"crayGetColorAtV", cray_skel_GetColorAtV},
  {"crayGetColorAtF", cray_skel_GetColorAtF}
};

int
cray_skel_init(void)
{
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("skel"));
  return 0;
}

void *cray_skel_HasColor(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  return (void *)(long)((s->geomflags & GEOM_COLOR) != 0);
}

void *cray_skel_HasFColor(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  return (void *)(long)((s->geomflags & FACET_C) != 0);
}

void *cray_skel_HasVColor(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  return (void *)(long)((s->geomflags & VERT_C) != 0);
}

/* Make sure the given SKEL has per-vertex colours, initialize a
 * default colour.
 */
void *cray_skel_UseVColor(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  int i, j, k, ic;
  ColorA *def;

  def = va_arg(*args, ColorA *);
  if (s->vc == NULL) {
    s->vc = OOGLNewNE(ColorA, s->nvert, msg);
  }

  /* install the given default colour */
  for (i = 0; i < s->nvert; i++) s->vc[i] = *def;

  /* if s has per-segment colours, initialize the vertex colours with
   * them. Of course, this cannot work properly. Should we do some
   * averaging?
   */
  if (s->geomflags & FACET_C) {
    for (i = 0; i < s->nlines; i++) {
      if (s->l[i].nc == 0) {
	continue;
      }
      ic = s->l[i].c0; /* index of first and only colour */
      for (j = 0; j < s->l[i].nv; j++) {
	k = s->vi[s->l[i].v0+j]; /* vertex colour index */
	s->vc[k] = s->c[ic];
      }
    }
  }

  s->geomflags |= VERT_C;

  return (void *)geom;
}

/* Same as above, but make sure the Skel object has facet (== segment) colour */
void *cray_skel_UseFColor(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  int i;
  ColorA *color, *def;

  def = va_arg(*args, ColorA *);
  color = OOGLNewNE(ColorA, s->nlines, msg);

  for (i = 0; i < s->nlines; i++) {
    if (s->l[i].nc == 1) {
      color[i] = s->c[s->l[i].c0];
    } else if (s->geomflags & VERT_C) {
      /* initialize with first vertex color */
      color[i] = s->vc[s->vi[s->l[i].v0]];
    } else {
      /* initialize with default colour */
      color[i] = *def;
    }
    s->l[i].c0 = i; /* use a linear mapping, no holes anymore */
  }

  if (s->c) {
    OOGLFree(s->c);
  }
  s->c = color;

  s->geomflags |= FACET_C;

  return (void *)geom;
}


void *cray_skel_EliminateColor(int sel, Geom *geom, va_list *args)
{
  int i;
  Skel *s = (Skel *)geom;
  if (!crayHasColor(geom, NULL)) return 0;

  if (s->c) {
    OOGLFree(s->c);
    s->c = NULL;
  }
  if (s->vc) {
    OOGLFree(s->vc);
    s->vc = NULL;
  }
  for (i = 0; i < s->nlines; i++) {
    s->l[i].nc = 0;
    s->l[i].c0 = -1;
  }
  s->geomflags &= ~GEOM_COLOR;
  return (void *)geom;
}

void *cray_skel_SetColorAll(int sel, Geom *geom, va_list *args)
{
  int i;
  Skel *s = (Skel *)geom;
  ColorA *color = va_arg(*args, ColorA *);

  if (!crayHasVColor(geom, NULL)) return NULL;

  if (s->c) {
    for (i = 0; i < s->nlines; i++) {
      s->c[s->l[i].c0] = *color;
    }
  }
  if (s->vc) {
    for (i = 0; i < s->nvert; i++) {
      s->vc[i] = *color;
    }
  }
  
  return geom;
}

void *cray_skel_SetColorAt(int sel, Geom *geom, va_list *args)
{
  ColorA *color;
  int vindex, findex, *eindex;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  eindex = va_arg(*args, int *);
  if (crayHasVColor(geom, NULL)) {
    if (vindex != -1)
      return (void *)(long)craySetColorAtV(geom, color, vindex, NULL, NULL);
    if (eindex[0] != eindex[1]) {
      craySetColorAtV(geom, color, eindex[0], NULL, NULL);
      return (void *)(long)craySetColorAtV(geom, color, eindex[1], NULL, NULL);
    }
  }
  return (void *)(long)craySetColorAtF(geom, color, findex, NULL);
}

void *cray_skel_SetColorAtV(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) {
    return NULL;
  }
  s->vc[index] = *color;

  return (void *)geom;
}

void *cray_skel_SetColorAtF(int sel, Geom *geom, va_list *args)
{
  int i;
  Skel *s = (Skel *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);

  if (index == -1) {
    return NULL;
  }

  if (crayHasFColor(geom, NULL)) {
    /* The current facet possibly has no color */
    if (s->l[index].nc == 0) {
      s->c = OOGLRenewNE(ColorA, s->c, s->nc+1, msg);
      s->l[index].c0 = s->nc++;
      s->l[index].nc = 1;
    }
    s->c[s->l[index].c0] = *color;
  } else if (crayHasVColor(geom, NULL)) {
    for (i = 0; i < s->l[index].nv; i++) {
      s->vc[s->vi[s->l[index].v0+i]] = *color;
    }
  }

  return (void *)geom;
}

void *cray_skel_GetColorAt(int sel, Geom *geom, va_list *args)
{
  ColorA *color;
  int vindex, findex;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  if (crayHasVColor(geom, NULL)) {
    return (void *)(long)crayGetColorAtV(geom, color, vindex, NULL, NULL);
  }
  if (crayHasFColor(geom, NULL)) {
    return (void *)(long)crayGetColorAtF(geom, color, findex, NULL);
  }
  return NULL;
}

void *cray_skel_GetColorAtV(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  ColorA *color;
  int index;
  
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) {
    return NULL;
  }
  *color = s->vc[index];

  return (void *)geom;
}

void *cray_skel_GetColorAtF(int sel, Geom *geom, va_list *args)
{
  Skel *s = (Skel *)geom;
  ColorA *color;
  int index;
  
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasFColor(geom, NULL) || index == -1) {
    return NULL;
  }
  if (s->l[index].nc == 0) {
    return NULL;
  }
  *color = s->c[s->l[index].c0];
  
  return (void *)geom;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
