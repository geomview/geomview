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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _DRAWER_H_
#define _DRAWER_H_

#include "common.h"
#include "transformn.h"
#include "pick.h"
#include "mg.h"

/* N-D color maps */
typedef struct cent {	/* Colormap entry */
  float v;			/* projection value */
  ColorA c;			/* color */
  int interp;			/* interpolate values between cmap points */
} cent;

typedef struct cmap {	/* Colormap table */
  HPointN *axis;		/* projection axis */
  vvec cents;			/* colormap; entries are 'cent's */
  int coords;			/* ID of axis' coordinate system; default UNIVERSE */
} cmap;
#define MAXCMAP 4

typedef struct NDstuff
{
  mgNDctx mgNDctx;
  TransformN *W2C;      /* World-to-Camera transform */
  TransformN *T;	/* object-to-ND-camera transform, including
			 * the projection to the lower-dimensional
			 * sub-space.
			 */

#if 0
  Transform  MGFactor;  /* 4x4 transform handed over to the mg layer */
  TransformN *rest;     /* remaining (Nd-4) lines, contained in the
			 * rows (4..Nd)
			 */
  int *perm;            /* permuation on the rows of T */
#endif

  TransformN *W2c;      /* Universe-to-Color transform */
  TransformN *Tc;	/* object-to-coloring-space-transform */
  int ncm;		/* number of colormaps */
  cmap *cm;		/* colormap array */
  HPointN *hc;	/* work area for color projections */
} NDstuff;

#define OBJECTFIELDS							\
  char *name[2];        /* array of names */				\
  Geom *Item;           /* For Geoms, the Inst holding the		\
                         *  modelling transform; for DViews,		\
                         *  an Inst pointing to the scene.		\
                         *  In all cases this Inst holds any		\
                         *  relevant Appearance.			\
                         */						\
  PFI updateproc;       /* motion update procedure */			\
  Handle  *incrhandle;  /* Handle to above */				\
  Transform Incr;       /* xform per frame update, if no updateproc */	\
  int id;               /* my id */					\
  int seqno;            /* sequence no for keeping track of changes */	\
  bool redraw;          /* object needs redrawing */			\
  bool moving;          /* moving -> Incr != identity || updateproc != NULL */ \
  unsigned changed      /* Something about this obj changed; redraw */

/* flags for the "changed" field */
#define CH_NOTHING      0
#define	CH_GEOMETRY	1
#define	CH_TRANSFORM	2

typedef struct DObject {
  OBJECTFIELDS;
} DObject;

typedef struct DGeom {
  OBJECTFIELDS;
  Geom *Inorm;          /* Inst holding normalization transform */
  Geom *Lgeom;          /* List node holding pointer to user's Geom */
  Geom *Lbbox;          /* List node holding the pointer to our Bbox */
  int  bbox_center;     /* bbox center as alien object */
  bool bboxdraw;        /* boolean */
  char normalization;   /* Normalization: NONE, EACH, or ALL */
  bool bboxvalid;       /* bbox is valid */
  bool pickable;        /* Is this Dgeom pickable ? */
  int  bezdice;         /* Meshsize for diced up bezier patch */
  Appearance *bboxap;
  enum citizenship {ORDINARY, THEWORLD, ALIEN} citizenship;
  TransformN *NDT, *NDTinv;     /* N-D transform and its inverse; NULL for 3-D objects */

/* Ordinary DGeoms are subgeoms of Theworld DGeom.
 * Theworld is the single DGeom containing the world transforms
 * and all ordinary DGeoms.
 * Alien DGeoms are in the Universe list but are not part of Theworld.
 */
} DGeom;

/* models of hyperbolic space; these must start with 0 and be
   consecutive because they are used as indices in the browser */
typedef enum {
  VIRTUAL=0,
  PROJECTIVE,
  CONFORMALBALL
} HModelValue;

typedef struct DView {
  OBJECTFIELDS;
  Camera *cam;          /* The camera itself */
  Handle *camhandle;    /* Handle to above */
  WnWindow *win;        /* The (abstract) window */
  unsigned frozen;      /* flag: frozen: Don't attempt to redraw this view */
  bool newcam;          /* flag: need to mgreshapeviewport() */
  mgcontext *mgctx;     /* Window's mg context pointer */
  int apseq;            /* seq no: has global default changed? */
  float lineznudge;     /* millionths of Z-range by which lines lie closer
                         * than surfaces */
  Color backcolor;      /* Window background color */
  bool cameradraw;      /* Do we draw cameras in this view? */
  Geom *hsphere;        /* hsphere at infy for use in hyp mode; when NULL
                           don't draw it, otherwise draw it */
  HModelValue hmodel;   /* model of hyperbollic space for this camera; value
			   is one of VIRTUAL, PROJECTIVE, CONFORMAL, and is
                           meaningful only when viewing hyperbolic geometry */
  PFV extradraw;        /* function to draw any extra stuff in this window */
  Keyword stereo;       /* {NO,HORIZONTAL,VERTICAL}_KEYWORD: stereo style */
  int stereogap;        /* gap between subwindows in stereo mode        */
  WnPosition vp[2];     /* Subwindow viewports, if stereo != MONO       */
  mgshadefunc shader;   /* Software shader function (NULL -> hardware)  */
  int NDPerm[4];        /* N-D -> 3-D axis permutation */
  int nNDcmap;
  cmap NDcmap[MAXCMAP]; /* N-D color map */
  struct NDcam *cluster;/* to which cluster do we belong?  NULL if not N-D */
} DView;

