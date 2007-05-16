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
#include "mgbuf.h"
#include "mgbufP.h"
#include "windowP.h"
#include <stdio.h>

static int weeble = 0;

void        mgbuf_reshapeviewport(void);
void        mgbuf_appearance( struct mgastk *ma, int mask );
void        mgbuf_setshader(mgshadefunc shader);
mgcontext * mgbuf_ctxcreate(int a1, ...);
int         mgbuf_ctxset( int a1, ...  );
int         mgbuf_feature( int feature );
void        mgbuf_ctxdelete( mgcontext *ctx );
int         mgbuf_ctxget( int attr, void* valueptr );
int         mgbuf_ctxselect( mgcontext *ctx );
void        mgbuf_sync( void );
void        mgbuf_worldbegin( void );
void        mgbuf_worldend( void );
void        mgbuf_identity( void );
void        mgbuf_transform( Transform T );
int         mgbuf_pushtransform( void );
int         mgbuf_poptransform( void );
void        mgbuf_gettransform( Transform T );
void        mgbuf_settransform( Transform T );
int         mgbuf_pushappearance( void );
int         mgbuf_popappearance( void );
const Appearance *mgbuf_setappearance(const Appearance* app, int merge );
int         mgbuf_setcamera( Camera* cam );
int         mgbuf_setwindow( WnWindow *win, int final );
mgbufcontext *mgbuf_newcontext( mgbufcontext *ctx );
static unsigned char *createRGB( void );

/* probably not here */
int         mgdevice_BUF();

extern void mgbuf_polygon();
extern void mgbuf_mesh();
extern void mgbuf_line();
extern void mgbuf_polyline();
extern void mgbuf_polylist();
extern void mgbuf_quads();

int _mgbuf_ctxset(int a1, va_list *alist);

WnWindow *mgbufwindow(WnWindow *win);

struct mgfuncs mgbuffuncs = {
  MGD_BUF,
  mgdevice_BUF,
  mgbuf_feature,
  (mgcontext *(*)())mgbuf_ctxcreate,
  mgbuf_ctxdelete,
  (int (*)())mgbuf_ctxset,
  mgbuf_ctxget,
  mgbuf_ctxselect,
  mgbuf_sync,
  mgbuf_worldbegin,
  mgbuf_worldend,
  mgbuf_reshapeviewport,
  /* geometry transform */
  mgbuf_settransform,
  mgbuf_gettransform,
  mgbuf_identity,
  mgbuf_transform,
  mgbuf_pushtransform,
  mgbuf_poptransform,
  /* texture transform */
  mg_settxtransform,
  mg_gettxtransform,
  mg_txidentity,
  mg_txtransform,
  mg_pushtxtransform,
  mg_poptxtransform,
  /* appearance */
  mgbuf_pushappearance,
  mgbuf_popappearance,
  mgbuf_setappearance,
  mg_getappearance,
  /*************/
  mgbuf_setcamera,
  mgbuf_polygon,
  mgbuf_polylist,
  mgbuf_mesh,
  mgbuf_line,
  mgbuf_polyline,
  mg_quads,
  mg_bezier,
  mg_bsptree,
  mg_tagappearance,
  mg_untagappearance,
  mg_taggedappearance
  };


static int curwidth=1;

void
mgbuf_reshapeviewport(void)
{
    CamSet(_mgc->cam, CAM_ASPECT, 
	   (double)_mgbufc->xsize/(double)_mgbufc->ysize, CAM_END);
}

/*-----------------------------------------------------------------------
 * Function:	mgdevice_BUF
 * Description:	select the BUF device as the current MG device
 * Returns:	1
 * Author:	daeron
 * Notes:	
 */
