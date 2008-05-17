/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#include "appearance.h"
#include "transobj.h"
#include "handleP.h"
#include <string.h>
#include <sys/stat.h>

extern int Apsavepfx(int valid, int override, int mask, char *keyword,
		     FILE *f, Pool *p);

Material *
_MtSet(Material *mat, int attr1, va_list *alist)
{
  int attr;

#define NEXT(type) va_arg(*alist, type)
    
  if (mat == NULL) {
    /*
     * New Material created here.
     */
    mat = OOGLNewE(Material, "new Material"); 
    MtDefault(mat);
  }

  for (attr = attr1; attr != MT_END; attr = NEXT(int)) {
    switch (attr) { /* parse argument list */
    case MT_AMBIENT:
      mat->ambient = *NEXT(Color *);
      mat->valid |= MTF_AMBIENT;
      break;
    case MT_DIFFUSE:
    {
      Color tmp;
      tmp = *NEXT(Color *);
      mat->diffuse.r = tmp.r;
      mat->diffuse.g = tmp.g;
      mat->diffuse.b = tmp.b;
      mat->valid |= MTF_DIFFUSE;
      break;
    }
    case MT_SPECULAR:
      mat->specular = *NEXT(Color *);
      mat->valid |= MTF_SPECULAR;
      break;
    case MT_EMISSION:
      mat->emission = *NEXT(Color *);
      mat->valid |= MTF_EMISSION;
      break;
    case MT_ALPHA: 
      mat->diffuse.a = NEXT(double);
      mat->valid |= MTF_ALPHA;
      break;
    case MT_Ka: 
      mat->ka = NEXT(double);
      mat->valid |= MTF_Ka;
      break;
    case MT_Kd: 
      mat->kd = NEXT(double);
      mat->valid |= MTF_Kd;
      break;
    case MT_Ks: 
      mat->ks = NEXT(double);
      mat->valid |= MTF_Ks;
      break;
    case MT_SHININESS: 
      mat->shininess = NEXT(double);
      mat->valid |= MTF_SHININESS;
      break;
    case MT_EDGECOLOR:
      mat->edgecolor = *NEXT(Color *);
      mat->valid |= MTF_EDGECOLOR;
      break;
    case MT_NORMALCOLOR:
      mat->normalcolor = *NEXT(Color *);
      mat->valid |= MTF_NORMALCOLOR;
      break;
    case MT_INVALID:
      mat->valid &= ~NEXT(int);
      break;
    case MT_OVERRIDE:
      mat->override |= NEXT(int);
      break;
    case MT_NOOVERRIDE:
      mat->override &= ~NEXT(int);
      break;
    default:
      OOGLError (0, "_MtSet: undefined option: %d\n", attr);
      return NULL;
      break; 
    }
  }

  return mat;

#undef NEXT
}

Material *
MtCreate(int a1, ... )
{
  va_list alist;
  Material *mat;

  va_start(alist,a1);
  mat = _MtSet(NULL, a1, &alist);
  va_end(alist);
  return mat;
}

Material *
MtSet(Material *mat, int attr, ... )
{
  va_list alist;
    
  va_start(alist,attr);
  mat = _MtSet(mat,attr,&alist);
  va_end(alist);
  return mat;
}

int
MtGet(Material *mat, int attr, void * value)
{
  if (mat == NULL)
    return -1;

  switch (attr) {
  case MT_AMBIENT:
    *(Color *)value = mat->ambient;
    break;
  case MT_DIFFUSE:
  {
    ((Color *)value)->r = mat->diffuse.r;
    ((Color *)value)->g = mat->diffuse.g;
    ((Color *)value)->b = mat->diffuse.b;
    break;
  }
  case MT_SPECULAR:
    *(Color *)value = mat->specular;
    break;
  case MT_EMISSION:
    *(Color *) value = mat->emission;
    break;
  case MT_ALPHA: 
    *(double *) value = mat->diffuse.a;
    break;
  case MT_Ka: 
    *(double *) value = mat->ka;
    break;
  case MT_Kd: 
    *(double *) value = mat->kd;
    break;
  case MT_Ks: 
    *(double *) value = mat->ks;
    break;
  case MT_SHININESS: 
    *(double *) value = mat->shininess;
    break;
  case MT_EDGECOLOR:
    *(Color *)value = mat->edgecolor;
    break;
  case MT_NORMALCOLOR:
    *(Color *)value = mat->normalcolor;
    break;
  case MT_OVERRIDE:
  case MT_NOOVERRIDE:
    *(int *) value = mat->override;
    break;
  case MT_VALID:
  case MT_INVALID:
    *(int *) value = mat->valid;
    break;
  default:
    OOGLError (0, "MtGet: undefined option: %d\n", attr);
    return -1;	
    break; 
  }
    
  return 1;
}

