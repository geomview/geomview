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

#ifndef _MG_PDEFS_
#define _MG_PDEFS_

/*
 * Private, but machine-independent, declarations for mg library.
 */
#include <stdlib.h>
#include "mg.h"

#define MGCONTEXTMAGIC  OOGLMagic('m', 'c'<<8 | 1)

/* struct mgcontext changed bits */
#define   MC_WIN   0x01 /*  window changed (reshaped, &c) */
#define   MC_CAM   0x02 /*  Camera changed */
#define   MC_AP    0x04 /*  Appearance changed */
#define   MC_MAT   0x08 /*  Material changed */
#define   MC_LIGHT 0x10 /*  Lighting changed */
#define   MC_TRANS 0x20 /*  Transformation changed */
#define   MC_OPT   0x40 /*  Options changed */
#define   MC_USED  0x80 /*  worldbegin called (used by mg_textureclock) */

/* flags for mgastk->flags */
#define MGASTK_TAGGED  0x01 /* sticky */
#define MGASTK_ACTIVE  0x02 /* in use */
#define MGASTK_SHADER  0x04 /* software shader */

/*
 * Appearance portion of state.
 * We have a global stack of these which represent the user-specified values,
 * with a stack push at every mgpushappearance().
 */
struct mgastk {
	REFERENCEFIELDS;         /* use count for sticky elements */ 
	struct mgastk   *next;   /* stack link */
	mgcontext       *tag_ctx;/* back-pointer for ap_tagged list */
	unsigned short  flags;   /* sticky flag */
	unsigned short  changed;
	short           ap_seq, mat_seq, light_seq;
	Appearance      ap;     /* Shallow copy of appearance -- don't delete */
	Material        mat;    /* Ditto, Material */
	LmLighting      lighting;/* Ditto, Lighting */

	mgshadefunc     shader; /* Software shading function, or NULL if none */
	void            *shaderdata; /* Data which shader might use */
};

/*
 * Transform stack, pushed by mgpushtransform().
 */
struct mgxstk {
	struct mgxstk   *next;  /* stack link */
	Transform       T;      /* Current object->world xform */
	short           xfm_seq;
	short           hasinv; /* Flag: Tinv is valid */
	Transform       Tinv;
};

/*
 * Texture transform stack. Needed to paste textured stuff together
 * with INSTs, but allowing for a texture which covers the entire
 * object. cH, 2007.
 */
struct mgtxstk 
{
	struct mgtxstk *next; /* stack link */
	Transform      T;
	/* No need for an inverse. Sequence count? Probably not. */
};

struct mgcontext {
	REFERENCEFIELDS;
	struct mgfuncs  *devfuncs; /* Pointers to devices */
	short   devno;          /* Device index -- MGD_GL, MGD_X11, ... */

	unsigned short  changed; /* flags: Must update device state because...*/

	WnWindow        *win;   /* Window */
	Camera          *cam;   /* Camera */
	mgcontext       *parent; /* Parent mg window, if any */
	mgcontext       *next;  /* Link in list of all mg contexts */
	struct mgxstk   *xstk;  /* Top of transform stack */
	struct mgtxstk  *txstk; /* Top of texture transform stack */
	struct mgastk   *astk;  /* Top of appearance stack */
	struct mgastk   *ap_tagged;/* Tagged (persistent) appearances */
	short           ap_min_tag, ap_max_tag; /* excluded seq. regions */
	short           mat_min_tag, mat_max_tag;
	short           light_min_tag, light_max_tag;
	int             shown;  /* Is window 'visible'? */
	ColorA          background; /* Background color */
	Image           *bgimage;

	int             opts;   /* MG_SETOPTIONS flag mask */

	Transform       W2C;    /* World->camera transform */
	Transform       C2W;    /* Camera->world transform */

	float           zfnudge; /* fraction of Z-range by which lines are
				  * nudged closer than surfaces.
				  */
	int             space;  /* space in which objects being drawn
				 * live; should be TM_EUCLIDEAN,
				 * TM_HYPERBOLIC, or TM_SPHERICAL
				 */


	Transform       W2S, S2W; /* world-to-screen, screen-to-world xfms */

