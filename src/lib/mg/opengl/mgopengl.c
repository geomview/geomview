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
#include "windowP.h"
#include "mgopenglshade.h"
#include "mgopenglstipple.h"

#ifdef GLUT
# include <GL/glut.h>
#else
#define GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glx.h>
#endif
#include <stdio.h>

#ifdef GLUT
# define  Window  int
#endif

#include "polylistP.h"

mgcontext * mgopengl_ctxcreate(int a1, ...);
int	    mgopengl_ctxset( int a1, ...  );
int	    mgopengl_feature( int feature );
void	    mgopengl_ctxdelete( mgcontext *ctx );
int	    mgopengl_ctxget( int attr, void* valueptr );
int	    mgopengl_ctxselect( mgcontext *ctx );
void	    mgopengl_sync( void );
void	    mgopengl_worldbegin( void );
void	    mgopengl_worldend( void );
void	    mgopengl_reshapeviewport( void );
void	    mgopengl_identity( void );
void	    mgopengl_transform( Transform T );
int	    mgopengl_pushtransform( void );
int	    mgopengl_poptransform( void );
void	    mgopengl_gettransform( Transform T );
void	    mgopengl_settransform( Transform T );
int	    mgopengl_pushappearance( void );
int	    mgopengl_popappearance( void );
const Appearance *mgopengl_setappearance(const Appearance* app, int merge );
int	    mgopengl_setcamera( Camera* cam );
int	    mgopengl_setwindow( WnWindow *win, int final );
mgopenglcontext *mgopengl_newcontext( mgopenglcontext *ctx );
void mgopengl_taggedappearance(const void *tag);

extern void mgopengl_polygon();
extern void mgopengl_line();
extern void mgopengl_polyline();
extern void mgopengl_polylist(int np, Poly *p, int nv, Vertex *v, int plflags);
extern void mgopengl_mesh(int wrap, int nu, int nv,
			  HPoint3 *meshP, Point3 *meshN, Point3 *meshNQ,
			  ColorA *meshC, TxST *meshST,
			  int mflags);
extern void mgopengl_quads(int count, HPoint3 *V, Point3 *N, ColorA *C,
			   int qflags);

int _mgopengl_ctxset(int a1, va_list *alist);
static void mgopengl_choosewin(void), mgopengl_initwin(void);
static void mgopengl_setviewport(void);
static void mgimgfile2cambgimage(void);

#ifndef GLUT
static GLXContext mgopengl_sharectx(void);
#endif

WnWindow *mgopenglwindow(WnWindow *win);

struct mgfuncs mgopenglfuncs = {
  MGD_OPENGL,
  mgdevice_OPENGL,
  mgopengl_feature,
  (mgcontext *(*)())mgopengl_ctxcreate,
  mgopengl_ctxdelete,
  (int (*)())mgopengl_ctxset,
  mgopengl_ctxget,
  mgopengl_ctxselect,
  mgopengl_sync,
  mgopengl_worldbegin,
  mgopengl_worldend,
  mgopengl_reshapeviewport,
  /* geometry transform */
  mgopengl_settransform,
  mgopengl_gettransform,
  mgopengl_identity,
  mgopengl_transform,
  mgopengl_pushtransform,
  mgopengl_poptransform,
  /* texture transform */
  mg_settxtransform,
  mg_gettxtransform,
  mg_txidentity,
  mg_txtransform,
  mg_pushtxtransform,
  mg_poptxtransform,
  /* appearance stack */
  mgopengl_pushappearance,
  mgopengl_popappearance,
  mgopengl_setappearance,
  mg_getappearance,
  /*******************/
  mgopengl_setcamera,
  mgopengl_polygon,
  mgopengl_polylist,
  mgopengl_mesh,
  mgopengl_line,
  mgopengl_polyline,
  mgopengl_quads,
  mg_bezier,
  mgopengl_bsptree,
  mg_tagappearance,
  mg_untagappearance,
  mgopengl_taggedappearance
};

