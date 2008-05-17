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

/*
 *	light -
 *		Support for describing light sources and lists of 
 *		light sources.
 *
 *	Pat Hanrahan 1989
 */
#include "appearance.h"
#include "ooglutil.h"

static inline void
norm(Color *color, float *coeff);

DEF_FREELIST(LtLight);

#if 0
#define	NewLtLight()	OOGLNewE(LtLight, "new LtLight")
#define	FreeLtLight(lt)	OOGLFree(lt)
#else
static inline LtLight* NewLtLight(void)
{
    LtLight *newlt;
    
    FREELIST_NEW(LtLight, newlt);
    
    return newlt;
}
static inline void FreeLtLight(LtLight *old)
{
    FREELIST_FREE(LtLight, old);
}

#endif

/*
 * Default light is full white and position along the z-axis.
 */

static Color black = { 0.0, 0.0, 0.0 };

LtLight *
_LtSet(LtLight *light, int a1, va_list *alist)
{
    
    int attr;

#define NEXT(type) va_arg(*alist, type)
    
    if (light == NULL) {
      /*
       * New LtLight created here.
       */
      light = NewLtLight();
      LtDefault(light);
    }
    
    for(attr = a1; attr != LT_END; attr = NEXT(int)) {
	switch (attr) { /* parse argument list */
	  case LT_AMBIENT:
	    light->ambient = *NEXT(Color *);
	    light->changed = 1;
	    break;
	  case LT_COLOR:
	    light->color = *NEXT(Color *);
	    light->changed = 1;
	    break;
	  case LT_POSITION:
	    light->position = *NEXT(HPoint3 *);
	    light->changed = 1;
	    break;
	  case LT_INTENSITY:
	    light->intensity = NEXT(double);
	    light->changed = 1;
	    break;
	  case LT_LOCATION:
	    light->location = NEXT(int);
	    light->changed = 1;
	    break;
	  default:
	    OOGLError (0, "_LtSet: undefined option: %d\n", attr);
	    return NULL;
	    break;
	}
    }
    return light;
#undef NEXT
}

LtLight *
LtCreate(int a1, ... )
{
    va_list alist;
    LtLight *light;
    
    va_start(alist,a1);
    light = _LtSet(NULL, a1, &alist);
    va_end(alist);
    return light;
}

LtLight *
LtSet(LtLight *light, int attr, ...)
{
    va_list alist;
    
    va_start(alist,attr);
    light = _LtSet(light,attr,&alist);
    va_end(alist);
    return light;
}

int
LtGet(LtLight *light, int attr, void * value)
{
    if (!light) return 0;

    switch (attr) { 
       case LT_AMBIENT:
	*(Color *) value = light->ambient;
	break;
      case LT_COLOR:
	*(Color *) value = light->color;
	break;
      case LT_POSITION:
	*(Point *) value = light->position;
	break;
      case LT_INTENSITY:
	*(double *) value = light->intensity;
	break;
      case LT_LOCATION:
	*(int *)value = light->location;
	break;
      default:
	OOGLError (0, "LtGet: undefined option: %d\n", attr);
	return -1;
	break;
      }
    return 1;
}

void
LtDelete(LtLight *l)
{
    if(l == NULL || RefDecr((Ref *)l) > 0)
	return;
    if(l->magic != LTMAGIC) {
	OOGLError(1, "LtDelete(%x) of non-light: magic %x != %x",
		l, l->magic, LTMAGIC);
	return;
    }
    l->magic = LTMAGIC ^ 0x80000000;
    FreeLtLight(l);
}

LtLight *
LtCopy( LtLight *l1, LtLight *l2 )
{
    if(l2 == NULL)
	l2 = NewLtLight();
    *l2 = *l1;		/* Don't reset the 'changed' & 'Private' fields */
    /* Reset private and changed flags */
    l2->Private = 0;
    l2->changed = 1;
    RefInit((Ref *)l2, LTMAGIC);
    return l2;
}

LtLight *
LtDefault( LtLight *light ) 
{
  static HPoint3 defposition = { 0.0, 0.0, 1.0, 0.0 };
  static Color deflight = { 1.0, 1.0, 1.0 };

  light->intensity = 1.0;
  light->ambient = black;
  light->color = deflight;
  light->position = defposition;
  light->location = LTF_GLOBAL;
  light->Private = 0;
  light->changed = 1;
  return light;
}

