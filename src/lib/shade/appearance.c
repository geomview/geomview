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

#include "appearance.h"
#include "transobj.h"
#include <string.h>

#define MAXSHININESS 1024.0

Texture *AllLoadedTextures = NULL;

extern LmLighting * _LmSet(LmLighting *, int attr1, va_list *); /* Forward */
Appearance *ApLoad(Appearance *into, char *fname);
Material * _MtSet(Material *, int attr1, va_list *);	/* Forward */
int Apsavepfx(int valid, int override, int mask, char *keyword, FILE *f);


Appearance *
_ApSet(Appearance *ap, int attr1, register va_list *alist)
{
    long mask;
    int attr;

#define NEXT(type) va_arg(*alist, type)

    if (ap == NULL) {
        /*
         * New Appearance created here.
         */
	ap = OOGLNewE(Appearance, "ApCreate Appearance");
	ApDefault(ap);
    }

    for ( attr = attr1; attr != AP_END; attr = NEXT(int)) {
	switch (attr) { /* parse argument list */
	  case AP_DO:
	    mask = NEXT(int);
	    ap->flag |= mask;
	    ap->valid |= mask;
	    break;
	  case AP_DONT:
	    mask = NEXT(int);
	    ap->flag &= ~mask;
	    ap->valid |= mask;
	    break;
	  case AP_INVALID:
	    ap->valid &= ~NEXT(int);
	    break;
	  case AP_OVERRIDE:
	    ap->override |= NEXT(int);
	    break;
	  case AP_NOOVERRIDE:
	    ap->override &= ~NEXT(int);
	    break;
	  case AP_MAT:
	    ap->mat = NEXT(Material*);
	    break;
	  case AP_MtSet:
	    ap->mat = _MtSet(ap->mat, va_arg(*alist, int), alist);
	    break;
	  case AP_LGT:
	    ap->lighting = NEXT(LmLighting*);
	    break;
	  case AP_LmSet:
	    if (!ap->lighting) ap->lighting = LmCreate(LM_END);
	    ap->lighting = _LmSet(ap->lighting, va_arg(*alist, int), alist);
	    break;
	  case AP_NORMSCALE:
	    ap->nscale = NEXT(double);
	    ap->valid |= APF_NORMSCALE;
	    break;
	  case AP_LINEWIDTH:
	    ap->linewidth = NEXT(int);
	    ap->valid |= APF_LINEWIDTH;
	    break;
	  case AP_SHADING:
	    /* should be APF_{CONSTANT,FLAT,SMOOTH} */
	    ap->shading = NEXT(int);
	    ap->valid |= APF_SHADING;
	    break;
	  case AP_DICE:
	    ap->dice[0] = NEXT(int);
	    ap->dice[1] = NEXT(int);
	    ap->valid |= APF_DICE;
	    break;
	  default:
	    OOGLError (0, "_ApSet: undefined option: %d\n", attr);
	    return NULL;
	    break; 
	}
    }
    return ap;

#undef NEXT
}


Appearance *
ApCreate(int a1, ...)
{
    va_list alist;
    Appearance *ap;

    va_start(alist,a1);
    ap = _ApSet(NULL, a1, &alist);
    va_end(alist);
    return ap;
}


Appearance *
ApSet(Appearance *ap, int a1, ... )
{
    va_list alist;
    va_start(alist,a1);
    ap = _ApSet(ap, a1, &alist);
    va_end(alist);
    return ap;
}

int
ApGet(Appearance *ap, int attr, void *value)
{
    if (!ap) return -1;

    switch (attr) {
      case AP_DO:
      case AP_DONT:	*(int *) value =  ap->flag; break;
      case AP_OVERRIDE:
      case AP_NOOVERRIDE: *(int *) value = ap->override; break;
      case AP_VALID:
      case AP_INVALID:	*(int *) value = ap->valid; break;
      case AP_MAT:	*(Material **) value = ap->mat; break;
      case AP_LGT:	*(LmLighting **) value = ap->lighting; break;
      case AP_NORMSCALE: *(double *) value = ap->nscale; break;
      case AP_LINEWIDTH: *(int *) value = ap->linewidth; break;
      case AP_SHADING:	*(int *)value = ap->shading; break;
      case AP_DICE:	((int *)value)[0] = ap->dice[0];
			((int *)value)[1] = ap->dice[1];
			break;
      default:
	OOGLError (0, "ApGet: undefined option: %d\n", attr);
	return -1;
	break; 
    }
    return attr;
}

void
ApDelete(Appearance *ap)
{
    if(ap == NULL || RefDecr((Ref *)ap) > 0)
	return;
    if(ap->magic != APMAGIC) {
	OOGLError(1, "ApDelete(%x) of non-Appearance: magic %x != %x",
		ap, ap->magic, APMAGIC);
	return;
    }
    if (ap->mat) MtDelete(ap->mat);
    if (ap->lighting) LmDelete(ap->lighting);
    if (ap->tex) TxDelete(ap->tex);
    
    ap->magic = APMAGIC ^ 0x80000000;
    OOGLFree(ap);
}

/*
 * Copies just the Appearance part, not its Material and LmLighting children.
 * Pointers to the latter are retained BUT their reference counts are NOT
 * incremented.  The caller MUST either RefIncr() or reassign mat and lighting.
 */