int
mgdevice_OPENGL()
{
  _mgf = mgopenglfuncs;
  if (_mgc != NULL && _mgc->devno != MGD_OPENGL)
    _mgc = NULL;
  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_ctxcreate
 * Date:	Thu Jul 18 18:55:18 1991
 * Author:	mbp
 * Notes:	see mg.doc for rest of spec
 */
mgcontext *
mgopengl_ctxcreate(int a1, ...)
{
  va_list alist;


  _mgc = (mgcontext*)mgopengl_newcontext(OOGLNewE(mgopenglcontext,
						  "mgopengl_ctxcreate"));

  va_start(alist, a1);
  if (_mgopengl_ctxset(a1, &alist) == -1) {
    mgopengl_ctxdelete(_mgc);
  }
  va_end(alist);
  return _mgc;
}

/*-----------------------------------------------------------------------
 * Function:	_mgopengl_ctxset
 * Description:	internal ctxset routine
 * Args:	a1: first attribute
 *		*alist: rest of attribute-value list
 * Returns:	-1 on error, 0 on success
 * Author:	mbp
 * Date:	Fri Sep 20 11:08:13 1991
 * Notes:	mgopengl_ctxcreate() and mgopengl_ctxset() call this to actually
 *		parse and interpret the attribute list.
 */
int
_mgopengl_ctxset(int a1, va_list *alist)
{
  int attr;
  char **ablock = NULL;

#define NEXT(type) OOGL_VA_ARG(type,alist,ablock)

  for (attr = a1; attr != MG_END; attr = NEXT(int)) {
    switch (attr) {
    case MG_ABLOCK:
      ablock = NEXT(char**);
      break;
    case MG_ApSet:
      {
	Appearance *ap;

	if (ablock) {
	  ap = ApSet(NULL, AP_ABLOCK, ablock);
	} else {
	  ap = _ApSet(NULL, va_arg(*alist, int), alist);
	}

	mgopengl_setappearance(ap, MG_MERGE);
	ApDelete(ap);
      }
      break;
    case MG_WnSet:
      if (ablock) {
	WnSet( _mgc->win, WN_ABLOCK, ablock);
      } else {
	_WnSet( _mgc->win, va_arg(*alist, int), alist);
      }
      mgopengl_setwindow( _mgc->win, 0 );
      break;
    case MG_CamSet:
      if (ablock) {
	CamSet( _mgc->cam, CAM_ABLOCK, ablock);
      } else {
	_CamSet( _mgc->cam, va_arg(*alist, int), alist); break;
      }
      break;

    case MG_APPEAR: mgsetappearance(NEXT(Appearance *), MG_SET);
      break;
    case MG_WINDOW: mgopengl_setwindow( NEXT(WnWindow *), 0 ); break;
    case MG_CAMERA: mgopengl_setcamera( NEXT(Camera*) ); break;
    case MG_SETOPTIONS: _mgc->opts |= NEXT(int); break;
    case MG_UNSETOPTIONS: _mgc->opts &= ~NEXT(int); break;
    case MG_SHOW: _mgc->shown = NEXT(int); break;
    case MG_PARENT: _mgc->parent = NEXT(mgcontext*); break;
    case MG_BACKGROUND: {
      _mgc->background = *NEXT(ColorA*);
      if (_mgc->cam) {
	CamSet(_mgc->cam, CAM_BGCOLOR, &_mgc->background, CAM_END);
      }
      break;
    }

    case MG_SHADER: mgopengl_setshader( NEXT(mgshadefunc) ); break;
    case MG_SHADERDATA: _mgc->astk->shaderdata = NEXT(void*); break;

    case MG_SPACE:
      {
	int space = NEXT(int);
	switch (TM_SPACE(space)) {
	case TM_EUCLIDEAN:
	case TM_SPHERICAL:
	  _mgc->space = space;
	  break;
	case TM_HYPERBOLIC:
	  switch (TM_MODEL(space)) {
	  case TM_VIRTUAL:
	  case TM_PROJECTIVE:
	  case TM_CONFORMAL_BALL:
	    _mgc->space = space;
	    break;
	  default:
	    fprintf(stderr, "_mgopengl_ctxset: Illegal space value %1d\n", space);
	    break;
	  }
	  break;
	default:
	  fprintf(stderr, "_mgopengl_ctxset: Illegal space value %1d\n", space);
	  break;
	}
      }
      break;

    case MG_NDCTX: _mgc->NDctx = NEXT(mgNDctx *); break;

    case MG_WINCHANGE: _mgc->winchange = NEXT(mgwinchfunc); break;
    case MG_WINCHANGEDATA: _mgc->winchangeinfo = NEXT(void *); break;

    case MG_ZNUDGE:
      _mgc->zfnudge = NEXT(double);
      if(_mgopenglc->born) mgopengl_init_zrange();
      break;

    case MG_BITDEPTH:
      /*ignored*/ NEXT(int);
      break;

    case MG_DITHER:
      _mgopenglc->dither = NEXT(int);
      break;

    case MG_DEPTHSORT:
      /*ignored*/ NEXT(int);
      break;

      /* Open GL-specific */
    case MG_GLWINID:
      _mgopenglc->win = NEXT(int);
      break;

#ifndef GLUT
    case MG_GLXDISPLAY:
      _mgopenglc->GLXdisplay = NEXT(void *);
      break;
    case MG_GLXSINGLECTX:
      _mgopenglc->cam_ctx[SGL] = NEXT(GLXContext);
      break;
    case MG_GLXDOUBLECTX:
      _mgopenglc->cam_ctx[DBL] = NEXT(GLXContext);
      break;
#endif

    case MG_GLXSINGLEWIN:
      _mgopenglc->winids[SGL] = NEXT(int);
      break;
    case MG_GLXDOUBLEWIN:
      _mgopenglc->winids[DBL] = NEXT(int);
      break;

    case MG_BGIMAGEFILE: {
      char *fname = NEXT(char *);

      if (_mgopenglc->bgimagefile) {
	free(_mgopenglc->bgimagefile);
      }
      _mgopenglc->bgimagefile = fname ? strdup(fname) : NULL;
      mgimgfile2cambgimage();
      break;
    }
    default:
      OOGLError (0, "_mgopengl_ctxset: undefined option: %d", attr);
      return -1;
    }
  }

  if (_mgc->shown && !_mgopenglc->born) {

    /* open the window */
    mgopenglwindow(_mgc->win);

    /* bring gl state into accordance with appearance state */
    {
      Appearance *ap = ApCopy( &(_mgc->astk->ap), NULL );
      mgopengl_setappearance( ap, MG_SET );
      ApDelete(ap);
    }

  }

#undef NEXT

  return 0;
}


/*-----------------------------------------------------------------------
 * Function:	mgopengl_ctxget
 * Description:	get a context attribute value
 * Args:	attr: the attribute to get
 *		value: place to write attr's value
 * Returns:	1 for success; -1 if attr is invalid
 * Author:	mbp
 * Date:	Fri Sep 20 11:50:25 1991
 * Notes:
 */
int
mgopengl_ctxget(int attr, void* value)
{
#define VALUE(type) ((type*)value)

  switch (attr) {

    /* Attributes common to all MG contexts: */
  case MG_APPEAR: *VALUE(Appearance*) = &(_mgc->astk->ap); break;
  case MG_CAMERA: *VALUE(Camera*) = _mgc->cam; break;
  case MG_WINDOW:
    /* In GLUT case, we demand that the caller use mgctxset(MG_WINDOW, ...)
     * whenever our window size changes.  Under GLX/X11, we do it ourselves.
     */
#ifndef GLUT

    if(_mgopenglc->win > 0) {
      int x0, y0;
      WnPosition wp;
      unsigned int xsize, ysize, border_width, depth;
      Window dpyroot, toss;

      XGetGeometry((Display *) (_mgopenglc->GLXdisplay),
		   (Window) _mgopenglc->win, &dpyroot, &x0, &y0, &xsize,
		   &ysize, &border_width, &depth);
      XTranslateCoordinates((Display *) (_mgopenglc->GLXdisplay),
			    (Window) _mgopenglc->win, dpyroot, 0, ysize-1,
			    &x0, &y0, &toss);
      y0 = HeightOfScreen(
			  DefaultScreenOfDisplay((Display *) (_mgopenglc->GLXdisplay)))
	- y0 - 1;

      wp.xmin = (long)x0; wp.xmax = (long)(x0+xsize-1);
      wp.ymin = (long)y0; wp.ymax = (long)(y0+ysize-1);
      WnSet(_mgc->win, WN_CURPOS, &wp, WN_END);
    }
#endif
    *VALUE(WnWindow*) = _mgc->win;
    break;

  case MG_PARENT:	*VALUE(mgcontext*) = _mgc->parent; break;

  case MG_SETOPTIONS:
  case MG_UNSETOPTIONS:	*VALUE(int) = _mgc->opts; break;

  case MG_BACKGROUND:	*VALUE(ColorA) = _mgc->background; break;

  case MG_SHADER:	*VALUE(mgshadefunc) = _mgc->astk->shader; break;
  case MG_SHADERDATA:	*VALUE(void *) = _mgc->astk->shaderdata; break;
  case MG_SPACE:	*VALUE(int) = _mgc->space; break;
  case MG_NDCTX:	*VALUE(mgNDctx *) = _mgc->NDctx; break;
  case MG_ZNUDGE:	*VALUE(float) = _mgc->zfnudge; break;

  case MG_DEPTHSORT:	*VALUE(int) = MG_ZBUFFER; break;
  case MG_BITDEPTH:	*VALUE(int) = 24; break;
  case MG_DITHER:	*VALUE(int) = _mgopenglc->dither; break;

  case MG_WINCHANGE:	*VALUE(mgwinchfunc) = _mgc->winchange; break;
  case MG_WINCHANGEDATA: *VALUE(void *) = _mgc->winchangeinfo; break;

    /* Attributes specific to GL contexts: */
  case MG_GLWINID: *VALUE(int) = _mgopenglc->win; break;
  case MG_GLBORN: *VALUE(int) = _mgopenglc->born; break;
  case MG_GLZMAX: *VALUE(double) = _mgopenglc->zmax; break;

  case MG_GLXSINGLEWIN: *VALUE(Window) = _mgopenglc->winids[SGL]; break;
  case MG_GLXDOUBLEWIN: *VALUE(Window) = _mgopenglc->winids[DBL]; break;
#ifndef GLUT
  case MG_GLXSINGLECTX: *VALUE(GLXContext) = _mgopenglc->cam_ctx[SGL]; break;
  case MG_GLXDOUBLECTX: *VALUE(GLXContext) = _mgopenglc->cam_ctx[DBL]; break;

  case MG_GLXSHARECTX: {
    *VALUE(GLXContext) = mgopengl_sharectx();
    break;
  }
#endif

  case MG_BGIMAGEFILE:
    *VALUE(char *) = _mgopenglc->bgimagefile;
    break;

  default:
    OOGLError (0, "mgopengl_ctxget: undefined option: %d", attr);
    return -1;

  }
  return 1;

#undef VALUE
}

/*-----------------------------------------------------------------------
 * Function:	mgopenglwindow
 * Description:	create a GL window
 * Args:	*win: the WnWindow structure to realize
 * Returns:	win if success, NULL if not
 * Author:	mbp, slevy
 * Date:	Fri Sep 20 11:56:31 1991
 * Notes:	makes the GL calls necessary to create a GL window
 *		  corresponding to *win.  This involves the call to
 *		  winopen(), as well as various configuration things.
 */
WnWindow *
mgopenglwindow(WnWindow *win)
{
  mgopengl_choosewin();
  mgopengl_setwindow(win, 1);
  mgopengl_initwin();
  return win;
}


#ifndef GLUT
static GLXContext
mgopengl_sharectx(void)
{
  mgcontext *another;
  GLXContext shareable = NULL;
  for(another = _mgclist; another != NULL; another = another->next) {
    if(another->devno == MGD_OPENGL) {
      if((shareable = ((mgopenglcontext *)another)->cam_ctx[SGL]) != NULL
	 || (shareable = ((mgopenglcontext *)another)->cam_ctx[DBL]) != NULL)
	break;
    }
  }
  return shareable;
}
#endif/*!GLUT*/

/*-----------------------------------------------------------------------
 * Function:	mgopengl_ctxset
 * Description:	set some context attributes
 * Args:	a1, ...: list of attribute-value pairs
 * Returns:	-1 on error, 0 on success
 * Author:	mbp
 * Date:	Fri Sep 20 12:00:18 1991
 */
int mgopengl_ctxset( int a1, ...  )
{
  va_list alist;
  int result;

  va_start( alist, a1 );
  result = _mgopengl_ctxset(a1, &alist);
  va_end(alist);
  return result;
}


/*-----------------------------------------------------------------------
 * Function:	mgopengl_feature
 * Description:	report whether the GL device has a particular feature
 * Args:	feature: the feature to report on
 * Returns:	an int giving info about feature
 * Author:	mbp
 * Date:	Fri Sep 20 12:00:58 1991
 * Notes:	-1 means the feature is not present.
 *
 *		NO OPTIONAL FEATURES SUPPORTED YET.  ALWAYS RETURNS -1.
 */
int mgopengl_feature( int feature )
{
  return(-1);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_ctxdelete
 * Description:	delete a GL context
 * Args:	*ctx: context to delete
 * Returns:	nothing
 * Author:	slevy
 * Date:	Tue Nov 12 10:29:04 CST 1991
 * Notes:	Deleting the current context leaves the current-context
 *		pointer set to NULL.
 */
void mgopengl_ctxdelete( mgcontext *ctx )
{
  if(ctx->devno != MGD_OPENGL) {
    mgcontext *was = _mgc;
    mgctxselect(ctx);
    mgctxdelete(ctx);
    if(was != ctx)
      mgctxselect(was);
  } else {
#define mgoglc  ((mgopenglcontext *)ctx)

#ifdef GLUT
    if(mgoglc->winids[0] > 0)
      glutDestroyWindow(mgoglc->winids[0]);
    if(mgoglc->winids[1] > 0)
      glutDestroyWindow(mgoglc->winids[1]);
#else /*GLX*/
    if(mgoglc->born) {
      int i;
      for(i = SGL; i <= DBL; i++) {
	if(mgoglc->cam_ctx[i] && mgoglc->win > 0)
	  glXDestroyContext( (Display *)(mgoglc->GLXdisplay),
			     mgoglc->cam_ctx[i]);
      }
    }
#endif

    vvfree(&((mgopenglcontext *)ctx)->room);
    if (mgoglc->light_lists)
      free(mgoglc->light_lists);
    if (mgoglc->texture_lists)
      free(mgoglc->texture_lists);
    if (mgoglc->translucent_lists)
      free(mgoglc->translucent_lists);

    if (mgoglc->bgimagefile) {
      free(mgoglc->bgimagefile);
    }

    mg_ctxdelete(ctx);
    if(ctx == _mgc)
      _mgc = NULL;
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_ctxselect
 * Description:	select an MG context --- make it current
 * Args:	*ctx: the context to become current
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:04:41 1991
 */
int
mgopengl_ctxselect( mgcontext *ctx )
{
  if(ctx == NULL || ctx->devno != MGD_OPENGL) {
    return mg_ctxselect(ctx);
  }
  /* Yes, it's an OpenGL context.  Do something useful. */
  _mgc = ctx;
  if(_mgopenglc->win) {
#ifdef GLUT
    if(_mgopenglc->win <= 0)
      mgopengl_choosewin();
    if(_mgopenglc->win > 0)
      glutSetWindow(_mgopenglc->win);

#else /* GLX */
    if(_mgopenglc->GLXdisplay != NULL) {
      _mgopenglc->win = _mgopenglc->winids[ _mgc->opts & MGO_DOUBLEBUFFER ? DBL : SGL ];
      _mgopenglc->curctx = _mgopenglc->cam_ctx[ _mgc->opts & MGO_DOUBLEBUFFER ? DBL : SGL ];
      if(_mgopenglc->win > 0)
        glXMakeCurrent( (Display *)_mgopenglc->GLXdisplay,
			(Window) _mgopenglc->win,
			_mgopenglc->curctx);
    }
#endif /*GLX*/
  }
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_sync
 * Description:	flush buffered GL commands
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:06:09 1991
 * Notes:	Just flushes the GL buffer -- needed for remote displays.
 */
void
mgopengl_sync( void )
{ glFlush(); }

/*static int glut_initted = 0;*/

static void
mgopengl_makewin(int which)
{

#define dpy (_mgopenglc->GLXdisplay)

  int xsize, ysize;
#ifdef GLUT
  WnPosition wp;

  if(!glut_initted) {
    static int unargc = 1;
    static char *unargv[] = { "mg_glut", NULL };
    glutInit(&unargc, (char **)&unargv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glut_initted = 0;
  }
  if(WnGet(_mgc->win, WN_XSIZE, &xsize) <= 0 ||
     WnGet(_mgc->win, WN_YSIZE, &ysize) <= 0) {
    xsize = ysize = 200;
    WnSet(_mgc->win, WN_XSIZE, xsize, WN_YSIZE, 200, WN_END);
  }
  if(WnGet(_mgc->win, WN_PREFPOS, &wp) > 0) {
    glutInitWindowPosition(wp.xmin, wp.ymin);
  }
  glutInitWindowSize(xsize, ysize);
  _mgopenglc->winids[which] = glutCreateWindow("GLUT mg");
 
#else /* GLX */

  static int dblBuf[] = {
    GLX_DOUBLEBUFFER,
    GLX_RGBA, GLX_DEPTH_SIZE, 23,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, GLX_ALPHA_SIZE, 1,
    None
  };
  XSetWindowAttributes xswa;
  int scr = XDefaultScreen(dpy);
  Window root = XRootWindow(dpy, scr);
  XVisualInfo *vi = glXChooseVisual(dpy, scr, which==SGL ? dblBuf+1 : dblBuf);

  if(vi == NULL) {
    OOGLError(1, "Can't find an OpenGL-capable X visual.");
    exit(1);
  }

  _mgopenglc->cam_ctx[which] =
    glXCreateContext(dpy, vi, mgopengl_sharectx(), GL_TRUE);

  xswa.colormap = (vi->visual == XDefaultVisual(dpy, scr)) ?
    XDefaultColormap(dpy, scr) :
    XCreateColormap(dpy, root, vi->visual, AllocNone);

  xswa.event_mask = KeyPressMask|ButtonPressMask
    |ButtonReleaseMask|ButtonMotionMask
    |EnterWindowMask|StructureNotifyMask|ExposureMask;

  xswa.border_pixel = xswa.background_pixel = xswa.backing_pixel =
    xswa.background_pixmap = None;

  if(WnGet(_mgc->win, WN_XSIZE, &xsize) <= 0 ||
     WnGet(_mgc->win, WN_YSIZE, &ysize) <= 0) {
    xsize = ysize = 200;
    WnSet(_mgc->win, WN_XSIZE, xsize, WN_YSIZE, 200, WN_END);
  }

  _mgopenglc->winids[which] = XCreateWindow( dpy, root,
					     0, 0, xsize, ysize, 0/*border*/, vi->depth,
					     InputOutput, vi->visual,
					     CWEventMask|CWColormap|CWBorderPixel
					     |CWBackPixel|CWBackPixmap, &xswa);

  XMapWindow(dpy, _mgopenglc->winids[which]);

#undef dpy

#endif /*GLX*/
	
}


static void
mgopengl_choosewin(void)
{
  int which = (_mgc->opts & MGO_DOUBLEBUFFER) ? DBL : SGL;

  /* Are we on our own?  If caller didn't initialize our X state, do it now. */
#ifdef GLUT
  if(_mgopenglc->winids[which] == 0) {
    mgopengl_makewin(which);
    if(_mgopenglc->winids[which] == 0)
      which = 1-which;
  }

  if(_mgc->winchange)
    (*_mgc->winchange)(_mgc, _mgc->winchangeinfo,
		       MGW_DOUBLEBUF, (which == DBL));

  _mgopenglc->win = _mgopenglc->winids[which];
  glutSetWindow(_mgopenglc->win);

#else /* GLX */

  if(_mgopenglc->GLXdisplay == NULL && _mgopenglc->winids[which] == 0) {
    if((_mgopenglc->GLXdisplay = XOpenDisplay(NULL)) == NULL) {
      OOGLError(1, "Can't open X display");
      return;
    }
  }

  /* Did they tell us single/double-buffering is possible? */
  /* Switch back to the other buffer if need be */
  if(_mgopenglc->cam_ctx[which] == NULL) {
    if(_mgopenglc->cam_ctx[1-which] == NULL) {
      mgopengl_makewin(which);
    } else {
      which = 1-which;
    }
  }

  if(_mgc->winchange)
    (*_mgc->winchange)(_mgc, _mgc->winchangeinfo, MGW_DOUBLEBUF,
		       (which == DBL));

  _mgopenglc->curctx = _mgopenglc->cam_ctx[which];
  _mgopenglc->win = _mgopenglc->winids[which];
  if(_mgopenglc->win > 0) {
    XRaiseWindow( (Display *)_mgopenglc->GLXdisplay, (Window) _mgopenglc->win );
    glXMakeCurrent( (Display *)_mgopenglc->GLXdisplay,
		    (Window) _mgopenglc->win,
		    _mgopenglc->curctx);
  }

#endif /* GLX */

  if (_mgopenglc->n_light_lists == 0)
    _mgopenglc->light_lists =
      mgopengl_realloc_lists(NULL, &_mgopenglc->n_light_lists);
  if (_mgopenglc->n_texture_lists == 0)
    _mgopenglc->texture_lists =
      mgopengl_realloc_lists(NULL, &_mgopenglc->n_texture_lists);
  if (_mgopenglc->n_translucent_lists == 0)
    _mgopenglc->translucent_lists = 
      mgopengl_realloc_lists(NULL, &_mgopenglc->n_translucent_lists);

}

static void
mgopengl_setviewport()
{
  WnPosition vp, whole;

  WnGet(_mgc->win, WN_VIEWPORT, &vp);
  glViewport(vp.xmin, vp.ymin, vp.xmax-vp.xmin+1, vp.ymax-vp.ymin+1);
  glScissor(vp.xmin, vp.ymin, vp.xmax-vp.xmin+1, vp.ymax-vp.ymin+1);
  if(WnGet(_mgc->win, WN_CURPOS, &whole) <= 0 ||
     vp.xmax-vp.xmin < whole.xmax-whole.xmin ||
     vp.ymax-vp.ymin < whole.ymax-whole.ymin) {
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
  _mgc->win->changed &= ~WNF_HASVP;
}

static void
mgopengl_initwin()
{
  GLdouble zrange[2];
  int i;
  LtLight **lp;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf((GLfloat *)TM_IDENTITY);
  glMatrixMode(GL_MODELVIEW);

  _mgopenglc->oldopts = _mgc->opts;
  _mgopenglc->born = 1;

  glGetDoublev(GL_DEPTH_RANGE, zrange);
  _mgopenglc->zmin = zrange[0]; _mgopenglc->zmax = zrange[1];
  mgopengl_init_zrange();

  glClearDepth(_mgopenglc->zmax);
  glClearColor(_mgc->background.r, _mgc->background.g, _mgc->background.b,
	       _mgc->background.a);
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  if((_mgc->opts&MGO_DOUBLEBUFFER) && !(_mgc->opts&MGO_INHIBITSWAP) ) {
#ifdef GLUT
    glutSwapBuffers();
#else
    if(_mgopenglc->win > 0)
      glXSwapBuffers((Display *) (_mgopenglc->GLXdisplay), (Window) _mgopenglc->win);
#endif
  }

  mgopengl_setviewport();

  /* Ensure the lighting gets properly entered into this new window --
   * mark all lights as having changed.
   */
  LM_FOR_ALL_LIGHTS(&_mgc->astk->lighting, i,lp) {
    (*lp)->changed = 1;
  }

  mgopengl_setappearance(&_mgc->astk->ap, MG_SET);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_worldbegin
 * Description:	prepare to draw a frame
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:06:58 1991
 */
void
mgopengl_worldbegin( void )
{
  static bool first_time = true;
  Transform V;
  int which = (_mgc->opts & MGO_DOUBLEBUFFER) ? DBL : SGL;

  if (first_time) {
    first_time = false;
    
    mgopengl_init_polygon_stipple();
  }

  mg_worldbegin();	/* Initialize W2C, C2W, W2S, S2W, etc. */

  if(
#ifndef GLUT
     _mgopenglc->curctx != _mgopenglc->cam_ctx[which] ||
#endif
     (_mgc->opts ^ _mgopenglc->oldopts) & MGO_DOUBLEBUFFER) {
    mgopengl_choosewin();
    mgopengl_initwin();
    _mgopenglc->oldopts = _mgc->opts;
  }

  glColorMask(_mgc->opts & MGO_NORED ? GL_FALSE : GL_TRUE,
	      _mgc->opts & MGO_NOGREEN ? GL_FALSE : GL_TRUE,
	      _mgc->opts & MGO_NOBLUE ? GL_FALSE : GL_TRUE,
	      GL_TRUE );

  if(_mgc->win->changed & WNF_HASVP) {
    mgopengl_setviewport();
  }

  /* Erase to background color & initialize z-buffer */
  if (_mgc->opts & MGO_INHIBITCLEAR) {
    glClearDepth(1.);
    glClear(GL_DEPTH_BUFFER_BIT);
  } else {
    Image *bgimg;

    glClearDepth( _mgopenglc->zmax);
    glClearColor(_mgc->background.r,
		 _mgc->background.g, _mgc->background.b, _mgc->background.a);

    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

    if((bgimg = _mgc->bgimage)) {
      static GLint formats[] =
	{ 0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
      static GLdouble pos[] = {0.0, 0.0, -1.0};
      int xsize, ysize, off;

      /* Could resize the image to fit the window here,
       * (or force the window to fit the image!)
       * but let's make that the user's problem.
       */
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      WnGet(_mgc->win, WN_XSIZE, &xsize);
      WnGet(_mgc->win, WN_YSIZE, &ysize);
      glOrtho(0, xsize, 0, ysize, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      if(xsize >= bgimg->width) {
	pos[0] = (xsize - bgimg->width)/2;
	off = 0;
      } else {
	pos[0] = 0;
	off = (bgimg->width - xsize) / 2;
	glPixelStorei(GL_UNPACK_ROW_LENGTH, bgimg->width);
      }
      if(ysize >= bgimg->height) {
	pos[1] = (ysize - bgimg->height)/2;
      } else {
	pos[1] = 0;
	off += bgimg->width * ((bgimg->height - ysize) / 2);
      }
      glRasterPos3dv(pos);
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
      if (bgimg->channels % 1 == 0) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
      }
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glDrawPixels(xsize < bgimg->width ? xsize : bgimg->width,
		   ysize < bgimg->height ? ysize : bgimg->height,
		   formats[bgimg->channels],
		   GL_UNSIGNED_BYTE, bgimg->data + bgimg->channels*off );
      if (bgimg->channels % 1 == 0) {
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glDisable(GL_BLEND);
      }
      glDepthMask(GL_TRUE);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
  }

  glEnable(GL_DEPTH_TEST);

  glRenderMode(GL_RENDER);

  if(_mgopenglc->dither) {
    glEnable(GL_DITHER);
  } else {
    glDisable(GL_DITHER);
  }

  _mgc->has = 0;

  /* Interpret the camera: load the proper matrices onto the GL matrix
     stacks.  */
  if(!(_mgc->opts & MGO_INHIBITCAM)) {
    glMatrixMode(GL_PROJECTION);
    CamViewProjection( _mgc->cam, V );
    glLoadMatrixf( (GLfloat *) V);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( (GLfloat *) _mgc->W2C);

  }

  /* Bind the lights; do this here so we get lights in world coords. */
  /* Only do this if we're supposed to do lighting */

  if (IS_SHADED(_mgc->astk->ap.shading))
    mgopengl_lights(&_mgc->astk->lighting, _mgc->astk);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_worldend
 * Description:	finish drawing a frame
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:08:02 1991
 */
void
mgopengl_worldend( void )
{
  GLuint list;
  
  /* Execute and delete any pending translucent display lists. */
  for (list = 0; list < _mgopenglc->translucent_seq; list++) {
    glCallList(_mgopenglc->translucent_lists[list]);
  }

  for (list = 0; list < _mgopenglc->translucent_seq; list++) {
    /* Destroy the contents of the dpy lists for translucent objects. */
    glNewList(_mgopenglc->translucent_lists[list], GL_COMPILE);
    glEndList();
  }
  _mgopenglc->translucent_seq = 0;

  if((_mgc->opts&MGO_DOUBLEBUFFER) && !(_mgc->opts&MGO_INHIBITSWAP)) {
#ifdef GLUT
    glutSwapBuffers();
#else
    if(_mgopenglc->win > 0)
      glXSwapBuffers(_mgopenglc->GLXdisplay, (Window) _mgopenglc->win);
#endif
  }
  glFlush();

}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_reshapeviewport
 * Description:	adjust to a new window size
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:08:30 1991
 * Notes:	adjusts both GL's internal viewport setting, as well as
 *		MG context WnWindow's current position and camera's
 *		aspect ratio.
 */
void
mgopengl_reshapeviewport( void )
{
  float pixasp = 1;
  WnPosition vp;

  WnGet(_mgc->win, WN_PIXELASPECT, &pixasp);
  WnGet(_mgc->win, WN_VIEWPORT, &vp);
  mgopengl_setviewport();
  CamSet(_mgc->cam, CAM_ASPECT,
	 pixasp * (double)(vp.xmax-vp.xmin+1) /
	 (double)(vp.ymax-vp.ymin+1),
	 CAM_END);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_identity
 * Description:	set the current object xform to identity
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:23:48 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
void
mgopengl_identity( void )
{
  /* [ obj xform ] = identity corresponds to having current W2C on
     ModelView stack */
  mgopengl_settransform( TM3_IDENTITY );
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_transform
 * Description:	premultiply the object xform by T
 * Args:	T
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:24:57 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
void
mgopengl_transform( Transform T )
{
  glMultMatrixf( (GLfloat *) (T));
  TmConcat(T, _mgc->xstk->T, _mgc->xstk->T);
  _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_pushtransform
 * Description:	push the object xform stack
 * Returns:	nothing (???)
 * Author:	mbp
 * Date:	Fri Sep 20 12:25:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
int
mgopengl_pushtransform( void )
{
  glPushMatrix();
  mg_pushtransform();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_popransform
 * Description:	pop the object xform stack
 * Returns:	nothing (???)
 * Author:	mbp
 * Date:	Fri Sep 20 12:25:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
int
mgopengl_poptransform( void )
{
  glPopMatrix();
  mg_poptransform();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_gettransform
 * Description:	get the current object xform
 * Args:	T: place to write the current object xform
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:29:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.  This means we must multiply on the right by
 *		the current C2W matrix after reading the GL ModelView
 *		matrix.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
void
mgopengl_gettransform( Transform T )
{
  TmCopy(_mgc->xstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_settransform
 * Description:	set the current object xform to T
 * Args:	T
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:29:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.  This means we must first load W2C onto the
 *		modelview stact, then premult by T.
 *
 *		This assumes we're already in GL_MODELVIEW mode.
 */
void
mgopengl_settransform( Transform T )
{
  glLoadMatrixf( (GLfloat *) _mgc->W2C);
  glMultMatrixf( (GLfloat *) T);
  TmCopy(T, _mgc->xstk->T);
  _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_new_translucent
 * Description: start a display-list which stores a translucent object,
 *              the dpy-list will be called later after all opaque objects
 *              have been drawn.
 * Returns:	the display list.
 * Author:	cH
 * Date:	Fri Nov 17 2006
 */
GLuint mgopengl_new_translucent(Transform T)
{
  
  if (_mgopenglc->n_translucent_lists <= (int)_mgopenglc->translucent_seq) {
    _mgopenglc->translucent_lists =
      mgopengl_realloc_lists(_mgopenglc->translucent_lists,
			     &_mgopenglc->n_translucent_lists);
  }

  glNewList(_mgopenglc->translucent_lists[_mgopenglc->translucent_seq],
	    GL_COMPILE /* do not execute now */);

  /* include the side-effects into the displaylist */
  mgopengl_pushtransform();
  mgopengl_pushappearance();

  /* Load the current transformation matrix into the display list */
  mgopengl_settransform(T);

  /* alpha-blending will be enabled on-demand in mgopengl_bsptree(). */

  return _mgopenglc->translucent_lists[_mgopenglc->translucent_seq++];
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_end_translucent
 * Description: end the definition of a translucent object.
 * Returns:	the display list.
 * Author:	cH
 * Date:	Fri Nov 17 2006
 */
void mgopengl_end_translucent(void)
{

  /* include the side-effects into the displaylist */
  mgopengl_popappearance();
  mgopengl_poptransform();

  glEndList();
  /* No need to disable alpha-blending because the corresponding
   * enable-commands are wrapped into a GL_COMPILE display list.
   */
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_pushappearance
 * Description:	push the MG context appearance stack
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:54:19 1991
 */
int
mgopengl_pushappearance( void )
{
  mg_pushappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_popappearance
 * Description:	pop the MG context appearance stack
 * Returns:	nothing
 * Author:	mbp, munzner
 * Date:	Fri Sep 20 12:54:19 1991
 * Note:        We call mgopengl_lighting and mgopengl_material instead of
 *              just doing lmbinds here because those procedures make the
 *		proper existence checks and reset the GL drawing state.
 */
int
mgopengl_popappearance( void )
{
  struct mgastk *mastk = _mgc->astk;
  struct mgastk *mastk_next;

  if (! (mastk_next=mastk->next)) {
    OOGLError(0, "mgopengl_popappearance: appearance stack has only 1 entry.");
    return 0;
  }

  if ( ( (mastk->light_seq != mastk_next->light_seq)	/* lighting changed */
#ifndef TRUE_EMISSION
	 || ((mastk->mat.valid ^ mastk_next->mat.valid) & MTF_EMISSION)
	 /* GL_LIGHT_MODEL_TWO_SIDE changed */
#endif
	 ) && IS_SHADED(mastk->next->ap.shading))    /* lighting on */
    mgopengl_lighting(mastk_next, mastk_next->lighting.valid);
  mgopengl_appearance(mastk_next, mastk_next->ap.valid);

  mg_popappearance();

  if (_mgopenglc->tevbound && _mgopenglc->curtex->tx != mastk_next->ap.tex) {
    mgopengl_notexture(); /* Unbind our no-longer-needed texture */
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_setappearance
 * Author:	munzner, mbp
 * Date:	Wed Aug  7 01:08:07 1991
 * Notes:	when app=NULL, mergeflag = MG_MERGE is assumed
 *		  (regardless of the actual value of mergeflag).
 *		  In this case, we make the GL calls to bring
 *		  the GL state into agreement with the current
 *		  appearance.
 *
 *		The above isn't true any more; update these comments
 *		soon.  -- mbp Mon Sep 23 19:07:39 1991
 *
 *		things set here: material, lights, shading,
 *		  linewidth, transparency
 *		things not set here: normals (drawing, scaling,
 *		  everting), drawing faces vs. edges
 */
const Appearance *
mgopengl_setappearance(const Appearance *ap, int mergeflag )
{
  int changed, mat_changed, lng_changed;
  struct mgastk *mastk = _mgc->astk;
  Appearance *ma;
  static float nullarray[] = { 0.0 };

  ma = &(mastk->ap);

  /* Decide what changes */
  if (mergeflag == MG_MERGE) {
    changed = ap->valid &~ (ma->override &~ ap->override);
    mat_changed =
      ap->mat ? ap->mat->valid &~ (ma->mat->override &~ ap->mat->override) : 0;
    lng_changed =
      ap->lighting ? ap->lighting->valid &~ 
      (ma->lighting->override &~ ap->lighting->override) : 0;
  }
  else {
    changed = ap->valid;
    mat_changed = ap->mat ? ap->mat->valid : 0; 
    lng_changed = ap->lighting ? ap->lighting->valid : 0; 
  }

  /*
   * Update current appearance; this needs to be done before making GL
   * calls because it is conceivable that we might need to make a GL call
   * corresponding to something in the current appearance for which the
   * valid bit in *ap isn't set. (???)  By updating the current
   * appearance before making GL calls, our GL calls can always take data
   * from the current appearance, rather than worrying about whether to
   * read *ap or the current appearance.
   */
  mg_setappearance( ap, mergeflag );

  /* Bring GL device into accord with new appearance */
  if (_mgopenglc->born) {

    /*
     * No bit in "changed" corresponds to {lighting,mat}.  We think of
     * ap->{lighting,mat} as an extension to *ap which is interpreted to
     * have all valid bits 0 if the {lighting,ap} pointer is NULL.  Note
     * that this means there is no way for the parent to override the
     * entire {lighting,mat} structure as a whole.  It can, however, set
     * the individual override bits in the {lighting,mat} structure.
     */
    if ((ap->lighting) && (mastk->next)) {
      if (mastk->light_seq == mastk->next->light_seq) {
	mastk->light_seq = next_light_seq(_mgc, mastk);	
	if (_mgopenglc->n_light_lists <= mastk->light_seq) {
	  _mgopenglc->light_lists =
	    mgopengl_realloc_lists(_mgopenglc->light_lists,
				   &_mgopenglc->n_light_lists);
	}	
	/*
	 * We need a new lighting model.
	 * To ensure we don't have any leftover garbage in GL's copy of this
	 * lighting model, we force GL to reset to defaults, then
	 * reinitialize everything.
	 */
	glNewList(_mgopenglc->light_lists[mastk->light_seq], GL_COMPILE);
	glMaterialf(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, *nullarray);
	glEndList();;
	lng_changed |= ma->lighting->valid;	/* "All fields changed" */
      }
    }
    if (ma->shading != APF_CONSTANT &&
	(ap->lighting != NULL 
#ifndef TRUE_EMISSION
	 /* Hack: must enable GL_LIGHT_MODEL_TWO_SIDE lighting if so */
	 || (mastk->next && 
	     (mastk->mat.valid ^ mastk->next->mat.valid) & MTF_EMISSION)
#endif
	 )) {
      mgopengl_lighting( mastk, lng_changed );
    }

    /* Let mgopengl_material() decide if we need a new material */
    if (ap->mat) {
      mgopengl_material( mastk, mat_changed );
    }

    mgopengl_appearance( mastk, changed );

    if (_mgopenglc->tevbound && _mgopenglc->curtex->tx != ap->tex) {
      mgopengl_notexture(); /* Unbind our no-longer-needed texture */
    }
  }

  return &_mgc->astk->ap;
}

void mgopengl_taggedappearance(const void *tag)
{
  struct mgastk *astk = (struct mgastk *)tag;
  Appearance *ap = &astk->ap;
  LmLighting *lm = &astk->lighting;

  mg_setappearance(ap, 0);

  if (lm->valid) {
    glCallList(_mgopenglc->light_lists[astk->light_seq]);
  }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(&_mgc->W2C[0][0] );
  mgopengl_lights(lm, astk);
  glPopMatrix();

  /* Let mgopengl_material() decide if we need a new material */
  if (ap->mat) {
    mgopengl_material(astk, ap->mat->valid);
  }

  mgopengl_appearance(astk, ap->valid);

  if (_mgopenglc->tevbound && _mgopenglc->curtex->tx != ap->tex) {
    mgopengl_notexture(); /* Unbind our no-longer-needed texture */
  }
}

static void mgimgfile2cambgimage(void)
{
  Image *bgimage;

  if (_mgc->cam && _mgopenglc->bgimagefile) {
    bgimage =
      ImgCreate(IMG_CHANNELS, IMGF_RGBA,
		IMG_DATA_CHAN_FILE, IMGF_RGB, NULL, _mgopenglc->bgimagefile,
		IMG_END);
    if (bgimage) {
      CamSet(_mgc->cam, CAM_BGIMAGE, bgimage);
      ImgDelete(bgimage);
    }
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_setcamera
 * Description:	set the context's camera (pointer)
 * Args:	*cam: the camera to use
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 13:07:31 1991
 * Notes:	The context stores a pointer to the camera, not a copy
 *		of it.
 */
int mgopengl_setcamera(Camera* cam)
{
  if (_mgc->cam != cam) {
    CamDelete(_mgc->cam);
    _mgc->cam = REFGET(Camera, cam);
    if (_mgc->cam) {
      CamGet(_mgc->cam, CAM_BGCOLOR, &_mgc->background);
      mgimgfile2cambgimage();
    }
  }
  return true;
}

/*
 * Change current Window structure.
 * If 'final' and otherwise appropriate, actually open the window.
 * Apply relevant changes to window, if needed.
 */
int
mgopengl_setwindow( WnWindow *win, int final )
{
  if(win == NULL) {
    return 0;
  }

  /* We can't do anything on our own.  Leave it up to 
   * the (possibly) user-supplied winchange callback.
   */
  if(_mgc->winchange)
    (*_mgc->winchange)(_mgc, _mgc->winchangeinfo, MGW_WINCHANGE, win);

  if(win != _mgc->win) {
    WnDelete(_mgc->win);
    _mgc->win = REFGET(WnWindow, win);
  }
  return 1;
}

GLuint *mgopengl_realloc_lists(GLuint *lists, int *n_lists)
{
  int i;
  GLuint new_lists;

  if ((new_lists = glGenLists(DPYLIST_INCR)) == 0) {
    OOGLError(0, "mgopengl_realloc_lists: no new lists available.");
    return NULL;
  }
  lists = realloc(lists, (*n_lists + DPYLIST_INCR)*sizeof(GLuint));
  for (i = *n_lists; i < *n_lists + DPYLIST_INCR; i++) {
    lists[i] = new_lists++;
  }
  *n_lists = i;

  return lists;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_newcontext
 * Description:	initialize a new mgopenglcontext structure
 * Args:	*ctx: the struct to initialize
 * Returns:	ctx
 * Author:	mbp
 * Date:	Fri Sep 20 13:11:03 1991
 */
mgopenglcontext *
mgopengl_newcontext( mgopenglcontext *ctx )
{
  mg_newcontext(&(ctx->mgctx));
  ctx->mgctx.devfuncs = &mgopenglfuncs;
  ctx->mgctx.devno = MGD_OPENGL;
  ctx->mgctx.astk->ap_seq = 0;
  ctx->mgctx.astk->mat_seq = 0;
  ctx->mgctx.astk->light_seq = 0;
  ctx->mgctx.zfnudge = 40.e-6;
  ctx->born = 0;
  ctx->win = 0;
  ctx->winids[SGL] = ctx->winids[DBL] = 0;
  {
    mgcontext *oldmgc = _mgc;
    _mgc = (mgcontext *)ctx;
    D4F_OFF();
    N3F_NOEVERT();
    _mgc = oldmgc;
  }
  ctx->lmcolor = GL_DIFFUSE;
  ctx->tevbound = 0;
  ctx->curtex = NULL;
  VVINIT(ctx->room, char, 180);
#ifndef GLUT
  ctx->GLXdisplay = NULL;
  ctx->cam_ctx[SGL] = ctx->cam_ctx[DBL] = 0;
  ctx->curctx = 0;
#endif
  ctx->should_lighting = ctx->is_lighting = 0;
  ctx->dither = 1;
  ctx->bgimagefile = NULL;

  /* reserve a number of display list numbers */
  ctx->n_light_lists = 0;
  ctx->light_lists = NULL;
  ctx->n_texture_lists = 0;
  ctx->texture_lists = NULL;
  ctx->n_translucent_lists = 0;
  ctx->translucent_lists = NULL;
  ctx->translucent_seq = 0;
  
  return ctx;
}

/*-----------------------------------------------------------------------
 * Function:	mgopengl_findctx
 * Description: Given a GL window ID, returns the associated mg context.
 * Returns:	mgcontext * for success, NULL if none exists.
 * Author:	slevy
 * Date:	Mon Nov 11 18:33:53 CST 1991
 * Notes:	This is a public routine.
 */
mgcontext *
mgopengl_findctx( int winid )
{
  struct mgcontext *mgc;

  for(mgc = _mgclist; mgc != NULL; mgc = mgc->next) {
    if(mgc->devno == MGD_OPENGL && ((mgopenglcontext *)mgc)->win == winid)
      return mgc;
  }
  return NULL;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