/*
 * Set the intensity, color and position of a light.
 */
void
LtProperties( LtLight *light, float intensity, Color *color, Point *position )
{
    light->intensity = intensity;
    light->color = *color;
    light->position = *position;
    light->location = LTF_GLOBAL;

    light->changed = 1;
}

/*
 * Load a lighting description from a file.
 */
LtLight *
  LtLoad(LtLight *li, char *name)
{
  IOBFILE *f;

  if(name == NULL || (f = iobfopen(name, "r")) == NULL) {
    OOGLError(1, "Can't find light file %s: %s", name, sperror());
    return NULL;
  }
  li = LtFLoad(li, f, name);
  iobfclose(f);
  return li;
}

/*
 * Load Light from file.
 * Syntax:
 *	< "filename_containing_material"	[or]
 *    {   keyword  value   keyword  value   ...  }
 *
 */

LtLight *
LtFLoad(LtLight *lite, IOBFILE *f, char *fname)
{
  char *w;
  int i;
  float v[4];
  int brack = 0;
  static char *lkeys[] = {
    "ambient", "color", "position", "location", "global", "camera", "local"
    };
  static short largs[] = { 3, 3, 4, 0, ~LTF_GLOBAL, ~LTF_CAMERA, ~LTF_LOCAL };
  int got;
  LtLight l;
  
  LtDefault(&l);
  
  for(;;) {
    switch(iobfnextc(f, 0)) {
    case '<':
      iobfgetc(f);
      if(LtLoad(&l, iobfdelimtok("(){}", f, 0)) == NULL) return NULL;
      if(!brack) goto done;
      break;
    case '{': brack++; iobfgetc(f); break;
    case '}': if(brack) { iobfgetc(f); } goto done;
    default:
      w = iobftoken(f, 0);
      if(w == NULL)
	goto done;
      
      for(i = sizeof(lkeys)/sizeof(lkeys[0]); --i >= 0; )
	if(!strcmp(w, lkeys[i]))
	  break;
      
      if( i < 0) {
	OOGLSyntax(f, "Reading light from %s: unknown keyword %s",fname,w);
	return NULL;
      } else if( largs[i] > 0 && (got=iobfgetnf(f, largs[i], v, 0)) != largs[i] ) {
	OOGLSyntax(f, "Reading light from %s: \"%s\" expects %d values, got %d",
		  fname, w, largs[i], got);
	return NULL;
      }
      switch(i) {
      case 0:
	  memcpy(&l.ambient, v, sizeof(Color));
	  break;
      case 1:
	  memcpy(&l.color, v, sizeof(Color));
	  norm( &l.color, &l.intensity );
	  break;
      case 2:
	  memcpy(&l.position, v, sizeof(HPoint3));
	  break;
      case 3:
	  break;
      default:
	  l.location = ~largs[i]; break;
      }
    }
  }
 done:
  lite = LtCopy(&l, lite);
  return lite;
}

void LtFSave(LtLight *l, FILE *f, Pool *p)
{
    PoolFPrint(p, f, "ambient %f %f %f\n",
	l->ambient.r,
	l->ambient.g,
	l->ambient.b);
    PoolFPrint(p, f, "color %f %f %f\n",
	l->intensity*l->color.r,
	l->intensity*l->color.g,
	l->intensity*l->color.b);
    PoolFPrint(p, f, "position %f %f %f %f\n",
	l->position.x,
	l->position.y,
	l->position.z,
	l->position.w);
    if(l->location != LTF_GLOBAL)
	PoolFPrint(p, f, "location %s\n",
		   l->location == LTF_CAMERA ? "camera" : "local");
    /*
    PoolFPrint(p, f,"intensity %f\n", la->intensity);
    */
}


/*
 * Create a list of lights and a lighting model.
 */
