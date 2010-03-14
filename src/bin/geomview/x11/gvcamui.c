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

#if HAVE_CONFIG_
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "mibload.h"
#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/worldio.h"
#include "../common/transform.h"
#include "../common/main.h"
#include "mibwidgets.h"
#include "gvui.h"
#include "windowP.h"
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h> /* for XtConfigureWidget() */

#if HAVE_XMU
#  include <X11/Xmu/StdCmap.h> /* XmuLookupStandardColormap */
#endif

/*
 * All compile-time dependencies on graphics type (X11/GL/OPENGL)
 * reside in this file.
 */

/* Don't worry about that #error stuff, configure makes sure that at
 * least MGX11 is defined (look in config.h)
 */
#if !MGGL && !MGX11 && !MGOPENGL
# error Need at least one graphics type (X11/OpenGL/GL) to render with!
#endif

/* Define the global option variabl `gv_no_opengl' to be always
 * logical true if we have no OpenGL available. This simplifies some
 * if-cases in this file which otherwise would have to be wrapped into
 * some more preprocessor `#if'-constructs.
 */
#if !MGGL && !MGOPENGL
# define gv_no_opengl 1
#endif

#include "mg.h"

#if MGX11
# include "mgx11.h"
#endif

#if MGGL
# include "mggl.h"
# include <gl/gl.h>
# include <X11/Xirisw/GlxMDraw.h>
#endif

#if MGOPENGL
# include "mgopengl.h"
# define GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glx.h>
# ifdef MESA
#  define OPENGL_BUILDINFOGRAPHICS "OpenGL(MESA)"
# else
#  define OPENGL_BUILDINFOGRAPHICS "OpenGL"
# endif /*!MESA*/
#endif

#ifdef OPENGL_BUILDINFOGRAPHICS
# if MGX11
char *buildinfographics = OPENGL_BUILDINFOGRAPHICS"/X11";
# else
char *buildinfographics = OPENGL_BUILDINFOGRAPHICS;
# endif
#else
char *buildinfographics = "X11";
#endif

/* Hack.  We need to associate some other windows with a DrawingArea widget.
 * In X11R6, there's a nice public XtRegisterDrawable() function.
 * In X11R5 and X11R4, there's an unpublished _XtRegisterWindow() instead.
 * So if we're not at least R6, use the latter.
 */
#if (XtSpecificationRelease < 6) || defined(AVOID_X11R6)
# define XtRegister_window(dpy, win, wid)   _XtRegisterWindow(win, wid)
# define XtUnregister_window(dpy, win, wid) _XtUnregisterWindow(win, wid)
#else
# define XtRegister_window(dpy, win, wid)   XtRegisterDrawable(dpy, win, wid)
# define XtUnregister_window(dpy, win, wid) XtUnregisterDrawable(dpy, win)
#endif

extern Display *dpy;
extern UIState  uistate;
extern Pixmap   geomicon;

/* private methods and variables */
/*****************************************************************************/

static void ui_delete_camera(Widget, XtPointer, XmAnyCallbackStruct *);
#if MGOPENGL
static Colormap getcmapfor(XVisualInfo *vi);
#endif

/*****************************************************************************/
enum buffering { SGL=0, DBL=1, BUFTYPES };

static vvec camshells;
struct cursor {
  int id;
  Cursor c;
} cursors[2];
struct camwins {
  Window shellwin;
  Window wins[BUFTYPES];
#if MGOPENGL
  XVisualInfo *vi[BUFTYPES];
#endif
  int curs;
};

#if MGOPENGL
static Colormap oglcmap[2];
#endif

/*****************************************************************************/

void cam_mgdevice()
{
  if (gv_no_opengl) {
#if MGX11
    mgdevice_X11();
#endif
  } else {
#if MGGL
    mgdevice_GL();
#endif

#if MGOPENGL
    mgdevice_OPENGL();
#endif
  }
}

/*****************************************************************************/

static Widget camshellof(int id, struct camwins **cwp)
{
  struct camwins *cw;
  Widget w;
  if (id < 0 || INDEXOF(id) >= VVCOUNT(camshells)) {
    return NULL;
  }
  cw = VVINDEX(camshells, struct camwins, INDEXOF(id));
  if ((w = XtWindowToWidget(dpy, cw->shellwin)) == NULL) {
    return NULL;
  }
  if (cwp) {
    *cwp = cw;
  }
  return w;
}

static void fitwins(Widget w, struct camwins *cw)
{
  /* If we have private windows, may need to resize them. */
  Dimension height = 100, width = 100;
  int n;
  XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
  for(n = SGL; n <= DBL; n++) {
    if (cw->wins[n] != 0)	/* (0 if unset from vvzero(&camwins)) */
      XResizeWindow(dpy, cw->wins[n], width, height);
  }
}

static int setwinargs(ArgList args, int n, WnWindow *win, int changed,
		      int *sizep, char **titlep, Widget w)
{
  WnPosition prefpos;
  int enlarge = 1, shrink = 1;
  int sx = 350, sy = 350, noborder = 0, haspref, hassize;
  static char geometry[30];

  WnGet(win, WN_ENLARGE, &enlarge);
  WnGet(win, WN_SHRINK, &shrink);
  hassize = WnGet(win, WN_XSIZE, &sx);
  hassize &= WnGet(win, WN_YSIZE, &sy);
  WnGet(win, WN_NAME, titlep);
  WnGet(win, WN_NOBORDER, &noborder);
  haspref = WnGet(win, WN_PREFPOS, &prefpos);

  if (changed & WNF_HASNAME) {
    XtSetArg(args[n], XmNtitle, *titlep); n++;
  }

  if ((changed & WNF_HASPREF) && haspref > 0) {
    sx = prefpos.xmax - prefpos.xmin + 1;
    sy = prefpos.ymax - prefpos.ymin + 1;
    sprintf(geometry, "=%dx%d+%d-%d", sx, sy, prefpos.xmin, prefpos.ymin);
    XtSetArg(args[n], XmNgeometry, geometry); n++;
    if (w != NULL)
      XtConfigureWidget(w,
			prefpos.xmin, HeightOfScreen(XtScreen(w))-prefpos.ymax-1,
			sx, sy, 0);
  }
  if ((changed & WNF_HASSIZE) && hassize == 1) {
    XtSetArg(args[n], XmNwidth, sx); n++;
    XtSetArg(args[n], XmNheight, sy); n++;
  }

  if (changed & WNF_NOBORDER) {
    XtSetArg(args[n], XmNmwmDecorations,
	     noborder ? 0 : MWM_DECOR_ALL); n++;
  }
  if (changed & (WNF_ENLARGE|WNF_SHRINK)) {
    XtSetArg(args[n], XmNmwmFunctions,
	     enlarge||shrink ? MWM_FUNC_ALL : (MWM_FUNC_ALL|MWM_FUNC_RESIZE)); n++;
  }

  if (sizep) {
    sizep[0] = sx;
    sizep[1] = sy;
  }
  return n;
}

