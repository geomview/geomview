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

#ifndef MINNEGRAPHICSDEFS
#define MINNEGRAPHICSDEFS

/*
 * Public interface to mg (minnegraphics) package.
 */

#include "color.h"
#include "point.h"
#include "transform.h"
#include "window.h"
#include "camera.h"
#include "appearance.h"
#include "bsptree.h"

#ifndef POLYLISTPDEFS
struct Poly;
struct Vertex;
#endif
#ifndef _GV_BSPTREE_P_H_
struct BSPTree;
#endif

typedef struct mgcontext mgcontext;     /* Opaque typedef */

typedef int (*mgshadefunc)( int npoints, HPoint3 *points, Point3 *normals,
			    ColorA *colors, ColorA *newcolors );
typedef void (*mgwinchfunc)( /*mgcontext *mgc, void *data, int win_why, ...*/ );

typedef struct mgNDctx mgNDctx;
typedef int (*mgNDmapfunc)(mgNDctx *NDctx,
                           HPointN *NDpoint, HPoint3 *newpoint,
                           ColorA *newcolor);
struct mgNDctx
{
  mgNDmapfunc mapHPtN;
  void *(*saveCTX)(mgNDctx *NDctx);
  void (*pushTN)(mgNDctx *NDctx, TransformN *leftTN);
  void (*pushT)(mgNDctx *NDctx, Transform leftT);
  void (*restoreCTX)(mgNDctx *NDctx, void *savedCTX);
  BSPTree *bsptree;
};

struct mgfuncs {  /* mg member functions; potentially changed per device */

  short         mg_devno;       /* Current device number */

  int           (*mg_setdevice)(void);
  int           (*mg_feature)(int feature);
  mgcontext *   (*mg_ctxcreate)(/* int a1, ... */);
  void          (*mg_ctxdelete)(mgcontext *ctx);
  int           (*mg_ctxset)(/* a1, ...  */);
  int           (*mg_ctxget)(int attr, void* valueptr);
  int           (*mg_ctxselect)(mgcontext *ctx);
  void          (*mg_sync)(void);
  void          (*mg_worldbegin)(void);
  void          (*mg_worldend)(void);
  void          (*mg_reshapeviewport)(void);
  /* geometry transform support */
  void          (*mg_settransform)(Transform T);
  void          (*mg_gettransform)(Transform T);
  void          (*mg_identity)(void);
  void          (*mg_transform)(Transform T);
  int           (*mg_pushtransform)(void);
  int           (*mg_poptransform)(void);
  /* texture transform support */
  void          (*mg_settxtransform)(Transform T);
  void          (*mg_gettxtransform)(Transform T);
  void          (*mg_txidentity)(void);
  void          (*mg_txtransform)(Transform T);
  int           (*mg_pushtxtransform)(void);
  int           (*mg_poptxtransform)(void);
  /* appearances */
  int           (*mg_pushappearance)(void);
  int           (*mg_popappearance)(void);
  const Appearance *(*mg_setappearance)(const Appearance *app, int merge);
  const Appearance *(*mg_getappearance)(void);
  int           (*mg_setcamera)(Camera* cam);
  void          (*mg_polygon)(int nv, HPoint3 *v, int nn,
                              Point3 *n, int nc,ColorA *c);
  void          (*mg_polylist)(int np, struct Poly *p,
                               int nv, struct Vertex *v, int plflags);
  void          (*mg_mesh)(int wrap,int nu,int nv,HPoint3 *p,
                           Point3 *n, Point3 *nq, ColorA *c, TxST *ST,
                           int mflags);
  void          (*mg_line)(HPoint3 *p1, HPoint3 *p2);
  void          (*mg_polyline)(int nv, HPoint3 *v, int nc, ColorA *c, 
                               int wrapped);
  void          (*mg_quads)(int nquads, HPoint3 *v, Point3 *n, ColorA *c,
                            int qflags);
  void          (*mg_bezier)(int du, int dv, int dimn, float *ctrlpts,
                             TxST *txmapst, ColorA *c);
  void          (*mg_bsptree)(struct BSPTree *bsptree);
  const void   *(*mg_tagappearance)(void);
  void          (*mg_untagappearance)(const void *tag);
  void          (*mg_taggedappearance)(const void *tag);
};

extern struct mgfuncs _mgf;

/* Common mg attributes -- for mgctxcreate(), mgctxset(), mgctxget() */

#define MG_END          0
#define MG_WINDOW       128     /* WnWindow *                           */
#define MG_PARENT       129     /* mgcontext *context (of parent window)*/
#define MG_SHOW         130     /* int visible  (make window visible)   */
#define MG_SETOPTIONS   131     /* int mask (of options to enable) see  */
#define MG_UNSETOPTIONS 132     /* int mask (options to disable)   mg.h */
#define MG_BACKGROUND   133     /* ColorA *                             */
#define MG_CAMERA       134     /* Camera *                             */
#define MG_APPEAR       135     /* Appearance *default_appearance       */
#define MG_ApSet        136     /* Appearance create list               */
#define MG_WnSet        137     /* Window create list           */
#define MG_CamSet       138     /* Camera create list           */
#define MG_ZNUDGE       139     /* float; fraction of Z-range by which lines &
                                 * edges are nudged nearer than polygons.
                                 */