Appearance *
ApCopyShallow( Appearance *ap, register Appearance *into )
{
    if(ap == NULL)
	return NULL;
    if(into == NULL) {
	into = OOGLNewE(Appearance, "ApCopy: Appearance");
	*into = *ap;
	into->mat = NULL;
	into->backmat = NULL;
	into->lighting = NULL;
	into->tex = NULL;
	RefInit((Ref *)into, APMAGIC);
    } else {
	into->flag = ap->flag;
	into->valid = ap->valid;
	into->override = ap->override;
	into->nscale = ap->nscale;
	into->linewidth = ap->linewidth;
	into->shading = ap->shading;
	into->dice[0] = ap->dice[0];
	into->dice[1] = ap->dice[1];
    }
    return into;
}
    
Appearance *
ApCopy( Appearance *ap, register Appearance *into )
{
    if (ap == NULL) return into;
    into = ApCopyShallow( ap, into );
    if(ap->mat) into->mat = MtCopy(ap->mat, into->mat);
    if(ap->backmat) into->backmat = MtCopy(ap->backmat, into->backmat);
    if(ap->lighting) into->lighting = LmCopy(ap->lighting, into->lighting);
    if(ap->tex) into->tex = TxCopy(ap->tex, into->tex);
    return into;
}

Appearance *
ApCopyShared( Appearance *ap, register Appearance *into )
{
    if (ap == NULL) return into;
    into = ApCopyShallow( ap, into );
    if(ap->mat) {
	if(into->mat) MtCopy(ap->mat, into->mat);
	else RefIncr((Ref *)(into->mat = ap->mat));
    }
    if(ap->backmat) {
	if(into->backmat) MtCopy(ap->backmat, into->backmat);
	else RefIncr((Ref *)(into->backmat = ap->backmat));
    }
    if(ap->lighting) {
	if(into->lighting) LmCopy(ap->lighting, into->lighting);
	else RefIncr((Ref *)(into->lighting = ap->lighting));
    }
    if(ap->tex) {
	if(into->tex) TxCopy(ap->tex, into->tex);
	else RefIncr((Ref *)(into->tex = ap->tex));
    }
    return into;
}

/*
 * Merges appearance properties of src into dst.
 * If "inplace" is true, data are modified in place.
 * Otherwise if any modifications are needed to dst, a copy is made & returned.
 * If no modifications are necessary, the returned Appearance's reference
 * count is still incremented.
 * Thus, in all cases, the caller should ApDelete() the returned value
 * when finished with it.
 */
Appearance *
ApMerge( register Appearance *src, register Appearance *dst, int mergeflags )
{
    int mask;
    Material *mt, *bmt;
    LmLighting *lts;
    Texture *tex;
 

    if(dst == NULL)
	return ApCopy(src, NULL);

    if(src == NULL) {
	RefIncr((Ref *)dst);
	return dst;
    }
    /* Mask of fields to copy to dst */
    mask = (mergeflags & APF_OVEROVERRIDE) ? src->valid :
			    src->valid &~ ( dst->override &~ src->override);

    mt = MtMerge(src->mat, dst->mat, mergeflags);
    bmt = MtMerge(src->backmat, dst->backmat, mergeflags);
    lts = LmMerge(src->lighting, dst->lighting, mergeflags);
    tex = TxMerge(src->tex, dst->tex, mergeflags);
    if((mergeflags & APF_INPLACE)
		 || (mask == 0 && mt == dst->mat &&
			lts == dst->lighting && bmt == dst->backmat
			&& tex == dst->tex)) {
	/*
	 * No changes, or we're asked to merge in place.  Bump ref count.
	 */
	RefIncr((Ref *)dst);
    } else {
	/*
	 * Another special case: Copy appearance, don't copy the items already
	 * copied by {Mt,Lm}Merge.  We're about to overwrite these values, so
	 * toss the old ones.  Pretty kludgy, but what can we do?
	 */
	dst = ApCopyShallow(dst, NULL);
    }
    if(mt != dst->mat) MtDelete(dst->mat);
    if(bmt != dst->backmat) MtDelete(dst->backmat);
    if(lts != dst->lighting) LmDelete(dst->lighting);
    if(tex != dst->tex && dst->tex) TxDelete(dst->tex);

    dst->mat = mt;
    dst->backmat = bmt;
    dst->lighting = lts;
    dst->tex = tex;
    if(mask) {
	/* Merge together appearance-specific data */
	dst->flag = (src->flag & mask) | (dst->flag & ~mask);
	dst->valid = (src->valid & mask) | (dst->valid & ~mask);
	dst->override = (src->override & mask) | (dst->override & ~mask);
	if(mask & APF_NORMSCALE) dst->nscale = src->nscale;
	if(mask & APF_LINEWIDTH) dst->linewidth = src->linewidth;
	if(mask & APF_SHADING) dst->shading = src->shading;
	if(mask & APF_DICE) {
	    dst->dice[0] = src->dice[0];
	    dst->dice[1] = src->dice[1];
	}
    }
    return dst;
}