void cam_winchange(mgcontext *mgc, void *data, int why, ...)
{
  va_list args;
  WnWindow *win;
  int n, changed;
  int id = (int)(long) data;
  struct camwins *cw;
  Widget w;
  char *title = "gvwin";

  if ((w = camshellof(id, &cw)) == NULL)
    return;

  va_start(args, why);

  switch(why) {
  case MGW_WINDELETE:
    ui_delete_camera(w, data, NULL);
    break;

  case MGW_WINCHANGE:
    win = va_arg(args, WnWindow *);
    changed = win->changed;
    if (changed & (WNF_ENLARGE|WNF_SHRINK|WNF_NOBORDER
		  |WNF_HASPREF|WNF_HASSIZE|WNF_HASNAME)) {
      Arg args[20];

      n = setwinargs(args, 0, win, win->changed, NULL, &title, w);
      XtSetValues(w, args, n);


      win->changed &= ~(WNF_ENLARGE|WNF_SHRINK|WNF_NOBORDER
			|WNF_HASPREF|WNF_HASSIZE|WNF_HASNAME);
    }
    if (changed & (WNF_HASCUR|WNF_HASSIZE|WNF_HASPREF))
      fitwins(w, cw);
    break;
  }
  va_end(args);
}

Widget ui_create_camera(Widget parent, DView *dv)
{
  int id = dv->id;
  Arg	 args[20];
  int	 n, size[2];
  char  *title;
  struct camwins *cw;
  Atom   AProtocol;
  Widget shell, camform, camdraw[2] = { NULL, NULL };
  int i;
#if MGOPENGL
  XSetWindowAttributes xswa;
  Window cmw[32];
  int cmwneeded;
#endif

#if MGOPENGL
  struct oglstuff {
    int setGLXwin, setGLXctx;
    int *attribs;
    GLXContext cx;
    Colormap cm;
  };
  static int dblBuf[] = {
    GLX_DOUBLEBUFFER,
    GLX_RGBA, GLX_DEPTH_SIZE, 16,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
  };
  static struct oglstuff ogl[2] = {
    { MG_GLXSINGLEWIN, MG_GLXSINGLECTX, dblBuf+1 },
    { MG_GLXDOUBLEWIN, MG_GLXDOUBLECTX, dblBuf },
  };
  struct oglstuff *oglp;
  GLXContext sharectx = None;
#endif

#if MGGL
  static GLXconfig db_rgb_desc[] = {
    { GLX_NORMAL, GLX_VISUAL, GLX_NONE },
    { GLX_NORMAL, GLX_COLORMAP, GLX_NONE },
    { GLX_NORMAL, GLX_WINDOW, GLX_NONE },
    { GLX_NORMAL, GLX_VISUAL, GLX_NONE },
    { GLX_NORMAL, GLX_DOUBLE, TRUE},
    { GLX_NORMAL, GLX_RGB, TRUE},
    { GLX_NORMAL, GLX_ZSIZE, GLX_NOCONFIG },
    { 0,0,0 }
  };
  static GLXconfig sg_rgb_desc[] = {
    { GLX_NORMAL, GLX_VISUAL, GLX_NONE },
    { GLX_NORMAL, GLX_COLORMAP, GLX_NONE },
    { GLX_NORMAL, GLX_WINDOW, GLX_NONE },
    { GLX_NORMAL, GLX_DOUBLE, FALSE},
    { GLX_NORMAL, GLX_RGB, TRUE},
    { GLX_NORMAL, GLX_ZSIZE, GLX_NOCONFIG },
    { 0,0,0 }
  };
#endif

  /*****************************************************************************/


  /*****************************************************************************/


  n = 0;


  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING);n++;

  n = setwinargs(args, n, dv->win, ~0, size, &title, NULL);

#if MGGL || MGX11
  if (MGGL || gv_no_opengl) { /* ... but not for MGOPENGL */
    XtSetArg(args[n], XmNvisual, gvvisual); n++;
    XtSetArg(args[n], XmNdepth, gvbitdepth); n++;
    XtSetArg(args[n], XmNcolormap, gvcolormap); n++;
  }
#endif

  /*****************************************************************************/

  shell = XtAppCreateShell(title, "gv", topLevelShellWidgetClass, dpy, args, n);

  XtVaSetValues(shell, XmNiconPixmap, geomicon, NULL);

  XtRealizeWidget(shell);
  camform = shell;

  /* Record the X window id of the top-level shell of each camera
   * in an array indexed by geomview camera number,
   * so cam_winchange() above can map cam id's into widgets.
   * We store X window numbers and use XtWindowToWidget() rather than
   * storing widget pointers to avoid possibly dereferencing junk.
   */
  if (VVCOUNT(camshells) == 0) {
    VVINIT(camshells, struct camwins, 4);
    vvzero(&camshells);
  }
  cw = VVINDEX(camshells, struct camwins, INDEXOF(id));
  cw->shellwin = XtWindow(shell);

  /****************************************************************************/

  n = 0;

