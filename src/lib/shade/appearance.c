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
#include "handleP.h"
#include <string.h>
#include <sys/stat.h>

#define MAXSHININESS 1024.0

extern LmLighting * _LmSet(LmLighting *, int attr1, va_list *);
extern Material * _MtSet(Material *, int attr1, va_list *);

Appearance *
_ApSet(Appearance *ap, int attr1, va_list *alist)
{
  int mask;
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
    case AP_TRANSLUCENCY:
      /* should be APF_ALPHA_BLENDING or APF_SCREEN_DOOR */
      ap->translucency = NEXT(int);
      ap->valid |= APF_TRANSP;
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
  case AP_DONT:         *(int *) value =  ap->flag; break;
  case AP_OVERRIDE:
  case AP_NOOVERRIDE:   *(int *) value = ap->override; break;
  case AP_VALID:
  case AP_INVALID:      *(int *) value = ap->valid; break;
  case AP_MAT:          *(Material **) value = ap->mat; break;
  case AP_LGT:          *(LmLighting **) value = ap->lighting; break;
  case AP_NORMSCALE:    *(double *) value = ap->nscale; break;
  case AP_LINEWIDTH:    *(int *) value = ap->linewidth; break;
  case AP_SHADING:      *(int *)value = ap->shading; break;
  case AP_TRANSLUCENCY: *(int *)value = ap->translucency; break;
  case AP_DICE: ((int *)value)[0] = ap->dice[0];
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
  if (ap == NULL || RefDecr((Ref *)ap) > 0)
    return;
  if (ap->magic != APMAGIC) {
    OOGLError(1, "ApDelete(%x) of non-Appearance: magic %x != %x",
              ap, ap->magic, APMAGIC);
    return;
  }
  if (ap->mat) MtDelete(ap->mat);
  if (ap->lighting) LmDelete(ap->lighting);
  if (ap->tex) TxDelete(ap->tex);

  OOGLFree(ap);
}

/*
 * Copies just the Appearance part, not its Material and LmLighting children.
 * Pointers to the latter are retained BUT their reference counts are NOT
 * incremented.  The caller MUST either RefIncr() or reassign mat and lighting.
 */
Appearance *
ApCopyShallow(const Appearance *ap, Appearance *into )
{
  if (ap == NULL)
    return NULL;
  if (into == NULL) {
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
    into->translucency = ap->translucency;
    into->dice[0] = ap->dice[0];
    into->dice[1] = ap->dice[1];
  }
  return into;
}
    
Appearance *
ApCopy(const Appearance *ap, Appearance *into )
{
  if (ap == NULL) return into;
  into = ApCopyShallow( ap, into );
  if (ap->mat) into->mat = MtCopy(ap->mat, into->mat);
  if (ap->backmat) into->backmat = MtCopy(ap->backmat, into->backmat);
  if (ap->lighting) into->lighting = LmCopy(ap->lighting, into->lighting);
  if (ap->tex) into->tex = TxCopy(ap->tex, into->tex);
  return into;
}

Appearance *
ApCopyShared(const Appearance *ap, Appearance *into )
{
  if (ap == NULL) return into;
  into = ApCopyShallow( ap, into );
  if (ap->mat) {
    if (into->mat) MtCopy(ap->mat, into->mat);
    else RefIncr((Ref *)(into->mat = ap->mat));
  }
  if (ap->backmat) {
    if (into->backmat) MtCopy(ap->backmat, into->backmat);
    else RefIncr((Ref *)(into->backmat = ap->backmat));
  }
  if (ap->lighting) {
    if (into->lighting) LmCopy(ap->lighting, into->lighting);
    else RefIncr((Ref *)(into->lighting = ap->lighting));
  }
  if (ap->tex) {
    if (into->tex) TxCopy(ap->tex, into->tex);
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
ApMerge(const Appearance *src, Appearance *dst, int mergeflags )
{
  int mask;
  Material *mt, *bmt;
  LmLighting *lts;
  Texture *tex;

  if (dst == NULL)
    return ApCopy(src, NULL);

  if (src == NULL) {
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
  if ((mergeflags & APF_INPLACE)
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
#if 0
  if (mt != dst->mat) MtDelete(dst->mat);
  if (bmt != dst->backmat) MtDelete(dst->backmat);
  if (lts != dst->lighting) LmDelete(dst->lighting);
  if (tex != dst->tex && dst->tex) TxDelete(dst->tex);
#else
  MtDelete(dst->mat);
  MtDelete(dst->backmat);
  LmDelete(dst->lighting);
  TxDelete(dst->tex);
#endif

  dst->mat = mt;
  dst->backmat = bmt;
  dst->lighting = lts;
  dst->tex = tex;
  if (mask) {
    /* Merge together appearance-specific data */
    dst->flag = (src->flag & mask) | (dst->flag & ~mask);
    dst->valid = (src->valid & mask) | (dst->valid & ~mask);
    dst->override = (src->override & mask) | (dst->override & ~mask);
    if (mask & APF_NORMSCALE) dst->nscale = src->nscale;
    if (mask & APF_LINEWIDTH) dst->linewidth = src->linewidth;
    if (mask & APF_SHADING) dst->shading = src->shading;
    if (mask & APF_TRANSP) dst->translucency = src->translucency;
    if (mask & APF_DICE) {
      dst->dice[0] = src->dice[0];
      dst->dice[1] = src->dice[1];
    }
  }
  return dst;
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
  if (ap == NULL) return;
  ap->override = ap->valid & use_overrides;
  if ((mat = ap->mat) != NULL) {
    mat->override = mat->valid & use_overrides;
    mat->changed = 1;
  }
  if ((mat = ap->backmat) != NULL) {
    mat->override = mat->valid & use_overrides;
    mat->changed = 1;
  }
  if (ap->lighting) {
    ap->lighting->override = ap->lighting->valid & use_overrides;
    ap->lighting->changed = 1;
  }
}

/*
 * We assume dst is a child of src in the inheritance tree.
 * Erase all settings in dst that are defined in src,
 * so that src's settings can propagate to (the children of) dst.
 */
void ApLetPropagate(Appearance *src, Appearance *dst)
{
  if (src == NULL || dst == NULL) return;
  dst->valid &= ~src->valid;
  dst->override &= ~src->valid;
  if (dst->mat && src->mat) {
    dst->mat->valid &= ~src->mat->valid;
    dst->mat->override &= ~src->mat->valid;
    dst->mat->changed = 1;
  }
  if (dst->backmat && src->backmat) {
    dst->backmat->valid &= ~src->backmat->valid;
    dst->backmat->override &= ~src->backmat->valid;
    dst->backmat->changed = 1;
  }
  if (dst->lighting && src->lighting) {
    dst->lighting->valid &= ~src->lighting->valid;
    dst->lighting->override &= ~src->lighting->valid;
  }
}


/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