static struct {
	char *word;
	int amask;
	int aval;
} ap_kw[] = {
	{ "appearance",	0,		1 },
	{ "face",	APF_FACEDRAW,	0 },
	{ "edge",	APF_EDGEDRAW,	0 },
	{ "vect",	APF_VECTDRAW,	0 },
	{ "transparent", APF_TRANSP,	0 },
	{ "evert",	APF_EVERT,	0 },
	{ "keepcolor",	APF_KEEPCOLOR,	0 },
	{ "texturing",	APF_TEXTURE,	0 },
	{ "backcull",	APF_BACKCULL,	0 },
	{ "shadelines",	APF_SHADELINES,	0 },
	{ "concave",	APF_CONCAVE,	0 },
	{ "shading",	APF_SHADING,	2 },
	{ "smooth",	APF_SHADING,	AP_SHADING + APF_SMOOTH },
	{ "flat",	APF_SHADING,	AP_SHADING + APF_FLAT },
	{ "constant",	APF_SHADING,	AP_SHADING + APF_CONSTANT },
	{ "csmooth",	APF_SHADING,	AP_SHADING + APF_CSMOOTH },
	{ "mipmap",	APF_TXMIPMAP,	0 },
	{ "mipinterp",	APF_TXMIPINTERP, 0 },
	{ "linear",	APF_TXLINEAR,   0 },
	{ "normal",	APF_NORMALDRAW,	0 },
	{ "normscale",	APF_NORMSCALE,  3 },
	{ "linewidth",	APF_LINEWIDTH,	4 },
	{ "material",	0,		5 },
	{ "backmaterial", 0,		7 },
	{ "patchdice",	APF_DICE,	9 },
	{ "light",	0,		6 },
	{ "lighting",	0,		6 },
	{ "texture",	0,		8 },
};

Appearance *
ApFLoad( Appearance *into, IOBFILE *stream, char *fname )
{
    register Appearance *ap;
    char *w;
    int i;
    int brack = 0;
    int over, not, value;
    int mask, flagmask;
    int more;
    int mine = 1;	/* Questionable -- we'll report all errors */

    ap = into;

    over = not = 0;  value = ~0;
    more = 0;
    do {
	(void) iobfnextc(stream, 0);

	switch(i = iobfgetc(stream)) {
	case '<': ap = ApLoad(ap, iobfdelimtok("{}()", stream, 0)); break;
	case EOF: brack = 0; break;
	case '{': brack++; break;
	case '}': if(brack-- <= 0) { iobfungetc(i, stream); } break;
	case '+': value = ~0; break;
	case '-': value = 0; break;
	case '!': not = 1; break;
	case '*': over = 1; break;

	default:
	    more = 0;
	    iobfungetc(i, stream);
	    w = iobfdelimtok("{}()", stream, 0);
	    if(w == NULL)
		break;
	    for(i = sizeof(ap_kw)/sizeof(ap_kw[0]); --i >= 0; )
		if(!strcmp(ap_kw[i].word, w))
		    break;
		
	    if(i < 0) {
		if(mine)
		    OOGLError(1,
			"ApFLoad: file %s: unknown appearance keyword %s", 
			fname, w);
		return NULL;
	    }
	    if(ap == NULL)
		ap = ApCreate(AP_END);

	    mask = flagmask = ap_kw[i].amask;
	    if(not) {
		switch(ap_kw[i].aval) {
		    case 5: MtDelete(ap->mat); ap->mat = NULL; break;
		    case 6: LmDelete(ap->lighting); ap->lighting = NULL; break;
		    case 8: TxDelete(ap->tex); ap->tex = NULL; break;
		}
		ap->flag &= ~mask;
		if(!over) ap->valid &= ~mask;
		ap->override &= ~mask;
	    } else {
		switch(ap_kw[i].aval) {
		case 0: break;
		case 1: mine = more = 1; break;
		case 2: iobfgetni(stream, 1, &ap->shading, 0); break;
		case 3:
		    if(iobfgetnf(stream, 1, &ap->nscale, 0) <= 0)
			OOGLError(1,"ApFLoad: %s: \"normscale\": value expected",
				fname);
		    break;
		case 4:
		    if(iobfgetni(stream, 1, &ap->linewidth, 0) <= 0)
			OOGLSyntax(stream, "%s \"linewidth\": value expected",
				fname);
		    break;
		case 5:
		    if((ap->mat = MtFLoad(ap->mat, stream, fname)) == NULL)
			OOGLSyntax(stream,"Can't read material in %s",
				fname);
		    break;
		case 6:
		    ap->lighting = LmFLoad(ap->lighting, stream, fname);
		    if(ap->lighting == NULL)
			OOGLError(1,"Can't read lighting, file %s",
				fname);
		    break;
		case 7:
		    if((ap->backmat = MtFLoad(ap->backmat, stream, fname)) == NULL)
			OOGLError(1,"Can't read backmaterial, file %s",
				fname);
		    break;
		case 8:
		    TxDelete(ap->tex);
		    if((ap->tex = TxFLoad(stream, fname)) == NULL)
			OOGLError(1, "%s: Can't read texture", fname);
		    break;
		case 9:
		    if(iobfgetns(stream, 2, ap->dice, 0) < 2)
			OOGLSyntax(stream,
			 "%s \"patchdice\": expected integer u- and v- dicing values",
			  fname);
		    break;
		default:
		    if(ap_kw[i].aval >= AP_SHADING)
			ap->shading = ap_kw[i].aval - AP_SHADING;
		}
		if(value) ap->flag |= flagmask;
		else ap->flag &= ~flagmask;
		ap->valid |= mask;
		if(over) ap->override |= mask;
	    }
	    /* Reset for next keyword */
	    over = not = 0;  value = ~0;

	}
    } while(brack > 0 || more);
    return ap;
}

Appearance *
ApLoad(Appearance *into, char *fname)
{
    IOBFILE *f = iobfopen(fname, "rb");
    Appearance *a;

    if(f == NULL) {
	OOGLError(0, "ApLoad: can't open %s: %s", fname, sperror());
	return NULL;
    }
    a = ApFLoad(into, f, fname);
    iobfclose(f);
    return a;
}


