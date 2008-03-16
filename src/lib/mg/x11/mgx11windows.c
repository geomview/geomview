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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <limits.h>
#include "mgP.h"
#include "mgx11P.h"
#include "windowP.h"
#include "mgx11windows.h"

#if HAVE_CONFIG_H
# include "config.h"
#endif

/*#ifdef AIX*/
/*#include "/usr/lpp/X11/Xamples/extensions/lib/XShm.c"*/
/*#endif*/

#define THEIGHT 200
#define TWIDTH 200
#define MAX(a,b) (((a)<(b)) ? b : a)
#define MIN(a,b) (((a)<(b)) ? a : b)

static mgx11_sort *mgx11sort = NULL;	/* global sorting structure */
static Display *mgx11display = NULL;	/* global X display pointer */

/* Z-buffer */
static float *mgx11zbuffer = NULL;
static int mgx11zsize = 0;

/* Dithering stuff  ... fun fun fun :-) */
int colorlevels = 5;			/* try 125 levels for starters */
					/* But can use up to 216 if envariable
					 * GEOMVIEW_COLORLEVELS is 6.
					 */
unsigned long mgx11colors[217];
static XColor mgx11colorcells[217];
static Colormap	cmap;
int mgx11multab[256];			/* premultiplied table for speed */
/*static ColorA black = {0.0, 0.0, 0.0, 0.0 };*/
static int curwidth = 1;
static int shm_message_shown = 0;

extern void dithermap(int, double, int [][3]);
extern unsigned long dithergb(int, int, int *, int);
extern int Xmg_primclip(mgx11prim *aprim);

static int globalXError;		/* X Error stuff */

/*
  Function: myXErrorHandler
  Description: handle error if shared memory is unavailable
  Author: Tim Rowley
*/
int myXErrorHandler(Display *d, XErrorEvent *e)
{
  globalXError = 1;
  return 1;
}

/*
  Function: Xmg_setx11display
  Description: digest info about this display, allocate space for colors
  Author: Daeron Meyer
*/
void Xmg_setx11display(Display *dpy)
{
  int		rgbmap[216][3], i, cube, colfail = 1;
  unsigned int	nplanes = 0;
  unsigned long	planemasks[1];
  char *colorenv;

  _mgx11c->mgx11display = dpy;

  if (mgx11display == dpy)
    return;

  mgx11display = dpy;

  if (_mgx11c->bitdepth == 1)  /* We are on a black and white screen! */
    return;

  if ((_mgx11c->bitdepth == 24) || (_mgx11c->bitdepth == 16))
    /* We are on a TrueColor screen! */
    {
      colorlevels = 0;
      return;
    }

  if((colorenv = getenv("GEOMVIEW_COLORLEVELS")) != NULL) {
    colorlevels = atoi(colorenv);
    if(colorlevels > 6) colorlevels = 6;
    if(colorlevels < 1) colorlevels = 1;
  }

  if (_mgx11c->pix)
    {
      if (_mgx11c->cmapset)
	cmap = _mgx11c->cmap;
      else
	cmap = XCreateColormap(mgx11display, DefaultRootWindow(mgx11display),
			       DefaultVisual(mgx11display,
					     DefaultScreen(mgx11display)),
			       AllocNone);
    }
  else {
    if (!_mgx11c->cmapset)
      cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(mgx11display));
    else
      cmap = _mgx11c->cmap;
  }

  while (colfail && colorlevels > 1)
    {
      if (XAllocColorCells(dpy, cmap, False, planemasks, nplanes, mgx11colors,
			   (unsigned int)(colorlevels*colorlevels*colorlevels+1)))
	colfail = 0;
      else
	colorlevels--;
    }

  if (colfail)
    {
      fprintf(stderr, "MG: Couldn't allocate enough colors. Sorry!");
      exit(0);
    }

  cube = colorlevels * colorlevels * colorlevels;

  for (i = 0; i <= cube; i++)
    mgx11colorcells[i].pixel = mgx11colors[i];

  dithermap(colorlevels, (double)1.0, rgbmap);

  for (i = 0; i < cube; i++)
    {
      mgx11colorcells[i].red = (unsigned short)(rgbmap[i][0] * 256);
      mgx11colorcells[i].green = (unsigned short)(rgbmap[i][1] * 256);
      mgx11colorcells[i].blue = (unsigned short)(rgbmap[i][2] * 256);
      mgx11colorcells[i].flags = DoRed | DoGreen | DoBlue;
    }

  XStoreColors(dpy, cmap, mgx11colorcells, cube + 1);

  for (i=0; i<256; i++)
    mgx11multab[i] = colorlevels*i;
}

/*
  Function: Xmg_initx11device
  Description: allocate space for the display list, initialize mgcontext
  Author: Daeron Meyer
*/
int Xmg_initx11device()
{

  if (!mgx11sort)
    {
      mgx11sort = (mgx11_sort *)malloc(sizeof(mgx11_sort));
      mgx11sort->primnum = 1000;
      VVINIT(mgx11sort->primsort, int, mgx11sort->primnum);
      vvneeds(&(mgx11sort->primsort), mgx11sort->primnum);
      VVINIT(mgx11sort->prims, mgx11prim, mgx11sort->primnum);
      vvneeds(&(mgx11sort->prims), mgx11sort->primnum);

      mgx11sort->pvertnum = 2024;
      VVINIT(mgx11sort->pverts, CPoint3, mgx11sort->pvertnum);
      vvneeds(&(mgx11sort->pverts), mgx11sort->pvertnum);

    }
  _mgx11c->mysort = mgx11sort;
  _mgx11c->myxwin = NULL;
  _mgx11c->bitdepth = 0;
  _mgx11c->visual = NULL;
  _mgx11c->noclear = 0;
  return 1;

}