#if MGX11 || MGOPENGL
  if (MGOPENGL || gv_no_opengl) { /* ... but not for MGGL */
    XtSetArg(args[n], XmNtitle, title); n++;
    XtSetArg(args[n], XmNrubberPositioning, False); n++;
  }
#endif

  /****************************************************************************/

  XtSetArg(args[n], XmNwidth, size[0]); n++;
  XtSetArg(args[n], XmNheight, size[1]); n++;

  if (gv_no_opengl) {
#if MGX11
    camdraw[SGL] = NULL;
    camdraw[DBL] =
      XtCreateManagedWidget("camdraw",
			    xmDrawingAreaWidgetClass, camform, args, n);

    XtManageChild(camdraw[DBL]);

    mgctxset(MG_X11DISPLAY, dpy,
	     MG_X11COLORMAP, gvcolormap,
	     MG_BITDEPTH, gvbitdepth,
	     MG_X11VISUAL, gvvisual,
	     MG_X11WINID, XtWindow(camdraw[DBL]),
	     MG_X11PARENT, XtWindow(shell),
	     MG_X11SIZELOCK, 1,
	     MG_END);
#endif
  } else { /* gv_no_opengl */
#if MGOPENGL
    /* In case we're not the first GLX context in town, find another
     * so we can share our display lists promiscuously.
     */
    mgctxset(MG_GLXDISPLAY, dpy, MG_END);
    mgctxget(MG_GLXSHARECTX, &sharectx);

    camdraw[SGL] = NULL;
    camdraw[DBL] =
      XtCreateManagedWidget("ogldraw",
			    xmDrawingAreaWidgetClass, camform, args, 0);

    n = 0;
    cmwneeded = 0;

    for(i = SGL; i <= DBL; i++) {
      oglp = &ogl[i];
      oglp->cx = NULL;
      oglp->cm = None;
      cw->vi[i] = glXChooseVisual(dpy,
				  XScreenNumberOfScreen(XtScreen(camform)),
				  oglp->attribs);
      if (cw->vi[i] == NULL) {
	continue;
      }
      if (oglcmap[i] == 0) {
	oglcmap[i] = getcmapfor(cw->vi[i]);
      }
      
      oglp->cx = glXCreateContext(dpy, cw->vi[i], sharectx, GL_TRUE);
      if (oglp->cx) {
	sharectx = oglp->cx;
      }

      xswa.colormap = oglcmap[i];
      xswa.event_mask =
	KeyPressMask|ButtonPressMask|
	ButtonReleaseMask|ButtonMotionMask|
	EnterWindowMask|StructureNotifyMask|ExposureMask;
      xswa.border_pixel =
	xswa.background_pixel =
	xswa.backing_pixel =
	xswa.background_pixmap = None;

      cw->wins[i] = XCreateWindow(dpy, XtWindow(camdraw[DBL]),
				  0, 0, size[0], size[1], 0/*border*/,
				  cw->vi[i]->depth,
				  InputOutput, cw->vi[i]->visual,
				  CWEventMask|CWColormap|CWBorderPixel
				  |CWBackPixel|CWBackPixmap, &xswa);

      XMapWindow(dpy, cw->wins[i]);

      XtRegister_window(dpy, cw->wins[i], camdraw[DBL]);

      if (oglcmap[i] != DefaultColormapOfScreen(XtScreen(camform))) {
	cmw[cmwneeded++] = cw->wins[i];
      }

      mgctxset(oglp->setGLXwin, cw->wins[i],
	       oglp->setGLXctx, oglp->cx, MG_END );
      n++;
    }

    if (n == 0) {
      OOGLError(0, "No OpenGL RGB visual available.");
      exit(1);
    }

    if (cmwneeded > 0) {
      Window *wp;
      int nw;
      if (XGetWMColormapWindows(dpy, XtWindow(shell), &wp, &nw)) {
	if (nw > COUNT(cmw) - 2) nw = COUNT(cmw) - 2;
	memcpy(&cmw[cmwneeded], wp, nw * sizeof(Window));
	XFree(wp);
	cmwneeded += nw;
      }
      XSetWMColormapWindows(dpy, XtWindow(shell), cmw, cmwneeded);
    }
#endif /* MGOPENGL */

#if MGGL
    XtSetArg (args[n], GlxNglxConfig, db_rgb_desc); n++;

    camdraw[DBL] = XtCreateManagedWidget("camdrawdbl",
					 glxMDrawWidgetClass,
					 shell, args, n);
    XtSetArg(args[n-1], GlxNglxConfig, sg_rgb_desc);
    camdraw[SGL] = XtCreateManagedWidget("camdrawsgl",
					 glxMDrawWidgetClass,
					 shell, args, n);
    mgctxset(MG_GLXDISPLAY, dpy,
	     MG_GLXSINGLEWIN, XtWindow(camdraw[SGL]),
	     MG_GLXDOUBLEWIN, XtWindow(camdraw[DBL]),
	     MG_END);
#endif /* MGGL */
  } /* !gv_no_opengl */

  /*****************************************************************************/

  AProtocol = XmInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(shell, AProtocol,
			  (XtCallbackProc) ui_delete_camera,
			  (XtPointer)(long)id);

  /* Even if this event handler isn't called for normal button- and keypress-
   * events, the "StructureNotifyMask" below means we hear about unmapping
   * events in case a window is iconified or etc.
   */
  D1PRINT(("XtAddEventHandler(... panel_input [0] ...)\n"));
  XtAddEventHandler(shell,
		    KeyPressMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask
		    |StructureNotifyMask|ExposureMask,
		    False,
		    (XtEventHandler) panel_input, (XtPointer)(long) id);

  for(i = SGL; i <= DBL; i++) {
    if (camdraw[i]) {

      XtAddCallback(camdraw[i], XmNexposeCallback,
		    (XtCallbackProc) cam_expose, (XtPointer)(long) id);
      XtAddCallback(camdraw[i], XmNresizeCallback,
		    (XtCallbackProc) cam_resize, (XtPointer)(long) id);

      D1PRINT(("XtAddEventHandler(... cam_expose ...)\n"));
      XtAddEventHandler(camdraw[i],
			ExposureMask|StructureNotifyMask,
			False, (XtEventHandler) cam_expose,
			(XtPointer)(long) id);

      D1PRINT(("XtAddEventHandler(... panel_input [1] ...)\n"));
      XtAddEventHandler(camdraw[i],
			KeyPressMask|ButtonPressMask|ButtonReleaseMask,
			False, (XtEventHandler) panel_input,
			(XtPointer)(long) id);

      D1PRINT(("XtAddEventHandler(... cam_mouse ...)\n"));
      XtAddEventHandler(camdraw[i], ButtonMotionMask, False,
			(XtEventHandler) cam_mouse,
			(XtPointer)(long) id);

      D1PRINT(("XtAddEventHandler(... cam_mousecross ...)\n"));
      XtAddEventHandler(camdraw[i], EnterWindowMask, False,
			(XtEventHandler) cam_mousecross, (XtPointer)(long) id);

      D1PRINT(("XtAddEventHandler(... cam_mousecross ...)\n"));
      XtAddEventHandler(camdraw[i], FocusChangeMask, False,
			(XtEventHandler) cam_focus, (XtPointer)(long) id);
    }
  }

  mgctxset(MG_WINCHANGE, cam_winchange, MG_WINCHANGEDATA, (void *)(long)id,
	   MG_END);

  return shell;
}