int
mgdevice_BUF()
{
    _mgf = mgbuffuncs;
    if(_mgc != NULL && _mgc->devno != MGD_BUF)
	_mgc = NULL;
    return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_newcontext
 * Description:	initialize an mgcontext structure
 * Args:	mgc: ptr to context structure to initialize
 * Returns:	mgc
 * Notes:	Further device-specific initialization is normally required.
 */
mgbufcontext *
mgbuf_newcontext( mgbufcontext *ctx )
{
  mg_newcontext(&(ctx->mgctx));
  ctx->mgctx.devfuncs = &mgbuffuncs;
  ctx->mgctx.devno = MGD_BUF;
  ctx->mgctx.astk->ap_seq = 1;
  ctx->mgctx.astk->mat_seq = 1;
  ctx->mgctx.astk->light_seq = 1;
  ctx->mgctx.astk->shader = mg_eucshade;
  ctx->mgctx.zfnudge = 40.e-6;
  ctx->buf = NULL;
  ctx->zbuf = NULL;
  ctx->xsize = ctx->ysize = 0;
  ctx->file = NULL;
  ctx->born = 0;
  VVINIT(ctx->room, char, 180);

  ctx->pvertnum = 100;
  VVINIT(ctx->pverts, CPoint3, ctx->pvertnum);
  vvneeds(&(ctx->pverts), ctx->pvertnum);

  return ctx;

}

/*-----------------------------------------------------------------------
 * Function:    _mgbuf_ctxset
 * Description: internal ctxset routine
 * Args:        a1: first attribute
 *              *alist: rest of attribute-value list
 * Returns:     -1 on error, 0 on success
 * Date:        Fri Sep 20 11:08:13 1991
 * Notes:       mgbuf_ctxcreate() and mgbuf_ctxset() call this to actually
 *              parse and interpret the attribute list.
 */
int
_mgbuf_ctxset(int a1, va_list *alist)
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

        mgbuf_setappearance(ap, MG_MERGE);
        ApDelete(ap);
      }
      break;

    case MG_WnSet:
      if (ablock)
	WnSet( _mgc->win, WN_ABLOCK, ablock);
      else
	_WnSet( _mgc->win, va_arg(*alist, int), alist);
      mgbuf_setwindow( _mgc->win, 0 );
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
      mgbuf_setwindow( NULL, 0 );
      break;

    case MG_CAMERA:
      mgbuf_setcamera( NEXT(Camera *) );
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
      mgbuf_setshader( NEXT(mgshadefunc) );
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
		fprintf(stderr, "_mgbuf_ctxset: Illegal space value %1d\n",
				space);
	    }
	    break;
	  default:
	    fprintf(stderr, "_mgbuf_ctxset: Illegal space value %1d\n",
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

    case MG_BUFFILE:
	_mgbufc->file = NEXT(FILE *);
	break;

    case MG_BUFFILEPATH:
	if (_mgbufc->file) fclose(_mgbufc->file);
        strcpy(_mgbufc->filepath, NEXT(char *));
        _mgbufc->file = fopen(_mgbufc->filepath, "w");
        break;

    default:
      OOGLError (0, "_mgbuf_ctxset: undefined option: %d", attr);
      return -1;
    }
  }
  if (_mgc->shown && !_mgbufc->born) {

    /* open the window */
    mgbufwindow(_mgc->win);

    /* bring BUF state into accordance with appearance state */
    {
      Appearance *ap = ApCopy( &(_mgc->astk->ap), NULL );
      mgbuf_setappearance( ap, MG_SET );
      ApDelete(ap);
    }

  }

#undef NEXT

  return 0;
}


/* This needs some checking  -- TOR */