/*
  Function: Xmg_openwin
  Description: open a window if we haven't been given one to draw into
  Author: Daeron Meyer
*/
int Xmg_openwin(char *id)
{
  mgx11win *current;
  Display  *dpy = 0;
  unsigned int      border_width = 0;
  int	   bitmap_pad = 0;
  XErrorHandler handler;

  if (!mgx11display)
    dpy = XOpenDisplay(NULL);

  /* If we are missing any of these values then allocate them all again. */
  /* This protects us from a user who only partially specifies his/her 
     desired X visual/colormap setup to us. */
  if (!_mgx11c->visual || !_mgx11c->bitdepth || !_mgx11c->cmapset)
    {
      int result = mgx11_getvisual(dpy, &(_mgx11c->visual),
				   &(_mgx11c->cmap), &(_mgx11c->bitdepth));
      if (result == MG_X11VISFAIL)
	{
	  fprintf(stderr, "MG: Couldn't find a 1, 8, 16 or 24 bit visual. Sorry!\n");
	  exit(0);
	}

      _mgx11c->cmapset = 1; /* We should have a colormap now */

      if (!mgx11display)
	Xmg_setx11display(dpy);
    }
  
  if (_mgx11c->myxwin == NULL)
    _mgx11c->myxwin = (mgx11win *)malloc(sizeof(mgx11win));

  current = _mgx11c->myxwin;
  current->xswa.colormap = _mgx11c->cmap;
  current->xswa.background_pixel = None;
  current->xswa.background_pixmap = None;
  current->xswa.backing_planes = 0;
  current->xswa.backing_pixel = None;
  current->window = XCreateWindow(mgx11display,
				  XRootWindow(mgx11display, XDefaultScreen(mgx11display)),
				  0, 0, TWIDTH, THEIGHT, border_width,
				  _mgx11c->bitdepth, InputOutput,
				  _mgx11c->visual,
				  CWColormap|CWBorderPixel|CWBackPixel|CWBackPixmap,
				  &current->xswa);


  XStoreName(_mgx11c->mgx11display, current->window, id);
  

  current->gc = XCreateGC(_mgx11c->mgx11display, current->window, 0, NULL);
  /*
    bg_color = BlackPixel(_mgx11c->mgx11display,
    DefaultScreen(_mgx11c->mgx11display));
    XSetBackground(_mgx11c->mgx11display, current->gc, bg_color);
  */
  XMapWindow(_mgx11c->mgx11display, current->window);
  XClearWindow(_mgx11c->mgx11display, current->window);

  current->image = NULL;
  _mgx11c->shm = 0;  

#ifndef NO_SHM
  if (XShmQueryExtension(_mgx11c->mgx11display) == True)
    {
      current->image = 
	XShmCreateImage(_mgx11c->mgx11display, _mgx11c->visual, 
			_mgx11c->bitdepth, ZPixmap, NULL, &(current->shminf),
			TWIDTH, THEIGHT);
    }

  if (current->image != NULL)
    {
      _mgx11c->shm = 1;
      current->shminf.shmid = 
	shmget(IPC_PRIVATE, current->image->bytes_per_line *
	       current->image->height, IPC_CREAT|0777);
      current->buf =
	shmat(current->shminf.shmid, NULL, 0);
      current->shminf.shmaddr = current->image->data = (char *)current->buf;
      current->shminf.readOnly = True;
      globalXError = 0;
      handler = XSetErrorHandler(myXErrorHandler);
      XShmAttach(_mgx11c->mgx11display, &(current->shminf));
      XSync(_mgx11c->mgx11display, False);
      XSetErrorHandler(handler);
      shmctl(current->shminf.shmid, IPC_RMID, 0);
      if (globalXError == 1)
	{
	  _mgx11c->shm = 0;
	  shmdt(current->shminf.shmaddr);
	}
    }
#endif /*has SHM*/

  if (_mgx11c->shm == 0)
    {
      if (!shm_message_shown)
	{
	  fprintf(stderr, "Shared memory unavailable, using fallback display method.\n");
	  shm_message_shown = 1;
	}

      switch (_mgx11c->bitdepth)
	{
	case 1:
	case 8: bitmap_pad = 8; break;
	case 16: bitmap_pad = 16; break;
	case 24: bitmap_pad = 32; break;
	default: fprintf(stderr, "Unknown bit depth %d\n", _mgx11c->bitdepth);
	}
      current->image =
	XCreateImage(_mgx11c->mgx11display, _mgx11c->visual,
		     _mgx11c->bitdepth, ZPixmap, 0, NULL, TWIDTH, THEIGHT,
		     bitmap_pad, 0);
      current->buf = 
	(unsigned char *)malloc(current->image->bytes_per_line * current->image->height);
      current->image->data = (char *) current->buf;
    }

  current->width = current->image->bytes_per_line;
  current->height = current->image->height;
  current->zwidth = TWIDTH;
  if (current->width*current->height > mgx11zsize)
    {
      mgx11zsize = current->width*current->height;
      if (!mgx11zbuffer)
        mgx11zbuffer = (float *) malloc(sizeof(float)*mgx11zsize);
      else
        mgx11zbuffer = (float *) realloc((void *)mgx11zbuffer,
					 sizeof(float)*mgx11zsize);
    }
  _mgx11c->sortmethod = MG_ZBUFFER;
  _mgx11c->dither = 1;
  _mgx11c->myxwin = current;
  return 1;
}

