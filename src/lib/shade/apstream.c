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
#include "handleP.h"

HandleOps AppearanceOps = {
  "ap",
  (int ((*)(Pool *p, Handle **hp, Ref **rp)))ApStreamIn,
  (int ((*)(Pool *p, Handle *h, Ref *r)))ApStreamOut,
  (void ((*)(Ref *rp)))ApDelete,
  NULL,
  NULL,
};

static struct {
  char *word;
  int amask;
  enum {
    _flag    = 0,
    _appearance,
    _shading,
    _normscale,
    _linewidth,
    _material,
    _backmaterial,
    _patchdice,
    _light,
    _texture,
    _translucency,
    _ap_alpha_blending = AP_TRANSLUCENCY + APF_ALPHA_BLENDING,
    _ap_screen_door = AP_TRANSLUCENCY + APF_SCREEN_DOOR,
    _ap_naive_blending = AP_TRANSLUCENCY + APF_NAIVE_BLENDING,
    _ap_smooth = AP_SHADING + APF_SMOOTH,
    _ap_flat  = AP_SHADING + APF_FLAT,
    _ap_constant = AP_SHADING + APF_CONSTANT,
    _ap_csmooth = AP_SHADING + APF_CSMOOTH,
    _ap_vcflat = AP_SHADING + APF_VCFLAT,
  } aval;
} ap_kw[] = {
  { "appearance",       0,      _appearance },
  { "face",     APF_FACEDRAW,   _flag },
  { "edge",     APF_EDGEDRAW,   _flag },
  { "vect",     APF_VECTDRAW,   _flag },
  { "transparent", APF_TRANSP,  _translucency },
  { "screendoor", APF_TRANSP, _ap_screen_door },
  { "blending", APF_TRANSP, _ap_alpha_blending },
  { "naive", APF_TRANSP, _ap_naive_blending },
  { "evert",    APF_EVERT,      _flag },
  { "keepcolor",        APF_KEEPCOLOR,  _flag },
  { "texturing",        APF_TEXTURE,    _flag },
  { "backcull", APF_BACKCULL,   _flag },
  { "shadelines",       APF_SHADELINES, _flag },
  { "concave",  APF_CONCAVE,    _flag },
  { "shading",  APF_SHADING,    _shading },
  { "smooth",   APF_SHADING,    _ap_smooth },
  { "flat",     APF_SHADING,    _ap_flat },
  { "constant", APF_SHADING,    _ap_constant },
  { "csmooth",  APF_SHADING,    _ap_csmooth },
  { "vcflat",   APF_VCFLAT,     _ap_vcflat },
  { "mipmap",   APF_TXMIPMAP,   _flag },
  { "mipinterp",        APF_TXMIPINTERP, _flag },
  { "linear",   APF_TXLINEAR,   _flag },
  { "normal",   APF_NORMALDRAW, _flag },
  { "normscale",        APF_NORMSCALE,  _normscale },
  { "linewidth",        APF_LINEWIDTH,  _linewidth },
  { "material", 0,              _material },
  { "backmaterial", 0,          _backmaterial },
  { "patchdice",        APF_DICE,       _patchdice },
  { "light",    0,              _light },
  { "lighting", 0,              _light },
  { "texture",  0,              _texture },
};

