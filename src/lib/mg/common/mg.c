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
 * $Id: mg.c,v 1.20 2010/03/14 11:45:43 rotdrop Exp $
 * Machine-independent part of MG library.
 * Initialization, common code, and some mgcontext maintenance.
 *
 * These are the common versions of MG functions (see mg.doc for
 * details).  The header comment for each common function includes an
 * entry labeled "DEVICE USE" which describes the way in which the
 * device version of the same function is expected to call the common
 * version.
 *
 * See mg.doc for more information about these functions.
 */

#include "mgP.h"

extern struct mgfuncs mgnullfuncs;      /* Forward */

mgcontext *_mgc = NULL;
mgcontext *_mgclist = NULL;

static struct mgastk  *mgafree  = NULL;
static struct mgastk  *mgatfree = NULL; /* tagged free-list */
static struct mgxstk  *mgxfree  = NULL;
static struct mgtxstk *mgtxfree  = NULL;

#define MGC     _mgc

/*
 * mgcurrentcontext() returns a pointer to the currently-selected context,
 * for use in a later call to mgctxselect().
 * Returns NULL if no context is selected.
 */
mgcontext *
mgcurrentcontext(void)
{
  return _mgc;
}


/*-----------------------------------------------------------------------
 * Function:    mgdevice_NULL
 * Description: select the NULL device as the current MG device
 * Returns:     1
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 10:36:21 1991
 * Notes:
 */
int
mgdevice_NULL(void)
{
  if (MGC != NULL && MGC->devno != MGD_NULL)
    MGC = NULL;
  _mgf = mgnullfuncs;
  return 1;
}

/*-----------------------------------------------------------------------
 * Function:    mg_newcontext
 * Description: initialize an mgcontext structure
 * Args:        mgc: ptr to context structure to initialize
 * Returns:     mgc
 * Author:      slevy (doc by mbp)
 * Date:        Wed Sep 18 16:42:52 1991
 * DEVICE USE:  required --- mgxx_ctxcreate() should call this immediately
 *              after allocating a new mgcontext structure.
 * Notes:       Further device-specific initialization is normally required.
 */
mgcontext *mg_newcontext(mgcontext *mgc)
{
  memset((char *)mgc, 0, sizeof(*mgc));
  RefInit((Ref *)mgc, MGCONTEXTMAGIC);
  mgc->shown = 1;
  mgc->win = WnCreate(WN_NAME, "minnegraphics", WN_END);
  mgc->cam = CamCreate( CAM_END );
  mgc->background.r = 0.0;
  mgc->background.g = 0.0;
  mgc->background.b = 0.0;
  mgc->background.a = 1.0;
  mgc->bgimage = NULL;
  /* initialize appearance stack */
  {
    struct mgastk *ma;

    mgc->astk = ma = OOGLNewE(struct mgastk, "mg appearance stack");
    memset((char *)ma, 0, sizeof(*ma)); /* Sets next = NULL, *_seq = 0 */
    MtDefault(&(ma->mat));
    LmDefault(&(ma->lighting));
    ApDefault(&(ma->ap));
    ma->ap.mat = &(ma->mat);
    ma->ap.lighting = &(ma->lighting);
    ma->flags |= MGASTK_ACTIVE;
    RefInit((Ref *)ma, 'a');
  }
  /* initialize transform stack */
  {
    struct mgxstk *mx;

    mgc->xstk = mx = OOGLNewE(struct mgxstk, "mg transform stack");
    mx->next = NULL;
    TmIdentity(mx->T);
    mx->xfm_seq = mx->hasinv = 0;
  }
  /* initialize texture transform stack */
  {
    mgc->txstk = OOGLNewE(struct mgtxstk, "mg texture transform stack");
    mgc->txstk->next = NULL;
    TmIdentity(mgc->txstk->T);
  }
  mgc->opts = MGO_HIDDEN|MGO_DOUBLEBUFFER;
  mgc->devno = MGD_NODEV;     /* Device-specific init should change this */

  TmIdentity(mgc->W2C); TmIdentity(mgc->C2W);
  TmIdentity(mgc->W2S); TmIdentity(mgc->S2W);
  TmIdentity(mgc->O2S); TmIdentity(mgc->S2O);

  mgc->space = TM_EUCLIDEAN;

  mgc->NDctx = NULL;
  VVINIT(mgc->point, HPoint3, 7);

  mgc->winchange = NULL;
  mgc->winchangeinfo = NULL;

  mgc->ap_min_tag =
    mgc->mat_min_tag =
    mgc->light_min_tag = ~0;

  mgc->next = _mgclist;
  _mgclist = mgc;

  return mgc;
}