#define MG_NDCTX        140     /* mgNDctx * */

/* Note 140, 141, 142 used to be MG_EUCLIDEAN, MG_HYPERBOLIC, and
   MG_SPHERICAL, which are now obsolete.  These numbers may be reused now. */

#define MG_SHADER       143     /* mgshadefunc *shader_function         */
#define MG_SHADERDATA   144     /* void *shaderdata (interpreted by shader) */

#define MG_ABLOCK       145     /* void *ablock */

#define MG_SPACE        146     /* TM_EUCLIDEAN, TM_HYPERBOLIC,
                                   or TM_SPHERICAL */

/* More control options: needed initially for X11, but might exist
 * some day for others.
 */
#define MG_DITHER       150     /* Dithering control: 1(on)/0(off) */
#define MG_DEPTHSORT    151     /* Depth sorting: one of: */
enum sortmethod {MG_NONE, MG_DEPTH, MG_ZBUFFER};

#define MG_BITDEPTH     152     /* frame buffer depth (1,8,24,...?) */
#define MG_WINCHANGE    153     /* void *winchange(ctx, void *data)
                                 *  function, called when someone asks this
                                 *  mg window to change (size, border, etc.)
                                 */
#define MG_WINCHANGEDATA 154    /* void *data param for winchange() callback */


/* win_why values for (*winchange) callback */

#define MGW_WINCHANGE   1  /* Window data (pos'n, size, title, ...) change */
#define MGW_WINDELETE   2  /* Window being deleted */
#define MGW_DOUBLEBUF   3  /* Double-buffering state changed; called as */
                           /* winchange(mgc, info, MGW_DOUBLEBUF, is_double) */
                           /* Other values, 16 and up, may be per-device */

/* NOTE:  the MG_SPACE attribute specifies the space in which the
   objects being drawn live.  It serves the same purpose as the
   NeXT-specific attribute MG_GEOMTYPE, which should eventually
   be phased out. mbp Fri Dec 11 14:51:30 1992 */

/* Possible MG Features */
#define MGF_BEZIER      100     /* Driver supports mgbezier */

/* Bitwise ORable options for mg_ctxset MG_[UN]SETOPTIONS */
#define MGO_DOUBLEBUFFER        0x1             /* Default single buffered */
#define MGO_HIDDEN              0x2             /* Default not Z-buffered */
#define MGO_BACKFACECULL        0x4             /* Default back-faces shown */
#define MGO_INHIBITBACKGROUND   0x8             /* Default background drawn */
#define MGO_INHIBITCLEAR        0x10            /* Default clear at worldbegin*/
#define MGO_INHIBITSWAP         0x20            /* Default swapbufs at worldend */
#define MGO_NORED               0x40            /* Inhibit drawing into red */
#define MGO_NOGREEN             0x80            /* ... green ... */
#define MGO_NOBLUE              0x100           /* ... or blue planes */

#define MGO_INHIBITCAM          0x200           /* Don't load matrix in worldbegin() */

/* mgmesh 'style' values */
#define MM_NOWRAP       0
#define MM_UWRAP        1
#define MM_VWRAP        2

/* mergeflag values for mgsetappearance() */
#define MG_SET          0
#define MG_MERGE        1

#define mgctxselect( mgc )              (*_mgf.mg_ctxselect)( mgc )
#define mgctxcreate                     (*_mgf.mg_ctxcreate)
#define mgctxdelete( mgc )              (*_mgf.mg_ctxdelete)( mgc )
#define mgctxset                        (*_mgf.mg_ctxset)
#define mgctxget( attr, valuep )        (*_mgf.mg_ctxget)( attr, valuep )

/* appearnce stack */
#define mgpushappearance()              (*_mgf.mg_pushappearance)()
#define mgpopappearance()               (*_mgf.mg_popappearance)()
#define mgsetappearance( ap, merge )    (*_mgf.mg_setappearance)( ap, merge )
#define mggetappearance()               (*_mgf.mg_getappearance)()
#define mgtagappearance()               (*_mgf.mg_tagappearance)()
#define mguntagappearance(tag)          (*_mgf.mg_untagappearance)(tag)
#define mgtaggedappearance(tag)         (*_mgf.mg_taggedappearance)(tag)

#define mgsetcamera( cam )              (*_mgf.mg_setcamera)( cam )