LmLighting *
_LmSet(LmLighting *lgt, int a1, va_list *alist)
{
    int attr;

#define NEXT(type) va_arg(*alist, type)

    if (!alist) return lgt;
    if (lgt == NULL) {
      /*
       * New Lighting created here.
       */
      lgt =  OOGLNewE(LmLighting, "LmCreate Lighting");
      LmDefault(lgt);
    }

    for(attr = a1; attr != LM_END; attr = NEXT(int)) {
      switch (attr) { /* parse argument list */
      case LM_LtSet:
	LmAddLight(lgt, _LtSet(NULL, va_arg(*alist, int), alist));
	break;
      case LM_LIGHT:
	LmAddLight(lgt, NEXT(LtLight *));
	break;
      case LM_REPLACELIGHTS:
	if (NEXT(int))
	  lgt->valid |= LMF_REPLACELIGHTS;
	else
	  lgt->valid &= ~LMF_REPLACELIGHTS;
	break;
      case LM_AMBIENT:
	lgt->ambient = *NEXT(Color *);
	lgt->valid |= LMF_AMBIENT;
	break;
      case LM_LOCALVIEWER:
	lgt->localviewer = NEXT(int);
	lgt->valid |= LMF_LOCALVIEWER;
	break;
      case LM_ATTENC:
	lgt->attenconst = NEXT(double);
	lgt->valid |= LMF_ATTENC;
	break;
      case LM_ATTENM:
	lgt->attenmult = NEXT(double);
	lgt->valid |= LMF_ATTENM;
	break;
      case LM_OVERRIDE:
	lgt->override |= NEXT(int);
	break;
      case LM_NOOVERRIDE:
	lgt->override &= ~NEXT(int);
	break;
      case LM_INVALID:
	lgt->valid &= ~NEXT(int);
	break;
      default:
	OOGLError (0, "_LmSet: undefined option: %d\n", attr);
	return NULL;
	break;
      }
    }

    return lgt;

#undef NEXT
}

LmLighting *
LmCreate(int attr, ... )
{
    va_list alist;
    LmLighting *lgt;
     
    va_start(alist,attr);
    lgt = _LmSet(NULL, attr, &alist);
    va_end(alist);
    return lgt;
}


LmLighting *
LmSet(LmLighting *lgt, int a1, ... )
{
    va_list alist;
    va_start(alist,a1);
    lgt = _LmSet(lgt, a1, &alist);
    va_end(alist);
    return lgt;
}


int
LmGet(LmLighting *lgt, int attr, void *value)
{
    if (!lgt) return 0;

    switch (attr) {
      case LM_LIGHT:
	*(LtLight ***) value = &lgt->lights[0];
	break;
      case LM_REPLACELIGHTS:
	*(int*)value = lgt->valid & LMF_REPLACELIGHTS;
	break;
      case LM_AMBIENT:
	*(Color *) value = lgt->ambient;
	break;
      case LM_LOCALVIEWER:
	*(double *) value = lgt->localviewer;
	break;
      case LM_ATTENC:
	*(double *) value = lgt->attenconst;
	break;
      case LM_ATTENM:
	*(double *) value = lgt->attenmult;
	break;
      case LM_ATTEN2:
	*(double *) value = lgt->attenmult2;
	break;
      case LM_OVERRIDE:
      case LM_NOOVERRIDE:
	*(int *) value = lgt->override;
	break;
      case LM_VALID:
      case LM_INVALID:
	*(int *) value = lgt->valid;
	break;
      default:
	OOGLError (0, "LmGet: undefined option: %d\n", attr);
	return -1;
	break;
      }
    return 1;
}

LmLighting *
LmMerge(LmLighting *src, LmLighting *dst, int mergeflags)
{
    unsigned int mask;

    if(dst == NULL)
	return LmCopy(src, NULL);

    mask = src ?
	(mergeflags & APF_OVEROVERRIDE) ?
		src->valid : src->valid & ~(dst->override &~ src->override)
	: 0;

    if(src == NULL || (mask == 0 && src->lights == NULL)) {
	RefIncr((Ref *)dst);
	return dst;
    }

    if(mask && !(mergeflags & APF_INPLACE))
	dst = LmCopy(dst, NULL);
    dst->changed |= src->changed;
    dst->valid = (src->valid & mask) | (dst->valid & ~mask);
    dst->override = (src->override & mask) | (dst->override & ~mask);
    if(mask & LMF_LOCALVIEWER) dst->localviewer = src->localviewer;
    if(mask & LMF_AMBIENT) dst->ambient = src->ambient;
    if(mask & LMF_ATTENC) dst->attenconst = src->attenconst;
    if(mask & LMF_ATTENM) dst->attenmult = src->attenmult;
    if(mask & LMF_ATTEN2) dst->attenmult2 = src->attenmult2;

    /* LMF_REPLACELIGHTS: replace lights rather than merging with them */
    if((mask & LMF_REPLACELIGHTS))
	LmDeleteLights(dst);

    if(LM_ANY_LIGHTS(src))
	LmCopyLights(src, dst);

    RefIncr((Ref *)dst);
    return dst;
}

