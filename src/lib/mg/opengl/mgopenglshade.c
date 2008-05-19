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

#include "mgP.h"
#include "mgopenglP.h"
#include "mgopenglshade.h"

#ifdef GLUT
# include <GL/glut.h>
#else
#define GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
#endif

#define ID_OFFSET 0

/* materialno, lightmodelno, and lightno between 1 and 65535 are
 * legal. p 9-9 GL PROG GUIDE (munzner 9/17/91)
 */
/*static int lightno = 1;*/
static float kd = 1.0;

/* Additional data needed if the image dimensions are not a power of 2. */
struct mgopengl_tudata 
{
  char *data;
  int xsize, ysize, channels;
  int qualflags;    /* APF_TX{MIPMAP,MIPINTERP,LINEAR}: if loaded, how? */
};

void
mgopengl_appearance(struct mgastk *ma, int mask)
{
  Appearance *ap = &(ma->ap);

  if (mask & APF_TRANSP) {
    if (ap->flag & APF_TRANSP) {
      switch (ap->translucency) {
      case APF_ALPHA_BLENDING:
	break;
      case APF_SCREEN_DOOR:
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE,  GL_ZERO);
	glDisable(GL_BLEND);
	/* Stipple pattern will be set later, based on alpha-value */
	break;
      case APF_NAIVE_BLENDING:
	/* Do not do this here; alpha-blending will be enabled as needed
	 * in mgopengl_bsptree(). This is for the sake of efficiency: if
	 * a geometry does not have any translucent component it is much
	 * more efficient to use the depth buffer instead of alpha
	 * blending, especially when there is no hardware rendering
	 * support.
	 */
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	break;
      default:
	break; /* ??? */
      }
    } else {
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_ONE,  GL_ZERO);
      glDisable(GL_BLEND);
    }
  }

  if (mask & APF_LINEWIDTH) {
    glLineWidth((GLfloat)(ap->linewidth));
    _mgc->has &= ~HAS_POINT;
  }

  if (mask & APF_BACKCULL) {
    if(ap->flag & APF_BACKCULL) {
      glEnable( GL_CULL_FACE );
    } else {
      glDisable( GL_CULL_FACE );
    }
  }


  if (mask & APF_SHADING) {
    if(!IS_SHADED(ap->shading) || ma->shader != NULL) {
      /* switch to constant shading by unbinding the lmodel */

      glDisable(GL_LIGHTING);
      _mgopenglc->should_lighting = _mgopenglc->is_lighting = 0;

      D4F_OFF();
      _mgopenglc->lmcolor = GL_DIFFUSE;

      glShadeModel(IS_SMOOTH(ap->shading) ? GL_SMOOTH : GL_FLAT );
      if (ma->shader != NULL && IS_SHADED(ap->shading)) {
	ma->flags |= MGASTK_SHADER;
      } else {
	ma->flags &= ~MGASTK_SHADER;
      }
    }
    else {
      /* turn shading on */
      glEnable(GL_LIGHTING);
      _mgopenglc->should_lighting = _mgopenglc->is_lighting = 1;

      glShadeModel( IS_SMOOTH(ap->shading) ? GL_SMOOTH : GL_FLAT );

      if (ap->lighting->valid)
	glCallList(_mgopenglc->light_lists[ma->light_seq]);

      mgopengl_material( ma, ma->mat.valid );

      D4F_ON();
      _mgopenglc->lmcolor = GL_DIFFUSE;
      ma->flags &= ~MGASTK_SHADER;
    }
  }

  if(mask & APF_EVERT) {
    /*
     * Do automatic normal-flipping if requested.
     */
    if(ap->flag & APF_EVERT) {
      N3F_EVERT();
    } else {
      N3F_NOEVERT();
    }
  }
 
  /*
   * No GL calls are needed for the following attributes because
   * they are always interpreted at draw-time:
   *		APF_FACEDRAW
   *		APF_EDGEDRAW
   *		APF_NORMSCALE
   */

}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_material
 * Description:	bind a material. define it if it's not yet defined.
 * Args:	*mat: Material to bind.
 *		mask: Bitmask telling which material fields are valid.
 *		      Passed into mgopengl_materialdef.
 * Returns:	
 * Author:	munzner
 * Date:	Wed Oct 16 16:06:47 1991
 * Notes:	We must reset the "current GL color" after binding a
 *		material.
 *		We want color calls to change the *diffuse* color when
 *		we're in shading mode. Thus we call glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);glEnable(GL_COLOR_MATERIAL).
 *		We also must keep track of the diffuse coefficient
 *		for use in mgopengl_d[3,4]f, our wrapper for color calls.
 *		C3f or c4f should never be called directly.
 *		mg draw routines are responsible for establishing the
 * 		correct drawing color.
 */