	int             has;    /* Flag bits, set when cached values valid */
#define HAS_CPOS   0x1             /* Camera position (cpos, camZ) */
#define HAS_S2O    0x2             /* S2O, O2S */
#define HAS_POINT  0x4             /* Outline for generic fat point */

	/* Cached values, computed when needed: */
	HPoint3         cpos;     /* Location of camera in object coordinates */
	Point3          camZ;     /* Direction of camera +Z vector */
	Transform       O2S, S2O; /* object-to-screen, screen-to-object xfms */
	vvec            point;  /* outline for fat points */

	mgNDctx         *NDctx;/* pointer to caller-defined ND closure */

	mgwinchfunc     winchange; /* Callback function for mg Window etc. changes */
	void            *winchangeinfo; /* We call (*winchange)(ctx, winchangeinfo, win, changemask) */
};

#define next_ap_seq(ctx, astk)				\
	((((astk)->ap_seq + 1) >= (ctx)->ap_min_tag &&	\
	  ((astk)->ap_seq + 1) <= (ctx)->ap_max_tag)	\
	 ? (ctx)->ap_max_tag+1 : (astk)->ap_seq + 1)

#define next_mat_seq(ctx, astk)                                 \
	((((astk)->mat_seq + 1) >= (ctx)->mat_min_tag &&        \
	  ((astk)->mat_seq + 1) <= (ctx)->mat_max_tag)          \
	 ? (ctx)->mat_max_tag+1 : (astk)->mat_seq + 1)

#define next_light_seq(ctx, astk)				\
	((((astk)->light_seq + 1) >= (ctx)->light_min_tag &&    \
	  ((astk)->light_seq + 1) <= (ctx)->light_max_tag)	\
	 ? (ctx)->light_max_tag+1 : (astk)->light_seq + 1)

/*
 * Pointer to the current mg context.
 */
extern mgcontext *_mgc;

/*
 * List of all extant mg contexts.
 */
extern mgcontext *_mgclist;

/*
 * Declarations of common mg routines, accessible to other mg packages
 * (but not intended to be public):
 */

/* geometry transform */
extern void mg_identity(void);
extern void mg_transform(Transform T);
extern void mg_settransform(Transform T);
extern void mg_gettransform(Transform T);
extern int mg_pushtransform(void);
extern int mg_poptransform(void);

/* texture transform */
extern void mg_txidentity(void);
extern void mg_txtransform(Transform T);
extern void mg_settxtransform(Transform T);
extern void mg_gettxtransform(Transform T);
extern int mg_pushtxtransform(void);
extern int mg_poptxtransform(void);

extern void mg_quads(int nquads, HPoint3 *v, Point3 *n, ColorA *c, int qflags);
extern void mg_bezier(int du, int dv, int dimn, float *ctrlpts, TxST *txmapst,
		      ColorA *c);
extern void mg_bsptree(struct BSPTree *bsptree);
extern void mg_findcam();
extern void mg_findS2O();
extern void mg_findO2S();
extern void mg_makepoint();

extern int mg_pushappearance(void);
extern int mg_popappearance(void);
extern void mg_reshapeviewport(void);
extern const Appearance *mg_getappearance(void);
extern const Appearance *mg_setappearance(const Appearance *ap, int mergeflag);

extern mgcontext *mg_newcontext(struct mgcontext *);
extern int mg_appearancebits( Appearance *ap, int merge, int *valid, int *flag );

extern void mg_ctxdelete( struct mgcontext * );
extern int  mg_ctxselect( struct mgcontext * );
extern void mg_findO2S( void );
extern void mg_findS2O( void );
extern void mg_findcam( void );
extern void mg_makepoint( void );
extern void mg_worldbegin( void );

extern int mg_inhaletexture( Texture *tx, int rgba );

extern TxUser *mg_find_shared_texture(Texture *tx, int mgtype);
extern bool    mg_same_texture(Texture *tx1, Texture *tx2, bool really);
extern void    mg_reassign_shared_textures(mgcontext *ctx, int mgtype);
extern int     mg_find_free_shared_texture_id(int mgtype);

extern void    mg_remove_txuser(TxUser *tu);

extern const void *mg_tagappearance(void);
extern void mg_untagappearance(const void *tag);
extern void mg_taggedappearance(const void *tag);

#endif /*_MG_PDEFS_*/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 8 ***
 * End: ***
 */