LmLighting *
LmCopy(LmLighting *from, LmLighting *to)
{
    Ref r;

    if (from == NULL || from == to)
	return NULL;

    if(to == NULL) {
	to = OOGLNewE(LmLighting, "LmCopy LmLighting");
	RefInit((Ref *)&r, LIGHTINGMAGIC);
    } else {
	r = *(Ref *)to;
    }
    *to = *from;
    RefInit((Ref *)to, LIGHTINGMAGIC);
    to->Private = 0;
    memset(&to->lights, 0, AP_MAXLIGHTS*sizeof(LtLight *));
    LmCopyLights(from, to);
    *(Ref *)to = r;		/* Restore ref header */
    return to;
}

void
LmCopyLights(LmLighting *from, LmLighting *to)
{
    int i;
    LtLight **lp, *tmp;

    LM_FOR_ALL_LIGHTS(from, i, lp) {
	LmAddLight(to, tmp = LtCopy(*lp, NULL));
	LtDelete(tmp);
    }
}
  
void
LmDefault( LmLighting *l ) 
{
  RefInit((Ref *)l, LIGHTINGMAGIC);
  l->valid = l->override = 0;
  l->ambient = black;
  l->localviewer = 1;
  l->attenconst = 1.0;
  l->attenmult = 0.0;
  l->attenmult2 = 0.0;
  l->changed = 1;
  l->Private = 0;
  memset(&l->lights, 0, AP_MAXLIGHTS*sizeof(LtLight *));
}

/*
 * Delete a list of lights and all the lights in the list.
 */
void
LmDelete(LmLighting *lm)
{
    if(lm == NULL || RefDecr((Ref *)lm) > 0)
	return;
    if(lm->magic != LIGHTINGMAGIC) {
	OOGLError(1, "LmDelete(%x) of non-LmLighting: magic %x != %x",
		lm, lm->magic, LIGHTINGMAGIC);
	return;
    }
    LmDeleteLights(lm);
    lm->magic = LIGHTINGMAGIC ^ 0x80000000;
    OOGLFree(lm);
}

void
LmRemoveLight(LmLighting *lm, LtLight *lt)
{
    LtLight **lp;
    LtLight **found = NULL, **last = NULL;
    int i;
    LM_FOR_ALL_LIGHTS(lm, i,lp) {
	if(*lp == lt)
	    found = lp;
	last = lp;
    }
    if(found != NULL) {
	*found = *last;
	*last = NULL;
    }
}
void
LmAddLight(LmLighting *lm, LtLight *lt)
{
  LtLight **lp;
  int i;
  if(lt == NULL)
    return;

  LM_FOR_ALL_LIGHTS(lm, i,lp) {
    if(*lp == lt) {
	OOGLWarn("add dup light?");
	break;
    }
  }
  if(i < AP_MAXLIGHTS) {
    *lp = lt;
    RefIncr((Ref *)lt);
  } else {
    OOGLError(1, "Can't have more than AP_MAXLIGHTS (%d) lights.", AP_MAXLIGHTS);
  }
}

void
LmDeleteLights(LmLighting *lm)
{
    LtLight **lp;
    int i;
    LM_FOR_ALL_LIGHTS(lm, i,lp) {
	LtDelete(*lp);
	*lp = NULL;
    }
}

#ifndef max
# define max(a, b) ((a) > (b) ?( a) : (b))
#endif

static inline void
norm(Color *color, float *coeff)
{
    *coeff = max(color->r, color->g);
    *coeff = max(color->b, *coeff);

    if( *coeff != 0.0 ) {
	color->r /= *coeff;
	color->g /= *coeff;
	color->b /= *coeff;
    }
}


/*
 * Load Lighting from file.
 * Syntax:
 *	< "filename_containing_material"	[or]
 *    {   keyword  value   keyword  value   ...  }
 *
 *   Each keyword may be prefixed by "*", indicating that its value should
 *   override corresponding settings in child objects.  [By default,
 *   children's appearance values supercede those of their parents.]
 *
 */