void
mgopengl_material(struct mgastk *astk, int mask)
{
  GLfloat f[4];
  Material *mat = &astk->mat;
#ifdef TRUE_EMISSION
  static float lmnull = (float) 0; /* LMNULL */
#endif

  mask &= mat->valid;
  if (mask & MTF_Kd)
    kd = mat->kd;

  if((mask & (MTF_EMISSION|MTF_DIFFUSE|MTF_AMBIENT|MTF_SPECULAR
	      |MTF_SHININESS|MTF_Kd|MTF_Ka|MTF_Ks|MTF_ALPHA)) == 0)
    return;		/* No GL changes to make. */

  if(astk->next && astk->next->mat_seq == astk->mat_seq) {
    /*
     * Fresh material needed.  Erase any previous GL definition.
     * We'll need to load all valid fields to initialize it.
     */
    astk->mat_seq = next_mat_seq(_mgc, astk);
#ifndef TRUE_EMISSION
    /* Needs fixing - TOR
       if(mat->valid & MTF_EMISSION) {
       glNewList( astk->mat_seq, GL_COMPILE);
       glMaterialf(GL_BACK, GL_AMBIENT_AND_DIFFUSE, * &lmnull);
       glEndList();
       glCallList(astk->mat_seq);
       astk->mat_seq++;
       }
    */
#endif /*!TRUE_EMISSION*/
    mask = mat->valid;
  }

  /* Build material definition */

  f[3] = 1.0;
    
#ifdef TRUE_EMISSION
  if( mask & MTF_EMISSION) {
    f[0] = mat->emission.r;
    f[1] = mat->emission.g;
    f[2] = mat->emission.b;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f);
  }
#endif

  if( mask & (MTF_Kd | MTF_DIFFUSE)) {
    f[0] = mat->kd * mat->diffuse.r;
    f[1] = mat->kd * mat->diffuse.g;
    f[2] = mat->kd * mat->diffuse.b;
    f[3] = mat->diffuse.a;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f);
  }
  f[3] = 1.0;
  if( mask & (MTF_Ka | MTF_AMBIENT)) {
    f[0] = mat->ka * mat->ambient.r;
    f[1] = mat->ka * mat->ambient.g;
    f[2] = mat->ka * mat->ambient.b;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f);
  }
  if( mask & (MTF_Ks | MTF_SPECULAR | MTF_SHININESS)) {
    f[0] = mat->ks * mat->specular.r;
    f[1] = mat->ks * mat->specular.g;
    f[2] = mat->ks * mat->specular.b;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
  }

#ifndef TRUE_EMISSION	/* Hack: use "emission" field as back diffuse color */
  if(mask & MTF_EMISSION) {
    f[0] = mat->kd * mat->emission.r;
    f[1] = mat->kd * mat->emission.g;
    f[2] = mat->kd * mat->emission.b;
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, f);
  }
#endif /*!TRUE_EMISSION*/
}