/*
  Function: Xmg_setparent
  Description: set parent window for current context
  Author: Daeron Meyer
*/
void Xmg_setparent(Window win)
{
  _mgx11c->myxwin->parent = win;
}

/*
  Function: Xmg_setwin
  Description: set window for current context and get colormap if its a pix
  Author: Daeron Meyer
*/
void Xmg_setwin(Window win)
{
  mgx11win *current;
  int toss;
  int bg_color = BlackPixel(_mgx11c->mgx11display,
			    DefaultScreen(_mgx11c->mgx11display));

  if (_mgx11c->myxwin == NULL)
    _mgx11c->myxwin = (mgx11win *)malloc(sizeof(mgx11win));

  _mgx11c->visible = 1;
  _mgx11c->myxwin->window = win;
  current = _mgx11c->myxwin;
  /*
    if (!_mgx11c->pix)
    {
    current->xswa.colormap = DefaultColormap(_mgx11c->mgx11display,
    DefaultScreen(_mgx11c->mgx11display));
    current->xswa.backing_store = Always;
    current->xswa.bit_gravity = CenterGravity;
    XChangeWindowAttributes(_mgx11c->mgx11display, current->window,
    CWBitGravity, &(*current).xswa);
    }
  */
  current->gc = XCreateGC(_mgx11c->mgx11display, win, 0, NULL);
  current->image = NULL;
  Xmg_getwinsize(&toss, &toss, &toss, &toss);
  XSetForeground(_mgx11c->mgx11display, current->gc,
		 WhitePixel(_mgx11c->mgx11display,
			    DefaultScreen(_mgx11c->mgx11display)));
  XSetBackground(_mgx11c->mgx11display, current->gc, bg_color);
  if (!_mgx11c->pix)
    XClearWindow(_mgx11c->mgx11display, current->window);
  _mgx11c->sortmethod = MG_ZBUFFER;
  _mgx11c->dither = 1;
  _mgx11c->noclear = 0;
}

/*
  Function: Xmg_poswin
  Description: currently nonfunctional... will eventually call callback
  Author: Daeron Meyer
*/
void Xmg_poswin(int x1, int y1, int x2, int y2)
{
}

/*
  Function: Xmg_prefposwin
  Description: currently nonfunctional... will eventually call callback
  Author: Daeron Meyer
*/
void Xmg_prefposwin(int x1, int y1, int x2, int y2)
{
}

/*
  Function: Xmg_prefposwin
  Description: resize window if its size isn't locked
  Author: Daeron Meyer
*/
void Xmg_sizewin(int x, int y)
{
  if (_mgx11c->sizelock)
    return;

  if (!_mgx11c->pix)
    XResizeWindow(_mgx11c->mgx11display, _mgx11c->myxwin->window,
		  (unsigned int) x, (unsigned int) y);
}

/*
  Function: Xmg_minsize
  Description: nonfunctional
  Author: Daeron Meyer
*/
void Xmg_minsize(int x, int y)
{
}

/* ditto */
void Xmg_winconstrain()
{
}

/* ditto */
void Xmg_titlewin(char *name)
{
}

/*
  Function: Xmg_closewin
  Description: close window and free shared memory
  Author: Daeron Meyer, Tim Rowley
*/
void Xmg_closewin(mgx11win *this)
{
  Display *dpy = _mgx11c->mgx11display;

#ifndef NO_SHM
  if (_mgx11c->shm)
    {
      XShmDetach(dpy, &(this->shminf));
      shmdt(this->shminf.shmaddr);
    }
#endif /*has SHM*/

  XDestroyImage(this->image);
}

/*
  Function: Xmg_flush
  Description: flush connection to X
  Author: Daeron Meyer
*/
void Xmg_flush()
{
  XFlush(_mgx11c->mgx11display);
}

/*
  Function: Xmg_newdisplaylist
  Description: initialize display list
  Author: Daeron Meyer
*/
void Xmg_newdisplaylist()
{

  _mgx11c->mysort->cprim = 0;
  _mgx11c->mysort->cvert = 0;
  _mgx11c->mysort->maxverts = 0;
  _mgx11c->znudgeby = 0.0;
  _mgx11c->oxmin = _mgx11c->xmin; _mgx11c->oymin = _mgx11c->ymin;
  _mgx11c->oxmax = _mgx11c->xmax; _mgx11c->oymax = _mgx11c->ymax;
  _mgx11c->xmin = _mgx11c->ymin = INT_MAX;
  _mgx11c->xmax = _mgx11c->ymax = INT_MIN;
}