/*****************************************************************************/

#if MGOPENGL
static Colormap
getcmapfor(XVisualInfo *vi)
{
  int i, nCmaps;
  XStandardColormap *standardCmaps;
  Colormap cm;
  Atom Ahpmap = XmInternAtom(dpy, "_HP_RGB_SMOOTH_TRUE_MAP", False);
  Window root = RootWindow(dpy, vi->screen);

#if HAVE_XMU
  XmuLookupStandardColormap(dpy,
			    vi->screen, vi->visualid, vi->depth, XA_RGB_DEFAULT_MAP,
			    /* replace */ False, /* retain */ True);
#endif
  /* Whether we called XmuLookupStandardColormap or not, and whether it succeeded
   * or not, let's see whether a suitable cmap is installed on the X server.
   */
  if ((Ahpmap != None && XGetRGBColormaps(dpy, root, &standardCmaps, &nCmaps, Ahpmap)) ||
     XGetRGBColormaps(dpy, root, &standardCmaps, &nCmaps, XA_RGB_DEFAULT_MAP)
     ) {
    for(i = 0; i < nCmaps; i++)
      if (standardCmaps[i].visualid == vi->visualid) {
	cm = standardCmaps[i].colormap;
	XFree(standardCmaps);
	return cm;
      }
  }

  if (vi->visual == DefaultVisual(dpy, vi->screen))
    return DefaultColormap(dpy, vi->screen);

  /* Oh well.  Let's just create an uninitialized colormap and hope
   * for the best.  This might be adequate if we've got a TrueColor or
   * other static visual.
   */
  return XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
}
#endif /*MGOPENGL*/

/*****************************************************************************/

void ui_raise_window( int camid )
{
  struct camwins *cw;
  Widget w = camshellof( camid, &cw );
  if (w != NULL && XtWindow(w) != None) {
    XRaiseWindow( dpy, XtWindow( w ) );
  }
}

/*****************************************************************************/

static void ui_delete_camera(Widget w, XtPointer data,
			     XmAnyCallbackStruct *cbs)
{
  extern int real_id(int);

  gv_delete((int)(long)data);
  uistate.targetcam = INDEXOF(FOCUSID);
  ui_target_cameraspanel(real_id(FOCUSID));

  if (w) {
    struct camwins *cw = NULL;
    if (camshellof((int)(long)data, &cw) != NULL && cw!=NULL) {
#if MGOPENGL
      if (!gv_no_opengl) {
	if (cw->wins[SGL]) XtUnregister_window(dpy, cw->wins[SGL], w);
	if (cw->wins[DBL]) XtUnregister_window(dpy, cw->wins[DBL], w);
      }
#endif
      cw->wins[SGL] = cw->wins[DBL] = 0;
    }
    XtDestroyWidget(w);
  }
}

/*****************************************************************************/

#if MGGL || MGOPENGL

static int snapsetup(DView *dv)
{
  int opts;
  int unset = 0;
  struct camwins *cw;
  struct timeval tdelay;
#if MGOPENGL
  GLint bitsdeep;
#endif

  if (camshellof(dv->id, &cw) == NULL) {
    OOGLError(1, "snapshot: no window for camera %s?", dv->name[1]);
    return -1;
  }

  mgctxselect(dv->mgctx);
  mgctxget(MG_SETOPTIONS, &opts);
#if MGOPENGL
  glGetIntegerv(GL_RED_BITS, &bitsdeep);
  if ((opts & MGO_DOUBLEBUFFER) && bitsdeep < 8) {
    mgctxset(MG_UNSETOPTIONS, MGO_DOUBLEBUFFER, MG_END);
    unset = MGO_DOUBLEBUFFER;
  }
#endif
#if MGGL
  if ((opts & MGO_DOUBLEBUFFER) && (getgdesc(GD_BITS_NORM_DBL_RED) < 8)) {
    mgctxset(MG_UNSETOPTIONS, MGO_DOUBLEBUFFER, MG_END);
    unset = MGO_DOUBLEBUFFER;
  }
#endif/*MGGL*/

  XRaiseWindow(dpy, cw->shellwin);	/* Ensure window fully visible */
  XSync(dpy, False);			/* Raise it right now */
  gv_redraw(dv->id);	/* schedule redraw */
  /* Short pause while window manager raises window.
   * Unfortunately I know of no way to tell when this happens;
   * we might not even get an Expose event if we're already
   * unobscured.  Sigh. -slevy
   */
  /* Note: we can't call 'select' below with '&delay' as the last arg, because
   * the final arg has to be a pointer to a struct timeval, and on 64-bit systems
   * a struct timeval isn't the same as an array of 2 ints.  Changed to use &tdelay
   * by mbp on Tue Oct  3 10:19:56 2006.
   */
#define DELAY_US 200000	/* 0.2 second */
  tdelay.tv_sec = 0;
  tdelay.tv_usec = DELAY_US;
  select(0,NULL,NULL,NULL,&tdelay);
  gv_draw(dv->id);	/* draw window now */

#if MGGL
  finish();
#endif
#if MGOPENGL
  glFinish();
#endif
  
  return unset;
}


