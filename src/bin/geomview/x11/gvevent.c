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

#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/comm.h"
#include "../common/space.h"
#include "../common/event.h"
#include "../common/motion.h"
#include "../common/worldio.h"
#include "../common/lang.h"
#include "clang.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/X.h>
#include <X11/keysym.h>
#include "mibload.h"
#include "gvui.h"

#if D1_DEBUG
#define GV_RAWEVENT(dev, val, x, y, t) \
   printf("calling gv_rawevent(dev=%3d, val=%3d, x=%4d, y=%4d, t=%1ld)\n", dev, val, x, y, t); \
   gv_rawevent( dev, val, x, y, t)
#else
#define GV_RAWEVENT(dev, val, x, y, t) \
   gv_rawevent( dev, val, x, y, t)
#endif


/* gvevent.c global vars */
/*****************************************************************************/

#define ESC	'\033'

extern Display *dpy;
static void perftick();
static struct perf {            /* Performance metering */
    int interval;               /* Interval between auto-reports */
    int mindt, maxdt, meandt;   /* Integer milliseconds */
    int cycles;                 /* # cycles where we actually did something */
    struct timeval then;
} perf;

static int lastx;
static int lasty;
static int dragx;
static int dragy;
static int nseen;
static int dragstop, dragging, deldrag, numdrags;
int    justdragged;

static Time lastt;
Time last_event_time;
static Event  gev; /* geomview event structure */

/*****************************************************************************/

#if D1_DEBUG
void print_event(char *s, Event *e) {
  printf("%s: event(dev=%3d, val=%3d, x=%4d, y=%4d, t=%1ld)\n",
	 s, e->dev, e->val, e->x, e->y, e->t);
}
#endif

static void process_events(int queuefd)
{
  struct timeval await;
#define	BRIEF	0.1
  static struct timeval brief = { 0, 100000 };
  float timelimit;
  fd_set thesefds;
  XtInputMask xim;

  if (queuefd < 0) {
    queuefd = ConnectionNumber(dpy);
  }

  if(drawerstate.pause) {
    if(!dragging && !drawer_moving())
      select(0, NULL, NULL, NULL, &brief);
    nseen = 0;
  } else {
    int nwatch = 0;

    timelimit = PoolInputFDs( &thesefds, &nwatch );

    if(timelimit > 0 && drawer_moving())
      timelimit = 0;	/* "Is anything moving?" */

    if (queuefd >= 0 )
      FD_SET(queuefd, &thesefds);

    if(queuefd >= nwatch)
      nwatch = queuefd+1;

    if(timelimit > BRIEF) timelimit = BRIEF;

    await.tv_sec = floor(timelimit);
    await.tv_usec = 1000000*(timelimit - await.tv_sec);

    nseen = select(nwatch, &thesefds, NULL, NULL, &await);
  }

  gettimeofday(&perf.then, NULL);

  if(!drawerstate.pause)
    PoolInAll( &thesefds, &nseen );

  ui_update();

  justdragged = 0;
  deldrag = gev.t;


  while ((xim = XtAppPending(App)) != 0) {
    XtAppProcessEvent(App, xim);
  }

  if (dragging && !justdragged && !dragstop) {
    D1PRINT(("gvevent at 1\n"));
    GV_RAWEVENT( gev.dev, -1, gev.x, gev.y, gev.t);
    dragstop = 1;
  }

  if (dragging) {
    deldrag = gev.t - deldrag;
  } else {
    deldrag = 0;
  }

  XSync(dpy, False);

  gv_update_draw( ALLCAMS, 0.001 * (float)deldrag );
  mg_textureclock();

  if(perf.interval > 0)
    perftick();

  numdrags = 0;
}

void main_loop()
{
  int queuefd;

  queuefd = ConnectionNumber(dpy);
  dragging = 0;
  dragstop = 1;
  justdragged = 0;
  deldrag = 0;
  numdrags = 0;

  while (1) {
    process_events(queuefd);
  }
}

LDEFINE(processevents, LVOID,
	"(process-events)\n"
	"Pass control back to the event loop of Geomview, then continue "
	"evaluating the current command-script.")
{
  Lake *caller;
  
  LDECLARE(("process-events", LBEGIN,
	    LLAKE, &caller,
	    LEND));

  PoolDetach(POOL(caller));
  do {
    process_events(-1);
  } while (PoolASleep(POOL(caller)));
  PoolReattach(POOL(caller));
  
  return Lt;
}

/*****************************************************************************/

static void
perftick()
{
    int dt;
    struct timeval now;
    gettimeofday(&now, NULL);
    dt = (now.tv_sec - perf.then.tv_sec)*1000 +
         (now.tv_usec - perf.then.tv_usec)/1000;
    if(dt > 0) {
        if(dt < perf.mindt) perf.mindt = dt;
        if(dt > perf.maxdt) perf.maxdt = dt;
        perf.meandt += dt;
        if(++perf.cycles == perf.interval)
            timing(perf.interval);
    }
}

/*****************************************************************************/

void
timing(int interval)
{
    if(perf.cycles > 0) {
        printf("%d..%d ms/cycle, mean %d ms over %d cycles\n",
                perf.mindt, perf.maxdt,
                perf.cycles ? perf.meandt/perf.cycles : 0,
                perf.cycles);
        fflush(stdout);
    }
    perf.mindt = 9999999, perf.maxdt = -1, perf.meandt = perf.cycles = 0;
    perf.interval = interval;
}

/*****************************************************************************/
void cam_input(Widget w, XtPointer data, XmDrawingAreaCallbackStruct *cbs)
{
  Boolean junk;
D1PRINT(("cam_input\n"));
  panel_input(w, data, cbs->event, &junk);
}

