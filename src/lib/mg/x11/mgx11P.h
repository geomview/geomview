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
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef NO_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif /*!NO_SHM*/

#include "mgx11.h"

typedef struct {
	Pt3Coord	x, y, z, w;	/* Point coordinates */
        ColorA		vcol;		/* Color of vertex */
	int		drawnext;	/* draw the next line segment? */
	} CPoint3;

#include "mgbufrender.h"
#include "mgx11render1.h"
#include "mgx11render8.h"
#include "mgx11render16.h"
#include "mgbufrender24.h"

typedef struct _mgx11win {
  int                   mgwinid;/* mg window id */
  Window                window; /* real X window structure */
  Window		parent; /* parent X windows */
  XImage                *image;   /* image for double buffering */
  unsigned char         *buf;    /* convience ptr */
  int                   width, height; /* actual size of shm seg */
  int			  zwidth;
  GC                    gc;     /* real X graphics context */
  XSizeHints            xsh;    /* X size hints */
  XWindowAttributes     xwa;    /* X window attributes */
  XSetWindowAttributes  xswa;
  XEvent                event;  /* X event structure for window */
  struct _mgx11win      *next;  /* next in list */
#ifndef NO_SHM
  XShmSegmentInfo       shminf;  /* book-keeping for shm */
#endif
} mgx11win;

/* Display List Structure */
/*****************************************************************************/

typedef struct _mgx11prim {
  int                   mykind; /* kind of primitive */
  int                   index;  /* index into array of vertices */
  int                   numvts; /* number of vertices */
  float                 depth;  /* z value for sorting (at least for now) */
  int			color[3]; /* rgb color of polygon */
  int			ecolor[3]; /* edge color of polygon */
  int			ewidth; /* edge width */
} mgx11prim;

typedef struct _mgx11_sort
{
  vvec 		 primsort;	/* pointers to mgx11prim array elements*/
  vvec		 prims;		/* array of mgx11prim structures */
  int            primnum;	/* number of primitives */
  int		 cprim;		/* current primitive being stored */

  vvec		 pverts;	/* array of vertices */
  int            pvertnum;
  int		 cvert;		/* current vertex being stored */
  int		 maxverts;	/* maximum number of vertices */

} mgx11_sort;

#define		MGX_NULL		0
#define		MGX_END			0
#define		MGX_BGNLINE		1
#define		MGX_BGNPOLY		2
#define		MGX_BGNEPOLY		3
#define		MGX_BGNSLINE		4
#define		MGX_BGNSPOLY		5
#define		MGX_BGNSEPOLY		6
#define		MGX_VERTEX		7
#define		MGX_CVERTEX		8
#define		MGX_COLOR		9
#define		MGX_ECOLOR		10

#define		PRIM_LINE		1
#define		PRIM_POLYGON		2
#define		PRIM_EPOLYGON		3
#define		PRIM_SLINE		4 /* smooth shaded primitives */
#define		PRIM_SPOLYGON		5
#define		PRIM_SEPOLYGON		6
#define		PRIM_INVIS		7

/* End of Display List Structure */
/*****************************************************************************/

typedef struct mgx11context {
  struct mgcontext mgctx;	/* The mgcontext */
  int visible;			/* has window been displayed on screen yet? */
  int win;			/* 1 if window is created */
  int pix;			/* 1 is window is a pixmap */
  int znudge;			/* znudge for drawing lines closer */
  float znudgeby;		/* how much of a nudge ? */
  long znear, zfar;		/* Current Z-buffer limits */
  enum sortmethod sortmethod;   /* MG_NONE, MG_DEPTH, MG_ZBUFFER */
  int dither;			/* Should we dither? */
  int bitdepth;			/* what bit depth ... 24, 8, or 1? */
  Visual *visual;		/* visual for window */
  int shm;			/* Do we use shared memory? */
  int xmin, xmax, ymin, ymax;
  int oxmin, oxmax, oymin, oymax;
  int exposed;
  int noclear;
  vvec room;			/* Scratch space */

  Display *mgx11display;	/* pointer to X-display */
  Colormap cmap;		/* Our colormap */
  int cmapset;			/* is colormap set or not? */
  mgx11win *myxwin;		/* pointer to mgx11window structure */
  mgx11_sort *mysort;		/* sorting structure */
  int	sizelock;		/* for prohibiting size change from within mg
				    when the cam window is part of a larger
				    hierarchy of windows as with Widgets */

  void (*deleted)(Window);
				/* callback for when I'm deleted */
				/* pass back parent window */

} mgx11context;

#define MAXZNUDGE	8	/* Max depth of mgx11_closer()/farther() */

#define _mgx11c		((mgx11context*)_mgc)