int ui_ppmscreensnapshot(char *fname, int id, DView *dv, WnWindow *wn, WnPosition *wp)
{
  FILE *f;
  char *data;
  int unset, xsize, ysize, row, i;
  int failed = 1;

  if (gv_no_opengl) {
    OOGLError(0,
	      "OpenGL disabled via command-line switch. "
	      "Screen snapshots are only implemented for OpenGL.\n");
    return failed;
  }

  unset = snapsetup(dv);
  if (unset < 0)
    return -1;

  if (fname[0] == '|') f = popen(fname+1, "w");
  else f = fopen(fname, "w");
  if (f == NULL) {
    OOGLError(0, "snapshot ... ppmscreen: can't create %s: %s",
	      fname, sperror());
    goto done;
  }

  xsize = wp->xmax - wp->xmin + 1;
  ysize = wp->ymax - wp->ymin + 1;

  fprintf(f, "P6\n# Geomview Snapshot of %s\n%d %d\n255\n",
	  dv->name[1], xsize, ysize);
#if MGOPENGL
  row = xsize*3;
  data = OOGLNewNE(char, row*ysize, "snapshot data");
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, xsize, ysize, GL_RGB, GL_UNSIGNED_BYTE, data);
  for(i = ysize; --i >= 0; ) {
    if (fwrite(data+i*row, row, 1, f) <= 0)
      break;
  }
#endif /*MGOPENGL*/
#if MGGL
  row = xsize*4;
  data = OOGLNewNE(char, row*ysize, "snapshot data");
  readdisplay(wp->xmin, wp->ymin, wp->xmax, wp->ymax,
	      (unsigned long *)data, 0);
  for(i = ysize; --i >= 0; ) {
    char *p = data + i*row;
    j = xsize;
    do {
      putc(p[3], f);
      putc(p[2], f);
      putc(p[1], f);
      p += 4;
    } while(--j > 0);
  }
#endif /*MGGL*/

  OOGLFree(data);

  failed = (fname[0] == '|') ? pclose(f) : fclose(f);
  if (failed) {
    OOGLError(0, "snapshot: Error writing to %s: %s", fname, sperror());
  }
 done:
  if (unset)
    mgctxset(MG_SETOPTIONS, unset, MG_END);
  return failed;
}

#if MGOPENGL && MESAGL && HAVE_LIBOSMESA

#include <GL/osmesa.h>

int
ui_ppmmesasnapshot(char *fname, int id, DView *dv, WnWindow *wn, WnPosition *wp)
{
  static OSMesaContext osmctx, sharectx;
  static mgcontext *osmmgctx;
  FILE *f;
  int opts;
  char *data;
  int xsize, ysize, row, i, j;
  mgcontext *oldmgctx = dv->mgctx;
  Camera *cam = NULL;
  Appearance *ap;
  WnWindow *win;
  WnPosition vp;
  int mgspace;
  float pixaspect, frameaspect;
  mgshadefunc shader = NULL;
  int failed = 1;

  if (gv_no_opengl) {
    OOGLError(0,
	      "OpenGL disabled via command-line switch. "
	      "Screen snapshots are only implemented for OpenGL.\n");
    return failed;
  }

  mgctxget(MG_GLXSHARECTX, &sharectx);

  xsize = wp->xmax - wp->xmin + 1;
  ysize = wp->ymax - wp->ymin + 1;

  mgctxget(MG_CAMERA, &cam);
  mgctxget(MG_SPACE, &mgspace);
  mgctxget(MG_SHADER, &shader);

  /* Copy so that changed flags are set */
  ap = ApCopy(mggetappearance(), NULL);

  mgctxget(MG_WINDOW, &win);
  win = WnCopy(win);
  vp.xmin = vp.ymin = 0;
  vp.xmax = xsize;
  vp.ymax = ysize;
  WnSet(win, WN_CURPOS, wp, WN_VIEWPORT, &vp, WN_END);
  if (osmctx == 0)
    osmctx = OSMesaCreateContext(OSMESA_RGBA, sharectx);

  row = xsize * 4;
  data = OOGLNewNE(char, row*ysize, "snapshot data");

  if (!OSMesaMakeCurrent( osmctx, data, GL_UNSIGNED_BYTE, xsize, ysize )) {
    OOGLError(0, "snapshot ... ppmmesa: can't create off-screen Mesa context");
    OOGLFree(data);
    return failed;
  }

  if (fname[0] == '|')
    f = popen(fname+1, "w");
  else
    f = fopen(fname, "w");
  if (f == NULL) {
    OOGLError(0, "snapshot ... ppmmesa: can't create %s: %s",
	      fname, sperror());
    return failed;
  }


  mgdevice_OPENGL();
  if (osmmgctx == NULL) {
    osmmgctx = mgctxcreate(
			   MG_GLXDOUBLECTX, osmctx, MG_GLXDOUBLEWIN, -1,
			   MG_GLXSINGLECTX, osmctx, MG_GLXSINGLEWIN, -1, MG_END);
  } else {
    mgctxselect(osmmgctx);
  }
  dv->mgctx = osmmgctx;
  mgctxset(	MG_CAMERA, cam,
		MG_APPEAR, ap,
		MG_WINDOW, win,
		MG_BACKGROUND, &dv->backcolor,
		MG_SPACE, mgspace,
		MG_SHADER, shader,
		MG_END);

  mgreshapeviewport();	/* Make camera aspect match window */
  {
    /* Try to be as realistic as possible when dumping a snapshot.
     * Create a drawing context, install (hopefully) all the
     * attributes that draw_view() doesn't reset on its own,
     * plug the ctx into the camera, and force a redraw.
     * Then undo the subterfuge.
     */
    int oldredraw = dv->redraw;
    int oldchanged = dv->changed;
    gv_redraw(dv->id);
    gv_draw(dv->id);
    dv->redraw = oldredraw, dv->changed = oldchanged;
  }

  fprintf(f, "P6\n# Geomview Snapshot of %s\n%d %d\n255\n",
	  dv->name[1], xsize, ysize);
  for(i = ysize; --i >= 0; ) {
    char *rgba = data + i*row;
    for(j = 0; j < xsize; j++, rgba += 4) {
      putc(rgba[0], f);
      putc(rgba[1], f);
      putc(rgba[2], f);
    }
  }

  OOGLFree(data);
  ApDelete(ap);
  dv->mgctx = oldmgctx;

  failed = (fname[0] == '|') ? pclose(f) : fclose(f);
  if (failed) {
    OOGLError(0, "snapshot: Error writing to %s: %s", fname, sperror());
  }
  return failed;
}
#endif /*MESA -- ppmmesasnapshot*/