int
mg_appearancebits( Appearance *ap, int mergeflag, int *valid, int *flag )
{
  Appearance *dst;

  if (!_mgc->astk) {
    OOGLError(0,"mg_appearanceflags: no global context");
    return 0;
  }
  dst = &(_mgc->astk->ap);

  /* Mask of fields to change in dst */
  if (ap == NULL) {
    *valid = dst->valid;
    *flag = dst->flag;
    mergeflag = MG_MERGE;
  } else {
    *valid = ap->valid;
    *flag = ap->flag;
  }
  if (mergeflag == MG_MERGE) {
    *valid &= ~dst->override;
  }
  return 1;
}

/*-----------------------------------------------------------------------
 * Function:    mg_pushappearance
 * Description: push the context's appearance stack
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 10:37:55 1991
 * Notes:
 * DEVICE USE:  required --- all devices must maintain this stack
 */
int
mg_pushappearance(void)
{
  struct mgastk *ma;

  if (mgafree) {
    ma = mgafree;
    mgafree = ma->next;
  } else {
    ma = OOGLNew(struct mgastk);
  }
  *ma = *_mgc->astk;
  ma->flags &= ~MGASTK_TAGGED; /* active and shader flags are inherited */
  RefInit((Ref *)ma, 'a');
  ma->next = _mgc->astk;
  LmCopy(&_mgc->astk->lighting, &ma->lighting);
  ma->ap.lighting = &(ma->lighting);
  ma->ap.mat = &(ma->mat);
  ma->ap.tex = REFGET(Texture, ma->ap.tex);
  _mgc->astk = ma;
  return 0;
}

/* Three support function to support BSP-trees with multiple
 * appearances: in the worst case every tree-node could have its own
 * appearance; this cannot be efficiently modelled with the
 * appearance stack approach.
 */

/* Make the current appearance persistent. */
const void *mg_tagappearance(void)
{
  struct mgastk *astk = _mgc->astk;

  astk->flags |= MGASTK_TAGGED;
  RefIncr((Ref *)astk);

  /* update excluded sequence region */
  if ((unsigned)_mgc->ap_min_tag > (unsigned)astk->ap_seq) {
    _mgc->ap_min_tag = astk->ap_seq;
  }
  if (_mgc->ap_max_tag < astk->ap_seq) {
    _mgc->ap_max_tag = astk->ap_seq;
  }
  if ((unsigned)_mgc->mat_min_tag > (unsigned)astk->mat_seq) {
    _mgc->mat_min_tag = astk->mat_seq;
  }
  if (_mgc->mat_max_tag < astk->mat_seq) {
    _mgc->mat_max_tag = astk->mat_seq;
  }
  if ((unsigned)_mgc->light_min_tag > (unsigned)astk->light_seq) {
    _mgc->light_min_tag = astk->light_seq;
  }
  if (_mgc->light_max_tag < astk->light_seq) {
    _mgc->light_max_tag = astk->light_seq;
  }

  return _mgc->astk;
}

/* Un-tag the appearance described by "tag" (i.e. undo its
 * persistency). This function should perform a clean-up, i.e. release
 * all resources associated with the corresponding appearance.
 */
void mg_untagappearance(const void *tag)
{
  struct mgastk *astk = (struct mgastk *)tag, *pos;
  struct mgcontext *ctx = astk->tag_ctx;  

  RefDecr((Ref *)astk);
  if (RefCount((Ref *)astk) > 1) {
    return;
  }
  
  if (!(astk->flags & MGASTK_ACTIVE)) {

    TxDelete(astk->ap.tex);
    astk->ap.tex = NULL;
    LmDeleteLights(&astk->lighting);

    /* Move to free-list if not active */
    if (ctx) {
      if (ctx->ap_tagged == astk) {
	ctx->ap_tagged = astk->next;
	if (ctx->ap_tagged == NULL) {
	  ctx->ap_min_tag =
	    ctx->mat_min_tag = 
	    ctx->light_min_tag = -1;
	  ctx->ap_max_tag =
	    ctx->mat_max_tag =
	    ctx->light_max_tag = 0;
	}
      } else {
	for (pos = ctx->ap_tagged; pos->next != astk; pos = pos->next);
	pos->next = astk->next;
      }
    } else {
      if (astk == mgatfree) {
	mgatfree = astk->next;
      } else {
	for (pos = mgatfree; pos->next != astk; pos = pos->next);
	pos->next = astk->next;
      }
    }
    astk->tag_ctx = NULL;
    astk->next = mgafree;
    mgafree = astk;
  }

  astk->flags &= ~MGASTK_TAGGED;
}