/*
  Function: Xmg_add
  Description: add a primitive (polygon, vertex, line) to the display list
  Author: Daeron Meyer
*/
void Xmg_add(int primtype, int numdata, void *data, void *cdata)
{
  HPoint3 *vt = (HPoint3 *)data;
  ColorA *colarray = (ColorA *)cdata;
  float *col = (float *)cdata;
  CPoint3 *vts;
  int i;

  static mgx11prim *prim;
  static ColorA color;
  static float average_depth;
  static int numverts, ecolor[3];
  static int maxlinewidth = 0;

#if 1 || HACK_THE_CODE_BUT_BETTER_FIX_IT
  if (!(_mgc->has & HAS_S2O)) {
    Transform S;
    WnPosition vp;

    mg_findS2O();
    mg_findO2S();

    WnGet(_mgc->win, WN_VIEWPORT, &vp);
    /* Fix up O2S and S2O matrices.  Since the X11
     * coordinate system has Y increasing downward, flip it
     * here, and translate by location of lower left corner
     * of viewport.
     */
    TmTranslate(S, (double)vp.xmin, (double)vp.ymax, 0.);
    S[1][1] = -1;		/* Invert sign of Y */
    TmConcat(_mgc->O2S, S, _mgc->O2S);
    TmInvert(_mgc->O2S, _mgc->S2O);
  }
#endif
  
  switch (primtype)
    {
    case MGX_BGNLINE:
    case MGX_BGNSLINE:
      average_depth = 0.0;
      prim =
	&(VVEC(_mgx11c->mysort->prims, mgx11prim)[_mgx11c->mysort->cprim]);

      if (primtype == MGX_BGNLINE)
	prim->mykind = PRIM_LINE;
      else
	prim->mykind = PRIM_SLINE;

      prim->index = _mgx11c->mysort->cvert;
      prim->depth = -100000; /* very far behind the viewer */
      numverts = 0;

      prim->ecolor[0] = ecolor[0];
      prim->ecolor[1] = ecolor[1];
      prim->ecolor[2] = ecolor[2];
      prim->ewidth = curwidth;
      if (curwidth > maxlinewidth)
	maxlinewidth = curwidth;

      VVEC(_mgx11c->mysort->primsort, int)[_mgx11c->mysort->cprim] =
	_mgx11c->mysort->cprim;

      if (!(_mgc->has & HAS_S2O)) {
	mg_findS2O();
	mg_findO2S();
      }
      break;

    case MGX_BGNEPOLY:
    case MGX_BGNSEPOLY:
      if (curwidth > maxlinewidth)
	maxlinewidth = curwidth;
    case MGX_BGNPOLY:
    case MGX_BGNSPOLY:
      average_depth = 0.0;
      prim = &(VVEC(_mgx11c->mysort->prims, mgx11prim)
	       [_mgx11c->mysort->cprim]);

      switch(primtype)
	{
	case MGX_BGNPOLY:
	  prim->mykind = PRIM_POLYGON;
	  break;
	case MGX_BGNSPOLY:
	  prim->mykind = PRIM_SPOLYGON;
	  break;
	case MGX_BGNEPOLY:
	  prim->mykind = PRIM_EPOLYGON;
	  break;
	case MGX_BGNSEPOLY:
	  prim->mykind = PRIM_SEPOLYGON;
	  break;
	}

      prim->ewidth = curwidth;
      prim->index = _mgx11c->mysort->cvert;
      prim->depth = -100000; /* very far behind the viewer */
      numverts = 0;
	  
      VVEC(_mgx11c->mysort->primsort, int)[_mgx11c->mysort->cprim] =
	_mgx11c->mysort->cprim;

      if (!(_mgc->has & HAS_S2O)) {
	mg_findS2O();
	mg_findO2S();
      }
      break;

    case MGX_VERTEX:
      for (i=0; i<numdata; i++)
	{
	  vts = &(VVEC(_mgx11c->mysort->pverts, CPoint3)[_mgx11c->mysort->cvert]);
	  HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	  vts->drawnext = 1;

	  vts->vcol = color;
	  _mgx11c->mysort->cvert++; numverts++;

	  if (_mgx11c->mysort->cvert > _mgx11c->mysort->pvertnum)
	    {
	      _mgx11c->mysort->pvertnum*=2;
	      vvneeds(&(_mgx11c->mysort->pverts), _mgx11c->mysort->pvertnum);
	    }

	  if (vts->z > prim->depth)
	    {
	      prim->depth = vts->z;
	    }

	  average_depth += vts->z;

	}
      break;

    case MGX_CVERTEX:
      for (i=0; i<numdata; i++)
	{
	  vts = &(VVEC(_mgx11c->mysort->pverts, CPoint3)[_mgx11c->mysort->cvert]);
	  HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	  vts->drawnext = 1;

	  vts->vcol = colarray[i];

	  _mgx11c->mysort->cvert++;
	  numverts++;
	  if (_mgx11c->mysort->cvert > _mgx11c->mysort->pvertnum)
	    {
	      _mgx11c->mysort->pvertnum*=2;
	      vvneeds(&(_mgx11c->mysort->pverts), _mgx11c->mysort->pvertnum);
	    }

	  if (vts->z > prim->depth)
	    {
	      prim->depth = vts->z;
	    }

	  average_depth += vts->z;

	}
      break;



    case MGX_COLOR:
      color = colarray[0];
      break;

    case MGX_ECOLOR:
      ecolor[0] = (int)(255.0 * col[0]);
      ecolor[1] = (int)(255.0 * col[1]);
      ecolor[2] = (int)(255.0 * col[2]);
      break;

    case MGX_END:
      prim->numvts = numverts;
      if (numverts > _mgx11c->mysort->maxverts)
	_mgx11c->mysort->maxverts = numverts;
      average_depth += prim->depth;
      average_depth /= (float)(numverts+1);
      prim->depth = average_depth;

      prim->color[0] = (int)(255.0 * color.r);
      prim->color[1] = (int)(255.0 * color.g);
      prim->color[2] = (int)(255.0 * color.b);

      prim->ecolor[0] = ecolor[0];
      prim->ecolor[1] = ecolor[1];
      prim->ecolor[2] = ecolor[2];

      if ((prim->mykind = Xmg_primclip(prim)) == PRIM_INVIS)
	_mgx11c->mysort->cvert = prim->index;
      else
	{
	  _mgx11c->mysort->cvert = prim->index + prim->numvts;
	  _mgx11c->mysort->cprim++;
	}

      if (_mgx11c->mysort->cprim > _mgx11c->mysort->primnum)
	{
	  _mgx11c->mysort->primnum*=2;
	  vvneeds(&(_mgx11c->mysort->prims), _mgx11c->mysort->primnum);
	  vvneeds(&(_mgx11c->mysort->primsort), _mgx11c->mysort->primnum);
	}

      _mgx11c->xmax += maxlinewidth;
      _mgx11c->xmin -= maxlinewidth;
      _mgx11c->ymax += maxlinewidth;
      _mgx11c->ymin -= maxlinewidth;
      maxlinewidth = 0;
      break;

    default:
      fprintf(stderr,"unknown type of primitive.\n");
      break;
    }
}