void
mgopengl_setshader(mgshadefunc shader)
{
  struct mgastk *ma = _mgc->astk;
  unsigned short wasusing = ma->flags & MGASTK_SHADER;

  ma->shader = shader;
  if (shader != NULL && IS_SHADED(ma->ap.shading)) {
    ma->flags |= MGASTK_SHADER;
  } else {
    ma->flags &= ~MGASTK_SHADER;
  }
  if((ma->flags & MGASTK_SHADER) != wasusing)
    mgopengl_appearance(_mgc->astk, APF_SHADING);
}

void mgopengl_lighting(struct mgastk *astk, int mask)
{
  LmLighting *lm = &astk->lighting;

  if (lm->valid) {
    mgopengl_lightmodeldef( astk->light_seq, lm, lm->valid & mask, astk );
    glCallList(_mgopenglc->light_lists[astk->light_seq]);
  }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf( &_mgc->W2C[0][0] );
  mgopengl_lights( lm, astk );
  glPopMatrix();
}

void
mgopengl_lights( LmLighting *lm, struct mgastk *astk )
{
  int i, lightsused;
  LtLight *light, **lp;
  int baselight = -1;
  GLint maxlights;

  glGetIntegerv(GL_MAX_LIGHTS, &maxlights);

  /* unbind all currently bound OpenGL lights */
  for (i=0; i < maxlights; i++)
    glDisable(GL_LIGHT0+i);

  lightsused = 0;
  LM_FOR_ALL_LIGHTS(lm, i,lp) {
    light = *lp;
    if (light->Private == 0) {
      /* this is a new light */
      if(baselight < 0) {
	struct mgastk *a;
	/* Count appearance stack depth */
	for(a = astk, baselight = 1; a != NULL; a = a->next)
	  baselight += maxlights;
      }
      light->Private = lightsused + baselight;
      light->changed = 1;  /* set changed, to force lmdef below */
    }

    if (light->changed) {
      mgopengl_lightdef(GL_LIGHT0+lightsused, light, lm, lm->valid);
      light->changed = 0;
    } else {
      /* Even unchanged lights might have moved, if the
       * camera has moved since we last installed them.
       */
      glLightfv( GL_LIGHT0 + lightsused, GL_POSITION,
		 (float *)&light->globalposition );
    }

    glEnable( GL_LIGHT0+lightsused);

    ++lightsused;
  }
}


int
mgopengl_lightdef( int lightno, LtLight *light, LmLighting *lgt, int mask)
{
  GLfloat f[4];

  f[3] = 1.0;

  glLightfv(lightno, GL_AMBIENT, (float *)&light->ambient);

  f[0] = light->intensity * light->color.r;
  f[1] = light->intensity * light->color.g;
  f[2] = light->intensity * light->color.b;
  glLightfv(lightno, GL_DIFFUSE, f);

  glLightfv(lightno, GL_POSITION, (float *)&light->globalposition);

  if( mask & (LMF_ATTENC | LMF_ATTENM)) {
    glLightf(lightno, GL_CONSTANT_ATTENUATION, lgt->attenconst);
    glLightf(lightno, GL_LINEAR_ATTENUATION, lgt->attenmult);
  }

  return lightno;
}


int
mgopengl_lightmodeldef(int lightmodel, LmLighting *lgt, int mask, struct mgastk *astk)
{
  GLfloat f[4];

  glNewList(_mgopenglc->light_lists[lightmodel], GL_COMPILE);

  f[3] = 1.0;
  if( mask & LMF_AMBIENT) {
    f[0] = lgt->ambient.r;
    f[1] = lgt->ambient.g;
    f[2] = lgt->ambient.b;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, f);
  }

  if( mask & LMF_LOCALVIEWER) {
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  }

#ifndef TRUE_EMISSION
  /* This causes trouble if the vertex order makes GL consider
   * our polygon to be backfacing -- then GL_LIGHT_MODEL_TWO_SIDE causes it
   * to be mis-shaded from both sides..
   */
  if((astk->mat.valid & MTF_EMISSION) ) {
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  }
#endif

  glEndList();
  return lightmodel;
}