LmLighting *
LmLoad(LmLighting *lgt, char *fname)
{
  IOBFILE *f = iobfopen(fname, "r");
  
  if(f == NULL)
    return NULL;
  lgt = LmFLoad(lgt, f, fname);
  iobfclose(f);
  return lgt;
}

LmLighting *
LmFLoad(LmLighting *lgt, IOBFILE *f, char *fname)
{
  char *w;
  int i;
  float v[3];
  int brack = 0;
  int over, not;
  static char *lkeys[] = {
    "ambient", "localviewer", "attenconst", "attenmult", "attenmult2", "light",
    "replacelights" 
    };
  static char largs[] = { 3, 1, 1, 1, 1, 0, 0};
  static unsigned short lbits[] = {
    LMF_AMBIENT, LMF_LOCALVIEWER, LMF_ATTENC, LMF_ATTENM, LMF_ATTEN2, 0, LMF_REPLACELIGHTS
    };
  int got;
  LmLighting l;

  if(lgt == NULL)
    lgt = LmCreate(LM_END);

  over = not = 0;

  for(;;) {
    switch(iobfnextc(f, 0)) {
    case '<':
      iobfgetc(f);
      if(LmLoad(&l, iobftoken(f, 0)) == NULL) return NULL;
      if(!brack) goto done;
      break;
    case '{': brack++; iobfgetc(f); break;
    case '}': if(brack) { iobfgetc(f); } goto done;
    case '*': over = 1; iobfgetc(f); break;		/* 'override' prefix */
    case '!': not = 1; iobfgetc(f); break;
    default:
      w = iobftoken(f, 0);
      if(w == NULL)
	return lgt;

      for(i = sizeof(lkeys)/sizeof(lkeys[0]); --i >= 0; )
	if(!strcmp(w, lkeys[i]))
	  break;
      
      if( i < 0) {
	OOGLError(1, "LmFLoad: %s: unknown lighting keyword %s",fname,w);
	return NULL;
      } else if( !not && (got=iobfgetnf(f, largs[i], v, 0)) != largs[i] ) {
	OOGLError(1, "LmFLoad: %s: \"%s\" expects %d values, got %d",
		  fname, w, largs[i], got);
	return NULL;
      }
      
      if(not) {
	if(!over) lgt->valid &= ~lbits[i];
	lgt->override &= ~lbits[i];
      } else {
	lgt->valid |= lbits[i];
	if(over) lgt->override |= lbits[i];
	switch(i) {
	case 0: memcpy(&lgt->ambient, v, sizeof(Color)); break;
	case 1: lgt->localviewer = v[0]; break;
	case 2: lgt->attenconst = v[0]; break;
	case 3: lgt->attenmult = v[0]; break;
	case 4: lgt->attenmult2 = v[0]; break;
	case 5:
	  LmAddLight( lgt, LtFLoad( NULL, f, fname ) );
	  break;
	}
      }
      over = not = 0;
    }
  }
 done:
  return lgt;
}


/*
 * Save a light description in a file.
 */

void
LmFSave(LmLighting *li, FILE *f, char *fname, Pool *p)
{
    LtLight **lp;
    int i;

    (void)fname;

    PoolFPrint(p, f, "ambient %.8g %.8g %.8g\n", 
	li->ambient.r,
	li->ambient.g,
	li->ambient.b);
    PoolFPrint(p, f, "localviewer %d\n", li->localviewer);
    PoolFPrint(p, f, "attenconst %.8g\n", li->attenconst);
    PoolFPrint(p, f, "attenmult %.8g\n", li->attenmult);
    if(li->valid & LMF_ATTEN2) {
	PoolFPrint(p, f,"attenmult2 %.8g\n", li->attenmult2);
    }
    if (li->valid & LMF_REPLACELIGHTS) {
	PoolFPrint(p, f, "replacelights\n");
    }
    LM_FOR_ALL_LIGHTS(li, i,lp) {
	PoolFPrint(p, f, "light {\n");
	PoolIncLevel(p, 1);	
	LtFSave( *lp, f, p );	
	PoolIncLevel(p, -1);
	PoolFPrint(p, f, "}\n");
    }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