void
MtDelete(Material *mat)
{
  if (mat && RefDecr((Ref *)mat) <= 0) {
    if (mat->magic != MATMAGIC) {
      OOGLError(1, "MtDelete(%x) of non-Material: magic %x != %x",
		mat, mat->magic, MATMAGIC);
      return;
    }
    mat->magic = MATMAGIC ^ 0x80000000;
    OOGLFree(mat);
  }
}

Material *
MtDefault( Material *mat )
{
  memset(mat, 0, sizeof(Material));
  RefInit((Ref *)mat, MATMAGIC);
  mat->valid = mat->override = 0;
  mat->diffuse.a = 1.0;
  mat->Private = 0;
  mat->changed = 1;
  return mat;
}


Material *
MtCopy( Material *src, Material *dst )
{
  if (!src) return NULL;
  if (dst == NULL)
    dst = OOGLNewE(Material, "MtCopy: Material");
  *dst = *src;	
  dst->Private = 0;
  RefInit((Ref *)dst, MATMAGIC);
  dst->changed = 1;
  return dst;
}

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#if 0
static void
norm( color, coeff )
     Color *color;
     float *coeff;
{
  *coeff = max(color->r, color->g);
  *coeff = max(color->b, *coeff);

  if ( *coeff != 0.0 ) {
    color->r /= *coeff;
    color->g /= *coeff;
    color->b /= *coeff;
  }
}
#endif

/*
 * MtMerge(src, dst, mergeflags)
 * Merge Material values:  src into dst, controlled by flag.
 * If "inplace" is true, changes are made in dst itself; otherwise,
 * the dst material is copied if any changes need be made to it.
 * The returned Material's reference count is incremented as appropriate;
 * thus the caller should MtDelete() the returned Material when done.
 */
Material *
MtMerge(Material *src, Material *dst, int mergeflags)
{
  int mask;

  if (dst == NULL)
    return MtCopy(src, NULL);

  /* Fields to merge in */
  mask = src ?
    (mergeflags & APF_OVEROVERRIDE) ?
    src->valid : src->valid & ~(dst->override &~ src->override)
    : 0;

  if (mergeflags & APF_INPLACE)
    RefIncr((Ref *)dst);
  else
    dst = MtCopy(dst, NULL);

  if (mask == 0)			/* No changes to dst */
    return dst;

  dst->changed |= src->changed;
  dst->valid = (src->valid & mask) | (dst->valid & ~mask);
  dst->override = (src->override & mask) | (dst->override & ~mask);
  if (mask & MTF_EMISSION) dst->emission = src->emission;
  if (mask & MTF_AMBIENT) dst->ambient = src->ambient;
  if (mask & MTF_DIFFUSE) {
    dst->diffuse.r = src->diffuse.r;
    dst->diffuse.g = src->diffuse.g;
    dst->diffuse.b = src->diffuse.b;
  }
  if (mask & MTF_SPECULAR) dst->specular = src->specular;
  if (mask & MTF_Ka) dst->ka = src->ka;
  if (mask & MTF_Kd) dst->kd = src->kd;
  if (mask & MTF_Ks) dst->ks = src->ks;
  if (mask & MTF_ALPHA) dst->diffuse.a = src->diffuse.a;
  if (mask & MTF_SHININESS) dst->shininess = src->shininess;
  if (mask & MTF_EDGECOLOR) dst->edgecolor = src->edgecolor;
  if (mask & MTF_NORMALCOLOR) dst->normalcolor = src->normalcolor;
  return dst;
}

int MtSave(Material *mat, char *name)
{
  FILE *f;
  int ok;

  f = fopen(name,"w");
  if (!f) {
    perror(name);
    return -1;
  }
  ok = MtFSave(mat, f, NULL);
  fclose(f);
  return ok;
}

Material *MtLoad(Material *mat, char *name)
{
    IOBFILE *f = iobfopen(name,"rb");

    if (f == NULL) {
	OOGLError(0, "MtLoad: can't open %s: %s", name, sperror());
	return NULL;
    }

    mat = MtFLoad(mat, f, name);
    iobfclose(f);
    return mat;
}

/*
 * Load Material from file.
 * Syntax:
 *	< "filename_containing_material"	[or]
 *    {   keyword  value   keyword  value   ...  }
 *
 *   Each keyword may be prefixed by "*", indicating that its value should
 *   override corresponding settings in child objects.  [By default,
 *   children's appearance values supercede those of their parents.]
 *
 *  Note: don't overwrite ka, kd, ks if they're already set when we read in 
 *        the corresponding color.
 */