#if defined(sgi) || defined(GL_EXT_texture_object) /* Might our -lGL know about EXT_texture_object? */
/* Since the SGI compiler/linkers seem to know about #pragma weak, but
 * others might not, compile in EXT_texture_object support if:
 *  - we're on an SGI; even if compiled under Irix 5 (no EXT_texture_object),
 *	  we might be run under a later version that does support it, and
 *	  the "#pragma weak" lets us test at run time whether the library
 *	  supports it;
 *  - or, if we're on a system that claims to support it at compile time,
 *	  we'll just hope that that system also supports it at run time.
 */

# pragma weak glBindTextureEXT
# pragma weak glDeleteTexturesEXT

# ifndef GL_EXT_texture_object
/* If <GL/gl.h> doesn't know about glBindTextureEXT etc., declare here. */
extern void glBindTextureEXT (GLenum target, GLuint texture);
extern void glDeleteTexturesEXT (GLsizei n, GLuint *textures);
# endif

/* If the library knows about it, does the X display also support the
 * texture extension?
 */

static int has_texture_object(void) {
  static int has = -1;
  if(has < 0)
    has = (&glBindTextureEXT != NULL && &glDeleteTexturesEXT != NULL
	   && NULL != strstr((char *)glGetString(GL_EXTENSIONS),
			     "EXT_texture_object"));
  return has;
}

#else

/* Stub for non-existent glBindTextureEXT() etc. functions. */
static int has_texture_object(void) { return 0; }
static void glBindTextureEXT(GLenum target, GLuint id) { }
static void glDeleteTexturesEXT(GLsizei n, GLuint *textures) { }

#endif

static void tex_predef(GLuint id)
{
  if(has_texture_object()) {
    glBindTextureEXT(GL_TEXTURE_2D, id);
  } else {
    if ((GLuint)_mgopenglc->n_texture_lists <= id) {
      _mgopenglc->texture_lists =
	mgopengl_realloc_lists(_mgopenglc->texture_lists,
			       &_mgopenglc->n_texture_lists);
    }
    glNewList(_mgopenglc->texture_lists[id], GL_COMPILE_AND_EXECUTE);
  }
}

static void tex_postdef(void)
{
  if(has_texture_object()) {
    /* Nothing -- leave things as they are */
  } else {
    glEndList();
  }
}

static void tex_bind(GLuint id) {
  if(has_texture_object()) {
    glBindTextureEXT(GL_TEXTURE_2D, id);
  } else {
    glCallList(_mgopenglc->texture_lists[id]);
  }
}

static void tex_delete(GLuint id) {
  if(has_texture_object()) {
    glDeleteTexturesEXT(1, &id);
  } else {
    glDeleteLists(_mgopenglc->texture_lists[id], 1);
  }
}

/* Is this texture loaded adequately for the given texture-quality setting? */
static bool tex_adequate(int apflags, TxUser *tu)
{
  struct mgopengl_tudata *tudata = tu->data;

  return
    (apflags & (APF_TXMIPMAP|APF_TXMIPINTERP|APF_TXLINEAR))
    ==
    tudata->qualflags;
}
  

/*
 * Function:    mgopengl_notexture
 * Description: Disable texture mapping
 */
void
mgopengl_notexture()
{
  glDisable(GL_TEXTURE_2D);
  _mgopenglc->tevbound = 0;
  glAlphaFunc(GL_ALWAYS, 0);
  glDisable(GL_ALPHA_TEST);
}


/*
 * Function:	mgopengl_txpurge
 * Description: Forget everything Open GL ever knew about this texture.
 * Author:	slevy
 * Date:	Thu May 30 12:03:26 CDT 1996
 * Notes:	The mg/common/mgtexture.c code decides when the time is ripe.
 *		We assume that textures (well, display-lists) are shared
 *		across all OpenGL windows, and take care to purge references
 *		to the no-longer-loaded texture in all mgopengl context.
 */
