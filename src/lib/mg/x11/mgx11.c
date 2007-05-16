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

#include "mgP.h"
#include "mgx11P.h"
#include "windowP.h"
#include "mgx11shade.h"
#include "mgx11windows.h"
#include <stdio.h>

mgcontext * mgx11_ctxcreate(int a1, ...);
int         mgx11_ctxset( int a1, ...  );
int         mgx11_feature( int feature );
void        mgx11_ctxdelete( mgcontext *ctx );
int         mgx11_ctxget( int attr, void* valueptr );
int         mgx11_ctxselect( mgcontext *ctx );
void        mgx11_sync( void );
void        mgx11_worldbegin( void );
void        mgx11_worldend( void );
void        mgx11_reshapeviewport( void );
void        mgx11_identity( void );
void        mgx11_transform( Transform T );
int         mgx11_pushtransform( void );
int         mgx11_poptransform( void );
void        mgx11_gettransform( Transform T );
void        mgx11_settransform( Transform T );
int         mgx11_pushappearance( void );
int         mgx11_popappearance( void );
const Appearance *mgx11_setappearance(const Appearance* app, int merge );
int         mgx11_setcamera( Camera* cam );
int         mgx11_setwindow( WnWindow *win, int final );
mgx11context *mgx11_newcontext( mgx11context *ctx );

extern void mgx11_polygon();
extern void mgx11_mesh();
extern void mgx11_line();
extern void mgx11_polyline();
extern void mgx11_polylist();
extern void mgx11_quads();
extern void mgx11_setshader(mgshadefunc shader);
extern void mgx11_appearance( struct mgastk *ma, int mask );
extern void mgx11_init_zrange();

int _mgx11_ctxset(int a1, va_list *alist);

WnWindow *mgx11window(WnWindow *win);

struct mgfuncs mgx11funcs = {
  MGD_X11,
  mgdevice_X11,
  mgx11_feature,
  (mgcontext *(*)())mgx11_ctxcreate,
  mgx11_ctxdelete,
  (int (*)())mgx11_ctxset,
  mgx11_ctxget,
  mgx11_ctxselect,
  mgx11_sync,
  mgx11_worldbegin,
  mgx11_worldend,
  mgx11_reshapeviewport,
  /* geometry transform */
  mgx11_settransform,
  mgx11_gettransform,
  mgx11_identity,
  mgx11_transform,
  mgx11_pushtransform,
  mgx11_poptransform,
  /* texture transform */
  mg_settxtransform,
  mg_gettxtransform,
  mg_txidentity,
  mg_txtransform,
  mg_pushtxtransform,
  mg_poptxtransform,
  /* appearance stack */
  mgx11_pushappearance,
  mgx11_popappearance,
  mgx11_setappearance,
  mg_getappearance,
  /********************/
  mgx11_setcamera,
  mgx11_polygon,
  mgx11_polylist,
  mgx11_mesh,
  mgx11_line,
  mgx11_polyline,
  mg_quads,
  mg_bezier,
  mg_bsptree,
  mg_tagappearance,
  mg_untagappearance,
  mg_taggedappearance
};


/*-----------------------------------------------------------------------
 * Function:	mgdevice_X11
 * Description:	select the X11 device as the current MG device
 * Returns:	1
 * Author:	daeron
 * Notes:	
 */
