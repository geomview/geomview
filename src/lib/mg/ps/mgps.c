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
#include "mgps.h"
#include "mgpsP.h"
#include "windowP.h"
#include "mgpswindows.h"
#include <stdio.h>

void        mgps_reshapeviewport(void);
void        mgps_appearance( struct mgastk *ma, int mask );
void        mgps_setshader(mgshadefunc shader);
mgcontext * mgps_ctxcreate(int a1, ...);
int         mgps_ctxset( int a1, ...  );
int         mgps_feature( int feature );
void        mgps_ctxdelete( mgcontext *ctx );
int         mgps_ctxget( int attr, void* valueptr );
int         mgps_ctxselect( mgcontext *ctx );
void        mgps_sync( void );
void        mgps_worldbegin( void );
void        mgps_worldend( void );
void        mgps_identity( void );
void        mgps_transform( Transform T );
int         mgps_pushtransform( void );
int         mgps_poptransform( void );
void        mgps_gettransform( Transform T );
void        mgps_settransform( Transform T );
int         mgps_pushappearance( void );
int         mgps_popappearance( void );
const Appearance *mgps_setappearance(const Appearance* app, int merge );
int         mgps_setcamera( Camera* cam );
int         mgps_setwindow( WnWindow *win, int final );
mgpscontext *mgps_newcontext( mgpscontext *ctx );

/* probably not here */
int         mgdevice_PS();

extern void mgps_polygon();
extern void mgps_mesh();
extern void mgps_line();
extern void mgps_polyline();
extern void mgps_polylist();
extern void mgps_quads();

int _mgps_ctxset(int a1, va_list *alist);

WnWindow *mgpswindow(WnWindow *win);

struct mgfuncs mgpsfuncs = {
  MGD_PS,
  mgdevice_PS,
  mgps_feature,
  (mgcontext *(*)())mgps_ctxcreate,
  mgps_ctxdelete,
  (int (*)())mgps_ctxset,
  mgps_ctxget,
  mgps_ctxselect,
  mgps_sync,
  mgps_worldbegin,
  mgps_worldend,
  mgps_reshapeviewport,
  /* geometry transform */
  mgps_settransform,
  mgps_gettransform,
  mgps_identity,
  mgps_transform,
  mgps_pushtransform,
  mgps_poptransform,
  /* texture geometry transform */
  mg_settxtransform,
  mg_gettxtransform,
  mg_txidentity,
  mg_txtransform,
  mg_pushtxtransform,
  mg_poptxtransform,
  /* appearances */
  mgps_pushappearance,
  mgps_popappearance,
  mgps_setappearance,
  mg_getappearance,
  /***************/
  mgps_setcamera,
  mgps_polygon,
  mgps_polylist,
  mgps_mesh,
  mgps_line,
  mgps_polyline,
  mg_quads,
  mg_bezier,
  mg_bsptree,
  mg_tagappearance,
  mg_untagappearance,
  mg_taggedappearance
  };


void
mgps_reshapeviewport(void)
{
    CamSet(_mgc->cam, CAM_ASPECT, 
	   (double)_mgpsc->xsize/(double)_mgpsc->ysize, CAM_END);
}

/*-----------------------------------------------------------------------
 * Function:	mgdevice_PS
 * Description:	select the PS device as the current MG device
 * Returns:	1
 * Author:	daeron
 * Notes:	
 */