/*
  Function: Xmg_primcomp
  Description: Depth sort by comparing two primitives in a call from qsort()
  (painters algorithm, no subdiv of polygons)
  Author: Daeron Meyer
*/
int Xmg_primcomp(const void *a, const void *b)
{
  mgx11prim *prim =
    VVEC(_mgx11c->mysort->prims, mgx11prim);

  if (prim[*(int*)a].depth < prim[*(int*)b].depth)
    return 1;
  else
    return -1;

}

/*
  Function: Xmg_sortdisplaylist
  Description: Does depth sorting of primitives.
  Author: Daeron Meyer
*/
void Xmg_sortdisplaylist()
{
  static int *primp;

  if (_mgx11c->sortmethod == MG_DEPTH)
    {
      primp = VVEC(_mgx11c->mysort->primsort, int);
      qsort(primp, _mgx11c->mysort->cprim, sizeof(int), &Xmg_primcomp);
    }
}

static unsigned int byterev(unsigned int v) {
  return (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
}

/*
  Function: Xmg_showdisplaylist
  Description: render display list to display
  Author: Daeron Meyer, Tim Rowley
*/
void Xmg_showdisplaylist()
{
  CPoint3 *vts;
  int ref, *primp;
  mgx11prim *prim, *prim2;
  Display *dpy = _mgx11c->mgx11display;
  Drawable win = _mgx11c->myxwin->window;
  unsigned char *buf = _mgx11c->myxwin->buf;
  float *zbuf = mgx11zbuffer;
  int w = _mgx11c->myxwin->width;
  int h = _mgx11c->myxwin->height;
  int zwidth = _mgx11c->myxwin->zwidth;
  GC gc = _mgx11c->myxwin->gc;
  static int width;
  static int height;
  void (*poly)(unsigned char *, float *, int, int, int, CPoint3 *, int, int *);
  void (*line)(unsigned char *, float *, int, int, int, 
	       CPoint3 *, CPoint3 *, int, int *);
  void (*polyline)(unsigned char *, float *, int, int, int, CPoint3 *, 
		   int, int, int *);
  void (*spolyline)(unsigned char *, float *, int, int, int, CPoint3 *, 
		    int, int, int *);
  void (*spoly)(unsigned char *, float *, int, int, int, CPoint3 *, 
		int, int *);
  void (*clear)(unsigned char *, float *, int, int, int, int *, int,
		int, int, int, int, int);
  int color[3];
  int xmin, ymin, xmax, ymax;
  int wantedzsize;
  int rmask = _mgx11c->visual->red_mask;
  int gmask = _mgx11c->visual->green_mask;
  int bmask = _mgx11c->visual->blue_mask;
  static int lsb = 1;

  /* Compare our native byte order to that of the server.
   * Must we byte-swap images?
   */
  if((*(char *)&lsb != 0) != (XImageByteOrder(_mgx11c->mgx11display) == LSBFirst)) {
    rmask = byterev(rmask);
    gmask = byterev(gmask);
    bmask = byterev(bmask);
  }
  
  /* Choose functions */

  if (_mgx11c->bitdepth == 8)
    {
      clear = Xmgr_8clear;
      if (_mgx11c->sortmethod == MG_ZBUFFER)
	{
	  if (_mgx11c->dither)
	    {
	      poly = Xmgr_8DZpoly;
	      line = Xmgr_8DZline;
	      polyline = Xmgr_8DZpolyline;
	      spolyline = Xmgr_8DGZpolyline;
	      spoly = Xmgr_8DGZpoly;
	    }
	  else
	    {
	      poly = Xmgr_8Zpoly;
	      line = Xmgr_8Zline;
	      spolyline = polyline = Xmgr_8Zpolyline;
	      spoly = Xmgr_8Zpoly;
	    }
	}
      else
	{
	  if (_mgx11c->dither)
	    {
	      poly = Xmgr_8Dpoly;
	      line = Xmgr_8Dline;
	      polyline = Xmgr_8Dpolyline;
	      spolyline = Xmgr_8DGpolyline;
	      spoly = Xmgr_8DGpoly;
	    }
	  else
	    {
	      poly = Xmgr_8poly;
	      line = Xmgr_8line;
	      spolyline = polyline = Xmgr_8polyline;
	      spoly = Xmgr_8poly;
	    }
	}
    }
  else if (_mgx11c->bitdepth == 24)
    {
      Xmgr_24fullinit(rmask, gmask, bmask);
      clear = Xmgr_24clear;
      if (_mgx11c->sortmethod == MG_ZBUFFER)
	{
	  poly = Xmgr_24Zpoly;
	  line = Xmgr_24Zline;
	  polyline = Xmgr_24Zpolyline;
	  spolyline = Xmgr_24GZpolyline;
	  spoly = Xmgr_24GZpoly;
	}
      else
	{
	  poly = Xmgr_24poly;
	  line = Xmgr_24line;
	  polyline = Xmgr_24polyline;
	  spolyline = Xmgr_24Gpolyline;
	  spoly = Xmgr_24Gpoly;
	}
    }
  else if (_mgx11c->bitdepth == 16)
    {
      Xmgr_16fullinit(rmask, gmask, bmask);
      clear = Xmgr_16clear;
      if (_mgx11c->sortmethod == MG_ZBUFFER)
	{
	  poly = Xmgr_16Zpoly;
	  line = Xmgr_16Zline;
	  polyline = Xmgr_16Zpolyline;
	  spolyline = Xmgr_16GZpolyline;
	  spoly = Xmgr_16GZpoly;
	}
      else
	{
	  poly = Xmgr_16poly;
	  line = Xmgr_16line;
	  polyline = Xmgr_16polyline;
	  spolyline = Xmgr_16Gpolyline;
	  spoly = Xmgr_16Gpoly;
	}
    }
  else if (_mgx11c->bitdepth == 1)
    {
      Xmgr_1init(BlackPixel(dpy, DefaultScreen(dpy)));
      clear = Xmgr_1clear;
      if (_mgx11c->sortmethod == MG_ZBUFFER)
	{
	  poly = Xmgr_1DZpoly;
	  line = Xmgr_1DZline;
	  polyline = Xmgr_1DZpolyline;
	  spolyline = Xmgr_1DGZpolyline;
	  spoly = Xmgr_1DGZpoly;
	}
      else
	{
	  poly = Xmgr_1Dpoly;
	  line = Xmgr_1Dline;
	  polyline = Xmgr_1Dpolyline;
	  spolyline = Xmgr_1DGpolyline;
	  spoly = Xmgr_1DGpoly;
	}
    }
  else
    {
      fprintf(stderr, "X11(Function Select): Unsupported bit depth %d\n", 
	      _mgx11c->bitdepth);
      return;
    }
  
  if (_mgx11c->sortmethod == MG_ZBUFFER)
    {
      wantedzsize = zwidth*h;

      if (wantedzsize > mgx11zsize)
	{
	  if (!mgx11zbuffer)
	    zbuf = mgx11zbuffer = (float *)malloc(sizeof(float)*wantedzsize);
	  else
	    zbuf = mgx11zbuffer = (float *)realloc((void *)mgx11zbuffer, 
						   sizeof(float)*wantedzsize);
	  mgx11zsize = wantedzsize;
	}
    }

  WnGet(_mgc->win, WN_XSIZE, &width);
  WnGet(_mgc->win, WN_YSIZE, &height);

  /* Get dirty rectangle */

  if (_mgx11c->xmin < _mgx11c->oxmin)
    xmin = MIN(width-1,MAX(_mgx11c->xmin,0));
  else
    xmin = MIN(width-1,MAX(_mgx11c->oxmin,0));
  if (_mgx11c->ymin < _mgx11c->oymin)
    ymin = MIN(height-1,MAX(_mgx11c->ymin,0));
  else
    ymin = MIN(height-1,MAX(_mgx11c->oymin,0));
  if (_mgx11c->xmax > _mgx11c->oxmax)
    xmax = MAX(0,MIN(_mgx11c->xmax,width-1));
  else
    xmax = MAX(0,MIN(_mgx11c->oxmax,width-1));
  if (_mgx11c->ymax > _mgx11c->oymax)
    ymax = MAX(0,MIN(_mgx11c->ymax,height-1));
  else
    ymax = MAX(0,MIN(_mgx11c->oymax,height-1));

  if (_mgx11c->exposed)
    {
      _mgx11c->xmin = _mgx11c->ymin = xmin = ymin = 0;
      _mgx11c->xmax = xmax = width-1;
      _mgx11c->ymax = ymax = height-1;
      _mgx11c->exposed = 0;
    }
  if ((_mgc->opts & MGO_INHIBITSWAP) || _mgx11c->noclear)
    {
      _mgx11c->xmin = _mgx11c->ymin = xmin = ymin = 0;
      _mgx11c->xmax = xmax = width-1;
      _mgx11c->ymax = ymax = height-1;
    }

  if (xmin > xmax || ymin > ymax) {
    /* nothing to be done, just bail out */
    return;
  }

  color[0] = _mgc->background.r*255.0;
  color[1] = _mgc->background.g*255.0;
  color[2] = _mgc->background.b*255.0;

  if (!_mgx11c->noclear)
    clear(buf, zbuf, zwidth, w, h, color, (_mgx11c->sortmethod==MG_ZBUFFER),
	  0, xmin, ymin, xmax, ymax);
  else
    _mgx11c->noclear = 0;

  primp = VVEC(_mgx11c->mysort->primsort, int);
  prim2 = VVEC(_mgx11c->mysort->prims, mgx11prim);
  vts = VVEC(_mgx11c->mysort->pverts, CPoint3);

  for (ref = 0; ref < _mgx11c->mysort->cprim; ref++)
    {
      prim = &(prim2[primp[ref]]);
      switch (prim->mykind)
	{
	case PRIM_POLYGON:
	  poly(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
	       prim->color);
	  break;

	case PRIM_SPOLYGON:
	  spoly(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
		prim->color);
	  break;

	case PRIM_EPOLYGON:
	  poly(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
	       prim->color);
	  polyline(buf, zbuf, zwidth, w, h, vts+prim->index,
		   prim->numvts, prim->ewidth, prim->ecolor);
	  if (vts[prim->index+prim->numvts-1].drawnext)
	    line(buf, zbuf, zwidth, w, h, &vts[prim->index+prim->numvts-1], 
		 &vts[prim->index], prim->ewidth, prim->ecolor);
	  break;

	case PRIM_SEPOLYGON:
	  spoly(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
		prim->color);
	  polyline(buf, zbuf, zwidth, w, h, vts+prim->index,
		   prim->numvts, prim->ewidth, prim->ecolor);
	  if (vts[prim->index+prim->numvts-1].drawnext)
	    line(buf, zbuf, zwidth, w, h, &vts[prim->index+prim->numvts-1],
		 &vts[prim->index], prim->ewidth, prim->ecolor);
	  break;

	case PRIM_SLINE:
	  spolyline(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
		    prim->ewidth, prim->ecolor);
	  break;
	case PRIM_LINE:
	  polyline(buf, zbuf, zwidth, w, h, vts+prim->index, prim->numvts,
		   prim->ewidth, prim->ecolor);
	  break;

	case PRIM_INVIS:
	  break;

	default:
	  break;
	}
    }

  if (!(_mgc->opts & MGO_INHIBITSWAP)) {
    if (_mgx11c->shm)
      {
	XShmPutImage(dpy, win, gc, _mgx11c->myxwin->image,
		     xmin, ymin, xmin, ymin,
		     xmax-xmin+1, ymax-ymin+1, False);
      }
    else
      {
	XPutImage(dpy, win, gc, _mgx11c->myxwin->image, xmin, ymin, xmin, ymin,
		  xmax-xmin+1, ymax-ymin+1);
      }
  } else {
    _mgx11c->noclear = 1;
  }
  Xmg_flush();
}

/*
  Function: Xmg_getwinsize
  Description: get the new size of the window and reorganize shared memory
  Author: Daeron Meyer, Tim Rowley
*/
void Xmg_getwinsize(int *xsize, int *ysize, int *xorig, int *yorig)
{
  Display *dpy = _mgx11c->mgx11display;
  Window dpyroot;
  Window Toss;
  Drawable win;
  unsigned int width, height, border_width, depth;
  int xpos, ypos, xold, yold;
  int bytes_per_line = 0;
  mgx11win *current=_mgx11c->myxwin;
  XErrorHandler handler;
  int bitmap_pad = 0;

  if(current == NULL)
    return;
  win = current->window;

  /*  fprintf(stderr,"X11: Get Window Size\n"); */
  if (_mgx11c->visible)
    {
      XGetGeometry(dpy, win, &dpyroot, &xpos, &ypos, &width,
		   &height, &border_width, &depth);
      *xsize = width;
      *ysize = height;

      if (_mgx11c->pix)
	{
	  *xorig = 0; *yorig = 0;
	}
      else if (XTranslateCoordinates(dpy, win, dpyroot, 0, height-1,
				     &xpos, &ypos, &Toss))
	{
	  *xorig=xpos; *yorig=HeightOfScreen(DefaultScreenOfDisplay(dpy)) - ypos;
	}
      else
	{
	  *xorig = 0; *yorig = 0;
	}
    }
  else
    {
      *xsize = 0; *ysize = 0;
    }
  WnGet(_mgc->win, WN_XSIZE, &xold);
  WnGet(_mgc->win, WN_YSIZE, &yold);
  if (_mgx11c->bitdepth == 0)
    return;
  if ((xold != (int)width) || (yold != (int)height) ||
      (_mgx11c->myxwin->image == NULL))
    {
      if (_mgx11c->myxwin->image != NULL)
	{
#ifndef NO_SHM
	  if (_mgx11c->shm)
	    {
	      XShmDetach(dpy, &(_mgx11c->myxwin->shminf));
	      shmdt(_mgx11c->myxwin->shminf.shmaddr);
	    }
#endif /*has SHM*/
	  XDestroyImage(_mgx11c->myxwin->image);
	}

      current->image = NULL;
      _mgx11c->shm = 0;
#ifndef NO_SHM
      if (XShmQueryExtension(_mgx11c->mgx11display) == True)
	{
	  current->image = 
	    XShmCreateImage(_mgx11c->mgx11display, _mgx11c->visual, 
			    _mgx11c->bitdepth,
			    ZPixmap, NULL, &(current->shminf), width, height);
	  bytes_per_line = current->image->bytes_per_line;
	}

      if (current->image != NULL)
	{
	  _mgx11c->shm = 1;
	  /*
	    fprintf(stderr, "X11: shm creating image with depth = %d visual = %p bit_pre_pixel=%d\n",
	    _mgx11c->bitdepth, _mgx11c->visual, current->image->bits_per_pixel);
	  */
	  current->shminf.shmid = 
	    shmget(IPC_PRIVATE, bytes_per_line * height, IPC_CREAT|0777);
	  current->buf =
	    shmat(current->shminf.shmid, NULL, 0);
	  current->shminf.shmaddr = current->image->data = (char *) current->buf;
	  current->shminf.readOnly = True;
	  globalXError = 0;
	  handler = XSetErrorHandler(myXErrorHandler);
	  XShmAttach(_mgx11c->mgx11display, &(current->shminf));
	  XSync(_mgx11c->mgx11display, False);
	  XSetErrorHandler(handler);
	  shmctl(current->shminf.shmid, IPC_RMID, 0);
	  if (globalXError == 1)
	    {
	      _mgx11c->shm = 0;
	      shmdt(current->shminf.shmaddr);
	    }
	}
#endif /*has SHM*/

      if (_mgx11c->shm == 0)
	{
	  if (!shm_message_shown)
	    {
	      fprintf(stderr, "Shared memory unavailable, using fallback display method.\n");
	      shm_message_shown = 1;
	    }

	  switch (_mgx11c->bitdepth)
	    {
	    case 1:
	    case 8: bitmap_pad = 8; break;
	    case 16: bitmap_pad = 16; break;
	    case 24: bitmap_pad = 32; break;
	    default: fprintf(stderr, "Unknown bit depth %d\n", _mgx11c->bitdepth);
	    }
	  current->image =
	    XCreateImage(_mgx11c->mgx11display, _mgx11c->visual,
			 _mgx11c->bitdepth, ZPixmap, 0, NULL, width, height,
			 bitmap_pad, 0);
	  if((bytes_per_line = current->image->bytes_per_line) == 0) {
	    int bpp = _mgx11c->bitdepth == 24 ? 32 : _mgx11c->bitdepth;
	    bytes_per_line = ((bpp * width + 31) >> 5) << 2;
	  }
	  current->buf = 
  	    (unsigned char *) malloc(bytes_per_line * height);
	  current->image->data = (char *)current->buf;
	}
	
      current->width = bytes_per_line;
      current->height = height;
      _mgx11c->myxwin->zwidth = width;
      _mgx11c->exposed = 1;
    }
}

/*
  Function: mgx11_nearestRGB
  Description: dither RGB pixel at x,y to color index from our colormap
  Author: Daeron Meyer
*/
unsigned long mgx11_nearestRGB(int x, int y, int *rgb)
{
  if (!colorlevels) /* handle TrueColor case */
    return (unsigned long)0;

  return dithergb(x, y, rgb, colorlevels);
}

/*
  Function: mgx11_RGB
  Description: get index of color nearest to RGB value, in colormap
  Author: Daeron Meyer
*/
unsigned long mgx11_RGB(int r, int g, int b)
{
  int col[3];

  if (!colorlevels) /* handle TrueColor case */
    return (unsigned long)0;

  col[0] = r; col[1] = g; col[2] = b;
  return dithergb(0, 0, col, colorlevels);
}

/*
  Function: mgx11_setRGB
  Description: use reserved colorcell for smooth dynamic color changing.
  For instance, this function is relied on by the
  Color Panel in Geomview.
  Author: Daeron Meyer
*/
unsigned long mgx11_setRGB(int r, int g, int b)
{
  int cell = colorlevels * colorlevels * colorlevels;

  if (!colorlevels) /* handle TrueColor case */
    return (unsigned long)0;

  mgx11colorcells[cell].red = (unsigned short)(r * 256);
  mgx11colorcells[cell].green = (unsigned short)(g * 256);
  mgx11colorcells[cell].blue = (unsigned short)(b * 256);
  mgx11colorcells[cell].flags = DoRed | DoGreen | DoBlue;
  XStoreColor(mgx11display, cmap, &(mgx11colorcells[cell]));

  return mgx11colors[cell];
}

/*
  Function: mgx11_linewidth
  Description: set current linewidth
  Author: Daeron Meyer
*/
void mgx11_linewidth(short width)
{
  curwidth = (int) width;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