void key_action(Widget w, XEvent *event, String *argv, Cardinal *argcp)
{
  Boolean junk;
D1PRINT(("key_action\n"));
  panel_input(w, NULL, event, &junk);
}
  
void panel_input(Widget w, XtPointer data, XEvent *event, Boolean *cont)
{
  char          str[1];
  KeySym	keysym;
  int		newstate, nc;

  switch (event->type)
  {
    case UnmapNotify:		/* Window iconified or otherwise temporarily gone */
D1PRINT(("panel_input: case UnmapNotify\n"));
      gv_freeze( (int)(long)data, 1 );	/* Hard-frozen; prevent gv_redraw from working until we say so */
      break;

    case MapNotify:
D1PRINT(("panel_input: case MapNotify\n"));
      gv_freeze( (int)(long)data, 0 );	/* Permit thawing. */
      gv_redraw( (int)(long)data );		/* Thaw it now, in case Expose arrived before MapNotify */
      break;

    case MotionNotify:
D1PRINT(("panel_input: case MotionNotify\n"));
      last_event_time = event->xmotion.time;
      cam_mouse(w, data, &event->xmotion, cont);
      return;

    case ButtonPress:
    case ButtonRelease:
D1PRINT(("panel_input: case ButtonPress/Release\n"));
      newstate = (event->type == ButtonPress);

      button.left = ((event->xbutton.state & Button1Mask) ? 1 : 0);
      button.middle = ((event->xbutton.state & Button2Mask) ? 1 : 0);
      button.right = ((event->xbutton.state & Button3Mask) ? 1 : 0);
      button.shift = ((event->xbutton.state & ShiftMask) ? 1 : 0);
      button.ctrl = ((event->xbutton.state & ControlMask) ? 1 : 0);
      dragging = newstate;

      lastt = last_event_time = event->xbutton.time;
D1PRINT(("gvevent: at 1.5, event->xbutton.time = %1ld\n", event->xbutton.time));
      dragx = event->xbutton.x_root;
      dragy = YScrnSize - event->xbutton.y_root;

      if (dragstop && !newstate) {
D1PRINT(("gvevent at 2\n"));
        GV_RAWEVENT( gev.dev, -1, dragx, dragy, lastt);
      }

      switch (event->xbutton.button) {
	 case 1:
           gev.dev = ELEFTMOUSE;
           button.left = newstate;
	   break;
	 case 2:
           gev.dev = EMIDDLEMOUSE;
           button.middle = newstate;
	   break;
	 case 3:
           gev.dev = ERIGHTMOUSE;
           button.right = newstate;
	   break;
      }

      if(gev.dev == ELEFTMOUSE && (event->xbutton.state & Mod1Mask)) {
	gev.dev = EMIDDLEMOUSE;
	button.middle = button.left;
	button.left = 0;
      }

      gev.x = dragx; gev.y = dragy; gev.t = lastt;
      deldrag = gev.t;
D1PRINT(("gvevent at 3\n"));
      GV_RAWEVENT( gev.dev, newstate, gev.x, gev.y, gev.t);
      dragstop = 1;
      break;

    case KeyPress:
D1PRINT(("panel_input: case KeyPress\n"));
      nc = XLookupString((XKeyEvent *) event, str, 1, &keysym, NULL);
      last_event_time = event->xkey.time;

      if ((int)keysym == (int)'Q')
	str[0] = ESC;

      if (keysym == XK_Escape)
	str[0] = ESC;

      if (nc > 0) {
	  gev.dev = str[0];
	  gev.x = event->xkey.x_root;
	  gev.y = YScrnSize - event->xkey.y_root;
	  gev.t = event->xkey.time;
D1PRINT(("gvevent at 4\n"));
          GV_RAWEVENT(str[0], 1, gev.x, gev.y, gev.t);
      }

      break;

    default:
      break;
  }
  
}

/*****************************************************************************/

void cam_mouse(Widget w, XtPointer data, XPointerMovedEvent *event,
	Boolean *cont)
{
  static int lasthere = 0;
  dragging = 1;
  dragstop = 0;
  justdragged = 1;

  numdrags++;
  if (numdrags > 2)
    return;

  gev.x = event->x_root; gev.y = YScrnSize - event->y_root; gev.t = event->time;

  PRINT_EVENT(("cam_mouse", &gev));

  if ((int)(gev.t - lastt) > uistate.cursor_still)
  {
    if (abs(gev.x - lastx) < uistate.cursor_twitch &&
	abs(gev.y - lasty) < uistate.cursor_twitch)
      {
	gev.x = dragx;
	gev.y = dragy;
      }
    lastx = gev.x;
    lasty = gev.y;
    lastt = gev.t;
  }
  dragx = gev.x;
  dragy = gev.y;
  if (lasthere != (int)gev.t) {
D1PRINT(("cam_mouse: at 1\n"));
    GV_RAWEVENT( gev.dev, -1, gev.x, gev.y, gev.t);
  } else {
D1PRINT(("cam_mouse: at 2\n"));
    GV_RAWEVENT( gev.dev, -1, gev.x, gev.y, gev.t+1);
    dragstop = 1;
  }

  lasthere = gev.t;
}

/*****************************************************************************/

void cam_mousecross(Widget w, XtPointer data, XEnterWindowEvent *event,
		    Boolean *cont)
{
  if (!uistate.cam_wm_focus) {
    int id = (int)(long)data;
    
    gv_winenter(id);
  }
}

/*****************************************************************************/

void cam_focus(Widget w, XtPointer data, XFocusChangeEvent *event,
	       Boolean *cont)
{
  if (uistate.cam_wm_focus && event->type == FocusIn) {
    int id = (int)(long)data;

    gv_winenter(id);
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