void
mgopengl_txpurge(TxUser *tu)
{
  mgcontext *ctx, *oldctx = _mgc;
  struct mgopengl_tudata *tudata;

  for(ctx = _mgclist; ctx != NULL; ctx = ctx->next) {
    if(ctx->devno == MGD_OPENGL) {
#define mgoglc  ((mgopenglcontext *)ctx)
      if(mgoglc->curtex == tu) {
	if(mgoglc->tevbound) {	/* Probably won't happen */
	  mgctxselect(ctx);
	  mgopengl_notexture();
	}
	mgoglc->curtex = NULL;
      }
#if 0
      if(mgoglc->bgimage == tu) {
	mgoglc->bgimage = NULL;
      }
#endif
#undef mgoglc
    }
  }
  if(tu->id > 0) {
    tex_delete(tu->id + ID_OFFSET);
  }

  if ((tudata = tu->data)) {
    if (tudata->data != tu->tx->image->data) {
      OOGLFree(tudata->data);
    }
    OOGLFree(tudata);
    tu->data = NULL;
  }

  /* Could also purge the current 2D texture, but maybe it's not worth it. */
  if (_mgc != oldctx) {
    mgctxselect(oldctx);
  }

}


/*-----------------------------------------------------------------------
 * Function:    mgopengl_needtexture
 * Description: Ask for a texture -- we need the texture currently in astk->ap.t
 ex
 * Author:      slevy
 * Date:        Mon Jan 29 21:16:13 CST 1996
 * Notes:       We only do this when a gprim asks for it, not as soon as
 *              we start the appearance in which the texture is installed,
 *              for efficiency's sake.  Textures are bulky and rendering with
 *              them can be slow.  Thus relevant gprim's test:
 *              if(_mgc->astk->flag&APF_TEXTURE &&
 *                      _mgc->astk->ap.tex != NULL &&
 *                      _mgglc->tevbound == 0)
 *                 mgopengl_needtexture();
 */