int
mgdevice_PS()
{
    _mgf = mgpsfuncs;
    if(_mgc != NULL && _mgc->devno != MGD_PS)
	_mgc = NULL;
    return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgps_newcontext
 * Description:	initialize an mgcontext structure
 * Args:	mgc: ptr to context structure to initialize
 * Returns:	mgc
 * Notes:	Further device-specific initialization is normally required.
 */
mgpscontext *
mgps_newcontext( mgpscontext *ctx )
{
  mg_newcontext(&(ctx->mgctx));
  ctx->mgctx.devfuncs = &mgpsfuncs;
  ctx->mgctx.devno = MGD_PS;
  ctx->mgctx.astk->ap_seq = 1;
  ctx->mgctx.astk->mat_seq = 1;
  ctx->mgctx.astk->light_seq = 1;
  ctx->mgctx.astk->shader = mg_eucshade;
  ctx->mgctx.zfnudge = 40.e-6;
  ctx->xsize = ctx->ysize = 0;
  ctx->file = NULL;
  ctx->born = 0;
  VVINIT(ctx->room, char, 180);


  return ctx;

}

/*-----------------------------------------------------------------------
 * Function:    _mgps_ctxset
 * Description: internal ctxset routine
 * Args:        a1: first attribute
 *              *alist: rest of attribute-value list
 * Returns:     -1 on error, 0 on success
 * Date:        Fri Sep 20 11:08:13 1991
 * Notes:       mgps_ctxcreate() and mgps_ctxset() call this to actually
 *              parse and interpret the attribute list.
 */
int
_mgps_ctxset(int a1, va_list *alist)
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

        mgps_setappearance(ap, MG_MERGE);
        ApDelete(ap);
      }
      break;

    case MG_WnSet:
      if (ablock)
	WnSet( _mgc->win, WN_ABLOCK, ablock);
      else
	_WnSet( _mgc->win, va_arg(*alist, int), alist);
      mgps_setwindow( _mgc->win, 0 );
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
      if (_mgc->win) WnDelete(_mgc->win);
      _mgc->win = NEXT(WnWindow *);
      mgps_setwindow( NULL, 0 );
      break;

    case MG_CAMERA:
      mgps_setcamera( NEXT(Camera *) );
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
      _mgc->background = bcolor;
    }
      break;

    case MG_SHADER:
      mgps_setshader( NEXT(mgshadefunc) );
      break;

    case MG_SHADERDATA:
      _mgc->astk->shaderdata = NEXT(void*);
      break;
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
		fprintf(stderr, "_mgps_ctxset: Illegal space value %1d\n",
				space);
	    }
	    break;
	  default:
	    fprintf(stderr, "_mgps_ctxset: Illegal space value %1d\n",
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

    case MG_PSFILE:
	_mgpsc->file = NEXT(FILE *);
	break;

    case MG_PSFILEPATH:
	if (_mgpsc->file) fclose(_mgpsc->file);
        strcpy(_mgpsc->filepath, NEXT(char *));
        _mgpsc->file = fopen(_mgpsc->filepath, "w");
        break;

    default:
      OOGLError (0, "_mgps_ctxset: undefined option: %d", attr);
      return -1;
    }
  }
  if (_mgc->shown && !_mgpsc->born) {

    /* open the window */
    mgpswindow(_mgc->win);

    /* bring PS state into accordance with appearance state */
    {
      Appearance *ap = ApCopy( &(_mgc->astk->ap), NULL );
      mgps_setappearance( ap, MG_SET );
      ApDelete(ap);
    }

  }

#undef NEXT

  return 0;
}


/* This needs some checking  -- TOR */