int ApStreamIn(Pool *p, Handle **hp, Appearance **app)
{
  Handle *h = NULL;
  Handle *hname = NULL;
  Appearance *ap = NULL;
  IOBFILE *inf;
  char *fname;
  char *w, *raww;
  int c, i;
  int brack = 0;
  bool over, not, more, empty, braces;
  int value;
  int mask, flagmask;
  bool mine = true;     /* Questionable -- we'll report all errors */
    
  if (p == NULL || (inf = PoolInputFile(p)) == NULL) {
    return 0;
  }
  fname = PoolName(p);

  over = not = false;
  value = ~0;
  more = false;
  empty = true;
  braces = false;
  do {
    switch(c = iobfnextc(inf, 0)) {
    case '{': brack++; braces = true; iobfgetc(inf); break;
    case '}': if (brack--) iobfgetc(inf); break;
    case '+': value = ~0; iobfgetc(inf); break;
    case '-': value = 0; iobfgetc(inf); break;
    case '!': not = 1; iobfgetc(inf); break;
    case '*': over = 1; iobfgetc(inf); break;
    case '<':
    case ':':
    case '@':
      iobfgetc(inf);
      w = iobfdelimtok("{}()", inf, 0);
      if (c == '<' &&
	  (h = HandleByName(w, &ImageOps)) == NULL && w[0] != '/') {
	w = findfile(PoolName(p), raww = w);
	if (w == NULL) {
	  OOGLSyntax(inf,
		     "Reading image from \"%s\": "
		     "can't find file \"%s\"",
		     PoolName(p), raww);
	  return false;
	}
      } else if (h) {
	/* HandleByName() increases the ref. count s.t. the
	 * caller of HandleByName() owns the returned handle.
	 */
	HandleDelete(h);
      }
      h = HandleReferringTo(c, w, &AppearanceOps, NULL);
      if (h) {
	ap = (Appearance *)HandleObject(h);
	/* Increment the ref. count. This way we can call
	 * HandleDelete() and ImgDelete() independently.
	 */
	REFGET(Appearance, ap);
      }
      break;

    default:
      more = 0;
      w = iobfdelimtok("{}()", inf, 0);
      if (w == NULL) {
	break;
      }
      if (strcmp(w, "define") == 0) {
	hname = HandleCreateGlobal(iobftoken(inf, 0), &AppearanceOps);  
	break;
      }
      for (i = sizeof(ap_kw)/sizeof(ap_kw[0]); --i >= 0; ) {
	if (!strcmp(ap_kw[i].word, w)) {
	  break;
	}
      }
                
      if (i < 0) {
	if (mine) {
	  OOGLError(1,
		    "ApStreamIn: file %s: unknown appearance keyword %s",
		    fname, w);
	  ApDelete(ap);
	  return false;
	}
      }
      if (ap == NULL) {
	ap = ApCreate(AP_END);
      }

      empty = false;
      mask = flagmask = ap_kw[i].amask;
      if (not) {
	switch(ap_kw[i].aval) {
	case _material: MtDelete(ap->mat); ap->mat = NULL; break;
	case _backmaterial: MtDelete(ap->backmat); ap->mat = NULL; break;
	case _light: LmDelete(ap->lighting); ap->lighting = NULL; break;
	case _texture: TxDelete(ap->tex); ap->tex = NULL; break;
	default: break;
	}
	ap->flag &= ~mask;
	if (!over) ap->valid &= ~mask;
	ap->override &= ~mask;
      } else {
	switch(ap_kw[i].aval) {
	case _flag:
	  break;
	case _appearance:
	  mine = more = 1;
	  break;
	case _translucency:
	  ap->translucency = APF_ALPHA_BLENDING;
	  iobfgetni(inf, 1, (int *)&ap->translucency, 0);
	  break;
	case _ap_alpha_blending:
	case _ap_screen_door:
	case _ap_naive_blending:
	  ap->translucency = ap_kw[i].aval - AP_TRANSLUCENCY;
	  break;
	case _shading:
	  iobfgetni(inf, 1, (int *)&ap->shading, 0);
	  break;
	case _ap_smooth:
	case _ap_flat:
	case _ap_constant:
	case _ap_csmooth:
	case _ap_vcflat:
	  ap->shading = ap_kw[i].aval - AP_SHADING;
	  break;
	case _normscale:
	  if (iobfgetnf(inf, 1, &ap->nscale, 0) <= 0) {
	    OOGLError(1,"ApFLoad: %s: \"normscale\": value expected", fname);
	  }       
	  break;
	case _linewidth:
	  if (iobfgetni(inf, 1, &ap->linewidth, 0) <= 0) {
	    OOGLSyntax(inf, "%s \"linewidth\": value expected", fname);
	  }
	  break;
	case _material:
	  if ((ap->mat = MtFLoad(ap->mat, inf, fname)) == NULL) {
	    OOGLSyntax(inf,"Can't read material in %s", fname);
	  }
	  break;
	case _backmaterial:
	  if ((ap->backmat = MtFLoad(ap->backmat, inf, fname)) == NULL) {
	    OOGLError(1,"Can't read backmaterial, file %s", fname);
	  }
	  break;
	case _light:
	  ap->lighting = LmFLoad(ap->lighting, inf, fname);
	  if (ap->lighting == NULL) {
	    OOGLError(1,"Can't read lighting, file %s", fname);
	  }
	  break;
	case _texture:
	  if (!TxStreamIn(p, NULL, &ap->tex)) {
	    OOGLError(1, "%s: Can't read texture", fname);
	  }
	  break;
	case _patchdice:
	  if (iobfgetni(inf, 2, ap->dice, 0) < 2) {
	    OOGLSyntax(inf,
		       "%s \"patchdice\": "
		       "expected integer u- and v- dicing values", fname);
	  }
	  break;
	default:
	  OOGLSyntax(inf,
		     "%s unknown appearance keyword", fname);
	}
	if (value) {
	  ap->flag |= flagmask;
	} else {
	  ap->flag &= ~flagmask;
	}
	ap->valid |= mask;
	if (over) {
	  ap->override |= mask;
	}
      }
      /* Reset for next keyword */
      over = not = 0;  value = ~0;

    }
  } while(brack > 0 || more);

  if (hname != NULL) {
    if (ap) {
      HandleSetObject(hname, (Ref *)ap);
    }
    if (h) {
      /* HandleReferringTo() has passed the ownership to use, so
       * delete h because we do not need it anymore.
       */
      HandleDelete(h);
    }
    h = hname;
  }

  /* Pass the ownership of h and ap to the caller if requested */

  if (hp != NULL) {
    /* pass on ownership of the handle h to the caller of this function */
    if (*hp != NULL) {
      if (*hp != h) {
	HandlePDelete(hp);
      } else {
	HandleDelete(*hp);
      }
    }
    *hp = h;
  } else if (h) {
    /* Otherwise delete h because we are its owner. Note that
     * HandleReferringTo() has passed the ownership of h to us;
     * explicitly defined handles (hdefine and define constructs)
     * will not be deleted by this call.
     */
    HandleDelete(h);
  }

  /* same logic as for hp */
  if (app != NULL) {
    if (*app != NULL) {
      ApDelete(*app);
    }
    *app = ap;
  } else if(ap) {
    ApDelete(ap);
  }

  return (h != NULL || ap != NULL || (empty && braces));
}