/* flags for "frozen" */
#define UNFROZEN    0
#define SOFT_FROZEN 1
#define HARD_FROZEN 2

typedef struct NDcam {
  char		*name;		/* camera-cluster name */
  TransformN	*C2W, *W2C;	/* N-D camera-to-universe transform */
  struct NDcam  *next;		/* next N-D camera */
} NDcam;

/*
 * The array of DGeoms is a flat structure with pointers into the
 * following OOGL hierarchy:
 *
 *
 * drawerstate.universe -->     LIST -------- LIST --- LIST... [rest of Aliens]
 *				  |		|  	|
 *				  |  	      INST    INST
 *				  |	      [Alien] [Alien]
 *				  |
 * 				drawerstate.world == dgeom[0]:
 *				  |
 * dgeom[0]->Item  -->		INST        [world modelling xform]
 *		 	        [Theworld]    
 *				  |
 * dgeom[0]->Inorm  -->		INST        [world normalizing xform]
 *				  |
 * dgeom[0]->Lgeom --> 	LIST  -----  LIST --- ... [all ordinary DGeoms]
 *				[Ordinary]  [Ordinary] 
 *				  |
 *				DGeom[x]:
 *				  |
 *				INST (= Item) [modelling xform, appearance]
 *		  		  |
 *		  		  |
 *				INST (= Inorm) [normalization xform]
 *		  		  |
 *		  		  |
 *				LIST (= Lgeom) -------- LIST (= Lbbox)
 *				  |			  |
 *				[user's Geom]   [BBox of user's Geom, or NULL]
 *
 * Note that the world appearance is *not* dgeom[0]->Item->ap, which
 * is always NULL.
 * Changes to the world appearance are broadcast to all individual dgeoms. 
 * worldap is kept around in drawer.c to let the ui know how to
 * set control panels when the world is the target. 
 */
typedef struct DrawerState {

  int 		      pause;	/* hiatus on emodule input? */
  float		 lastupdate;	/* Time (seconds since program began) of last xfm update */
  DView		    defview;	/* default DView */
  WnPosition         winpos;	/* screen position of current mg context's window */
  int         normalization;	/* NONE, EACH, KEEP, or ALL (default EACH) */
  int                 space;	/* TM_EUCLIDEAN, TM_HYPERBOLIC,
				   TM_SPHERICAL (default EUC) */
  int		    changed;	/* Flag: geom or camera or something changed,
				 * must redraw. */
  Appearance *           ap;	/* Default appearance: backstop + .v.ap */
  int		      apseq;	/* Sequence number of default appearance;
				 * each drawer mg context should reexamine ap
				 * if drawerstate.apseq != view->apseq
				 */
  Geom *	      world;	/* Everything in the world, including
				 * global modelling xform & appearance.
				 */
  Geom *           universe;	/* List of everything in the universe: 
				 * world and aliens. */

  int		   NDim;	/* Dimension of all ND items */
  NDcam		  *NDcams;	/* List of all ND camera clusters, or NULL */
#ifdef MANIFOLD

#define MTMAX 500
#define DVMAX 50

  Transform		MT[MTMAX];/* manifold transforms */
  int			nMT;	  /* number of transforms actually in MT */
  HPoint3		DV[DVMAX];/* vertices of manifold Dirichlet domain */
  int			nDV;	  /* number of vertices actually in DV */
  float			d1, i1;
#endif

  float			motionscale; /* set by user; used in scaling some
					(not all) motions */

  Geom			*camgeom;    /* Object used when drawing cameras */
  int			 camproj;    /* Apply camera's projection to it? */

  /* drawerstate.world is the first node in drawerstate.universe
   * dgeom[0]->Item == drawerstate.world
   * dgeom[0]->Inorm is the child of the above
   * dgeom[0]->Lgeom is the child of dgeom[0]->Inorm
   */
} DrawerState;