#if MGOPENGL && HAVE_GLXCREATEGLXPIXMAP

int
ui_ppmglxsnapshot(char *fname, int id, DView *dv, WnWindow *wn, WnPosition *wp)
{
  static mgcontext *osglxmgctx;
  struct camwins *cw;
  GLXContext osglxctx;
  Pixmap pixmap;
  GLXPixmap glxpixmap;
  FILE *f;
  char *data;
  int xsize, ysize, row, i;
  mgcontext *oldmgctx = dv->mgctx;
  Camera *cam = NULL;
  Appearance *ap;
  WnWindow *win;
  WnPosition vp;
  int mgspace;
  mgshadefunc shader = NULL;
  int failed = 1;
  double osfactor = 1.0;
  char *osscale;

  if (gv_no_opengl) {
    OOGLError(0,
	      "OpenGL disabled via command-line switch. "
	      "Screen snapshots are only implemented for OpenGL.\n");
    return failed;
  }

  if (camshellof(dv->id, &cw) == NULL) {
    OOGLError(1, "snapshot: no window for camera %s?", dv->name[1]);
    return -1;
  }

  if ((osscale = getenv("GEOMVIEW_OFFSCREEN_FACTOR")) != NULL) {
    osfactor = strtod(osscale, NULL);
    if (osfactor == 0.0) {
      osfactor = 1.0;
    }
  }

  xsize = wp->xmax - wp->xmin + 1;
  ysize = wp->ymax - wp->ymin + 1;
  
  xsize = (int)(osfactor * (double)xsize);
  ysize = (int)(osfactor * (double)ysize);

  mgctxget(MG_CAMERA, &cam);
  mgctxget(MG_SPACE, &mgspace);
  mgctxget(MG_SHADER, &shader);
  mgctxget(MG_GLXSINGLECTX, &osglxctx);
  mgctxget(MG_WINDOW, &win);

  ap = ApCopy(mggetappearance(), NULL);

  win = WnCopy(win);
  vp.xmin = vp.ymin = 0;
  vp.xmax = xsize - 1;
  vp.ymax = ysize - 1;
  WnSet(win, WN_CURPOS, osfactor != 1.0 ? &vp : wp, WN_VIEWPORT, &vp, WN_END);

  /* Create an X11 pixmap and a GLX pixmap to render to */
  pixmap = XCreatePixmap(dpy, cw->shellwin, xsize, ysize, cw->vi[SGL]->depth);
  if (pixmap == 0) {
    return failed;
  }
  glxpixmap = glXCreateGLXPixmap(dpy, cw->vi[SGL], pixmap);
  if (glxpixmap == 0) {
    XFreePixmap(dpy, pixmap);    
    return failed;
  }

  if (!glXMakeCurrent(dpy, glxpixmap, osglxctx)) {
    XFreePixmap(dpy, pixmap);    
    glXDestroyGLXPixmap(dpy, glxpixmap);
    return failed;
  }

  if (fname[0] == '|') {
    f = popen(fname+1, "w");
  } else {
    f = fopen(fname, "w");
  }
  if (f == NULL) { 
    OOGLError(0, "snapshot ... ppmmesa: can't create %s: %s",
	      fname, sperror());
    XFreePixmap(dpy, pixmap);    
    glXDestroyGLXPixmap(dpy, glxpixmap);
    return failed;
  }


  mgdevice_OPENGL();
  if (osglxmgctx == NULL) {
    osglxmgctx = mgctxcreate(MG_GLXDOUBLEWIN, -1,
			     MG_GLXSINGLEWIN, -1,
			     MG_GLXSINGLECTX, osglxctx,
			     MG_UNSETOPTIONS, MGO_DOUBLEBUFFER,
			     MG_END);
  } else {
    mgctxselect(osglxmgctx);
  }
  dv->mgctx = osglxmgctx;
  mgctxset(MG_GLXSINGLECTX, osglxctx,
	   MG_CAMERA, cam,
	   MG_APPEAR, ap,
	   MG_WINDOW, win,
	   MG_BACKGROUND, &dv->backcolor,
	   MG_SPACE, mgspace,
	   MG_SHADER, shader,
	   MG_END);

  mgreshapeviewport();	/* Make camera aspect match window */
  {
    /* Try to be as realistic as possible when dumping a snapshot.
     * Create a drawing context, install (hopefully) all the
     * attributes that draw_view() doesn't reset on its own,
     * plug the ctx into the camera, and force a redraw.
     * Then undo the subterfuge.
     */
    int oldredraw = dv->redraw;
    int oldchanged = dv->changed;
    gv_redraw(dv->id);
    gv_draw(dv->id);
    dv->redraw = oldredraw, dv->changed = oldchanged;
  }

  fprintf(f, "P6\n# Geomview Snapshot of %s\n%d %d\n255\n",
	  dv->name[1], xsize, ysize);
  row = xsize*3;
  data = OOGLNewNE(char, row*ysize, "snapshot data");
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, xsize, ysize, GL_RGB, GL_UNSIGNED_BYTE, data);
  for(i = ysize; --i >= 0; ) {
    if (fwrite(data+i*row, row, 1, f) <= 0) {
      break;
    }
  }

  OOGLFree(data);
  XFreePixmap(dpy, pixmap);    
  glXDestroyGLXPixmap(dpy, glxpixmap);
  ApDelete(ap);
  dv->mgctx = oldmgctx;

  failed = (fname[0] == '|') ? pclose(f) : fclose(f);
  if (failed) {
    OOGLError(0, "snapshot: Error writing to %s: %s", fname, sperror());
  }

  return failed;
}
#endif /* GLX offscreen snapshot */