int
mgbuf_setwindow( WnWindow *win, int final )
{
  /*WnPosition wp;
    WnPosition pos, vp;*/
  int xsize, ysize;
  /*
  int flag, reconstrain;
  int positioned = 0;
  int zmin;
  char *name, *oname;*/

  if (win == NULL)
   return 0;

/*
  WnGet(_mgc->win, WN_CURPOS, &wp);
  xsize = wp.xmax - wp.xmin + 1;
  ysize = wp.ymax - wp.ymin + 1;
  */
  WnGet(_mgc->win, WN_XSIZE, &xsize);
  WnGet(_mgc->win, WN_YSIZE, &ysize);
 
  if (final)
  { 
/*      fprintf(stderr, "BUF: setwindow xsize=%d ysize=%d\n", xsize, ysize);  */
      if (_mgbufc->buf)
          free(_mgbufc->buf);
      _mgbufc->buf = (unsigned char *) malloc(4*xsize*ysize);
      if (_mgbufc->zbuf)
          free(_mgbufc->zbuf);
      _mgbufc->zbuf = (float *) malloc(xsize*ysize*sizeof(float));
  }
  _mgbufc->xsize = xsize;
  _mgbufc->ysize = ysize;

  return 1;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_identity
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
mgbuf_identity( void )
{
   mgbuf_settransform( TM3_IDENTITY );
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_settransform
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
mgbuf_settransform( Transform T )
{
    TmCopy(T, _mgc->xstk->T);
    _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_gettransform
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
mgbuf_gettransform( Transform T )
{
    TmCopy(_mgc->xstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_transform
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
mgbuf_transform( Transform T )
{
    TmConcat(T, _mgc->xstk->T, _mgc->xstk->T);
    _mgc->has = _mgc->xstk->hasinv = 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_pushappearance
 * Description:	push the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all devices must maintain this stack
 */
int
mgbuf_pushappearance()
{
  mg_pushappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_popappearance
 * Description:	pop the context's appearance stack
 * Returns:	nothing
 * Notes:	
 * DEVICE USE:	required --- all deviced must maintain this stack
 */
int
mgbuf_popappearance()
{
  struct mgastk *mastk = _mgc->astk;
  struct mgastk *mastk_next;

  if (!(mastk_next = mastk->next))
  {
    OOGLError(0, "mgbuf_popappearance: appearance stack has only 1 entry.");
    return 0;
  }
  mgbuf_appearance(mastk_next, mastk_next->ap.valid);
  mg_popappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_setappearance
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
 *		than mgbuf_setappearance currently does???  This
 *		seems common to all devices.
 */
const Appearance *
mgbuf_setappearance(const Appearance *ap, int mergeflag )
{
  int changed, lng_changed;
  struct mgastk *mastk = _mgc->astk;
  Appearance *ma;

  ma = &(mastk->ap);

  if(mergeflag == MG_MERGE)
  {
    changed = ap->valid &~ (ma->override &~ ap->override);
    lng_changed = ap->lighting ? ap->lighting->valid &~
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

  mgbuf_appearance( mastk, changed);

  return &_mgc->astk->ap;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_setcamera
 * Description:	Set the context's camera
 * Args:	*cam: the camera to use
 * Returns:	nothing
 * Notes:	The context does not maintain an internal copy of the
 *		camera.  Only the pointer is stored.
 * DEVICE USE:  required
 */
int
mgbuf_setcamera( Camera *cam )
{
  if (_mgc->cam)
    CamDelete(_mgc->cam);
  _mgc->cam = cam;
  RefIncr((Ref*) cam);
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_ctxset
 * Description:	set some attributes in the current context
 * Args:	attr, ...: list of attribute-value pairs, terminated
 *		  by MG_END
 * Returns:	-1 on error and 0 on success
 * Notes:	DO NOT CALL THIS (yet)!  It currently does nothing.
 * DEVICE USE:  forbidden --- devices have their own mgxx_ctxset()
 *
 *		This needs to be modified to work as the NULL device.
 *		Use by other devices may never be needed.
 */
int
mgbuf_ctxset( int attr, ... /*, MG_END */ )
{
  va_list alist;
  int result;

  va_start( alist, attr );
  result = _mgbuf_ctxset(attr, &alist);
  va_end(alist);
  return result;
}


/*-----------------------------------------------------------------------
 * Function:	mgbuf_ctxget
 * Description:	get an attribute from the current context
 * Args:	attr: the attribute to get
 *		value: place to write attr's value
 *
 */
int
mgbuf_ctxget( int attr, void *value )
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

    case MG_BUFFILE:
      *VALUE(FILE *) = _mgbufc->file;
      break;

    case MG_BUFFILEPATH:
      *VALUE(char *) = _mgbufc->filepath;
      break;

    case MG_BUFMEMORY:
      *VALUE(unsigned char *) = createRGB();
      break;

    default:
      OOGLError (0, "mgbuf_ctxget: undefined option: %d\n", attr);
      return -1;

  }
  return 1;

#undef VALUE
}


/*-----------------------------------------------------------------------
 * Function:	mgbuf_feature
 * Description:	determine whether the NULL device has a particular feature
 * Args:	feature: feature to test for
 * Returns:	-1 (means feature is not present)
 * Notes:	BUF device is rather featureless at present, :-)
 * DEVICE USE:  forbidden --- devices have their own mgxx_feature()
 */
int
mgbuf_feature( int feature )
{
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_ctxcreate
 * Description:	create a new MG context for the BUF device
 * Args:	a1, ...: list of attribute-value pairs
 * Returns:	ptr to new context
 * Author:	daeron
 */
mgcontext *
mgbuf_ctxcreate( int a1, ... )
{
  va_list alist;

  _mgc =
    (mgcontext*)mgbuf_newcontext( OOGLNewE(mgbufcontext, "mgbuf_ctxcreate") );

  va_start(alist, a1);
  _mgbuf_ctxset(a1, &alist);
  va_end(alist);
  return _mgc;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_ctxdelete
 * Description:	delete an MG context for the BUF device
 * Args:	*ctx: ptr to context to delete
 * Returns:	nothing
 * Author:	daeron
 */
void
mgbuf_ctxdelete( mgcontext *ctx )
{
  mgbufcontext *_mgx = (mgbufcontext *)ctx;

  if(ctx->devno != MGD_BUF) {
    mgcontext *was = _mgc;
    mgctxselect(ctx);
    mgctxdelete(ctx);
    if (was != ctx)
      mgctxselect(was);
  } else {
    free(_mgx->buf);
    free(_mgx->zbuf);
    vvfree(&_mgx->pverts);
    vvfree(&_mgx->room);
    mg_ctxdelete(ctx);
    if (ctx == _mgc)
	  _mgc = NULL;
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_ctxselect
 * Description:	select the current context
 * Args:	*ctx: the context to select
 * Returns:	0 (why ???)
 * Author:	daeron
 */
int
mgbuf_ctxselect( mgcontext *ctx )
{
  if (ctx == NULL || ctx->devno != MGD_BUF) {
    return mg_ctxselect(ctx);
  }
  /* BUF context */
  _mgc = ctx;
  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_pushtransform
 * Description:	push the mg context xform stack
 * Returns:	nothing
 */
int
mgbuf_pushtransform( void )
{
  mg_pushtransform();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgbuf_poptransform
 * Description:	pop the mg context xform stack
 * Returns:	nothing
 */
int
mgbuf_poptransform( void )
{
  mg_poptransform();
  return 0;
}

void
mgbuf_sync( void )
{
}

void
mgbuf_worldbegin( void )
{
  int color[4];

  color[0] = _mgc->background.r * 255;
  color[1] = _mgc->background.g * 255;
  color[2] = _mgc->background.b * 255;

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
    /* Fix up W2S and S2W matrices.  Since the BUF coordinate system
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

/*  fprintf(stderr, "BUF: mgbuf_worldbegin -- clearing buffers\n"); */
  Xmgr_24fullinit(0xFF0000, 0xFF00, 0xFF);
  Xmgr_24clear(_mgbufc->buf, _mgbufc->zbuf, _mgbufc->xsize, _mgbufc->xsize*4,
	      _mgbufc->ysize, color, 1, 1, 0, 0, 0, 0);

  _mgbufc->znudgeby = 0.0;
}

void
mgbuf_worldend( void )
{
    int i, size;
    FILE *file;
    int *buf;

/*    fprintf(stderr, "BUF: mgbuf_worldend\n"); */
    if (_mgbufc->file == NULL)
	return;
    fprintf(_mgbufc->file, "P6\n%d %d\n255\n", _mgbufc->xsize, _mgbufc->ysize);
    size = _mgbufc->xsize * _mgbufc->ysize;
    buf = (int *)_mgbufc->buf;
    file = _mgbufc->file;
    for (i=0; i<size; i++)
    {
	fputc((buf[i]&0xFF0000)>>16, file);
	fputc((buf[i]&0xFF00)>>8, file);
	fputc(buf[i]&0xFF, file);
    }
}

static unsigned char *
createRGB( void )
{
    int i, size;
    int *buf;
    unsigned char *rgb;

    rgb = (unsigned char *)malloc(3 * _mgbufc->xsize * _mgbufc->ysize);
    if (rgb == NULL)
	return NULL;
    size = _mgbufc->xsize * _mgbufc->ysize;
    buf = (int *)_mgbufc->buf;
    for (i=0; i<size; i++)
    {
	rgb[3*i] = (buf[i]&0xFF0000)>>16;
	rgb[3*i+1] = (buf[i]&0xFF00)>>8;
	rgb[3*i+2] = buf[i]&0xFF;
    }
    return rgb;
}

WnWindow *
mgbufwindow(WnWindow *win)
{
 _mgbufc->born = 1;
 mgbuf_setwindow(win, 1);
 mgbuf_init_zrange();
 return(win);
}


/* funky low level stuff... don't worry... be happy :) */

static void frob(void) {}

/*
   Function: BUFmg_add
   Description: add a primitive (polygon, vertex, line)
   Author: Daeron Meyer, Timothy Rowley
*/
void BUFmg_add(int primtype, int numdata, void *data, void *cdata)
{
  HPoint3 *vt = (HPoint3 *)data;
  float *col = (float *)cdata;
  ColorA *colarray = (ColorA *)cdata;
  CPoint3 *vts;
  int i;
  static mgbufprim prim;

/*
  float frazzle;
*/

  static int color[3], ecolor[3];
  static int numverts;
  static int kind;
  static int ewidth;

#if 1 || HACK_THE_CODE_BUT_BETTER_FIX_IT
  if (!(_mgc->has & HAS_S2O)) {
    Transform S;
    WnPosition vp;

    mg_findS2O();
    mg_findO2S();

    WnGet(_mgc->win, WN_VIEWPORT, &vp);
    /* Fix up W2S and S2W matrices.  Since the BUF coordinate system
     * has Y increasing downward, flip it here, and translate by
     * location of lower left corner of viewport.
     */
    TmTranslate(S, (double)vp.xmin, (double)vp.ymax, 0.);
    S[1][1] = -1;		/* Invert sign of Y */
    TmConcat(_mgc->O2S, S, _mgc->O2S);
    TmInvert(_mgc->O2S, _mgc->S2O);
  }
#endif

/*  fprintf(stderr, "%p (xstk=%p)\n", _mgc, _mgc->xstk); */
  switch (primtype)
  {
  case MGX_BGNLINE:
  case MGX_BGNSLINE:

      if (primtype == MGX_BGNLINE)
	  kind = PRIM_LINE;
      else
	  kind = PRIM_SLINE;
      
      numverts = 0;
      ewidth = curwidth;

      if (!(_mgc->has & HAS_S2O)) {
	mg_findS2O();
	mg_findO2S();
      }
      break;

  case MGX_BGNPOLY:
  case MGX_BGNSPOLY:
  case MGX_BGNEPOLY:
  case MGX_BGNSEPOLY:

      if (primtype == MGX_BGNPOLY)
	  kind = PRIM_POLYGON;
      else if (primtype == MGX_BGNSPOLY)
	  kind = PRIM_SPOLYGON;
      else if (primtype == MGX_BGNEPOLY)
	  kind = PRIM_EPOLYGON;
      else
	  kind = PRIM_SEPOLYGON;

      ewidth = curwidth;
      numverts = 0;
	  
      if (!(_mgc->has & HAS_S2O)) {
	mg_findS2O();
	mg_findO2S();
      }
      break;

  case MGX_VERTEX:
      for (i=0; i<numdata; i++)
      {
	  vts = &(VVEC(_mgbufc->pverts, CPoint3)[numverts]);

	  HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	  vts->drawnext = 1;

/*
	  if (vts->w <= 0.0) {vts->x = _mgbufc->xsize - vts->x; vts->y
 = _mgbufc->ysize - vts->y; vts->z *= -1.0;}
*/


	  vts->vcol.r = 0;
	  vts->vcol.g = 0;
	  vts->vcol.b = 0;
	  vts->vcol.a = 1;
	  numverts++;
	  if (numverts > _mgbufc->pvertnum)
	  {
	    _mgbufc->pvertnum *= 2;
	    vvneeds(&(_mgbufc->pverts), _mgbufc->pvertnum);
	  }
      }
      break;

  case MGX_CVERTEX:
      for (i=0; i<numdata; i++)
      {
	  vts = &(VVEC(_mgbufc->pverts, CPoint3)[numverts]);

	  HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	  vts->drawnext = 1;

/*
	  if (vts->w <= 0.0) {vts->x = _mgbufc->xsize - vts->x; vts->y
 = _mgbufc->ysize - vts->y; vts->z *= -1.0;}
*/


	  vts->vcol = colarray[i];
	  numverts++;
	  if (numverts > _mgbufc->pvertnum)
	  {
	    _mgbufc->pvertnum *= 2;
	    vvneeds(&(_mgbufc->pverts), _mgbufc->pvertnum);
	  }
      }
      break;

  case MGX_COLOR:
      color[0] = (int)(255.0 * col[0]);
      color[1] = (int)(255.0 * col[1]);
      color[2] = (int)(255.0 * col[2]);
      break;

  case MGX_ECOLOR:
      ecolor[0] = (int)(255.0 * col[0]);
      ecolor[1] = (int)(255.0 * col[1]);
      ecolor[2] = (int)(255.0 * col[2]);
      break;

  case MGX_END:

      prim.mykind = kind;
      prim.index = 0;
      prim.numvts = numverts;
      kind = Xmgr_primclip(&prim);
      numverts = prim.numvts;

      {
	  unsigned char *buf = _mgbufc->buf;
	  float *zbuf = _mgbufc->zbuf;
	  int zwidth = _mgbufc->xsize;
	  int w = 4*_mgbufc->xsize;
	  int h = _mgbufc->ysize;
	  vts = &(VVEC(_mgbufc->pverts, CPoint3)[0]);
	  switch (kind)
	  {
	  case PRIM_POLYGON:
	      Xmgr_24Zpoly(buf, zbuf, zwidth, w, h, vts, numverts, color);
	      break;
	  case PRIM_SPOLYGON:
	      if (weeble==2383)
		frob();
	      Xmgr_24GZpoly(buf, zbuf, zwidth, w, h, vts, numverts, color);
	      break;
	  case PRIM_EPOLYGON:
	      Xmgr_24Zpoly(buf, zbuf, zwidth, w, h, vts, numverts, color);
	      Xmgr_24Zpolyline(buf, zbuf, zwidth, w, h, vts,
			       numverts, ewidth, ecolor);
	      Xmgr_24Zline(buf, zbuf, zwidth, w, h, &vts[numverts-1],
			   &vts[0], ewidth, ecolor);
	      break;
	  case PRIM_SEPOLYGON:
	      Xmgr_24GZpoly(buf, zbuf, zwidth, w, h, vts, numverts, color);
	      Xmgr_24Zpolyline(buf, zbuf, zwidth, w, h, vts,
			       numverts, ewidth, ecolor);
	      Xmgr_24Zline(buf, zbuf, zwidth, w, h, &vts[numverts-1],
			   &vts[0], ewidth, ecolor);
	      break;
	  case PRIM_SLINE:
	      Xmgr_24GZpolyline(buf, zbuf, zwidth, w, h, vts, numverts,
			ewidth, ecolor);
	      break;
	  case PRIM_LINE:
	      Xmgr_24Zpolyline(buf, zbuf, zwidth, w, h, vts, numverts,
		       ewidth, ecolor);
	      break;
	  case PRIM_INVIS:
	      break;
	  }
      }
      break;

  default:
      fprintf(stderr,"unknown type of primitive.\n");
      break;
  }
}

/* .................................... */

void
mgbuf_appearance( struct mgastk *ma, int mask )
{
    Appearance *ap = &(ma->ap);

    if (mask & APF_LINEWIDTH)
    {
	curwidth = ap->linewidth;
	_mgc->has &= ~HAS_POINT;
    }

    if (mask & APF_SHADING) {
	if (IS_SHADED(ap->shading) && ma->shader != NULL) {
	    /* Use software shader if one exists and user wants lighting */
	    ma->flags |= MGASTK_SHADER;
	} else {
	    /* No software shading, just use raw colors */
	    ma->flags &= ~MGASTK_SHADER;
	}
    }
}


void
mgbuf_setshader(mgshadefunc shader)
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
	mgbuf_appearance(_mgc->astk, APF_SHADING);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