/*
 * LOOPGEOMS(i,dg) loops over all defined geoms; for each defined
 * geom, int i is the index in the dgeoms array, and DGeom *dg
 * points to the geom.  Usage:
 *   LOOPGEOMS(i,dg) {
 *     < body of loop --- use i and dg as needed >
 *   }
 * Note we skip dgeom[0], since it's by definition the entire world.
 */
#define LOOPGEOMS(i,dg) \
  for (i=1; i<dgeom_max; ++i) if ( (dg=dgeom[i]) != NULL)

/*
 * LOOPSOMEGEOMS(i,dg,cit) is like LOOPGEOMS but only returns ordinary DGeoms
 */

#define LOOPSOMEGEOMS(i,dg,cit) \
  for (i=1; i<dgeom_max; ++i) if ( (dg=dgeom[i]) && (dg->citizenship==cit) )

/*
 * LOOPVIEWS(i,dv) is like LOOPGEOMS but for views.
 */
#define LOOPVIEWS(i,dv) \
  for (i=0; i<dview_max; ++i) if ( (dv=dview[i]) != NULL)


/* Broadcast by looping the correct array if index == ALLINDEX
 * else broadcast to all dgeoms that are not aliens if id == WORLDGEOM
 * else don't go through loop at all if improper index
 * else just go through for loop once if index is normal.
 * Obj is set to correct value on each pass through the loop.
 *
 * int i is index in the dgeoms or views array,
 * type is T_GEOM or T_CAM (or T_NONE if either is acceptable),
 * and objtype *obj is set to successive elements in the dgeoms/views array.
 *
 * Usage:
 *  MAYBE_LOOP(id,i,type,objtype,obj) {
 *    < body of loop: use i and obj as needed >
 *                    is set correctly on any given pass through the loop >
 *  }
 */

#define MAYBE_LOOP(id, i, type, objtype, obj) \
    for(i = 0; (obj = (objtype *)drawer_next_bcast(id, &i, type)); i++)

#define MAYBE_LOOP_ALL(id, i, type, objtype, obj) \
    for(i = 0; (obj = (objtype *)drawer_next_object(id, &i, type)); i++)


/*
 * An ID encodes an index (int) and a type (cam or geom).
 */
#define	T_NONE	0
#define T_GEOM  1
#define T_CAM	2
#define	T_MAX	3	/* Max T_* value plus one */

#define	GEOMID(i)	((i)<<2 | T_GEOM)	/* Geometric object #i */
#define	CAMID(i)	((i)<<2 | T_CAM)	/* Camera #i */
#define	NOID		((0)<<2 | T_NONE)
#define ID(type,i)	((i)<<2 | type)

#define	ISGEOM(id)	(((id) & 3) == T_GEOM)
#define	ISCAM(id)	(((id) & 3) == T_CAM)
#define ISTYPE(type,id)	(((id) & 3) == type)
#define TYPEOF(id)	 ((id) & 3)

#define	INDEXOF(id)	((id) >> 2)

#define	ALLINDEX	-1
#define	WORLDGEOM	GEOMID(0)

#define	FOCUSID		CAMID(-4)
					/* these are not necessarily geoms! */
#define TARGETID	GEOMID(-5)	/* "current target" */
#define CENTERID	GEOMID(-8)	/* "current center" */

#define TARGETGEOMID	GEOMID(-6)	/* "current target geom" */
#define TARGETCAMID	CAMID(-7)	/* "current target cam" */
#define	DEFAULTCAMID	CAMID(-5)

#define	SELF		GEOMID(-9)	/* "this geomview object" */
#define	UNIVERSE	GEOMID(-10)	/* the universe (as coordinate system) */
#define	PRIMITIVE	GEOMID(-11)	/* this picked OOGL primitive (coord system) */
#define BBOXCENTERID    GEOMID(-12)     /* center of bbox object */  

#define ALLCAMS		CAMID(ALLINDEX)
#define ALLGEOMS	GEOMID(ALLINDEX)