/* Insert a persistent apearance into the current appearance model */
void mg_taggedappearance(const void *tag)
{
  struct mgastk *astk = (struct mgastk *)tag;

  mg_setappearance(&astk->ap, 0);
}

/*-----------------------------------------------------------------------
 * Function:    mg_popappearance
 * Description: pop the context's appearance stack
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 10:51:12 1991
 * Notes:
 * DEVICE USE:  required --- all devices must maintain this stack
 */
int
mg_popappearance(void)
{
  struct mgastk *mp;
  struct mgcontext *ctx = _mgc;

  mp = ctx->astk->next;
  if (mp == NULL) {
    return -1;
  }
  if (ctx->astk->ap_seq != mp->ap_seq) ctx->changed |= MC_AP;
  if (ctx->astk->mat_seq != mp->mat_seq) ctx->changed |= MC_MAT;
  if (ctx->astk->light_seq != mp->light_seq) ctx->changed |= MC_LIGHT;

  ctx->astk->flags &= ~MGASTK_ACTIVE;
  if (ctx->astk->flags & MGASTK_TAGGED) {
    ctx->astk->next = ctx->ap_tagged;
    ctx->ap_tagged = ctx->astk;
    ctx->ap_tagged->tag_ctx = ctx;
    ctx->astk = mp;
  } else {
    TxDelete(ctx->astk->ap.tex);
    ctx->astk->ap.tex = NULL;
    LmDeleteLights(&ctx->astk->lighting);
    ctx->astk->next = mgafree;
    mgafree = ctx->astk;
    ctx->astk = mp;
  }

  return 0;
}

/*
 * Transform light(s) to global coordinate system, if they aren't already.
 * Transforms them in place.
 */
void
mg_globallights( LmLighting *lm, int worldbegin )
{
  LtLight *lt, **lp;
  HPoint3 oldpos;
  int i;

  for (i = 0, lp = &lm->lights[0]; i < AP_MAXLIGHTS && *lp != NULL; i++, lp++) {
    lt = *lp;
    oldpos = lt->globalposition;
    switch(lt->location) {
    case LTF_GLOBAL:
      lt->globalposition = lt->position;
      break;
    case LTF_CAMERA:
      HPt3Transform(_mgc->C2W, &lt->position, &lt->globalposition);
      break;
    case LTF_LOCAL:
      HPt3Transform(_mgc->xstk->T, &lt->position, &lt->position);
      lt->globalposition = lt->position;
      lt->location = LTF_GLOBAL;
      break;
    }
    if (memcmp(&oldpos, &lt->globalposition, sizeof(HPoint3)) != 0)
      lt->changed = 1;
  }
}

/*-----------------------------------------------------------------------
 * Function:    mg_setappearance
 * Description: Operate on appearance in current context
 * Args:        *ap: the appearance to assign or merge
 *              mergeflag: MG_MERGE or MG_SET
 * Returns:     ptr to current appearance
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 10:59:25 1991
 * Notes:       Modifies the context's current apperance.  Does not
 *              modify *ap.
 *                mergeflag = MG_MERGE: merge *ap into current appearance
 *                mergeflag = MG_SET: set current appearance to *ap
 * DEVICE USE:  required --- when ???
 *
 *              Can we modify this to do some of the flag setting
 *              than mggl_setappearance currently does???  This
 *              seems common to all devices.
 */
const Appearance *
mg_setappearance(const Appearance *ap, int mergeflag)
{
  struct mgastk *ma = _mgc->astk;

  if (mergeflag == MG_MERGE) {
    ApMerge(ap, &ma->ap, 1);  /* Merge, in place */
    ma->changed |= MC_AP;
    /* ma->ap = *nap; */ /* <- not necessary, ApMerge(a, b) returns b
     * unless b would be NULL on entry, which canot be the case here.
     */
    /* Assign mat and light too? */
  } else {
    /* Kludge: map->ap->tex is only a pointer, calling copy would
     * override its contents (now that TxCopy() has been
     * implemented). So get rid of ma->ap->tex before calling
     * ApCopyShared(). This is ok, because we always call REFGET(tex)
     * before hanging any texture into our appearance stack, so if
     * ap->tex == ma->ap.tex then the texture won't actually be
     * deleted.
     */
    TxDelete(ma->ap.tex);
    ma->ap.tex = NULL;
    ApCopyShared(ap, &ma->ap);
    ma->changed |= MC_AP | MC_MAT | MC_LIGHT;
  }
  if (ap->lighting) {
    mg_globallights(&ma->lighting, 0);
  }
  if (ap->tex) {
    ap->tex->flags |= TXF_USED;
  }
  return &_mgc->astk->ap;
}