void mgopengl_needtexture(void)
{
  Texture *wanttx = _mgc->astk->ap.tex;
  int apflag = _mgc->astk->ap.flag;
  TxUser *tu;
  struct mgopengl_tudata *tudata;
  Image *image;
  int id, mustload = 0;

  static GLint formats[] =
    { 0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };

  static GLfloat minfilts[] = {
    GL_NEAREST, GL_LINEAR,
    GL_NEAREST, GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, 
    GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
  };

  if(wanttx == NULL || wanttx->image == NULL) {
#if 1
    mgopengl_notexture();
#else
    /* Remove any texture */
    glDisable(GL_TEXTURE_2D);
    _mgopenglc->tevbound = 0;
#endif
    /* Let's leave the texture bound, in case we need it again soon. */
    return;
  }

  image = wanttx->image;

  if ((tu = _mgopenglc->curtex) &&
      mg_same_texture(tu->tx, wanttx, true) && tex_adequate(apflag, tu)) {

    if (_mgopenglc->tevbound != tu->id) {
      /* We just need to bind the texture environment */
      _mgopenglc->tevbound = tu->id;

      tex_bind(tu->id + ID_OFFSET);

      if (image->channels % 2 == 0) {
	glAlphaFunc(GL_NOTEQUAL, 0);
	glEnable(GL_ALPHA_TEST);
      }
    }

    /* Load the texture tfm, even if stuff is otherwise identical. */
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf((GLfloat *)_mgc->txstk->T);
    glMultMatrixf((GLfloat *)wanttx->tfm);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    return;
  }

  /* Is our texture in the cache? */
  tu = mg_find_shared_texture(wanttx, MGD_OPENGL);

  if (tu == NULL || !tex_adequate(apflag, tu)) {
    /* Find a free texture id. */
    id = mg_find_free_shared_texture_id(MGD_OPENGL);
    tu = TxAddUser(wanttx, id, NULL, mgopengl_txpurge);
    tu->ctx  = _mgc;
    tudata   = OOGLNewE(struct mgopengl_tudata, "OpengGL TxUser Data");
    tudata->data     = image->data;
    tudata->xsize    = image->width;
    tudata->ysize    = image->height;
    tudata->channels = image->channels;
    tu->data = tudata;
    mustload = 1;
  } else {
    tudata = tu->data;
    if (!mg_same_texture(tu->tx, wanttx, true)) {
      /* force the texture parameters to be reprogrammed */
      _mgopenglc->tevbound = 0;
    }
  }

  /* Configure texturing as described in wanttx, except for the data. */
  if (_mgopenglc->tevbound != tu->id || mustload) {
    switch (wanttx->apply) {
    case TXF_BLEND:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
      break;
    case TXF_DECAL:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      break;
    case TXF_REPLACE:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      break;
    default:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    }
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
	       (GLfloat *)&wanttx->background);
    _mgopenglc->tevbound = tu->id;

    if (image->channels == 2 || image->channels == 4) {
      glAlphaFunc(GL_NOTEQUAL, 0);
      glEnable(GL_ALPHA_TEST);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf( (GLfloat *) wanttx->tfm);
    glMatrixMode(GL_MODELVIEW);
  }

  if (mustload) {
    /* Stuff texture data into GL */
    GLint format = formats[image->channels];

    tex_predef(tu->id + ID_OFFSET);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		    (wanttx->flags & TXF_SCLAMP) ? GL_CLAMP : GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		    (wanttx->flags & TXF_TCLAMP) ? GL_CLAMP : GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    minfilts[((apflag & APF_TXMIPMAP) ? 4 : 0) |
			     ((apflag & APF_TXMIPINTERP) ? 2 : 0) |
			     ((apflag & APF_TXLINEAR) ? 1 : 0)]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		    (apflag & APF_TXLINEAR) ? GL_LINEAR : GL_NEAREST);

    if(apflag & APF_TXMIPMAP) {
      gluBuild2DMipmaps(GL_TEXTURE_2D,
			tudata->channels, tudata->xsize, tudata->ysize,
			format, GL_UNSIGNED_BYTE,
			tudata->data);
    } else {
      if (tudata->data == image->data &&
	  ((image->width & (image->width - 1)) != 0 ||
	   (image->height & (image->height - 1)) != 0)) {
	GLint newx = 4, newy = 4;

	/* Approximate round-to-nearest */
	while (newx*3 < tudata->xsize*2) newx *= 2;
	while (newy*3 < tudata->ysize*2) newy *= 2;

	tudata->data = (char *)malloc(newx * newy * tudata->channels);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	gluScaleImage(format, tudata->xsize, tudata->ysize,
		      GL_UNSIGNED_BYTE, image->data, newx, newy,
		      GL_UNSIGNED_BYTE, tudata->data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); /* default */
	tudata->xsize = newx;
	tudata->ysize = newy;
      }

      glTexImage2D(GL_TEXTURE_2D, 0, tudata->channels,
		   tudata->xsize, tudata->ysize,
		   0, format, GL_UNSIGNED_BYTE, tudata->data);
    }
    tex_postdef();
    /* Remember the conditions under which we loaded this texture. */
    tudata->qualflags = apflag & (APF_TXMIPMAP|APF_TXMIPINTERP|APF_TXLINEAR);
    _mgopenglc->curtex = tu;
  }

  if (_mgopenglc->curtex != tu) {
    /* Now bind the texture and select display mode. */
    tex_bind(tu->id + ID_OFFSET);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    minfilts[((apflag & APF_TXMIPMAP) ? 4 : 0) |
			     ((apflag & APF_TXMIPINTERP) ? 2 : 0) |
			     ((apflag & APF_TXLINEAR) ? 1 : 0)]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		    (apflag & APF_TXLINEAR) ? GL_LINEAR : GL_NEAREST);
    _mgopenglc->curtex = tu;
  }

  glEnable(GL_TEXTURE_2D);
}
/*-----------------------------------------------------------------------
 * Function:	mgopengl_d4f
 * Description:	wrapper for c4f
 * Args:	c:
 * Returns:	
 * Author:	munzner
 * Date:	Wed Sep 18 21:48:08 1991
 * Notes:	We must multiply by kd (diffuse coefficient of the material)
 * 		since we called glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);glEnable(GL_COLOR_MATERIAL) earlier in mgopengl_material
 * 		so we're overwriting the diffuse material with every
 *		c4f call.
 */