typedef enum {
  DRAWER_NOKEYWORD = 1,
  DRAWER_NEAR,
  DRAWER_FAR,
  DRAWER_FOV,
  DRAWER_FOCALLENGTH,
  DRAWER_KA,
  DRAWER_KS,
  DRAWER_KD,
  DRAWER_SHININESS,
  DRAWER_NORMSCALE,
  DRAWER_LIGHT_INTENSITY,
  DRAWER_LINE_ZNUDGE,
  
  DRAWER_FACEDRAW,
  DRAWER_EDGEDRAW,
  DRAWER_VECTDRAW,
  DRAWER_NORMALDRAW,
  DRAWER_TEXTUREDRAW,
  DRAWER_TEXTUREQUAL,
  DRAWER_SHADELINES,
  DRAWER_CONCAVE,
  DRAWER_BACKCULL,
  DRAWER_SHADING,
  DRAWER_LINEWIDTH,
  DRAWER_PERSPECTIVE,
  DRAWER_EVERT,
  DRAWER_BBOXDRAW,
  DRAWER_NORMALIZATION,
  DRAWER_SAVE,
  DRAWER_PROJECTION,
  DRAWER_BEZDICE,
  DRAWER_CAMERADRAW,
  DRAWER_PICKABLE,
  DRAWER_HSPHERE,
  DRAWER_DOUBLEBUFFER,
  DRAWER_APOVERRIDE,

  DRAWER_DIFFUSE,
  DRAWER_EDGECOLOR,
  DRAWER_NORMALCOLOR,
  DRAWER_BBOXCOLOR,
  DRAWER_BACKCOLOR,
  DRAWER_LIGHTCOLOR,

  DRAWER_TRANSLUCENCY,
  DRAWER_ALPHA,

  DRAWER_INERTIA,
  DRAWER_CONSTRAIN,
  DRAWER_OWNMOTION
} DrawerKeyword;

extern DView **		dview;
extern DGeom **		dgeom;
extern DrawerState	drawerstate;
extern int		dgeom_max;
extern int		dview_max;

extern TransformStruct ts_identity;

DObject *    drawer_get_object( int id );/* Get DObject * for object 'id' */
DObject * drawer_next_object( int id, int *indexp, int type );
DObject *drawer_next_bcast( int id, int *indexp, int type );
Appearance * drawer_get_ap( int id );	/* Get net appearance for DGeom 'id' */

void	drawer_make_bbox(DGeom *dg, bool combine);
int	drawer_geom_count();
int	drawer_cam_count();
int	drawer_idbyctx( mgcontext *ctx );
bool	drawer_moving();	/* is anything (DGeom, DView) moving? */
int	drawer_idbyname(char *name);
char    *drawer_id2name(int id);
int     drawer_name2metaid(char *name);
int	drawer_objok(int id, int type);
int	drawer_pick( int id, int x, int y, Pick *pick);
void	drawer_init(char *apdefault, char *camgeometry, char *windefault);

void	drawer_merge_baseap( ApStruct *as );
void	drawer_merge_camera( int id, Camera *cam );
void	drawer_merge_window( int id, WnWindow *win );
void	drawer_name_object(int id, int ni, char *name);
void	drawer_updateproc(int id, PFI func);
void	drawer_center(int id);
void	drawer_stop(int id);
void	drawer_pause(int pause);
void	drawer_set_window(int id, WnWindow *win);
void	drawer_snap_window(int id, char *fname);
void	drawer_init_dgeom(DGeom *dg, int id, enum citizenship citizenship);
void	drawer_nuke_world();
void	drawer_nuke_cameras(int keepzero);
void	drawer_get_transform(int id, Transform T, int cumulative);
TransformN *drawer_get_ND_transform(int from_id, int to_id);
TransformN *drawer_ND_CamView(DView *dv, TransformN *TN);
int	drawer_idmatch(int id1, int id2);
mgshadefunc softshader(int camid);

NDstuff *drawer_init_ndstuff(DView *dv, TransformN *W2C, TransformN *W2U);
void drawer_destroy_ndstuff(NDstuff *nds);
void drawer_transform_ndstuff(NDstuff *nds, TransformN *T);

void	drawer_float(int id, DrawerKeyword key, float fval);
   /* key = DRAWER_NEAR, DRAWER_FAR, DRAWER_KA, DRAWER_KS,
             DRAWER_KD, DRAWER_SHININESS, DRAWER_NORMSCALE,
	     DRAWER_LIGHT_INTENSITY */

void	drawer_int(int id, DrawerKeyword key, int ival);
   /* key = DRAWER_FACEDRAW, DRAWER_EDGEDRAW, DRAWER_NORMALDRAW,
	     DRAWER_SHADING, DRAWER_LINEWIDTH, DRAWER_PERSPECTIVE,
	     DRAWER_EVERT, DRAWER_BEZDICE, DRAWER_NORMALIZATION,
	     DRAWER_CAMERADRAW */

void	drawer_color(int id, DrawerKeyword key, Color *col);
   /* key = DRAWER_DIFFUSE, DRAWER_EDGECOLOR, DRAWER_NORMALCOLOR,
            DRAWER_BBOXCOLOR, DRAWER_BACKCOLOR */

float scaleof(int id);
int spaceof(int id);
extern Keyword hmodelkeyword(char *s, HModelValue val);

void drawer_set_ap(int id, Handle *h, Appearance *ap);

#ifdef MANIFOLD

int drawer_read_manifold(char *file);

#endif /* MANIFOLD */

#endif /* ! _DRAWER_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