int
Apsavepfx(int valid, int override, int mask, char *keyword, FILE *f, Pool *p)
{
  if ((valid & mask) == 0) {
    return 0;
  }
  PoolFPrint(p, f, "");
  if (override & mask) {
    fputc('*', f);
  }
  if (keyword && *keyword) {
    fprintf(f, "%s ", keyword);
  }
  return 1;
}

int ApStreamOut(Pool *p, Handle *h, Appearance *ap)
{
  FILE *f = PoolOutputFile(p);
  int valid;
  int mask;
  int i;

  if (f == NULL ||
      (ap == NULL &&
       (h == NULL || (ap = (Appearance *)HandleObject(h)) == NULL))) {
    return false;
  }

  valid = ap->valid;
  fprintf(f, "appearance {\n");
  PoolIncLevel(p, 1);
  if (PoolStreamOutHandle(p, h, ap != NULL)) {
    for (i = 0; i < (int)(sizeof(ap_kw)/sizeof(ap_kw[0])); i++) {
      mask = ap_kw[i].amask;
      if ((valid & mask) == 0) {
	continue;
      }
      Apsavepfx(valid, ap->override, mask, "", f, p);
      if (ap_kw[i].aval == 0 || ap_kw[i].aval == _translucency) {
	if ((mask & ap->flag) == 0) {
	  fputc('-', f);
	}
	fputs(ap_kw[i].word, f);
      }
      valid &= ~mask;
      switch (mask) {
      case APF_TRANSP:
	if ((mask & ap->flag) != 0) {
	  switch (ap->translucency) {
	  case APF_SCREEN_DOOR:
	    fputs(" screendoor", f);
	    break;
	  case APF_NAIVE_BLENDING:
	    fputs(" naive", f);
	    break;
	  case APF_ALPHA_BLENDING:
	    fputs(" blending", f);
	    break;
	  default:
	    fprintf(f, "%d", ap->translucency); break;
	    break;
	  }
	}
	break;
      case APF_SHADING:
	fputs("shading ", f);
	switch(ap->shading) {
	case APF_SMOOTH:   fputs("smooth", f);  break;
	case APF_FLAT:     fputs("flat", f);    break;
	case APF_CONSTANT: fputs("constant", f);        break;
	case APF_CSMOOTH:  fputs("csmooth", f); break;
	case APF_VCFLAT:   fputs("vcflat", f);  break;
	default:           fprintf(f, "%d", ap->shading); break;
	}
	break;
      case APF_NORMSCALE:
	fprintf(f, "normscale %.8g", ap->nscale);
	break;
      case APF_LINEWIDTH:
	fprintf(f, "linewidth %d ", ap->linewidth);
	break;
      case APF_DICE:
	fprintf(f, "patchdice %d %d", ap->dice[0], ap->dice[1]);
	break;
      }
      fputc('\n', f);
    }

    if (ap->mat) {
      PoolFPrint(p, f, "material {\n");
      PoolIncLevel(p, 1);
      MtFSave(ap->mat, f, p);
      PoolIncLevel(p, -1);
      PoolFPrint(p, f, "}\n");
    }
    if (ap->backmat) {
      PoolFPrint(p, f, "backmaterial {\n");
      PoolIncLevel(p, 1);      
      MtFSave(ap->backmat, f, p);
      PoolIncLevel(p, -1);
      PoolFPrint(p, f, "}\n");
    }
    if (ap->lighting) {
      PoolFPrint(p, f, "lighting {\n");
      PoolIncLevel(p, 1);      
      LmFSave(ap->lighting, f, p->poolname, p);
      PoolIncLevel(p, -1);
      PoolFPrint(p, f, "}\n");
    }
    if (ap->tex) {
      TxStreamOut(p, NULL, ap->tex);
    }
  }
  PoolIncLevel(p, -1);
  PoolFPrint(p, f, "}\n");

  return !ferror(f);
}

Appearance *
ApFLoad(IOBFILE *inf, char *fname)
{
  Pool *p;
  Appearance *ap = NULL;
  p = PoolStreamTemp(fname, inf, NULL, 0, &AppearanceOps);
  ApStreamIn(p, NULL, &ap);
  PoolDelete(p);
  return ap;
}

Appearance *
ApFSave(Appearance *ap, FILE *outf, char *fname)
{
  Pool *p;
  int ok;

  p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
  PoolSetOType(p, PO_DATA);
  PoolIncLevel(p, 1);
  ok = ApStreamOut(p, NULL, ap);
  PoolDelete(p);
  return ok ? ap : NULL;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