static char *mt_kw[] = {
  "shininess",	"ka",		"kd",		"ks",		"alpha",
  "backdiffuse", "emission",	"ambient",	"diffuse",	"specular",
  "edgecolor",	"normalcolor",	"material"
};
static unsigned short mt_flags[] = {
  MTF_SHININESS,  MTF_Ka,	MTF_Kd,		MTF_Ks,		MTF_ALPHA,
  MTF_EMISSION,   MTF_EMISSION, MTF_AMBIENT, MTF_DIFFUSE, MTF_SPECULAR,
  MTF_EDGECOLOR, MTF_NORMALCOLOR, 0
};
static char mt_args[] = { 1,1,1,1,1,  3,3,3,3,3,3,3, 0 };

/* fname used for error msgs, may be NULL */
Material *
MtFLoad(Material *mat, IOBFILE *f, char *fname)
{
  char *w;
  int i;
  float v[3];
  int brack = 0;
  int over, not;
  int got;
  Material m;

  MtDefault(&m);

  over = not = 0;
  for (;;) {
    switch(iobfnextc(f, 0)) {
    case '<':
      iobfgetc(f);
      if (MtLoad(&m, iobfdelimtok("{}()", f, 0)) == NULL) return NULL;
      if (!brack) goto done;
      break;
    case '{': brack++; iobfgetc(f); break;
    case '}': if (brack) { iobfgetc(f); } goto done;
    case '*': over = 1; iobfgetc(f); break;		/* 'override' prefix */
    case '!': not = 1; iobfgetc(f); break;
    default:
      w = iobfdelimtok("{}()", f, 0);
      if (w == NULL)
	return MtCopy(&m, mat);
      /* break;	*/				/* done */

      for (i = sizeof(mt_kw)/sizeof(mt_kw[0]); --i >= 0; )
	if (!strcmp(w, mt_kw[i]))
	  break;

      if ( i < 0) {
	OOGLError(1, "MtFLoad: %s: unknown material keyword %s",fname,w);
	return NULL;
      } else if ( !not && (got=iobfgetnf(f, mt_args[i], v, 0)) != mt_args[i] ) {
	OOGLError(1, "MtFLoad: %s: \"%s\" expects %d values, got %d",
		  fname, w, mt_args[i], got);
	return NULL;
      }

      if (not) {
	if (!over) m.valid &= ~mt_flags[i];
	m.override &= ~mt_flags[i];
      } else {
	switch(i) {
	case 0: m.shininess = v[0]; break;
	case 1: m.ka = v[0]; break;
	case 2: m.kd = v[0]; break;
	case 3: m.ks = v[0]; break;
	case 4: m.diffuse.a = v[0]; break;
	case 5: case 6: memcpy(&m.emission, v, sizeof(Color)); break;
	case 7: memcpy(&m.ambient, v, sizeof(Color)); break;
	case 8: memcpy(&m.diffuse, v, sizeof(Color)); break;
	case 9: memcpy(&m.specular, v, sizeof(Color)); break;
	case 10: memcpy(&m.edgecolor, v, sizeof(Color)); break;	 
	case 11: memcpy(&m.normalcolor, v, sizeof(Color)); break;
	}
	m.valid |= mt_flags[i];
	if (over) m.override |= mt_flags[i];
      }
      over = not = 0;
    }
  }
 done:
  return MtCopy(&m, mat);
}

int MtFSave(Material *mat, FILE *f, Pool *p)
{
  int i;
  float v;
  Color *c;

  for (i = 0; i < (int)(sizeof(mt_kw)/sizeof(mt_kw[0])); i++) {
    if (Apsavepfx(mat->valid, mat->override, mt_flags[i], mt_kw[i], f, p)) {
      switch(mt_flags[i]) {
      case MTF_Ka: v = mat->ka; goto pfloat;
      case MTF_Kd: v = mat->kd; goto pfloat;
      case MTF_Ks: v = mat->ks; goto pfloat;
      case MTF_SHININESS: v = mat->shininess; goto pfloat;
      case MTF_ALPHA: v = mat->diffuse.a; goto pfloat;
      pfloat:
	fprintf(f, "%f\n", v);
	break;

      case MTF_DIFFUSE: c = (Color *)(void *)&mat->diffuse; goto pcolor;
      case MTF_AMBIENT: c = &mat->ambient; goto pcolor;
      case MTF_EMISSION: c = &mat->emission; goto pcolor;
      case MTF_SPECULAR: c = &mat->specular; goto pcolor;
      case MTF_EDGECOLOR: c = &mat->edgecolor; goto pcolor;
      case MTF_NORMALCOLOR: c = &mat->normalcolor; goto pcolor;
      pcolor:
	fprintf(f, "%f %f %f\n", c->r, c->g, c->b);
	break;
      }
    }
  }
  return ferror(f);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