#include <signal.h>
int ui_sgisnapshot(char *fname, int id, DView *dv, WnWindow *wn, WnPosition *wp)
{
  int pid, wpid, unset;
  char x0[12], x1[12], y0[12], y1[12];
  void (*oldchld)();
  int failed = 1;
  int status;

  unset = snapsetup(dv);
  if (unset < 0)
    return -1;

  sprintf(x0, "%d", wp->xmin);
  sprintf(x1, "%d", wp->xmax);
  sprintf(y0, "%d", wp->ymin);
  sprintf(y1, "%d", wp->ymax);
  oldchld = signal(SIGCHLD, SIG_DFL);
  switch(pid = fork()) {
  case -1:
    OOGLError(0, "snapshot: can't fork to create \"scrsave\" subprocess: %s", sperror());
    break;
  case 0:
    execlp("scrsave", "scrsave", fname, x0, x1, y0, y1, NULL);
    OOGLError(0, "snapshot: can't exec 'scrsave': %s", sperror());
    exit(1);
  default:
    while((wpid = wait(&status)) != pid && wpid != -1)
      ;
    failed = status;
  }
  signal(SIGCHLD, oldchld);
  if (unset)
    mgctxset(MG_SETOPTIONS, unset, MG_END);
  return failed;
}


snap_entry snapshot_table[] = {
  { ui_ppmscreensnapshot, "ppmscreen" },
  { ui_sgisnapshot, "sgiscreen" },
#if MGOPENGL && MESAGL && HAVE_LIBOSMESA
  { ui_ppmmesasnapshot, "ppmosmesa" },
#endif
#if MGOPENGL && HAVE_GLXCREATEGLXPIXMAP
  { ui_ppmglxsnapshot, "ppmosglx" },
#endif
  { NULL, NULL }
};

void
ui_init_snaphelp()
{
  LHelpDef("snapshot",
	   "(snapshot       CAM-ID     FILENAME [FORMAT [XSIZE [YSIZE]]])\n"
	   "Save a snapshot of CAM-ID in the FILENAME (a string). The FORMAT \
argument is optional; it may be \"ppmscreen\" (the default), \"ps\",	\
\"ppm\", \"sgi\" (on SGI machines), \"ppmosmesa\" (if built with	\
libOSMesa) or \"ppmosglx\".  A \"ppmscreen\" snapshot is created by	\
reading the image directly from the given window; the window is popped	\
above other windows and redrawn first, then its contents are written	\
as a PPM format image. A \"ppmosmesa\" snapshot is drawn by Mesa's	\
software renderer into a memory buffer in RAM. A \"ppmosglx\" snapshot	\
is rendered into a GLX Pixmap buffer, which is also off-screen but	\
may or may not reside in video RAM. Rendering may or may not be		\
accelerated. The problem with on-screen snapshots is that the window	\
must be mapped and not obscured by other windows. So on-screen		\
snapshots will not work in the background, or when a screen safer is	\
active. With \"ps\", dumps a Postscript picture representing the view	\
from that window; hidden-surface removal might be incorrect.  With	\
\"ppm\", dumps a PPM-format image produced by geomview's internal	\
software renderer; this may be of arbitrary size.  If the FILENAME	\
argument begins with \"|\", it's interpreted as a /bin/sh command to	\
which the PPM or PS data should be piped.  Optional XSIZE and YSIZE	\
values are relevant only for \"ppm\" formats, and render to a window	\
of that size (or scaled to that size, with aspect fixed, if only XSIZE	\
is given)");
}

#else /* only MGX11 */

snap_entry snapshot_table[] = {
  {NULL, NULL}
};

void
ui_init_snaphelp()
{
  LHelpDef("snapshot",
	   "(snapshot       CAM-ID     FILENAME [FORMAT [XSIZE [YSIZE]]])\n"
	   "Save a snapshot of CAM-ID in the FILENAME (a string).  The\n"
	   "FORMAT argument is optional; it may be \"ppm\" (the default) or \"ps\".\n"
	   "With \"ps\", dumps a Postscript picture representing the view from\n"
	   "that window; hidden-surface removal might be incorrect.\n"
	   "With \"ppm\", dumps a PPM-format image produced by geomview's internal\n"
	   "software renderer; this may be of arbitrary size.\n"
	   "If the FILENAME argument begins with \"|\", it's interpreted as a /bin/sh\n"
	   "command to which the PPM or PS data should be piped.\n"
	   "Optional XSIZE and YSIZE values are relevant only for \"ppm\" format,\n"
	   "and render to a window of that size (or scaled to that size,\n"
	   "with aspect fixed, if only XSIZE is given)."
	   );
}
#endif /* MGX11 */


/*****************************************************************************/
/*****************************************************************************/

/* These routines live here just to keep all MG-type dependencies in one file. */