#if 0
void
mgopengl_d4f(float c[4])
{
  float d[4];
  d[0] = c[0] * kd;
  d[1] = c[1] * kd;
  d[2] = c[2] * kd;
  d[3] = c[3];
  /* Let appearance alpha override object's alpha */
  if (_mgc->astk->mat.valid & MTF_ALPHA &&
      _mgc->astk->mat.override & MTF_ALPHA)
    d[3] = _mgc->astk->mat.diffuse.a;
  glColor4fv(d);
}
#else
/* cH: even for constant shading we want to have the possibility that
 * the material alpha overrides the alpha value of the color. Also,
 * the decision about that can be done in mgopengl_appearance(). We
 * just define 4 functions: shaded w/o alpha override and constant
 * w/o alpha override.
 */

/* shaded, but without alpha override */
void
mgopengl_d4f_shaded(float c[4])
{
  float d[4];
  d[0] = c[0] * kd;
  d[1] = c[1] * kd;
  d[2] = c[2] * kd;
  d[3] = c[3];
  glColor4fv(d);
}

/* shaded, with alpha override */
void
mgopengl_d4f_shaded_alpha(float c[4])
{
  float d[4];
  d[0] = c[0] * kd;
  d[1] = c[1] * kd;
  d[2] = c[2] * kd;
  d[3] = _mgc->astk->mat.diffuse.a;
  glColor4fv(d);
}

/* constant, without alpha override: this is just glColor4fv */

/* constant, with alpha override */
void
mgopengl_d4f_constant_alpha(float c[4])
{
  float orig_a = c[3];
  
  c[3] = _mgc->astk->mat.diffuse.a; /* what if c is not writable? */
  glColor4fv(c);
  c[3] = orig_a;
}

#endif

void
mgopengl_n3fevert(Point3 *n, HPoint3 *p)
{
  Point3 tn, diff;
  HPoint3 *cp;

  if (!(_mgc->has & HAS_CPOS)) {
    mg_findcam();
  }
  cp = &_mgc->cpos;
  /* we must not assume that p is normalized here. Why should this be
   * the case? cp is a Point3, but p is a fully qualified HPoint3
   * here.
   */
  HPt3SubPt3(p, cp, &diff);
  if ((cp->w != 0 ? cp->w : 1.0) * Pt3Dot(&diff, n) > 0) {
    tn.x = -n->x;
    tn.y = -n->y;
    tn.z = -n->z;
    glNormal3fv((float *)&tn);
  } else {
    glNormal3fv((float *)n);
  }
}


void
mgopengl_v4fcloser(HPoint3 *p)
{
  HPoint3 tp;
  HPoint3 *cp = &_mgc->cpos;
  HPt3Coord wn = _mgc->zfnudge * p->w;
    
  if(!(_mgc->has & HAS_CPOS)) {
    mg_findcam();
  }
  if (cp->w != 0.0) {
    wn /= cp->w;
  }
  tp.x = p->x + wn * cp->x;
  tp.y = p->y + wn * cp->y;
  tp.z = p->z + wn * cp->z;
  tp.w = p->w + wn;
  glVertex4fv((float *)&tp);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
