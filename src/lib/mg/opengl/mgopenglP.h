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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mgopengl.h"

#ifdef GLUT
# include <GL/glut.h>
#else
# define GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glx.h>
#endif

#include "bsptree.h"

enum { SGL=0, DBL=1 };

typedef struct mgopenglcontext {
  struct mgcontext mgctx;	/* The mgcontext */
  int born;			/* Has window been displayed on the screen? */
  int win;			/* GL window ID, or 0 */
  double zmin, zmax;		/* max z-buffer value */
  ColorA Cd;			/* Cached diffuse color = mat->diffuse * Kd */
  int lmcolor;			/* lmcolor: GL_DIFFUSE or XXX lmc_COLOR */

  double znudge;		/* _mgc.zfnudge in integer Z-buffer units */
  double znear, zfar;		/* Current Z-buffer glDepthRange(, $2) limits */
  int oldopts;			/* For knowing when we need to reconfigure */
  vvec room;			/* Scratch space */
#ifdef _WIN32

  short used4f;			/* Windows VC++ doesn't like us to use the */
  short useevn3f; 		/* above function pointers. Use flags instead.*/

#  define D4F_ON()	_mgopenglc->used4f = 1
#  define D4F_OFF()	_mgopenglc->used4f = 0
#  define D4F(c)   {	if(_mgopenglc->used4f) mgopengl_d4f(c); \
			else glColor4fv(c); \
		   }
#  define N3F_NOEVERT()	_mgopenglc->useevn3f = 0
#  define N3F_EVERT()	_mgopenglc->useevn3f = 1
#  define N3F(n,p) {	if(_mgopenglc->useevn3f) mgopengl_n3fevert(n,p); \
			else glNormal3fv(n); \
		   }

#else /* use function pointers if the compiler permits */

  void (*d4f)();		/* For shaded colors: apply mat.kd or no? */
  void (*n3f)();		/* Evert normals to face viewer? */
#  define D4F_ON()					\
  if (_mgc->astk->mat.valid & MTF_ALPHA &&		\
      _mgc->astk->mat.override & MTF_ALPHA) {		\
    _mgopenglc->d4f = mgopengl_d4f_shaded_alpha;	\
  } else {						\
    _mgopenglc->d4f = mgopengl_d4f_shaded;		\
  }
#  define D4F_OFF()					\
  if (_mgc->astk->mat.valid & MTF_ALPHA &&		\
      _mgc->astk->mat.override & MTF_ALPHA) {		\
    _mgopenglc->d4f = mgopengl_d4f_constant_alpha;	\
  } else {						\
    _mgopenglc->d4f = glColor4fv;			\
  }
#  define D4F(c)	(*_mgopenglc->d4f)(c)
#  define N3F_EVERT()	_mgopenglc->n3f = mgopengl_n3fevert
#  define N3F_NOEVERT()	_mgopenglc->n3f = glNormal3fv
#  define N3F(n,p)	(*_mgopenglc->n3f)(n,p)

#endif

#ifndef GLUT
  void *GLXdisplay;		/* X11 Display pointer; if non-NULL, mixed model */
  GLXContext cam_ctx[2];	/* single[0] and double-buf[1] OGL contexts */
  GLXContext curctx;		/* Current OpenGL context */
#endif
  int winids[2];		/* single[0] and double-buffered[1] X win ids */
  int should_lighting, is_lighting; /* Lighting flags so we can turn off
					lighting for points&lines */
  int tevbound;                 /* Texturing currently enabled? */
  TxUser *curtex;		/* Currently-bound texture (NULL if none) */
  int dither;			/* Dither enabled? */
  char *bgimagefile;            /* compatibility: bgimage file name */

  GLuint *light_lists;
  int n_light_lists;
  GLuint *texture_lists;
  int n_texture_lists;
  GLuint *translucent_lists;
  int n_translucent_lists;
  GLuint translucent_seq;
} mgopenglcontext;

#define DPYLIST_INCR 10

#define	MAXZNUDGE	8	/* Max possible depth of mgopengl_closer()/farther() calls */

/* We save the current W2C/C2W xforms on each call to mgopengl_worldbegin()
 * because the ModelView matrix stack actually stores both our object
 * xform and the view matrix:
 *
 *   ModelView = [ obj xform ] * [ W2C ]
 *
 * Since we use the GL ModelView stack to keep track of our object xform,
 * instead of using the actual stack in the mgcontext struct, having
 * ModelView = W2C corresponds to [ obj xform ] = identity.  We save W2C
 * and C2W on each mgopengl_worldbegin() because _mgc->cam might change
 * during the course of the frame, but isn't supposed to be reinterpreted
 * until the next mgopengl_worldbegin().
 */

#define _mgopenglc		((mgopenglcontext*)_mgc)

#if 0
extern void mgopengl_d4f( float c[4] );
#else
extern void mgopengl_d4f_shaded(float c[4]);
extern void mgopengl_d4f_shaded_alpha(float c[4]);
extern void mgopengl_d4f_constant_alpha(float c[4]);
#endif
extern void mgopengl_n3fevert( Point3 *n, HPoint3 *p );

extern void mgopengl_notexture(void);
extern void mgopengl_needtexture(void);
extern void mgopengl_closer(void);
extern void mgopengl_farther(void);
extern void mgopengl_setshader(mgshadefunc shader);
extern void mgopengl_drawnormal(HPoint3 *p, Point3 *n);
extern void mgopengl_init_zrange();
extern void mgopengl_v4fcloser(HPoint3 *);
extern void mgopengl_txpurge(TxUser *);
extern void mgopengl_lights(LmLighting *lm, struct mgastk *astk);

extern void mgopengl_appearance( struct mgastk *ma, int mask );

extern GLuint *mgopengl_realloc_lists(GLuint *lists, int *n_lists);
extern GLuint mgopengl_new_translucent(Transform T);
extern void mgopengl_end_translucent(void);
extern void mgopengl_bsptree(BSPTree *bsptree);

#ifdef _WIN32
extern void mgopengl_c4f(float *);
extern void mgopengl_n3f(float *);
#else
#define mgopengl_c4f glColor4fv
#define mgopengl_n3f glNormal3fv
#endif

#define	MAY_LIGHT()  { \
	if(_mgopenglc->should_lighting && !_mgopenglc->is_lighting) { \
	    glEnable(GL_LIGHTING); \
	    _mgopenglc->is_lighting = 1; \
	} }

#define DONT_LIGHT() { \
	    if(_mgopenglc->is_lighting) { \
		glDisable(GL_LIGHTING); \
		_mgopenglc->is_lighting = 0; \
	    } \
	}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