/*-----------------------------------------------------------------------
 * Function:    mg_getappearance
 * Description: get the current appearance
 * Returns:     ptr to the current appearance
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:08:06 1991
 * DEVICE USE:  optional
 * Notes:       The pointer returned points to the context's private copy
 *              of the appearance.  Don't modify it!
 *
 *              Should we allow this?  Or should this copy the appearance
 *              to an address passed as an argument ???
 */
const Appearance *mg_getappearance(void)
{
  return &_mgc->astk->ap;
}


/*-----------------------------------------------------------------------
 * Function:    mg_setcamera
 * Description: Set the context's camera
 * Args:        *cam: the camera to use
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:16:46 1991
 * Notes:       The context does not maintain an internal copy of the
 *              camera.  Only the pointer is stored.
 * DEVICE USE:  required
 */
int
mg_setcamera( Camera *cam )
{
  RefIncr((Ref *)cam); /* Incr count first: allow setting same camera */
  CamDelete(_mgc->cam);
  _mgc->cam = cam;
  _mgc->changed |= MC_CAM;
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_ctxset
 * Description: set some attributes in the current context
 * Args:        attr, ...: list of attribute-value pairs, terminated
 *                by MG_END
 * Returns:     -1 on error 0 on success
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:22:28 1991
 * Notes:       DO NOT CALL THIS (yet)!  It currently does nothing.
 * DEVICE USE:  forbidden --- devices have their own mgxx_ctxset()
 *
 *              This needs to be modified to work as the NULL device.
 *              Use by other devices may never be needed.
 */
int
mg_ctxset( int attr, ... /*, MG_END */ )
{
  return 0;
}


/*-----------------------------------------------------------------------
 * Function:    mg_ctxget
 * Description: get an attribute from the current context
 * Args:        attr: the attribute to get
 *              *valuep: place to write attr's value
 * Returns:     ???
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:26:49 1991
 * Notes:       DO NOT CALL THIS (yet)!  It currently does nothing.
 * DEVICE USE:  forbidden --- devices have their own mgxx_ctxget()
 *
 *              This needs to be modified to work as the NULL device.
 *              Use by other devices may never be needed.
 */
int
mg_ctxget( int attr, void *valuep )
{
  return -1;
}


/*-----------------------------------------------------------------------
 * Function:    mg_feature
 * Description: determine whether the NULL device has a particular feature
 * Args:        feature: feature to test for
 * Returns:     -1 (means feature is not present)
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:29:51 1991
 * Notes:       NULL device is rather featureless at present, :-)
 * DEVICE USE:  forbidden --- devices have their own mgxx_feature()
 */
int
mg_feature( int feature )
{
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:    mg_ctxcreate
 * Description: create a new MG context for the NULL device
 * Args:        a1, ...: list of attribute-value pairs
 * Returns:     ptr to new context
 * Author:      mbp
 * Date:        Thu Sep 19 11:35:42 1991
 * Notes:       DO NOT CALL THIS --- it currently does nothing
 *              needs to be modified to work with NULL device ???
 * DEVICE USE:  forbidden --- devices have their own mgxx_ctxcreate(()
 */
mgcontext *
mg_ctxcreate( int a1, ... )
{
  return NULL;
}

/*-----------------------------------------------------------------------
 * Function:    mg_ctxdelete
 * Description: delete an MG context for the NULL device
 * Args:        *ctx: ptr to context to delete
 * Returns:     nothing
 * Author:      mbp
 * Date:        Thu Sep 19 11:38:50 1991
 * DEVICE USE:  Call this to do common stuff after each device has
 *              cleaned up its own data.
 */
void
mg_ctxdelete( mgcontext *ctx )
{
  struct mgcontext **mp;
  struct mgastk *astk, *nextastk;
  struct mgxstk *xstk, *nextxstk;
  struct mgtxstk *txstk, *nexttxstk;

  if (ctx == NULL)
    return;

  if (ctx->winchange) {
    (*ctx->winchange)(ctx, ctx->winchangeinfo, MGW_WINDELETE, ctx->win);
  }

  for (mp = &_mgclist; *mp != NULL; mp = &(*mp)->next) {
    if (*mp == ctx) {
      *mp = ctx->next;
      break;
    }
  }

  for (xstk = ctx->xstk; xstk != NULL; xstk = nextxstk) {
    nextxstk = xstk->next;
    xstk->next = mgxfree;
    mgxfree = xstk;
  }

  for (txstk = ctx->txstk; txstk != NULL; txstk = nexttxstk) {
    nexttxstk = txstk->next;
    txstk->next = mgtxfree;
    mgtxfree = txstk;
  }

  for (astk = ctx->astk; astk != NULL; astk = nextastk) {
    nextastk = astk->next;
    if (!(astk->flags & MGASTK_TAGGED)) {
      if (astk->ap.tex != NULL &&
	  (nextastk == NULL || astk->ap.tex != nextastk->ap.tex)) {
	TxDelete(ctx->astk->ap.tex);
	ctx->astk->ap.tex = NULL;
      }
      LmDeleteLights(&astk->lighting);
      astk->next = mgafree;
      mgafree = astk;
    } else {
      OOGLWarn("Tagged, but active?");
    }
  }

  /* Move the tagged appearances to the global tagged free-list; the
   * ap's are moved to the real free list when they are finally
   * untagged.
   */
  for (astk = ctx->ap_tagged; astk != NULL; astk = nextastk) {
    nextastk = astk->next;
    astk->tag_ctx = NULL;
    astk->flags &= ~MGASTK_ACTIVE; /* should already have been cleared */
    astk->next = mgatfree;
    mgatfree = astk;
  }

 /* make sure textures only needed by this context are purged,
  * otherwise the mg-backends may fail to reload the texture if a new
  * camera is opened
  */
  ctx->changed |= MC_USED;
  mg_textureclock();

  WnDelete(ctx->win);
  CamDelete(ctx->cam);

  /* Free other data here someday XXX */
  if (_mgc == ctx) {
    _mgc = NULL;
  }
  
  OOGLFree(ctx);
}

/*-----------------------------------------------------------------------
 * Function:    mg_ctxselect
 * Description: select the current context
 * Args:        *ctx: the context to select
 * Returns:     0 (why ???)
 * Author:      slevy (doc by mbp)
 * Date:        Thu Sep 19 11:40:15 1991
 * DEVICE USE:  required --- mgxx_ctxselect() should call this if
 *              the context to switch to if of a different device.
 *              This procedure then does the switch.
 */
int
mg_ctxselect( mgcontext *ctx )
{
  if (ctx != NULL && _mgf.mg_devno != ctx->devno) {
    /*
     * For another device.
     * Install that device's function pointers, and
     * call its selectcontext routine.
     */
    (*ctx->devfuncs->mg_setdevice)();
    mgctxselect(ctx);
  }
  _mgc = ctx;
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_identity
 * Description: Set the current object xform to the identity
 * Args:        (none)
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Wed Sep 18 16:46:06 1991
 * Notes:       Sets the xform on the top of the current context's xform
 *              stack to the identity.  Also sets the MC_TRANS bit of
 *              the context's "changed" flag and increment's the current xfm
 *              sequence number.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_identity( void )
{
  TmIdentity(_mgc->xstk->T);
  _mgc->changed |= MC_TRANS;
  _mgc->xstk->xfm_seq++;
}

/*-----------------------------------------------------------------------
 * Function:    mg_settransform
 * Description: Set the current object xform
 * Args:        T
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Wed Sep 18 16:46:06 1991
 * Notes:       Sets the xform on the top of the current context's xform
 *              stack to T.  Also sets the MC_TRANS bit of
 *              the context's "changed" flag and increment's the current xfm
 *              sequence number.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_settransform( Transform T )
{
  TmCopy(T, _mgc->xstk->T);
  _mgc->changed |= MC_TRANS;
  _mgc->xstk->xfm_seq++;
}

/*-----------------------------------------------------------------------
 * Function:    mg_gettransform
 * Description: Get the current object xform
 * Args:        T
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Wed Sep 18 16:46:06 1991
 * Notes:       Writes the current object xform, from the top of the
 *              context's xform stack, into T.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_gettransform( Transform T )
{
  TmCopy(_mgc->xstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:    mg_transform
 * Description: premultiply the current object xform by a transform
 * Args:        T: the transform to premultiply by
 * Returns:     nothing
 * Author:      slevy (doc by mbp)
 * Date:        Wed Sep 18 16:46:06 1991
 * Notes:       If X is the context's current object xform, replaces X
 *              by T X.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_transform( Transform T )
{
  TmConcat(T, _mgc->xstk->T, _mgc->xstk->T);
  _mgc->changed |= MC_TRANS;
  _mgc->xstk->xfm_seq++;
  _mgc->xstk->hasinv = 0;
  _mgc->has = 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_pushtransform
 * Description: push the context xform stack
 * Returns:     nothing
 * Author:      mbp
 * Date:        Thu Sep 19 12:23:26 1991
 * DEVICE USE:  optional --- use if device actually uses our stack
 */
int
mg_pushtransform( void )
{
  struct mgxstk *xfm;
  if (mgxfree) xfm = mgxfree, mgxfree = xfm->next;
  else xfm = OOGLNewE(struct mgxstk, "mgpushtransform");
  *xfm = *MGC->xstk;
  xfm->next = MGC->xstk;
  MGC->xstk = xfm;
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_poptransform
 * Description: pop the context xform stack
 * Returns:     nothing
 * Author:      mbp
 * Date:        Thu Sep 19 12:23:51 1991
 * DEVICE USE:  optional --- use if device actually uses our stack
 */
int
mg_poptransform( void )
{ struct mgxstk *xfm = MGC->xstk;
  if (xfm->next == NULL)
    return -1;
  MGC->xstk = xfm->next;
  xfm->next = mgxfree;
  mgxfree = xfm;
  _mgc->has = 0;
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_txidentity
 * Description: Set the current texture xform to the identity
 * Args:        (none)
 * Returns:     nothing
 * Author:      cH
 * Date:        2007
 * Notes:       Sets the xform on the top of the current context's texture 
 *              xform stack to the identity.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_txidentity(void)
{
  TmIdentity(_mgc->txstk->T);
}

/*-----------------------------------------------------------------------
 * Function:    mg_settransform
 * Description: Set the current object xform
 * Args:        T
 * Returns:     nothing
 * Author:      cH
 * Date:        2007
 * Notes:       Sets the xform on the top of the current context's texture
 *              xform stack to T.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_settxtransform(Transform T)
{
  TmCopy(T, _mgc->txstk->T);
}

/*-----------------------------------------------------------------------
 * Function:    mg_gettxtransform
 * Description: Get the current object xform
 * Args:        T
 * Returns:     nothing
 * Author:      cH
 * Date:        2007
 * Notes:       Writes the current object xform, from the top of the
 *              context's texture xform stack, into T.
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_gettxtransform(Transform T)
{
  TmCopy(_mgc->txstk->T, T);
}

/*-----------------------------------------------------------------------
 * Function:    mg_txtransform
 * Description: premultiply the current object xform by a transform
 * Args:        T: the transform to premultiply by
 * Returns:     nothing
 * Author:      cH
 * Date:        2007
 * Notes:       If X is the context's current object texture xform, replaces X
 *              by (T X).
 * DEVICE USE:  optional --- if the device actually uses the context
 *              structure's xform stack, call this to do the work.  If
 *              the device keeps its own stack, it doesn't have to call
 *              this.
 */
void
mg_txtransform(Transform T)
{
  TmConcat(T, _mgc->txstk->T, _mgc->txstk->T);
}

/*-----------------------------------------------------------------------
 * Function:    mg_pushtxtransform
 * Description: push the context texture xform stack
 * Returns:     -1 on error, else 0 (error case does not happen)
 * Author:      cH
 * Date:        2007
 * DEVICE USE:  optional --- use if device actually uses our stack
 */
int
mg_pushtxtransform( void )
{
  struct mgtxstk *xfm;

  if (mgtxfree) {
    xfm = mgtxfree;
    mgtxfree = xfm->next;
  } else {
    xfm = OOGLNewE(struct mgtxstk, "mgpushtxtransform");
  }
  *xfm = *MGC->txstk;
  xfm->next = MGC->txstk;
  MGC->txstk = xfm;

  return 0;
}

/*-----------------------------------------------------------------------
 * Function:    mg_poptxtransform
 * Description: pop the context xform stack
 * Returns:     -1 on error, else 0
 * Author:      cH
 * Date:        2007
 * DEVICE USE:  optional --- use if device actually uses our stack
 */
int
mg_poptxtransform(void)
{
  struct mgtxstk *xfm = MGC->txstk;
  if (xfm->next == NULL) {
    return -1;
  }
  MGC->txstk = xfm->next;
  xfm->next = mgtxfree;
  mgtxfree = xfm;

  return 0;
}

void
mg_sync( void )
{}

void
mg_worldbegin( void )
{
  Transform S, V;
  WnPosition vp;

  _mgc->changed |= MC_USED;
  CamGet(_mgc->cam, CAM_W2C, _mgc->W2C);
  CamGet(_mgc->cam, CAM_C2W, _mgc->C2W);
  CamGet(_mgc->cam, CAM_BGCOLOR, &_mgc->background);
  CamGet(_mgc->cam, CAM_BGIMAGE, &_mgc->bgimage);
  CamView(_mgc->cam, V);
  WnGet(_mgc->win, WN_VIEWPORT, &vp);
  /* V maps world to [-1..1],[-1..1],[-1..1] */
  TmTranslate(S, 1., 1., 0.);
  TmConcat(V, S, V); /* now maps to [0..2],[0..2],[-1..1] */
  TmScale(S, .5*(vp.xmax-vp.xmin+1), .5*(vp.ymax-vp.ymin+1), 1.);
  /* now maps to [0..xsize],[0..ysize],[-1..1] */
  TmConcat(V, S, _mgc->W2S);    /* final world-to-screen matrix */
  TmInvert(_mgc->W2S, _mgc->S2W);       /* and screen-to-world */
  TmCopy(_mgc->W2S, _mgc->O2S);
  TmCopy(_mgc->S2W, _mgc->S2O);
  TmIdentity(_mgc->xstk->T);
  _mgc->xstk->hasinv = 0;
  _mgc->has |= HAS_S2O;
  mg_globallights(&_mgc->astk->lighting, 1);
}

void
mg_findO2S(void)
{
  TmConcat(_mgc->xstk->T, _mgc->W2S, _mgc->O2S);
}

void
mg_findS2O(void)
{
  if (!(_mgc->has & HAS_S2O)) {
    if (!_mgc->xstk->hasinv) {
      TmInvert(_mgc->xstk->T, _mgc->xstk->Tinv);
      _mgc->xstk->hasinv = 1;
    }
    TmConcat(_mgc->S2W, _mgc->xstk->Tinv, _mgc->S2O);
    TmConcat(_mgc->xstk->T, _mgc->W2S, _mgc->O2S);
    _mgc->has |= HAS_S2O;
  }
}

/* Construct a prototype polygonal outline for creating fat points.
 * Curiously, we can do this independently of the position of the point,
 * if we operate in homogeneous space.
 */
void mg_makepoint(void)
{
  int i, n;
  float t, r, c, s;
  HPoint3 *p;
  static float nsides = 3.0;

  if (!(_mgc->has & HAS_S2O))
    mg_findS2O();

  if (_mgc->astk->ap.linewidth <= 3) n = 4;
  else n = nsides * sqrt((double)_mgc->astk->ap.linewidth);
  vvneeds(&_mgc->point, n);
  VVCOUNT(_mgc->point) = n;
  r = .5 * _mgc->astk->ap.linewidth;
  for (i = 0, p = VVEC(_mgc->point, HPoint3);  i < n;  i++, p++) {
    t = 2*M_PI*i/n; s = r * sin(t); c = r * cos(t);
    p->x = _mgc->S2O[0][0]*c + _mgc->S2O[1][0]*s;
    p->y = _mgc->S2O[0][1]*c + _mgc->S2O[1][1]*s;
    p->z = _mgc->S2O[0][2]*c + _mgc->S2O[1][2]*s;
    p->w = _mgc->S2O[0][3]*c + _mgc->S2O[1][3]*s;
  }
  _mgc->has |= HAS_POINT;
}

void mg_findcam(void)
{
  HPoint3 camZ;
  /*
   * Figure out where the camera is in the current coordinate system
   */
  if (!_mgc->xstk->hasinv) {
    TmInvert(_mgc->xstk->T, _mgc->xstk->Tinv);
    _mgc->xstk->hasinv = 1;
  }
  /* XXX assumes left multiplication: we take C2W[3] as a point! */
  HPt3Transform(_mgc->xstk->Tinv, (HPoint3 *)&_mgc->C2W[3][0], &_mgc->cpos);
  HPt3Transform(_mgc->xstk->Tinv, (HPoint3 *)&_mgc->C2W[2][0], &camZ);
  camZ.w = Pt3Length(HPoint3Point3(&camZ));
  HPt3ToPt3(&camZ, &_mgc->camZ);
  _mgc->has |= HAS_CPOS;
}

void
mg_worldend( void )
{}

void
mg_reshapeviewport( void )
{}

void
mg_polygon( int nv, HPoint3 *v, int nn,
	    Point3 *n, int nc,ColorA *c )
{}


void
mg_polylist(int np, struct Poly *p, int nv, struct Vertex *v, int plflags)
{}


void
mg_mesh(int wrap,int nu,int nv, HPoint3 *p,
        Point3 *n, Point3 *nq, ColorA *c, TxST *st, int mflags)
{}

void
mg_line( HPoint3 *p1, HPoint3 *p2 )
{}

void
mg_polyline( int nv, HPoint3 *verts,
	     int nc, ColorA *colors,
	     int wrapped )
{}

void
mg_quads(int nquads, HPoint3 *verts, Point3 *normals, ColorA *colors,
	 int qflags)
{
  int i;
  HPoint3 *v = verts;
  Point3 *n = normals;
  ColorA *c = colors;
  int dn = normals ? 4 : 0;
  int dc = colors ? 4 : 0;

  for (i = 0; i < nquads; i++, v += 4, n += dn, c += dc)
    mgpolygon(4, v, dn, n, dc, c);
}

void
mg_bezier(int du, int dv, int dimn, float *ctrlpts, TxST *txmapst, ColorA *c)
{
}

void
mg_bsptree(struct BSPTree *bsptree)
{
}

#define NULLFUNCS							\
  {									\
    MGD_NODEV,								\
      mgdevice_NULL,          /* mg_setdevice        */			\
      mg_feature,             /* mg_feature          */			\
      (mgcontext *(*)(void))mg_ctxcreate,         /* mg_ctxcreate        */ \
      mg_ctxdelete,           /* mg_ctxdelete        */			\
      (int (*)(void))mg_ctxset,          /* mg_ctxset           */	\
      mg_ctxget,              /* mg_ctxget           */			\
      mg_ctxselect,           /* mg_ctxselect        */			\
      mg_sync,                /* mg_sync             */			\
      mg_worldbegin,          /* mg_worldbegin       */			\
      mg_worldend,            /* mg_worldend         */			\
      mg_reshapeviewport,     /* mg_reshapeviewport  */			\
      /* geometry transform */						\
      mg_settransform,        /* mg_settransform     */			\
      mg_gettransform,        /* mg_gettransform     */			\
      mg_identity,            /* mg_identity         */			\
      mg_transform,           /* mg_transform        */			\
      mg_pushtransform,       /* mg_pushtransform    */			\
      mg_poptransform,        /* mg_poptransform     */			\
      /* texture transform */						\
      mg_settxtransform,      /* mg_settxtransform   */			\
      mg_gettxtransform,      /* mg_gettxtransform   */			\
      mg_txidentity,          /* mg_txidentity       */			\
      mg_txtransform,         /* mg_txtransform      */			\
      mg_pushtxtransform,     /* mg_pushtxtransform  */			\
      mg_poptxtransform,      /* mg_poptxtransform   */			\
      /* appearences */							\
      mg_pushappearance,      /* mg_pushappearance   */			\
      mg_popappearance,       /* mg_popappearance    */			\
      mg_setappearance,       /* mg_setappearance    */			\
      mg_getappearance,       /* mg_getappearance    */			\
      /***************/							\
      mg_setcamera,           /* mg_setcamera        */			\
      mg_polygon,             /* mg_polygon          */			\
      mg_polylist,            /* mg_polylist         */			\
      mg_mesh,                /* mg_mesh             */			\
      mg_line,                /* mg_line             */			\
      mg_polyline,            /* mg_polyline         */			\
      mg_quads,               /* mg_quads            */			\
      mg_bezier,              /* mg_bezier           */			\
      mg_bsptree,             /* mg_bsptree          */			\
      mg_tagappearance,       /* mg_tagappearance    */			\
      mg_untagappearance,     /* mg_untagappearance  */			\
      mg_taggedappearance,    /* mg_taggedappearance */			\
      }

struct mgfuncs mgnullfuncs =    /* mgfuncs for null (default) output device */
  NULLFUNCS;

struct mgfuncs _mgf =
  NULLFUNCS;

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