void ApFSave( Appearance *ap, Handle *aphandle, FILE *f, char *fname )
{
    int valid;
    int mask;
    register int i;
    Pool *p;

    if(ap == NULL)
	return;

    valid = ap->valid;
    fprintf(f, "appearance {\n");

    for(i = 0; i < sizeof(ap_kw)/sizeof(ap_kw[0]); i++) {
	mask = ap_kw[i].amask;
	if((valid & mask) == 0)
	    continue;
	Apsavepfx(valid, ap->override, mask, "", f);
	if(ap_kw[i].aval == 0) {
	    if((mask & ap->flag) == 0)
		fputc('-', f);
	    fputs(ap_kw[i].word, f);
	}
	valid &= ~mask;
	switch(mask) {
	case APF_SHADING:
	    fputs(" shading ", f);
	    switch(ap->shading) {
	    case APF_SMOOTH:	fputs("smooth", f);	break;
	    case APF_FLAT:	fputs("flat", f);	break;
	    case APF_CONSTANT:	fputs("constant", f);	break;
	    case APF_CSMOOTH:   fputs("csmooth", f);	break;
	    default:		fprintf(f, "%d", ap->shading); break;
	    }
	    break;
	case APF_NORMSCALE:
	    fprintf(f, "  normscale %g", ap->nscale);
	    break;
	case APF_LINEWIDTH:
	    fprintf(f, "  linewidth %d ", ap->linewidth);
	    break;
	case APF_DICE:
	    fprintf(f, "  patchdice %d %d", ap->dice[0], ap->dice[1]);
	    break;
	}
	fputc('\n', f);
    }

    if(ap->mat) {
	fprintf(f, "  material {\n");
	MtFSave(ap->mat, f);
	fprintf(f, "  }\n");
    }
    if(ap->backmat) {
	fprintf(f, "  backmaterial {\n");
	MtFSave(ap->backmat, f);
	fprintf(f, "  }\n");
    }
    if(ap->lighting) {
	fprintf(f, "  lighting {\n");
	LmFSave(ap->lighting, f, fname);
	fprintf(f, "  }\n");
    }
    if(ap->tex) {
        p = PoolStreamTemp(fname, NULL, f, 1, &TextureOps);
	TxStreamOut(p, NULL, ap->tex);
	PoolDelete(p);
    }
    fprintf(f, "}\n");
    return;
}