int
mgdevice_X11()
{
  _mgf = mgx11funcs;
  if(_mgc != NULL && _mgc->devno != MGD_X11)
    _mgc = NULL;
  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_newcontext
 * Description:	initialize an mgcontext structure
 * Args:	mgc: ptr to context structure to initialize
 * Returns:	mgc
 * Notes:	Further device-specific initialization is normally required.
 */
mgx11context *
mgx11_newcontext( mgx11context *ctx )
{
  mg_newcontext(&(ctx->mgctx));
  ctx->mgctx.devfuncs = &mgx11funcs;
  ctx->mgctx.devno = MGD_X11;
  ctx->mgctx.astk->ap_seq = 1;
  ctx->mgctx.astk->mat_seq = 1;
  ctx->mgctx.astk->light_seq = 1;
  ctx->mgctx.astk->shader = mg_eucshade;
  ctx->mgctx.zfnudge = 40.e-6;
  ctx->win = 0;
  ctx->pix = 0;
  ctx->cmapset = 0;
  ctx->sizelock = 0;
  ctx->deleted = NULL;
  ctx->visible = 0;
  VVINIT(ctx->room, char, 180);
  return ctx;

}

/*-----------------------------------------------------------------------
 * Function:    _mgx11_ctxset
 * Description: internal ctxset routine
 * Args:        a1: first attribute
 *              *alist: rest of attribute-value list
 * Returns:     -1 on error, 0 on success
 * Date:        Fri Sep 20 11:08:13 1991
 * Notes:       mgx11_ctxcreate() and mgx11_ctxset() call this to actually
 *              parse and interpret the attribute list.
 */
int
_mgx11_ctxset(int a1, va_list *alist)
{
  int attr;
  char **ablock = NULL;

#define NEXT(type) OOGL_VA_ARG(type, alist, ablock)

  for (attr = a1; attr != MG_END; attr = NEXT(int)) {
    switch (attr) {

    case MG_ABLOCK:
      ablock = NEXT(char**);
      break;

    case MG_ApSet:
      {
        Appearance *ap;

	if (ablock)
	  ap = ApSet(NULL, AP_ABLOCK, ablock);
	else
	  ap = _ApSet(NULL, va_arg(*alist, int), alist);

        mgx11_setappearance(ap, MG_MERGE);
        ApDelete(ap);
      }
      break;

    case MG_WnSet:
      if (ablock)
	WnSet( _mgc->win, WN_ABLOCK, ablock);
      else
	_WnSet( _mgc->win, va_arg(*alist, int), alist);
      mgx11_setwindow( _mgc->win, 0 );
      break;

    case MG_CamSet:
      if (ablock)
	CamSet( _mgc->cam, CAM_ABLOCK, ablock);
      else
	_CamSet( _mgc->cam, va_arg(*alist, int), alist);
      break;

    case MG_APPEAR:
      mgsetappearance( NEXT(Appearance *), MG_SET );
      break;

    case MG_WINDOW:
      mgx11_setwindow( NEXT(WnWindow *), 0 );
      break;

    case MG_CAMERA:
      mgx11_setcamera( NEXT(Camera *) );
      break;

    case MG_SETOPTIONS:
      _mgc->opts |= NEXT(int);
      break;

    case MG_UNSETOPTIONS:
      _mgc->opts &= ~NEXT(int);
      break;

    case MG_SHOW:
      _mgc->shown = NEXT(int);
      break;

    case MG_PARENT:
      _mgc->parent = NEXT(mgcontext*);
      break;

    case MG_BACKGROUND:
      {
	ColorA bcolor;
	bcolor = *NEXT(ColorA*);
	if ((bcolor.r != _mgc->background.r) ||
	    (bcolor.g != _mgc->background.g) ||
	    (bcolor.b != _mgc->background.b))
	  _mgx11c->exposed = 1;
	_mgc->background = bcolor;
      }
      break;

    case MG_SHADER:	mgx11_setshader( NEXT(mgshadefunc) ); break;
    case MG_SHADERDATA:	_mgc->astk->shaderdata = NEXT(void*); break;

    case MG_WINCHANGE:	_mgc->winchange = NEXT(mgwinchfunc); break;
    case MG_WINCHANGEDATA: _mgc->winchangeinfo = NEXT(void *); break;

    case MG_SPACE:
      {
	int space = NEXT(int);
	switch (TM_SPACE(space))
	  {
	  case TM_EUCLIDEAN:
	  case TM_SPHERICAL:
	    _mgc->space = space;
	    break;
	  case TM_HYPERBOLIC:
	    switch (TM_MODEL(space))
	      {
	      case TM_VIRTUAL:
	      case TM_PROJECTIVE:
	      case TM_CONFORMAL_BALL:
		_mgc->space = space;
		break;
	      default:
		fprintf(stderr, "_mgx11_ctxset: Illegal space value %1d\n",
			space);
	      }
	    break;
	  default:
	    fprintf(stderr, "_mgx11_ctxset: Illegal space value %1d\n",
		    space);
	  }
      }
      break;

    case MG_NDCTX:
      _mgc->NDctx = NEXT(mgNDctx *);
      break;

    case MG_ZNUDGE:
      _mgc->zfnudge = NEXT(double);
      break;

    case MG_DEPTHSORT:
      _mgx11c->sortmethod = (enum sortmethod)NEXT(int);
      break;

    case MG_DITHER:
      _mgx11c->dither = NEXT(int);
      break;

    case MG_BITDEPTH:
      _mgx11c->bitdepth = NEXT(int);
      Xmg_setx11display(_mgx11c->mgx11display);
      /*        fprintf(stderr," Got bit depth %d\n", _mgx11c->bitdepth); */
      break;

    case MG_X11PIXID:
      _mgx11c->win = 1;
      _mgx11c->pix = 1;
      Xmg_setwin(NEXT(Window));
      _mgc->shown = 1;
      break;

    case MG_X11WINID:
      _mgx11c->win = 1;
      Xmg_setwin(NEXT(Window));
      _mgc->shown = 1;
      break;

    case MG_X11PARENT:
      Xmg_setparent(NEXT(Window));
      break;

    case MG_X11DISPLAY:
      _mgx11c->mgx11display = NEXT(Display *);
      Xmg_initx11device();
      break;

    case MG_X11SIZELOCK:
      _mgx11c->sizelock = NEXT(int);
      break;

    case MG_X11VISUAL:
      _mgx11c->visual = NEXT(Visual *);
      break;

    case MG_X11EXPOSE:
      _mgx11c->exposed = 1;
      break;

    case MG_X11COLORMAP:
      _mgx11c->cmap = NEXT(Colormap);
      _mgx11c->cmapset = 1;
      break;

    default:
      OOGLError (0, "_mgx11_ctxset: undefined option: %d", attr);
      return -1;
    }
  }
  if (_mgc->shown && !_mgx11c->visible) {

    /* open the window */
    mgx11window(_mgc->win);

    /* bring X11 state into accordance with appearance state */
    {
      Appearance *ap = ApCopy( &(_mgc->astk->ap), NULL );
      mgx11_setappearance( ap, MG_SET );
      ApDelete(ap);
    }

  }

#undef NEXT

  return 0;
}

int mgx11_setwindow( WnWindow *win, int final )
{
#ifdef UNNEEDED
  WnPosition pos, vp;
  int xsize, ysize, flag, reconstrain;
  int positioned = 0;
  int zmin;
  char *name, *oname;
#endif

  if (win == NULL)
    return 0;

  if(_mgc->winchange)
    (*_mgc->winchange)(_mgc, _mgc->winchangeinfo, MGW_WINCHANGE, win);

#ifdef UNNEEDED
  reconstrain = 0;
  if (WnGet(win, WN_PREFPOS, (void*)&pos) == 1 && (win->changed & WNF_HASPREF))
    {
      if (_mgx11c->visible)
	Xmg_poswin(pos.xmin, pos.xmax, pos.ymin, pos.ymax);
      else
	{
	  Xmg_prefposwin(pos.xmin, pos.xmax, pos.ymin, pos.ymax);
	  reconstrain = positioned = 1;
	}
      win->changed &= ~(WNF_HASPREF | WNF_HASSIZE);
    }
  else
    if ((WnGet(win, WN_XSIZE, (void*)&xsize) == 1
	 && (WnGet(win, WN_YSIZE, (void*)&ysize) == 1
	     && (win->changed & WNF_HASSIZE)))
	&& !(_mgx11c->sizelock)
	&& _mgx11c->visible)
      {
	Xmg_sizewin(xsize, ysize);
	reconstrain = 1;
	win->changed &= ~WNF_HASSIZE;
      }

  if (reconstrain && _mgx11c->visible)
    {
      WnGet(win, WN_NOBORDER, &flag);

      /*
	if (flag) noborder(); what to replace these with for X?
	winconstraints();*/

      reconstrain = 0;
    }


  if (_mgc->shown) {
    WnGet(win, WN_NAME, &name);
    if (_mgx11c->visible) {
      if(name && (win->changed & WNF_HASNAME))
	{
	  Xmg_titlewin(name);
	  win->changed &= ~WNF_HASNAME;
	}
      if (WnGet(win, WN_VIEWPORT, &vp) > 0 && win->changed & WNF_HASVP)
	{
	  /* some sort of
	     viewport(vp.xmin, vp.xmax, vp.ymin, vp.ymax) function */
	  win->changed &=~WNF_HASVP;
	}
    }
    else
      if (final)
	{
	  _mgx11c->win = Xmg_openwin(name);
	  mgx11_sync();
	  if (_mgx11c->win == 0) {
	    OOGLError(0,"mgx11window: unable to open window");
	    return 0;
	  }
	  if (positioned)
	    Xmg_poswin(pos.xmin, pos.xmax, pos.ymin, pos.ymax);
	  _mgx11c->visible = 1;
	  reconstrain = 0;
	}
  }

  if ((reconstrain && _mgx11c->visible)
      || (win->changed & (WNF_NOBORDER | WNF_ENLARGE | WNF_SHRINK)))
    {
      WnGet(win, WN_SHRINK, &flag);
      if (flag)
	Xmg_minsize((long)40, (long)30);
      reconstrain = 1;
    }
  if (reconstrain && _mgx11c->visible)
    {
      Xmg_winconstrain();
      win->changed &= ~(WNF_NOBORDER | WNF_ENLARGE | WNF_SHRINK);
    }
#endif /*UNNEEDED*/

  if (win != _mgc->win)
    {
      RefIncr((Ref *)win);
      WnDelete(_mgc->win);
      _mgc->win = win;
    }
  return 1;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_identity
 * Description:	Set the current object xform to the identity
 * Args:	(none)
 * Returns:	nothing
 * Notes:	Sets the xform on the top of the current context's xform
 *		stack to the identity.  Also sets the MC_TRANS bit of
 *		the context's "changed" flag and increment's the current xfm
 *		sequence number.
 * DEVICE USE:  optional --- if the device actually uses the context
 *		structure's xform stack, call this to do the work.  If
 *		the device keeps its own stack, it doesn't have to call
 *		this.
 */
void
mgx11_identity( void )
{
  mgx11_settransform( TM3_IDENTITY );
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_settransform
 * Description:	Set the current object xform
 * Args:	T
 * Returns:	nothing
 * Notes:	Sets the xform on the top of the current context's xform
 *		stack to T.  Also sets the MC_TRANS bit of
 *		the context's "changed" flag and increment's the current xfm
 *		sequence number.
 * DEVICE USE:  optional --- if the device actually uses the context
 *		structure's xform stack, call this to do the work.  If
 *		the device keeps its own stack, it doesn't have to call
 *		this.
 */
void
mgx11_settransform( Transform T )
{
  TmCopy(T, _mgc->xstk->T);
  _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_gettransform
 * Description:	Get the current object xform
 * Args:	T
 * Returns:	nothing
 * Notes:	Writes the current object xform, from the top of the
 *		context's xform stack, into T.
 * DEVICE USE:  optional --- if the device actually uses the context
 *		structure's xform stack, call this to do the work.  If
 *		the device keeps its own stack, it doesn't have to call
 *		this.
 */
void
mgx11_gettransform( Transform T )
{
  TmCopy(_mgc->xstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_transform
 * Description:	premultiply the current object xform by a transform
 * Args:	T: the transform to premultiply by
 * Returns:	nothing
 * Notes:	If X is the context's current object xform, replaces X
 *		by T X.
 * DEVICE USE:  optional --- if the device actually uses the context
 *		structure's xform stack, call this to do the work.  If
 *		the device keeps its own stack, it doesn't have to call
 *		this.
 */
void
mgx11_transform( Transform T )
{
  TmConcat(T, _mgc->xstk->T, _mgc->xstk->T);
  _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_pushappearance
 * Description:	push the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all devices must maintain this stack
 */
int
mgx11_pushappearance()
{
  mg_pushappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_popappearance
 * Description:	pop the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all deviced must maintain this stack
 */
int
mgx11_popappearance()
{
  struct mgastk *mastk = _mgc->astk;
  struct mgastk *mastk_next;

  if (!(mastk_next = mastk->next))
    {
      OOGLError(0, "mgx11_popappearance: appearance stack has only 1 entry.");
      return 1;
    }
  mgx11_appearance(mastk_next, mastk_next->ap.valid);
  mg_popappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_setappearance
 * Description:	Operate on appearance in current context
 * Args:	*ap: the appearance to assign or merge
 *		mergeflag: MG_MERGE or MG_SET
 * Returns:	ptr to current appearance
 * Notes:	Modifies the context's current apperance.  Does not
 *		modify *ap.
 *		  mergeflag = MG_MERGE: merge *ap into current appearance
 *		  mergeflag = MG_SET: set current appearance to *ap
 * DEVICE USE:  required --- when ???
 *
 *		Can we modify this to do some of the flag setting
 *		than mgx11_setappearance currently does???  This
 *		seems common to all devices.
 */
const Appearance *
mgx11_setappearance(const Appearance *ap, int mergeflag )
{
  int changed, lng_changed;
  struct mgastk *mastk = _mgc->astk;
  Appearance *ma;

  ma = &(mastk->ap);

  if(mergeflag == MG_MERGE)
    {
    changed = ap->valid &~ (ma->override &~ ap->override);
    lng_changed =
      ap->lighting ? ap->lighting->valid &~
      (ma->lighting->override &~ ap->lighting->override) : 0;
  }
  else
  {
    changed = ap->valid;
    lng_changed = ap->lighting ? ap->lighting->valid : 0;
  }
  mg_setappearance( ap, mergeflag );
  if (_mgx11c->visible)
  {
    if ((ap->lighting) && (mastk->next)) {
      if (mastk->light_seq == mastk->next->light_seq) {
        mastk->light_seq++;
        lng_changed |= ma->lighting->valid;     /* "All fields changed" */
      }
    }
  }
  mgx11_appearance( mastk, changed);

  return &_mgc->astk->ap;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_setcamera
 * Description:	Set the context's camera
 * Args:	*cam: the camera to use
 * Returns:	nothing
 * Notes:	The context does not maintain an internal copy of the
 *		camera.  Only the pointer is stored.
 * DEVICE USE:  required
 */
int
mgx11_setcamera( Camera *cam )
{
  if (_mgc->cam)
    CamDelete(_mgc->cam);
  _mgc->cam = cam;
  RefIncr((Ref*) cam);

  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_ctxset
 * Description:	set some attributes in the current context
 * Args:	attr, ...: list of attribute-value pairs, terminated
 *		  by MG_END
 * Returns:	-1 on error, 0 on success
 * Notes:	DO NOT CALL THIS (yet)!  It currently does nothing.
 * DEVICE USE:  forbidden --- devices have their own mgxx_ctxset()
 *
 *		This needs to be modified to work as the NULL device.
 *		Use by other devices may never be needed.
 */
int
mgx11_ctxset( int attr, ... /*, MG_END */ )
{
  va_list alist;
  int result;

  va_start( alist, attr );
  result = _mgx11_ctxset(attr, &alist);
  va_end(alist);
  return result;
}


/*-----------------------------------------------------------------------
 * Function:	mgx11_ctxget
 * Description:	get an attribute from the current context
 * Args:	attr: the attribute to get
 *		value: place to write attr's value
 *
 */
int
mgx11_ctxget( int attr, void *value )
{
#define VALUE(type) ((type*)value)

  switch (attr)
    {
    case MG_APPEAR:
      *VALUE(Appearance*) = &(_mgc->astk->ap);
      break;

    case MG_CAMERA:
      *VALUE(Camera*) = _mgc->cam;
      break;

    case MG_WINDOW:
      if (_mgx11c->visible)
	{
	  int x0, y0, xsize, ysize;
	  WnPosition wp;
	  Xmg_getwinsize(&xsize, &ysize, &x0, &y0);
	  wp.xmin = (long)x0; wp.xmax = (long)x0+(long)xsize-1;
	  wp.ymin = (long)y0; wp.ymax = (long)y0+(long)ysize-1;
	  WnSet(_mgc->win, WN_CURPOS, &wp, WN_END);
	}
      *VALUE(WnWindow*) = _mgc->win;
      break;

    case MG_PARENT:
      *VALUE(mgcontext*) = _mgc->parent;
      break;

    case MG_SETOPTIONS:
    case MG_UNSETOPTIONS:
      *VALUE(int) = _mgc->opts;
      break;

    case MG_BACKGROUND: *VALUE(ColorA) = _mgc->background; break;

    case MG_SHADER: *VALUE(mgshadefunc) = _mgc->astk->shader; break;
    case MG_SHADERDATA: *VALUE(void *) = _mgc->astk->shaderdata; break;

    case MG_WINCHANGE: *VALUE(mgwinchfunc) = _mgc->winchange; break;
    case MG_WINCHANGEDATA: *VALUE(void *) = _mgc->winchangeinfo; break;

    case MG_SPACE: *VALUE(int) = _mgc->space; break;

    case MG_NDCTX: *VALUE(mgNDctx *) = _mgc->NDctx; break;

    case MG_X11WINID: *VALUE(int) = _mgx11c->win; break;
    case MG_X11VISIBLE: *VALUE(int) = _mgx11c->visible; break;

    case MG_ZNUDGE: *VALUE(float) = _mgc->zfnudge; break;

    case MG_DEPTHSORT: *VALUE(int) = _mgx11c->sortmethod; break;

    case MG_DITHER: *VALUE(int) = _mgx11c->dither; break;

    case MG_BITDEPTH: *VALUE(int) = _mgx11c->bitdepth; break;

    case MG_X11VISUAL: *VALUE(Visual *) = _mgx11c->visual; break;

    default:
      OOGLError (0, "mgx11_ctxget: undefined option: %d\n", attr);
      return -1;

    }
  return 1;

#undef VALUE
}


/*-----------------------------------------------------------------------
 * Function:	mgx11_feature
 * Description:	determine whether the NULL device has a particular feature
 * Args:	feature: feature to test for
 * Returns:	-1 (means feature is not present)
 * Notes:	X11 device is rather featureless at present, :-)
 * DEVICE USE:  forbidden --- devices have their own mgxx_feature()
 */
int
mgx11_feature( int feature )
{
  if(feature == MG_X11VISUAL)
    return 1;
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_ctxcreate
 * Description:	create a new MG context for the X11 device
 * Args:	a1, ...: list of attribute-value pairs
 * Returns:	ptr to new context
 * Author:	daeron
 */
mgcontext *
mgx11_ctxcreate( int a1, ... )
{
  va_list alist;

  _mgc =
    (mgcontext*)mgx11_newcontext( OOGLNewE(mgx11context, "mgx11_ctxcreate") );

  if (!Xmg_initx11device())
    OOGLError(0,"mgdevice_X11: unable to open X-display");

  va_start(alist, a1);
  if (_mgx11_ctxset(a1, &alist) == -1) {
    mgx11_ctxdelete(_mgc);
  }
  va_end(alist);
  return _mgc;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_ctxdelete
 * Description:	delete an MG context for the X11 device
 * Args:	*ctx: ptr to context to delete
 * Returns:	nothing
 * Author:	daeron
 */
void
mgx11_ctxdelete( mgcontext *ctx )
{
  mgx11context *_mgx = (mgx11context *)ctx;

  if(ctx->devno != MGD_X11) {
    mgcontext *was = _mgc;
    mgctxselect(ctx);
    free(_mgx->myxwin);
    mgctxdelete(ctx);
    if (was != ctx)
      mgctxselect(was);
  } else {
    if (((mgx11context *)ctx)->visible)
      Xmg_closewin(_mgx->myxwin);
    free(_mgx->myxwin);
    mg_ctxdelete(ctx);
    if (ctx == _mgc)
      _mgc = NULL;
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_ctxselect
 * Description:	select the current context
 * Args:	*ctx: the context to select
 * Returns:	0 (why ???)
 * Author:	daeron
 */
int
mgx11_ctxselect( mgcontext *ctx )
{
  if (ctx == NULL || ctx->devno != MGD_X11) {
    return mg_ctxselect(ctx);
  }
  /* X11 context */
  _mgc = ctx;
  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_pushtransform
 * Description:	push the mg context xform stack
 * Returns:	nothing
 */
int
mgx11_pushtransform( void )
{
  mg_pushtransform();

  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgx11_poptransform
 * Description:	pop the mg context xform stack
 * Returns:	nothing
 */
int
mgx11_poptransform( void )
{
  mg_poptransform();
  return 0;
}

void
mgx11_sync( void )
{
  Xmg_flush();
}

void
mgx11_worldbegin( void )
{
  mg_worldbegin();
#if 0 && !DO_NOT_HACK_THE_CODE_THIS_WAY
  {
    /* This cannot be done here because there are objects which do not
     * belong to the world coordinate system; this stuff needs to be
     * done in mgx11windows.c in Xmg_add().
     */
    Transform S;
    WnPosition vp;

    WnGet(_mgc->win, WN_VIEWPORT, &vp);
    /* Fix up W2S and S2W matrices.  Since the X11 coordinate system
     * has Y increasing downward, flip it here, and translate by
     * location of lower left corner of viewport.
     */
    TmTranslate(S, (double)vp.xmin, (double)vp.ymax, 0.);
    S[1][1] = -1;		/* Invert sign of Y */
    TmConcat(_mgc->W2S, S, _mgc->W2S);
    TmInvert(_mgc->W2S, _mgc->S2W);
  }
#endif
  _mgc->has = 0;
  Xmg_newdisplaylist();
}

void
mgx11_worldend( void )
{
  Xmg_sortdisplaylist();
  Xmg_showdisplaylist();
}

void
mgx11_reshapeviewport( void )
{
  int w, h, x0, y0;
  float pixasp = 1.0;
  WnPosition vp;

  WnGet(_mgc->win, WN_PIXELASPECT, &pixasp);
  if (WnGet(_mgc->win, WN_VIEWPORT, &vp) <= 0)
    {
      WnPosition wp;
      Xmg_getwinsize(&w, &h, &x0, &y0);
      wp.xmin = (long)x0; wp.xmax = (long)x0+(long)w-1;
      wp.ymin = (long)y0; wp.ymax = (long)y0+(long)h-1;
      WnSet(_mgc->win, WN_CURPOS, &wp, WN_END);
    }
  else
    {
      w = vp.xmax - vp.xmin + 1;
      h = vp.ymax - vp.ymin + 1;
    }
  CamSet(_mgc->cam, CAM_ASPECT, pixasp * (double)w/(double)h, CAM_END);
}

WnWindow *
mgx11window(WnWindow *win)
{
  mgx11_setwindow(win, 1);
  mgx11_init_zrange();
  return(win);
}

/*-----------------------------------------------------------------------
 * Function:    mgx11_findctx
 * Description: Given an X11 window ID, returns the associated mg context.
 * Returns:     mgcontext * for success, NULL if none exists.
 * Author:      daeron
 * Notes:       This is a public routine.
 */
mgcontext *
mgx11_findctx( Window winid )
{
  struct mgcontext *mgc;

  for(mgc = _mgclist; mgc != NULL; mgc = mgc->next) {
    if(mgc->devno == MGD_X11 && ((mgx11context *)mgc)->myxwin->window == winid)
      return mgc;
  }
  return NULL;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