int
mgps_setwindow( WnWindow *win, int final )
{
  int xsize, ysize;

  if (win == NULL)
   return 0;

  WnGet(_mgc->win, WN_XSIZE, &xsize);
  WnGet(_mgc->win, WN_YSIZE, &ysize);
 
  _mgpsc->xsize = xsize;
  _mgpsc->ysize = ysize;

  return 1;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_identity
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
mgps_identity( void )
{
   mgps_settransform( TM3_IDENTITY );
}

/*-----------------------------------------------------------------------
 * Function:	mgps_settransform
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
mgps_settransform( Transform T )
{
    TmCopy(T, _mgc->xstk->T);
    _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_gettransform
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
mgps_gettransform( Transform T )
{
    TmCopy(_mgc->xstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:	mgps_transform
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
mgps_transform( Transform T )
{
    TmConcat(T, _mgc->xstk->T, _mgc->xstk->T);
    _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_pushappearance
 * Description:	push the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all devices must maintain this stack
 */
int
mgps_pushappearance()
{
  mg_pushappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_popappearance
 * Description:	pop the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all deviced must maintain this stack
 */
int
mgps_popappearance()
{
  struct mgastk *mastk = _mgc->astk;
  struct mgastk *mastk_next;

  if (!(mastk_next = mastk->next))
  {
    OOGLError(0, "mgps_popappearance: appearance stack has only 1 entry.");
    return 0;
  }
  mgps_appearance(mastk_next, mastk_next->ap.valid);
  mg_popappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_setappearance
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
 *		than mgps_setappearance currently does???  This
 *		seems common to all devices.
 */
const Appearance *
mgps_setappearance(const Appearance *ap, int mergeflag )
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

  if ((ap->lighting) && (mastk->next)) {
      if (mastk->light_seq == mastk->next->light_seq) {
	  mastk->light_seq++;
	  lng_changed |= ma->lighting->valid;     /* "All fields changed" */
      }
  }

  mgps_appearance( mastk, changed);
  return ap;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_setcamera
 * Description:	Set the context's camera
 * Args:	*cam: the camera to use
 * Returns:	nothing
 * Notes:	The context does not maintain an internal copy of the
 *		camera.  Only the pointer is stored.
 * DEVICE USE:  required
 */
int
mgps_setcamera( Camera *cam )
{
  if (_mgc->cam)
    CamDelete(_mgc->cam);
  _mgc->cam = cam;
  RefIncr((Ref*) cam);
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_ctxset
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
mgps_ctxset( int attr, ... /*, MG_END */ )
{
  va_list alist;
  int result;

  va_start( alist, attr );
  result = _mgps_ctxset(attr, &alist);
  va_end(alist);
  return result;
}


/*-----------------------------------------------------------------------
 * Function:	mgps_ctxget
 * Description:	get an attribute from the current context
 * Args:	attr: the attribute to get
 *		value: place to write attr's value
 *
 */
int
mgps_ctxget( int attr, void *value )
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
	/* pretty silly concept, IMHO  :-) */
      *VALUE(WnWindow*) = _mgc->win;
      break;

    case MG_PARENT:
      *VALUE(mgcontext*) = _mgc->parent;
      break;

    case MG_SETOPTIONS:
    case MG_UNSETOPTIONS:
      *VALUE(int) = _mgc->opts;
      break;

    case MG_BACKGROUND:
      *VALUE(ColorA) = _mgc->background;
      break;

    case MG_SHADER:
      *VALUE(mgshadefunc) = _mgc->astk->shader;
      break;

    case MG_SHADERDATA:
      *VALUE(void *) = _mgc->astk->shaderdata;
      break;

    case MG_SPACE:
      *VALUE(int) = _mgc->space;
      break;

    case MG_NDCTX:
      *VALUE(mgNDctx *) = _mgc->NDctx;
      break;

    case MG_ZNUDGE:
      *VALUE(float) = _mgc->zfnudge;
      break;

    case MG_PSFILE:
      *VALUE(FILE *) = _mgpsc->file;
      break;

    case MG_PSFILEPATH:
      *VALUE(char *) = _mgpsc->filepath;
      break;

    default:
      OOGLError (0, "mgps_ctxget: undefined option: %d\n", attr);
      return -1;

  }
  return 1;

#undef VALUE
}


/*-----------------------------------------------------------------------
 * Function:	mgps_feature
 * Description:	determine whether the NULL device has a particular feature
 * Args:	feature: feature to test for
 * Returns:	-1 (means feature is not present)
 * Notes:	PS device is rather featureless at present, :-)
 * DEVICE USE:  forbidden --- devices have their own mgxx_feature()
 */
int
mgps_feature( int feature )
{
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_ctxcreate
 * Description:	create a new MG context for the PS device
 * Args:	a1, ...: list of attribute-value pairs
 * Returns:	ptr to new context
 * Author:	daeron
 */
mgcontext *
mgps_ctxcreate( int a1, ... )
{
  va_list alist;

  _mgc =
    (mgcontext*)mgps_newcontext( OOGLNewE(mgpscontext, "mgps_ctxcreate") );
  mgps_initpsdevice();
  va_start(alist, a1);
  if (_mgps_ctxset(a1, &alist) == -1) {
      mgps_ctxdelete(_mgc);
  }
  va_end(alist);
  return _mgc;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_ctxdelete
 * Description:	delete an MG context for the PS device
 * Args:	*ctx: ptr to context to delete
 * Returns:	nothing
 * Author:	daeron
 */
void
mgps_ctxdelete( mgcontext *ctx )
{
  mgpscontext *_mgx = (mgpscontext *)ctx;

  if(ctx->devno != MGD_PS) {
    mgcontext *was = _mgc;
    mgctxselect(ctx);
    mgctxdelete(ctx);
    if (was != ctx)
	mgctxselect(was);
  } else {
    vvfree(&_mgx->room);
    mg_ctxdelete(ctx);
    if (ctx == _mgc)
	_mgc = NULL;
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgps_ctxselect
 * Description:	select the current context
 * Args:	*ctx: the context to select
 * Returns:	0 (why ???)
 * Author:	daeron
 */
int
mgps_ctxselect( mgcontext *ctx )
{
  if (ctx == NULL || ctx->devno != MGD_PS) {
    return mg_ctxselect(ctx);
  }
  /* PS context */
  _mgc = ctx;
  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgps_pushtransform
 * Description:	push the mg context xform stack
 * Returns:	nothing
 */
int
mgps_pushtransform( void )
{
  mg_pushtransform();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgps_poptransform
 * Description:	pop the mg context xform stack
 * Returns:	nothing
 */
int
mgps_poptransform( void )
{
  mg_poptransform();
  return 0;
}

void
mgps_sync( void )
{
}

void
mgps_worldbegin( void )
{

  mg_worldbegin();

  mgps_newdisplaylist();
  _mgpsc->znudgeby = 0.0;
}

void
mgps_worldend( void )
{
    if (_mgpsc->file == NULL)
	return;
    mgps_sortdisplaylist();
    mgps_showdisplaylist(_mgpsc->file);
}


WnWindow *
mgpswindow(WnWindow *win)
{
 _mgpsc->born = 1;
 mgps_setwindow(win, 1);
 mgps_init_zrange();
 return(win);
}


void
mgps_setshader(mgshadefunc shader)
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
	mgps_appearance(_mgc->astk, APF_SHADING);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