void
ApSave(Appearance *ap, char *fname)
{
    FILE *f = fname == NULL ? stdout : fopen(fname, "w");

    if(f) {
	ApFSave(ap, NULL, f, fname);
	if(fname != NULL)
	    fclose(f);
    }
}

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

    for(attr = attr1; attr != MT_END; attr = NEXT(int)) {
      switch (attr) { /* parse argument list */
      case MT_AMBIENT:
	mat->ambient = *NEXT(Color *);
	mat->valid |= MTF_AMBIENT;
	break;
      case MT_DIFFUSE:
	*(Color *)(void *)&mat->diffuse = *NEXT(Color *);
	mat->valid |= MTF_DIFFUSE;
	break;
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
MtGet(register Material *mat, int attr, void * value)
{
    if (mat == NULL)
	return -1;

    switch (attr) {
      case MT_AMBIENT:
	*(Color *) value = mat->ambient;
	break;
      case MT_DIFFUSE:
	*(Color *) value = *(Color *)(void *)&mat->diffuse;
	break;
      case MT_SPECULAR:
	*(Color *) value = mat->specular;
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
    if(mat && RefDecr((Ref *)mat) <= 0) {
	if(mat->magic != MATMAGIC) {
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
    if(dst == NULL)
      dst = OOGLNewE(Material, "MtCopy: Material");
    *dst = *src;	
    dst->Private = 0;
    RefInit((Ref *)dst, MATMAGIC);
    dst->changed = 1;
    return dst;
}

#define max(a,b) (a)>(b)?(a):(b)

#if 0
static void
norm( color, coeff )
    Color *color;
    float *coeff;
{
    *coeff = max(color->r, color->g);
    *coeff = max(color->b, *coeff);

    if( *coeff != 0.0 ) {
	color->r /= *coeff;
	color->g /= *coeff;
	color->b /= *coeff;
    }
}
#endif

Material *MtLoad(Material *mat, char *name)
{
    IOBFILE *f = iobfopen(name,"rb");

    if(f == NULL) {
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
    register int i;
    float v[3];
    int brack = 0;
    int over, not;
    int got;
    Material m;

    MtDefault(&m);

    over = not = 0;
    for(;;) {
	switch(iobfnextc(f, 0)) {
	case '<':
	    iobfgetc(f);
	    if(MtLoad(&m, iobfdelimtok("{}()", f, 0)) == NULL) return NULL;
	    if(!brack) goto done;
	    break;
	case '{': brack++; iobfgetc(f); break;
	case '}': if(brack) { iobfgetc(f); } goto done;
	case '*': over = 1; iobfgetc(f); break;		/* 'override' prefix */
	case '!': not = 1; iobfgetc(f); break;
	default:
	    w = iobfdelimtok("{}()", f, 0);
	    if(w == NULL)
		return MtCopy(&m, mat);
		/* break;	*/				/* done */

	    for(i = sizeof(mt_kw)/sizeof(mt_kw[0]); --i >= 0; )
		if(!strcmp(w, mt_kw[i]))
		    break;

	    if( i < 0) {
		OOGLError(1, "MtFLoad: %s: unknown material keyword %s",fname,w);
		return NULL;
	    } else if( !not && (got=iobfgetnf(f, mt_args[i], v, 0)) != mt_args[i] ) {
		OOGLError(1, "MtFLoad: %s: \"%s\" expects %d values, got %d",
		    fname, w, mt_args[i], got);
		return NULL;
	    }

	    if(not) {
		if(!over) m.valid &= ~mt_flags[i];
		m.override &= ~mt_flags[i];
	    } else {
		switch(i) {
		case 0: m.shininess = v[0]; break;
		case 1: m.ka = v[0]; break;
		case 2: m.kd = v[0]; break;
		case 3: m.ks = v[0]; break;
		case 4: m.diffuse.a = v[0]; break;
		case 5: case 6: m.emission = *(Color *)(void *)v; break;
		case 7: m.ambient = *(Color *)(void *)v; break;
		case 8: *(Color *)(void *)&m.diffuse = *(Color *)(void *)v; break;
		case 9: m.specular = *(Color *)(void *)v; break;
		case 10: m.edgecolor = *(Color *)(void *)v; break;
		case 11: m.normalcolor = *(Color *)(void *)v; break;
		}
		m.valid |= mt_flags[i];
		if(over) m.override |= mt_flags[i];
	    }
	    over = not = 0;
	}
    }
  done:
    return MtCopy(&m, mat);
}

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
    register int mask;

    if(dst == NULL)
	return MtCopy(src, NULL);

    /* Fields to merge in */
    mask = src ?
	(mergeflags & APF_OVEROVERRIDE) ?
	    src->valid : src->valid & ~(dst->override &~ src->override)
	: 0;

    if(mergeflags & APF_INPLACE)
	RefIncr((Ref *)dst);
    else
	dst = MtCopy(dst, NULL);

    if(mask == 0)			/* No changes to dst */
	return dst;

    dst->changed |= src->changed;
    dst->valid = (src->valid & mask) | (dst->valid & ~mask);
    dst->override = (src->override & mask) | (dst->override & ~mask);
    if(mask & MTF_EMISSION) dst->emission = src->emission;
    if(mask & MTF_AMBIENT) dst->ambient = src->ambient;
    if(mask & MTF_DIFFUSE) dst->diffuse = src->diffuse;
    if(mask & MTF_SPECULAR) dst->specular = src->specular;
    if(mask & MTF_Ka) dst->ka = src->ka;
    if(mask & MTF_Kd) dst->kd = src->kd;
    if(mask & MTF_Ks) dst->ks = src->ks;
    if(mask & MTF_ALPHA) dst->diffuse.a = src->diffuse.a;
    if(mask & MTF_SHININESS) dst->shininess = src->shininess;
    if(mask & MTF_EDGECOLOR) dst->edgecolor = src->edgecolor;
    if(mask & MTF_NORMALCOLOR) dst->normalcolor = src->normalcolor;
    return dst;
}

int
MtSave(mat,name)
    Material *mat;
    char *name;
{
    FILE *f;
    int ok;

    f = fopen(name,"w");
    if(!f) {
	perror(name);
	return -1;
    }
    ok = MtFSave(mat, f);
    fclose(f);
    return ok;
}

int
Apsavepfx(int valid, int override, int mask, char *keyword, FILE *f)
{
    if((valid & mask) == 0)
	return 0;
    fputc('\t', f);
    if(override & mask)
	fputc('*', f);
    fprintf(f, "%s ", keyword);
    return 1;
}


int
MtFSave(mat,f)
    register Material *mat;
    FILE *f;
{
    register int i;
    float v;
    Color *c;

    for(i = 0; i < sizeof(mt_kw)/sizeof(mt_kw[0]); i++) {
	if(Apsavepfx(mat->valid, mat->override, mt_flags[i], mt_kw[i], f)) {
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

Appearance *
ApDefault(Appearance *ap)
{
  ap->valid = ap->override = 0;
  memset(ap, 0, sizeof(Appearance));
  RefInit((Ref *)ap, APMAGIC);
  ap->mat = NULL;
  ap->lighting = NULL;
  return ap;
}

void ApUseOverrides(Appearance *ap, int use_overrides)
{
  Material *mat;
  if(ap == NULL) return;
  ap->override = ap->valid & use_overrides;
  if((mat = ap->mat) != NULL) {
    mat->override = mat->valid & use_overrides;
    mat->changed = 1;
  }
  if((mat = ap->backmat) != NULL) {
    mat->override = mat->valid & use_overrides;
    mat->changed = 1;
  }
  if(ap->lighting) {
    ap->lighting->override = ap->lighting->valid & use_overrides;
    ap->lighting->changed = 1;
  }
}

/*
 * We assume dst is a child of src in the inheritance tree.
 * Erase all settings in dst that are defined in src,
 * so that src's settings can propagate to (the children of) dst.
 */
void ApLetPropagate(register Appearance *src, register Appearance *dst)
{
  if(src == NULL || dst == NULL) return;
  dst->valid &= ~src->valid;
  dst->override &= ~src->valid;
  if(dst->mat && src->mat) {
    dst->mat->valid &= ~src->mat->valid;
    dst->mat->override &= ~src->mat->valid;
    dst->mat->changed = 1;
  }
  if(dst->backmat && src->backmat) {
    dst->backmat->valid &= ~src->backmat->valid;
    dst->backmat->override &= ~src->backmat->valid;
    dst->backmat->changed = 1;
  }
  if(dst->lighting && src->lighting) {
    dst->lighting->valid &= ~src->lighting->valid;
    dst->lighting->override &= ~src->lighting->valid;
  }
}


Texture *
TxLoad(Texture *into, char *fname)
{
    IOBFILE *f = iobfopen(fname, "rb");
    Texture *t;

    if(f == NULL) {
	OOGLError(0, "TxLoad: can't open %s: %s", fname, sperror());
	return NULL;
    }
    t = TxFLoad(f, fname);
    iobfclose(f);
    return t;
}


Texture *
TxFLoad(IOBFILE *inf, char *fname)
{
    Pool *p;
    Texture *tx = NULL;
    p = PoolStreamTemp(fname, inf, NULL, 0, &TextureOps);
    TxStreamIn(p, NULL, &tx);
    PoolDelete(p);
    return tx;
}

Texture *
TxDefault(Texture *tx)
{
    memset((char *)tx, 0, sizeof(Texture));
    RefInit((Ref *)tx, TXMAGIC);
    TmIdentity(tx->tfm);
    return tx;
}


Texture *
_TxSet(Texture *tx, int attr1, register va_list *alist)
{
    long mask;
    ColorA *co;
    Handle *h;
    float *f;
    int attr;
    char *str;

#define NEXT(type) va_arg(*alist, type)
	
    if (tx == NULL) {
        /*
         * New Appearance created here.
         */
	tx = OOGLNewE(Texture, "TxCreate Texture");
	TxDefault(tx);
    }

    for ( attr = attr1; attr != TX_END; attr = NEXT(int)) {
	switch (attr) { /* parse argument list */
	  case TX_DOCLAMP:
	    mask = NEXT(int);
	    tx->flags = (tx->flags & ~(TXF_SCLAMP|TXF_TCLAMP)) |
			(mask & (TXF_SCLAMP|TXF_TCLAMP));
	    break;
	  case TX_APPLY:
	    mask = NEXT(int);
	    if(mask < TXF_MODULATE || mask > TXF_DECAL) {
		OOGLError(1, "TxSet: bad value for TX_APPLY: %d must be %d..%d",
			mask, TXF_MODULATE, TXF_DECAL);
		return NULL;
	    }
	    tx->apply = mask;
	    break;
	  case TX_FILE:
	    str = NEXT(char *);
	    if(str && tx->filename && strcmp(str, tx->filename) == 0)
		break;
	    if(tx->filename)
		OOGLFree(tx->filename);
	    tx->filename = str ? strdup(str) : NULL;
	    tx->flags &= ~TXF_LOADED;
	    break;
	  case TX_ALPHAFILE:
	    str = NEXT(char *);
	    if(str && tx->alphafilename && strcmp(str, tx->alphafilename) == 0)
		break;
	    if(tx->alphafilename)
		OOGLFree(tx->alphafilename);
	    tx->alphafilename = str ? strdup(str) : NULL;
	    tx->flags &= ~TXF_LOADED;
	    break;
	  case TX_DATA:
	    str = NEXT(char *);
	    if(tx->data && str != tx->data)
		OOGLFree(tx->data);
	    tx->data = str;
	    if(str)
		tx->flags |= TXF_LOADED;
	    break;
	  case TX_XSIZE:
	    mask = NEXT(int);
	    if(mask != tx->xsize)
		tx->flags &= ~TXF_LOADED;
	    tx->xsize = mask;
	    break;
	  case TX_YSIZE:
	    mask = NEXT(int);
	    if(mask != tx->ysize)
		tx->flags &= ~TXF_LOADED;
	    tx->ysize = mask;
	    break;
	  case TX_CHANNELS:
	    mask = NEXT(int);
	    if(mask != tx->channels)
		tx->channels &= ~TXF_LOADED;
	    tx->channels = mask;
	    break;
	  case TX_COORDS:
	    mask = NEXT(int);
	    if(mask < TXF_COORD_GIVEN||mask > TXF_COORD_NORMAL) {
		OOGLError(1, "TxSet: bad value for TX_APPLY: %d must be %d..%d",
			mask, TXF_MODULATE, TXF_DECAL);
		return NULL;
	    }
	    tx->coords = mask;
	    break;
	  case TX_BACKGROUND:
	    co = NEXT(ColorA *);
	    tx->background = *co;
	    break;
	  case TX_HANDLE_TRANSFORM:
	    h = NEXT(Handle *);
	    f = NEXT(float *);
	    if(tx->tfmhandle)
		HandlePDelete( &tx->tfmhandle );
	    if(f)
		memcpy(tx->tfm, f, sizeof(Transform));
	    else
		TmIdentity(tx->tfm);
	    tx->tfmhandle = h;
	    if(h)
		HandleRegister(&tx->tfmhandle, (Ref *)tx, tx->tfm, TransUpdate);
	    RefIncr((Ref *)h);
	    break;
	  default:
	    OOGLError(1, "TxSet: unknown attribute %d", attr);
	    return NULL;
	}
    }
    return tx;
}



Texture *
TxCreate(int a1, ...)
{
    va_list alist;
    Texture *tx;

    va_start(alist,a1);
    tx = _TxSet(NULL, a1, &alist);
    va_end(alist);
    return tx;
}

Texture *
TxSet(Texture *tx, int a1, ... )
{
    va_list alist;
    va_start(alist,a1);
    tx = _TxSet(tx, a1, &alist);
    va_end(alist);
    return tx;
}

TxUser *
TxAddUser(Texture *tx, long id, int (*needed)(TxUser *), void (*purge)(TxUser *))
{
    TxUser *tu = OOGLNew(TxUser);
    memset((void *)tu, 0, sizeof(TxUser));
    /* flags = 0, ctx = NULL, data = NULL */
    tu->tx = tx;
    tu->id = id;
    tu->needed = needed;
    tu->purge = purge;
    tu->next = tx->users;
    tx->users = tu;
    if(tx->next == NULL) {
	/* Ensure this texture is on the AllLoadedTextures list. */
	Texture *otx;
	for(otx = AllLoadedTextures; otx != tx; otx = otx->next) {
	    if(otx == NULL) {
		tx->next = AllLoadedTextures;
		AllLoadedTextures = tx;
		break;
	    }
	}
    }
    return tu;
}


/* Reclaim memory from texture, but don't discard essential stuff */
/* Notify all users to purge their information. This will be their only notice.*/
void
TxPurge(Texture *tx)
{
    /* Tell each user to get lost */
    TxUser *u, *nextu;
    Texture **loadedp;

    for(loadedp = &AllLoadedTextures; *loadedp != NULL; loadedp = &(*loadedp)->next) {
	if(tx == *loadedp) {
	    *loadedp = tx->next;
	    tx->next = NULL;
	    break;
	}
    }

    for(u = tx->users; u != NULL; u = nextu) {
	nextu = u->next;
	if(u->purge)
	   (*u->purge)(u);
	OOGLFree(u);
    }
    tx->users = NULL;
    if(tx->data)
	OOGLFree(tx->data);
    tx->data = NULL;
    tx->flags &= ~(TXF_LOADED|TXF_RGBA|TXF_USED);
}

void
TxDelete(Texture *tx)
{
    if(tx == NULL)
	return;
    if(tx->magic != TXMAGIC) {
	OOGLWarn("Internal warning: TxDelete on non-Texture %x (%x != %x)",
		tx, tx->magic, TXMAGIC);
	return;
    }
    if(RefDecr((Ref *)tx) > 0)
	return;

    TxPurge(tx);
    if(tx->filename) OOGLFree(tx->filename);
    if(tx->alphafilename) OOGLFree(tx->alphafilename);
    if(tx->tfmhandle) HandlePDelete(&tx->tfmhandle);
    tx->magic = TXMAGIC ^ 0x80000000;
    OOGLFree(tx);
}


HandleOps TextureOps = {
	"texture",
	(int ((*)()))TxStreamIn,
	(int ((*)()))TxStreamOut,
	(void ((*)()))TxDelete,
	NULL,
	NULL
};

static struct txkw {
	char *word;
	unsigned short aval;
	short args;
} tx_kw[] = {
	{ "texture",	0,		1 },
	{ "clamp",	TX_DOCLAMP,	-4 },
	{  "none",	 0,		0 },
	{  "s",		 TXF_SCLAMP,	0 },
	{  "t",		 TXF_TCLAMP,	0 },
	{  "st",	 TXF_SCLAMP|TXF_TCLAMP, 0 },
	{ "file",	TX_FILE,	0 },
	{ "alphafile",	TX_ALPHAFILE,	0 },
	{ "apply",	TX_APPLY,	-3 },
	{  "blend",	TXF_BLEND,	0 },
	{  "modulate",	TXF_MODULATE,	0 },
	{  "decal",	TXF_DECAL,	0 },
	{ "transform",	TX_HANDLE_TRANSFORM,	0 },  /*(s,t,r,q) . tfm = tx coords */
	{ "xsize",	TX_XSIZE,	1 },
	{ "ysize",	TX_YSIZE,	1 },
	{ "channels",	TX_CHANNELS,	1 },
	{ "background",	TX_BACKGROUND,	4 },
	{ "coords",	TX_COORDS,	-4 },  /* Let's not implement this initially */
	{  "given",	 TXF_COORD_GIVEN,  0 },
	{  "local",	 TXF_COORD_LOCAL, 0 },
	{  "camera",	 TXF_COORD_CAMERA, 0 },
	{  "normal",	 TXF_COORD_NORMAL, 0 },
};


int
TxStreamIn( Pool *p, Handle **hp, Texture **txp )
{
    IOBFILE *stream;
    char *fname;
    Handle *h = NULL;
    register Texture *tx = NULL;
    float val[16];
    struct txkw *kw;
    char *w, *raww;
    int i, k = 0;
    int brack = 0;
    int empty = 1, braces = 0;
    int plus = 0;
    int more;
    int mine = 1;	/* Questionable -- we'll report all errors */

    if((stream = PoolInputFile(p)) == NULL)
	return 0;
    fname = PoolName(p);

    more = 0;
    do {
	iobfnextc(stream, 0);

	switch(i = iobfgetc(stream)) {
	case ':':
	case '<':
            w = iobfdelimtok("{}()", stream, 0);
            /*
             * Consider doing a path search.
             * Do this before calling HandleReferringTo()
             * to prevent spurious error messages.
	     */
	    if(i == '<' && (h = HandleByName(w, &TextureOps)) == NULL && w[0] != '/') {
		w = findfile(fname, raww = w);
		if(w == NULL) {
		    OOGLSyntax(PoolInputFile(p),
			"Error reading \"%s\": can't find file \"%s\"",
			fname, raww);
		}
	    }
	    h = HandleReferringTo(i, w, &TextureOps, NULL);
	    if(h != NULL) {
		tx = (Texture *)HandleObject(h);
		RefIncr((Ref*)tx);
	    }
	    break;

	case EOF: brack = 0; break;
	case '{': brack++; braces++; break;
	case '}':
		if(brack-- <= 0) {
		    iobfungetc(i, stream);
		}
		break;
	case '-':
	case '!':
		plus = -1;
		break;
	case '+':
		plus = 1;
		break;
	case '*':  break;

	default:
	    more = 0;
	    iobfungetc(i, stream);
	    w = iobfdelimtok("{}()", stream, 0);
	    if(w == NULL)
		break;
	    for(i = sizeof(tx_kw)/sizeof(tx_kw[0]), kw = tx_kw; --i >= 0; kw++)
		if(!strcmp(kw->word, w))
		    break;
		
	    if(i < 0) {
		if(mine)
		    OOGLSyntax(stream, "%s: unknown texture keyword %s",
			fname, w);
		return 0;
	    }
	    if(tx == NULL)
		tx = TxCreate(TX_END);

	    if(kw->args < 0) {
		char allowed[256], *tail = allowed;
		w = iobfdelimtok("{}()", stream, 0);
		if(w == NULL) w = "";
		allowed[0] = '\0';
		for(k = 1; strcmp((kw+k)->word, w); k++) {
		    sprintf(tail, " %s", (kw+k)->word);
		    tail += strlen(tail);
		    if(k + kw->args >= 0) {
			OOGLSyntax(stream, "%s: %s %s: expected one of: %s",
			   fname, kw->word, w, allowed);
			return 0;
		    }
		}
	    } else if(kw->args > 0) {
		int n = iobfgetnf(stream, kw->args, val, 0);
		if(n != kw->args) {
		    OOGLSyntax(stream, "%s: %s expected %d numeric values",
			fname, w, kw->args);
		    return 0;
		}
	    }

	    empty++;

	    switch((int)kw->aval) {
		case -1:
		    mine = more = 1;
		    empty--;
		    break;

		case TX_APPLY:
		    tx->apply = (kw+k)->aval;
		    break;

		case TX_FILE:
		case TX_ALPHAFILE:
			raww = iobfdelimtok("{}()", stream, 0);
			w = findfile(fname, raww);
			if(w == NULL) {
			    OOGLSyntax(stream,
				"Warning: reading \"%s\": can't find file \"%s\", ignoring texture",
				fname, raww);
			} else {
			    TxSet(tx, kw->aval, w, TX_END);
			}
			break;

		case TX_XSIZE:
		case TX_YSIZE:
		case TX_CHANNELS:
		    if(val[0] < 1 || val[0] > (tx_kw[i].aval==TX_CHANNELS) ? 4 : 100000) {
			OOGLSyntax(stream, "%s: Bad value for %s: %s",
				fname, kw->word, w);
			return 0;
		    }
		    TxSet(tx, kw->aval, (int)val[0], TX_END);
		    break;

		case TX_COORDS:
		    tx->coords = (kw+k)->aval;
		    break;
		case TX_BACKGROUND:
		    TxSet(tx, kw->aval, val, TX_END);
		    break;
		case TX_HANDLE_TRANSFORM:
		    if(!TransStreamIn(p, &tx->tfmhandle, tx->tfm)) {
			OOGLSyntax(stream, "%s: 4x4 texture transform expected",
				fname);
			return 0;
		    }
		    if(tx->tfmhandle)
			HandleRegister(&tx->tfmhandle, (Ref *)tx,
				tx->tfm, TransUpdate);
		    break;
		case TX_DOCLAMP:
		    tx->flags = (kw+k)->aval;
		    break;


		default:
		    break;
	    }
	    plus = 0;
	}
    } while(brack > 0 || more);

    if(h != NULL && hp != NULL && *hp != h) {
	if(*hp)
	    HandlePDelete(hp);
	*hp = h;
    }
    if(tx != NULL && txp != NULL && *txp != tx) {
	if(*txp != NULL)
	    TxDelete(*txp);
	*txp = tx;
    } else if(tx)		/* Maintain ref count */
	TxDelete(tx);

    return (tx != NULL || h != NULL || (empty && braces));
}

int
TxStreamOut(Pool *p, Handle *h, Texture *tx)
{
    static char *clamps[] = {"none","s","t","st"};
    static char *applies[] = {"modulate","blend","decal"};
    FILE *f = PoolOutputFile(p);
    if(f == NULL)
	return 0;
    fprintf(f, "texture {\n");
    fprintf(f, "	clamp %s\n", clamps[tx->apply & (TXF_SCLAMP|TXF_TCLAMP)]);
    if(tx->filename)
	fprintf(f, "	file %s\n", tx->filename);
    if(tx->alphafilename)
	fprintf(f, "	file %s\n", tx->alphafilename);
    fprintf(f, "	apply %s\n",
		(unsigned)tx->apply < COUNT(applies) ? applies[tx->apply]
				: "???");
    fprintf(f, "	background %g %g %g %g\n",
	tx->background.r, tx->background.g,
	tx->background.b, tx->background.a);
    fprintf(f, "	");
    TransStreamOut(p, tx->tfmhandle, tx->tfm);
    fprintf(f, "}\n");
    return !ferror(f);
}

Texture *
TxCopy(Texture *src, Texture *dst)
{
    OOGLError(0, "TxCopy not implemented yet.  Who needs it?");
    if(src==NULL) return dst;
    RefIncr((Ref *)src);
    return src;
}

Texture *
TxMerge(Texture *src, Texture *dst, int mergeflags)
{
    if(src == NULL)
	return dst;
    if(dst == NULL) {
	RefIncr((Ref *)src);
	return src;
    }
    /* Oh, well.  XXX.  Leave real merging for later, if at all.
     * Meanwhile, any new texture completely replaces any old one.
     */
    RefIncr((Ref *)src);
    return src;
}