Pixel ui_RGB(Colormap cm, int permanent, float r, float g, float b)
{
  static XColor	tcol;
  static int wasalloced = 0;
  XColor pcol;
  XColor *col = permanent ? &pcol : &tcol;
  Visual *vis = ((cm == gvcolormap) ?	/* Guess which visual it's on! */
		 gvvisual : DefaultVisual(dpy, DefaultScreen(dpy)));

  if (!permanent) {
    if (wasalloced) {
      XFreeColors(dpy, cm, &tcol.pixel, 1, 0);
      wasalloced = 0;
    }
#if MGX11
    if (vis->class == PseudoColor)
      return mgx11_setRGB((int)(255.0 * r), (int)(255.0 * g), (int)(255.0 * b));
#endif

  }

  col->red = (unsigned short)(r*65535.0);
  col->green = (unsigned short)(g*65535.0);
  col->blue = (unsigned short)(b*65535.0);

  if (XAllocColor(dpy, cm, col)) {
    if (!permanent && (vis->class & 1) != 0)
      wasalloced = 1;  /* XAllocColor only really allocates in dynamic visuals */
    return col->pixel;
  }

  /* give up. */
  return r+g+b>.5 ? WhitePixel(dpy, DefaultScreen(dpy)) :
    BlackPixel(dpy, DefaultScreen(dpy));
}

/*****************************************************************************/

/* This one gets called both as an event handler and as a plain callback;
 * don't rely on the ev or cont fields.
 */
void cam_expose(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
  int id = (int)(long) data;
  DView *dv = (DView *)drawer_get_object(id);

  if (!ISCAM(id) || dv == NULL || dv->mgctx == NULL)
    return;

  if (gv_no_opengl) {
#if MGX11
    mgctxselect(dv->mgctx);
    mgctxset(MG_X11EXPOSE, MG_END);
#endif
  }

  gv_redraw(id);
}

void cam_resize(Widget w, XtPointer id, XmDrawingAreaCallbackStruct *cbs)
{
#if MGOPENGL
  /* Do the work the GLwDrawA widget would have done: resize our subwindows */
  struct camwins *cw;

  if (!gv_no_opengl) {
    if (camshellof((int)(long)id, &cw) != NULL)
      fitwins(w, cw);
  }
#endif
}


/*****************************************************************************/

void ui_find_visual()
{
  int result;

  if (!gv_no_opengl) {
#if MGGL || MGOPENGL
    gvvisual = DefaultVisual(dpy, DefaultScreen(dpy));
    gvcolormap = DefaultColormap(dpy, DefaultScreen(dpy));
    gvbitdepth = DefaultDepth(dpy, DefaultScreen(dpy));
#endif
  } else {
#if MGX11
    result = mgx11_getvisual(dpy, &gvvisual, &gvcolormap, &gvbitdepth);

    if (result == MG_X11VISFAIL) {
      OOGLError(0,
		"X-Geomview currently supports only 24, 16, 8, and 1 bit\n"
		"\tdeep displays.  No supported display depth was found.");
      exit(1);
    } else {
      if (result == MG_X11VISPRIVATE) {
	gvpriv = 1;
	OOGLError(0, "Not enough colors available. Using private colormap.");
      } else {
	return;
      }
    }
#endif
  }
}

/*****************************************************************************/

LDEFINE(bgimagefile, LSTRING,
	"(background-image CAMID [filename])\n\
	Paint that image on the background of that camera window.\n\
	Currently works only with GL and OpenGL graphics.\n\
	Use \"\" for filename to remove background.  With no arguments,\n\
	returns name of that window's current background image, or \"\".")
{
  char *fname = NULL;
  int camid;
#if MGOPENGL || MGGL
  DView *dv;
#endif

  LDECLARE(("background-image", LBEGIN,
	    LID, &camid,
	    LOPTIONAL,
	    LSTRING, &fname,
	    LEND));

#if MGOPENGL || MGGL
  dv = (DView *)drawer_get_object(camid);
  if (dv == NULL || !ISCAM(dv->id) || dv->mgctx == NULL)
    return Lnil;
  mgctxselect(dv->mgctx);
  if (fname != NULL) {
    char *tfname = (fname && fname[0] != '\0') ? fname : NULL;
    if (tfname) {
      tfname = findfile(NULL, tfname);
      if (tfname == NULL) {
	OOGLError(0, "background-image: Can't use image file %s: %s",
		  fname, sperror());
      }
    }
    mgctxset(MG_BGIMAGEFILE, tfname, MG_END);
    gv_redraw(camid);
    return fname ? Lt : Lnil;
  } else {
    mgctxget(MG_BGIMAGEFILE, &fname);
    return LNew(LSTRING, &fname);
  }
#else
  OOGLError(0, "Sorry, background-image only supported with GL and OpenGL graphics");
  return Lnil;
#endif
}

/*****************************************************************************/


void
ui_curson(int on)
{
  Widget w;
  struct camwins *cw;
  Cursor c;

  if ((w = camshellof(real_id(FOCUSID), &cw)) == NULL || XtWindow(w) == None)
    return;

  if (on < 0) on = !cw->curs;
  if (on > 1) {
    if (cursors[1].c) XFreeCursor(dpy, cursors[1].c);
    cursors[1].id = on;
    cursors[1].c = XCreateFontCursor(dpy, cursors[1].id);
    on = 1;
  }
  if (on == 0 && cursors[0].c == 0) {
    /* Need to create blank cursor */
    Pixmap pix = XCreatePixmap(dpy, XtWindow(w), 1, 1, 1);
    XColor col;
    col.pixel = WhitePixelOfScreen(XtScreen(w));
    cursors[0].c = XCreatePixmapCursor(dpy, pix, pix, &col, &col, 0, 0);
  }
  cw->curs = on;
  c = cursors[on].c;
  XDefineCursor(dpy, XtWindow(w), c);
  if (cw->wins[SGL]) XDefineCursor(dpy, cw->wins[SGL], c);
  if (cw->wins[DBL]) XDefineCursor(dpy, cw->wins[DBL], c);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