/* transform stack */
#define mgsettransform( T )             (*_mgf.mg_settransform)( T )
#define mggettransform( T )             (*_mgf.mg_gettransform)( T )
#define mgidentity()                    (*_mgf.mg_identity)()
#define mgtransform( T )                (*_mgf.mg_transform)( T )
#define mgpushtransform()               (*_mgf.mg_pushtransform)()
#define mgpoptransform()                (*_mgf.mg_poptransform)()

/* texture transform stack */
#define mgsettxtransform( T )             (*_mgf.mg_settxtransform)( T )
#define mggettxtransform( T )             (*_mgf.mg_gettxtransform)( T )
#define mgtxidentity()                    (*_mgf.mg_txidentity)()
#define mgtxtransform( T )                (*_mgf.mg_txtransform)( T )
#define mgpushtxtransform()               (*_mgf.mg_pushtxtransform)()
#define mgpoptxtransform()                (*_mgf.mg_poptxtransform)()

#define mgreshapeviewport()             (*_mgf.mg_reshapeviewport)()


#define mgworldbegin()                  (*_mgf.mg_worldbegin)()
#define mgsync( )                       (*_mgf.mg_sync)()
#define mgworldend()                    (*_mgf.mg_worldend)()

#define mgpolygon(nv,v, nn, n, nc, c)           \
  (*_mgf.mg_polygon)( nv,v, nn,n, nc,c )
#define mgpolylist(np, p, nv, v, flags)         \
  (*_mgf.mg_polylist)(np, p, nv, v, flags)
#define mgmesh(style, nu, nv, v, n, nq, c, mflags)              \
  (*_mgf.mg_mesh)(style, nu, nv, v, n, nq, c, NULL, mflags)
#define mgmeshst(style, nu, nv, v, n, nq, c ,st, mflags)        \
  (*_mgf.mg_mesh)(style, nu, nv, v, n, nq, c, st, mflags)
#define mgline(p1, p2)                          \
  (*_mgf.mg_line)( p1, p2 )
#define mgpolyline(nv, v, nc,c, wrap)           \
  (*_mgf.mg_polyline)(nv, v, nc, c, wrap )
#define mgquads(nquad, v, n, c, qflags)		\
  (*_mgf.mg_quads)(nquad, v, n, c, qflags)
#define mgbezier(du, dv, dimn, ctrlpts, tx, c )		\
  (*_mgf.mg_bezier)(du, dv, dimn, ctrlpts, tx, c)
#define mgbsptree(bsptree)                      \
  (*_mgf.mg_bsptree)(bsptree)

#define mgfeature( feature )            (*_mgf.mg_feature)( feature )

extern mgcontext *mgcurrentcontext();           /* Get ptr to current context */
extern int mgbegin( void );                     /* Init, select default dev */
extern int mgend( void );                       /* Shut down */ 

extern int mgclonefrom( mgcontext *other );     /* Init cur mgctx from other */

/*
 * Sample software-shading function, usable as MG_SHADER.
 */
extern int mg_eucshade( int npoints, HPoint3 *points, Point3 *normals,
			ColorA *colors, ColorA *newcolors );

/*
 * Polling routine, intended to be called roughly once per
 * render cycle, which detects and purges textures which don't
 * seem to be in use.
 */
extern int mg_textureclock(void);

#define CURRENTMGTYPE()  (_mgf.mg_devno)

/*
 * Device select routines -- set default for future mgctxcreate()'s
 */
extern int mgdevice_GL( void );         
extern int mgdevice_X11( void );
extern int mgdevice_XGL( void );
extern int mgdevice_RIB( void );
extern int mgdevice_RI( void );
extern int mgdevice_PS( void );
extern int mgdevice_VT100( void );
extern int mgdevice_XT( void );
extern int mgdevice_MAC( void );
extern int mgdevice_TEK( void );
extern int mgdevice_TELETYPE( void );
extern int mgdevice_PUNCHCARD( void );
extern int mgdevice_SIMSTIM( void );
extern int mgdevice_NULL( void );
extern int mgdevice_BUF( void );
extern int mgdevice_OPENGL( void );
extern int mgdevice_PICK( void );
extern int mgdevice_PRIM( void );

extern int mgdevice( int deviceno );    /* Alternative to the above */

#define  MGD_NODEV      -1      /* error return: can't select that device */
#define  MGD_NULL       0       /* no device, just return current one */
#define  MGD_GL         1       /* SGI GL */
#define  MGD_XGL        2       /* Sun XGL */
#define  MGD_X11        3       /* Generic X11 */
#define  MGD_RIB        4       /* Renderman RIB*/
#define  MGD_PS         5       /* Postscript */
#define  MGD_RI         6       /* Quick Renderman */
#define  MGD_BUF        7       /* Raster buffer */
#define  MGD_OPENGL     8       /* Open GL */
#define  MGD_PICK       9       /* software pick */
#define  MGD_PRIM       10      /* geometric data */

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
