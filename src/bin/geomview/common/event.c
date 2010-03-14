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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "mg.h"

#include "drawer.h"
#include "event.h"
#include "lang.h"
#include "ui.h"
#include "comm.h"
#include "pickP.h"
#include "transform.h"
#include "streampool.h"
#include "lights.h"
#include "mouse.h"
#include "rman.h"

#define DOUBLECLICKTIME 333	/* millisecs between double clicks */

EventState estate;	/* External motion state */

struct button button;	/* Shift, etc. button state */

void emit_pick(int pickedid, Pick *pick);
static int view_pick( DView *dv, int x, int y, Pick *pick );

#define	ESC	'\033'

struct num {
  int has;	/* 0 if no number, or -1 (negative) or +1 (positive) */
  int val;
  int expon;
} number, onum;

static int keyshorts = 1;
static int pickon = 1;
static int prefixid = NOID;
static float getreal(float defaultvalue);
static int getint(int defaultvalue);
/*static int toggle(int val);*/
static void tog_ap_flag(int id, int flagbit);
static int retarget(int defaultid);

#define	SEQ(prefix, ch)  ((prefix)<<8 | (ch))

static enum { KEYGEOM=0, KEYCAM=1, KEYNONE=2 } keymode = KEYNONE;
static int prefix = 0;

static char Help[] = "\
Keyboard commands apply while cursor is in any graphics window and most \n\
control panels. Most commands allow one of the following selection prefixes \n\
(if none is provided the command applies to the current object): \n\
   g  world geom	g#  #'th geom	g*  All geoms\n\
   c  current camera	c#  #'th camera	c*  All cameras\n\
Many allow a numeric prefix:  if none they toggle or reset current value.\n\
Appearance:\n\
 Draw:		     Shading:		Other:\n\
  af  Faces		0as Constant	 av  eVert normals: always face viewer\n\
  ae  Edges		1as Flat	#aw  Line Width (pixels)\n\
  an  Normals		2as Smooth	#ac  edges Closer than faces(try 5-100)\n\
  ab  Bounding Boxes	3as Smooth, non-lighted     al  Shade lines\n\
  aV  Vectors		aT  allow transparency      at  Texture-mapping\n\
 Color:			aC  allow concave polygons  aq  Texture quality\n\
  Cf Ce Cn Cb CB   face/edge/normal/bbox/backgnd\n\
Motions:				      Viewing:\n\
  r rotate	   [ Leftmouse=X-Y plane,	0vp Orthographic view\n"
#ifdef NeXT
  "  t translate	     Alt-Left=Z axis,		1vp Perspective view\n"
#else
  "  t translate	     Middle=Z axis,		1vp Perspective view\n"
#endif
  "  z zoom FOV	     Shift=slow motion,		 vd Draw other views' cameras\n\
  f fly		     in r/t modes.      ]	#vv field of View\n\
  o orbit           [Left=steer, Middle=speed ]	#vn near clip distance\n\
  s scale					#vf far clip distance\n\
  w/W recenter/all				 v+ add new camera\n\
  h/H halt/halt all				 vx cursor on/off\n\
  @  select center of motion (e.g. g3@)		 vb backfacing poly cull on/off\n\
						#vl focal length\n\
  L  Look At object				 v~ Software shading on/off\n\
show Panel:	Pm Pa Pl Po	main/appearance/lighting/obscure\n\
		Pt Pc PC Pf	tools/cameras/Commands/file-browser\n\
		Ps P-		saving/read commands from tty\n\
Lights:  ls le		Show lights / Edit Lights\n\
Metric:  me mh ms  	Euclidean Hyperbolic Spherical\n\
Model:   mv mp mc	Virtual Projective Conformal\n\
Other:\n\
  N normalization < Pf  load geom/command file\n\
   0N none	  > Ps  save something to file	ui  motion has inertia\n\
   1N each	  TV	NTSC mode toggle	uc  constrained (X/Y) motion\n\
   2N all	  				uo  motion in Own coord system\n\
  Rightmouse-doubleclick  pick as current target object\n\
  Shift-Rightmouse        pick interest (center) point\n"
#ifdef NeXT
  "  Alt-Leftmouse is synonym for Rightmouse.\n"
#endif
  ;

void 
print_help()
{
  printf("\n%s", Help);
  rman_do('?', 0,0);
  fflush(stdout);
}

void
event_init()
{
  estate.motionproc = NULL;
}

LDEFINE(event_mode, LVOID,
	"(event-mode     MODESTRING)\n\
	Set the mouse event (motion) mode; MODESTRING should be one of\n\
	the strings that appears in the motion mode browser (including\n\
	the keyboard shortcut, e.g. \"[r] Rotate\").")
{
  static Event ev_enter = { EMODEENTER, 0, 0, 0, 0 };
  static Event ev_exit = { EMODEEXIT, 0, 0, 0, 0 };
  char *modename;

  LDECLARE(("event-mode", LBEGIN,
	    LSTRING, &modename,
	    LEND));

  if ( estate.motionproc != NULL ) {
    estate.motionproc(&ev_exit);
  }
  estate.motionproc = 
    uistate.modes[uistate.mode_current = ui_mode_index(modename) ];
  D1PRINT(("gv_event_mode: estate.motionproc <- %1x\n", estate.motionproc)); 
  ui_event_mode( modename );
  if ( estate.motionproc != NULL ) {
    estate.motionproc(&ev_enter);
  }
  return Lt;
}



/*
 * Report time elapsed since the epoch (or since the program began if
 * since == NULL).   Possibly remember the current time in "nextepoch".
 * Time is measured in floating-point seconds.
 */
float
elapsed(float *since, float *nextepoch)
{
  static struct timeval t0 = { 0, 0 };
  struct timeval tnow;
  float now = 0;
  float sincetime = 0;

  gettimeofday(&tnow, NULL);
  if(t0.tv_sec == 0) {
    t0 = tnow;
    tnow.tv_usec++;
  }
  now = tnow.tv_sec - t0.tv_sec + 1e-6*(tnow.tv_usec - t0.tv_usec);
  if(since) {
    if((sincetime = *since) == 0)
      sincetime = *since = now;
  }
  if(nextepoch) *nextepoch = now;
  return now - sincetime;
}

LDEFINE(pick_invisible, LVOID,
	"(pick-invisible [yes|no])\n\
	Selects whether picks should be sensitive to objects whose appearance\n\
	makes them invisible; default yes.\n\
	With no arguments, returns current status.")
{
  Keyword kw = NOT_A_KEYWORD;

  LDECLARE(("pick-invisible", LBEGIN,
	    LOPTIONAL,
	    LKEYWORD, &kw,
	    LEND));

  if(kw == NOT_A_KEYWORD) {
    return uistate.pick_invisible ? Lt : Lnil;
  }

  uistate.pick_invisible = boolval("pick_invisible", kw);

  return Lt;
}
  
LDEFINE(rawevent, LVOID,
	"(rawevent       dev val x y t)\n\
	Enter the specified raw event into the event queue.  The\n\
	arguments directly specify the members of the event structure\n\
	used internally by geomview.  This is the lowest level event\n\
	handler and is not intended for general use.")
/*
  This used to be dispatch_event().
*/
{
  int id;
  int apfl;
  DrawerKeyword k;
  int err = 0;
  float v;
  Appearance *ap;
  DView *dv;
  DGeom *dg;
  char *s;

  Event event;
  LDECLARE(("rawevent", LBEGIN,
	    LINT, &event.dev,
	    LINT, &event.val,
	    LINT, &event.x,
	    LINT, &event.y,
	    LLONG, &event.t,
	    LEND));

  PRINT_EVENT(("in gv_rawevent", &event));

  /*
   * Call the current motion proc, if any.  This proc returns 1 if it
   * used the event, in which case we don't do any further processing.
   */
  D1PRINT(("gv_rawevent: estate.motionproc = %x\n", estate.motionproc));
  if ( estate.motionproc != NULL ) {
    D1PRINT(("gv_rawevent:   calling estate.motionproc\n"));
    if ( estate.motionproc(&event) ) {
      D1PRINT(("gv_rawevent:     returning Lt\n"));
      return Lt;
    }
    D1PRINT(("gv_rawevent:     falling through\n"));
  }

  /* The rightmouse and doubleclick is now hardcoded but should be
     bindable through lang later, through a control mechanism similar to
     current motionproc stuff */

  if (event.dev == ERIGHTMOUSE) {
    if ((event.val > 0) && pickon) {
      static unsigned long int lastt = 0;
      Pick *pick = PickSet(NULL, PA_WANT,
			   uistate.pick_invisible ? PW_EDGE|PW_VERT|PW_FACE
			   : PW_VISIBLE|PW_EDGE|PW_VERT|PW_FACE,
			   PA_END);
      int pickedid = view_pick( (DView *)drawer_get_object(FOCUSID),
				event.x, event.y, pick );

      if(button.shift) {
	/* Could change FOCUSID here to ALLCAMS,
	 * to force setting everyone's focal length to 
	 * their distance from the pick.
	 */
	if(pickedid != NOID)
	  make_center_from_pick("CENTER", pick, FOCUSID);
	else
	  gv_ui_center(TARGETID);
      } else {
	if(pickedid != NOID)
	  emit_pick(pickedid, pick);
	if (event.t - lastt < DOUBLECLICKTIME) {
	  lastt = 0;
	  gv_ui_target( pickedid!=NOID ? pickedid : WORLDGEOM, IMMEDIATE );
	}
      }
      PickDelete(pick);
      lastt = event.t;
    }
  }

  if(!isascii(event.dev))
    return Lt;


  if (!keyshorts)  /* are keyboard shortcuts on? :-) */
    return Lt;     /* no? then we don't want to process them... */

  /* Only keyboard events from here on down */

  ui_keyboard(event.dev);
  if(event.dev >= '0' && event.dev <= '9') {
    if(!number.has) {
      number.val = 0;
      number.has = 1;
    }
    number.val = 10*number.val + (event.dev - '0');
    if(number.expon) number.expon++;
    prefix = 0;
  } else {

    id = GEOMID(uistate.targetgeom);

  rescan:

    switch(SEQ(prefix, event.dev)) {

    case '-':
    case '*':
      number.has = -1;
      number.expon = 0;
      number.val = 0;
      prefix = 0;
      goto keepmode;

    case '.':
      number.expon = 1;
      prefix = 0;
      goto keepmode;


    case 'g': keymode = KEYGEOM; goto gotmode; /* Select geom:  'g' prefix */
    case 'c': keymode = KEYCAM; goto gotmode;  /* Select camera: 'c' prefix */
    gotmode:
      onum = number;
      number.has = 0;
      prefix = 0;
      goto keepmode;

    case 'p':
      {
	int id;

	if (pickon) {
	  if (keymode == KEYNONE) {
	    id = gv_rawpick(FOCUSID, event.x, event.y);
	    if (id == NOID) id = WORLDGEOM;
	  } else {
	    id = retarget(NOID);
	  }

	  gv_ui_target(id, IMMEDIATE);
	}
      }

    case '@':
      gv_ui_center(retarget(uistate.centerid));
      break;

    case 'N':
      id = retarget(GEOMID(uistate.targetgeom));
      if(!number.has) {
	dg = (DGeom *)drawer_get_object(id);
	if(dg) number.val = dg->normalization == NONE ? EACH : NONE;
      }
      drawer_int(id, DRAWER_NORMALIZATION, number.val);
      break;

    case '<':
      s = "Load"; number.val = 1; goto pickpanel;	/* Load file */
    case '>':
      s = "Save"; number.val = 1; goto pickpanel;	/* Save State */

      /* use bounding box center as CENTER position */
    case 'B':
      gv_ui_center_origin(uistate.bbox_center
			  ? ORIGIN_KEYWORD : BBOX_CENTER_KEYWORD);
      break;

      /* Halt current object */
    case 'h':
      drawer_stop(retarget(uistate.targetid)); break;

    case 'H':	/* Halt Everything */
      drawer_stop(NOID); break;

    case 'w':     /* Recenter current thing */
      drawer_center(retarget(uistate.targetid)); break;

    case 'W':     /* Recenter (and halt) Everything */
      drawer_center(NOID); break;

    case 'L':
      gv_look(retarget(uistate.targetid),FOCUSID); 
      break;
	
      /*
       * r/t/z/f/o apply to the currently selected object unless target specified.
       */
    case 'f': s = OBJFLY; goto mote;
    case 'o': s = OBJORBIT; goto mote;
    case 'r': s = OBJROTATE; goto mote;
    case 't': s = OBJTRANSLATE; goto mote;
    case 'z': s = OBJZOOM; goto mote;
    case 's': s = OBJSCALE; goto mote;

    mote:
      id = retarget(NOID);
      if (id) gv_ui_target( id, IMMEDIATE);
      gv_event_mode( s );
      break;

    case '?':
      print_help();
      break;

#ifdef sgi
    case 'T':   /* NTSC */
#endif
    case 'v':	/* view-related prefix */
    case 'a':	/* appearance-related prefix */
    case 'm':	/* metric (euclidean/hyperbolic/spherical) */
    case 'l':	/* light-related prefix */
    case 'd':   /* delete */
    case 'R':   /* renderman */
    case 'C':	/* color-pick */
    case 'P':	/* panel show */
    case 'u':	/* motion style */
    case ESC:	/* quit prefix */
      if(keymode != KEYNONE) {
	prefixid = retarget(NOID);
	if(ISGEOM(prefixid))
	  gv_ui_target( prefixid, IMMEDIATE);
      }
      prefix = event.dev;
      goto keepnumber;

    case SEQ('P','m'): 
    case SEQ('P','g'): s = "main"; goto pickpanel;
    case SEQ('P','a'): s = "Appearance"; goto pickpanel;
    case SEQ('P','o'): s = "Obscure"; goto pickpanel;
    case SEQ('P','l'): s = "Lighting"; goto pickpanel;
    case SEQ('P','C'): s = "Command"; goto pickpanel;
    case SEQ('P','c'): s = "Camera"; goto pickpanel;
    case SEQ('P','t'): s = "Tools"; goto pickpanel;
    case SEQ('P','f'): s = "Files"; goto pickpanel;
    case SEQ('P','s'): s = "Save"; goto pickpanel;
    case SEQ('P','M'): s = "Materials"; goto pickpanel;	
    case SEQ('P','A'): s = "Credits"; goto pickpanel;
    pickpanel:
    ui_showpanel(ui_name2panel(s), getint(-1));
    break;
    case SEQ('P','P'):
      ui_manual_browser("pdf");
      break;
    case SEQ('P','H'):
      ui_manual_browser("html");
      break;
    case SEQ('P','-'): 
      comm_object("(read command < -)", &CommandOps, NULL, NULL, COMM_LATER);
      break;


    case SEQ(ESC,ESC):
      gv_exit();
      /*NOTREACHED*/

    case SEQ('C','f'): k = DRAWER_DIFFUSE; goto pickcolor;
    case SEQ('C','e'): k = DRAWER_EDGECOLOR; goto pickcolor;
    case SEQ('C','n'): k = DRAWER_NORMALCOLOR; goto pickcolor;
    case SEQ('C','b'): k = DRAWER_BBOXCOLOR; goto pickcolor;
    case SEQ('C','v'):
    case SEQ('C','B'): k = DRAWER_BACKCOLOR; goto pickcolor;
    pickcolor:
    ui_pickcolor( k );
    break;

    case SEQ('u','i'): k = DRAWER_INERTIA; goto motstyle;
    case SEQ('u','c'): k = DRAWER_CONSTRAIN; goto motstyle;
    case SEQ('u','o'): k = DRAWER_OWNMOTION;
    motstyle:
    drawer_int( WORLDGEOM, k, getint(-1) );
    break;

    case SEQ('v','+'): 		/* Add camera */
      { CameraStruct cs;
	id = retarget(FOCUSID);
	dv = ISCAM(id) ? (DView *)drawer_get_object(id) : NULL;
	cs.h = NULL;
	cs.cam = dv && dv->cam ? CamCopy(dv->cam, NULL) : NULL; 
	gv_new_camera(NULL, &cs);
      }
      break;

    case SEQ('v','p'):		/* Projection: orthographic or perspective */
      id = retarget(FOCUSID);
      if(!number.has) {
	dv = (DView *)drawer_get_object(id);
	if(dv) CamGet(dv->cam, CAM_PERSPECTIVE, &number.val);
	number.val = !number.val;
      }
      drawer_int( id, DRAWER_PROJECTION, number.val );
      break;

    case SEQ('v','d'):			/* toggle "Draw other cameras" */
      id = retarget(FOCUSID);
      if(!number.has) {
	dv = (DView *)drawer_get_object(id);
	number.val = dv ? !dv->cameradraw : true;
      }
      drawer_int( id, DRAWER_CAMERADRAW, number.val );
      break;

    case SEQ('v','D'): /* Toggle dithering */
      id = retarget(FOCUSID);
      gv_dither(id, TOGGLE_KEYWORD);
      break;

      /* stuff for X11 version */
    case SEQ('v','h'): /* pick hidden surface removal method: */
      id = retarget(FOCUSID);
      dv = (DView *)drawer_get_object(id);
      if(dv == NULL || dv->mgctx == NULL)
	break;
      if (!number.has) {
	mgctxselect(dv->mgctx);
	mgctxget(MG_DEPTHSORT, &number.val);
	number.val = (number.val+1) % 3;
      }
      mgctxset(MG_DEPTHSORT,
	       number.val>=0&&number.val<=2 ? number.val : 2, MG_END);
      gv_redraw(dv->id);
      ui_maybe_refresh(dv->id);
      break;
      /* end of stuff for X11 version */

    case SEQ('v','x'): /* Toggle/enable/disable cursor */
      ui_curson( number.has ? number.val : -1 ); 
      break;

    case SEQ('v','b'):
      tog_ap_flag( id, APF_BACKCULL );
      break;

    case SEQ('v','s'):
      id = retarget(FOCUSID);
      number.val = !number.val;	/* "1vs" => single-buffered */
      drawer_int( id, DRAWER_DOUBLEBUFFER, getint(-1) );
      break;

      /* For testing software shading */
    case SEQ('v','~'):
      id = retarget(FOCUSID);
      gv_soft_shader(id,
		     number.has ? (number.val?ON_KEYWORD:OFF_KEYWORD) : TOGGLE_KEYWORD);
      break;


      /* Viewing options */
    case SEQ('a','c'): 
    case SEQ('v','c'): k = DRAWER_LINE_ZNUDGE; v = 10.; goto setcam;
    case SEQ('v','v'): k = DRAWER_FOV;  v = 45.; goto setcam;
    case SEQ('v','n'): k = DRAWER_NEAR; v = .1;	goto setcam;
    case SEQ('v','f'): k = DRAWER_FAR;  v = 100.; goto setcam;
    case SEQ('v','l'): k = DRAWER_FOCALLENGTH; v = 3.; goto setcam;
    setcam:
    drawer_float( retarget(FOCUSID), k, getreal(v) );
    break;

    /* Might add others here, e.g. a viewfinder mode. */

    /* Metrics / Models */
    case SEQ('m','e'):
      gv_space(EUCLIDEAN_KEYWORD);
      break;
    case SEQ('m','h'):
      gv_space(HYPERBOLIC_KEYWORD);
      break;
    case SEQ('m','s'):
      gv_space(SPHERICAL_KEYWORD);
      break;
    case SEQ('m','v'):
      gv_hmodel(retarget(FOCUSID), VIRTUAL_KEYWORD);
      break;
    case SEQ('m','p'):
      gv_hmodel(retarget(FOCUSID), PROJECTIVE_KEYWORD);
      break;
    case SEQ('m','c'):
      gv_hmodel(retarget(FOCUSID), CONFORMALBALL_KEYWORD);
      break;

      /* Appearance settings */
    case SEQ('a','f'): apfl = APF_FACEDRAW; goto togapflag;
    case SEQ('a','e'): apfl = APF_EDGEDRAW; goto togapflag;
    case SEQ('a','l'): apfl = APF_SHADELINES; goto togapflag;
    case SEQ('a','n'): apfl = APF_NORMALDRAW; goto togapflag;
    case SEQ('a','v'): apfl = APF_EVERT; goto togapflag;
    case SEQ('a','t'): apfl = APF_TEXTURE; goto togapflag;
    case SEQ('a','T'): apfl = APF_TRANSP; goto togapflag;
    case SEQ('a','V'): apfl = APF_VECTDRAW; goto togapflag;
    case SEQ('a','C'): apfl = APF_CONCAVE; goto togapflag;
    case SEQ('a','q'): apfl = APF_TXMIPMAP|APF_TXMIPINTERP|APF_TXLINEAR; goto togapflag;
    togapflag:
    tog_ap_flag( id, apfl );
    break;
    case SEQ('a','x'): drawer_set_ap( id, NULL, NULL ); break;
    case SEQ('a','o'): gv_ap_override( getint( !uistate.apoverride ) ); break;


    case SEQ('a','b'): /* Bounding box drawing */
      if(!number.has) {
	DGeom *dg = (DGeom *)drawer_get_object( id );
	if(dg) number.val = !dg->bboxdraw;
      }
      drawer_int(id, DRAWER_BBOXDRAW, number.val);
      break;

    case SEQ('a','s'):	/* Shading */
      if(!number.has) {
	ap = drawer_get_ap(id);
	ApGet(ap, AP_SHADING, &number.val);
	ApDelete(ap);
	number.val++;
      }
      drawer_int(id, DRAWER_SHADING, number.val % 5);
      break;

    case SEQ('a','w'):	/* line width */
      if(!number.has) {
	ap = drawer_get_ap(id);
	ApGet(ap, AP_LINEWIDTH, &number.val);
	ApDelete(ap);
	number.val = (number.val > 1) ? 1 : 2;
      }
      drawer_int(id, DRAWER_LINEWIDTH, number.val);
      break;

      /* Scale normals */
    case SEQ('a','h'): drawer_float(id, DRAWER_NORMSCALE, getreal(1.0)); break;

      /* Patch dicing */
    case SEQ('a','d'): drawer_int( id, DRAWER_BEZDICE, number.val ); break;

      /* hyperbolic sphere at infinity */
    case SEQ('a', 'i'): drawer_int( retarget(FOCUSID), DRAWER_HSPHERE,
				    getint(-1) );
      break;
	
      /* Delete */
    case SEQ('d','d'): gv_delete(uistate.targetid); break;

      /* NTSC */
#ifdef sgi
    case SEQ('T','V'): ntsc(getint(-1)); break;
#endif

      /* Timing -- ctrl-T
       * ^T : print accumulated timing status now
       * <nnn>^T : print timing status now and every <nnn> main-loop cycles
       * -^T : quit timing
       */
    case 'T'&0x1f:
      timing( number.has<0 ? 0 : number.has ? number.val : 9999999 );
      break;

      /* Edit Lights */
    case SEQ('l','e'):
      if (!(uistate.lights_shown)) light_edit_mode(1);
      else gv_event_mode( LIGHTEDIT );
      break;
	
      /* Toggle Show Lights */
    case SEQ('l','s'): light_edit_mode(2); break;

      /*
       * All R* commands moved to rman.c - slevy.
       */
    default:
      err = EOF;
      if(prefix == 'R') {
	rman_do(event.dev,number.has,number.val);
	break;
      } else if(prefix != 0) {		/* No such command? */
	prefix = 0;
	goto rescan;		/* Try same char without prefix */
      }
      keymode = KEYNONE;
    }
    number.has = number.expon = onum.has = onum.expon = 0;
    prefix = 0;
    prefixid = NOID;
    ui_keyboard(err);			/* 0 = OK, EOF = -1 = error */
  keepnumber:
    keymode = KEYNONE;
  keepmode:
    ;
  }

  return Lt;
}

/* 
 * Interpret a g[N] or c[N] prefix; return the id.
 */
static int
retarget(int defindex)
{
  int t;
  static int allid[2] = { ALLGEOMS, ALLCAMS };
  static char ch[2] = { 'g', 'c' };
  char code[12];

  if(keymode == KEYNONE) {
    if(number.expon && !number.has) {  /* a "." prefix, sans number */
      number.expon = 0;
      return TARGETID;
    }
    return prefixid != NOID ?
      prefixid : defindex;	/* No prefix, or just numeric */
  }

  sprintf(code, "%c%d", ch[keymode], number.val);
  if(number.has > 0) t = drawer_idbyname(code);
  else if(number.has < 0) t = allid[keymode];
  else t = (keymode == KEYGEOM) ? WORLDGEOM
    : FOCUSID;
  number = onum;
  onum.has = onum.expon = 0;
  keymode = KEYNONE;
  return t;
}

/* Return current number if any; otherwise return given default value. */

static float
getreal(float defval)
{
  float v = number.has * number.val;

  if(!number.has) return defval;
  while(--number.expon > 0)
    v *= 0.1;
  return v;
}

static int
getint(int defaultvalue)
{
  return number.has ? number.val*number.has : defaultvalue;
}

#if 0
static int
toggle(int val)
{
  return number.has ? (number.has = number.expon = 0, number.val) : !val;
}
#endif

static void
tog_ap_flag( int id, int flagbit )
{
  ApStruct as;
  int val;

  memset(&as, 0, sizeof(as));
    
  if(number.has) {
    val = number.val;
  } else {
    as.ap = drawer_get_ap(id);
    val = as.ap ? !(as.ap->flag & flagbit) : 1;
  }
  as.ap = ApCreate(val ? AP_DO : AP_DONT, flagbit,
		   AP_OVERRIDE, uistate.apoverride & flagbit, AP_END);
  gv_merge_ap(id, &as);
  ApDelete(as.ap);
}



/* NB - I've put in a total hack to avoid calling gvpick more than 
 * once - I think it's pretty stable (so when it stays in for years
 * and years it might(?) keep working!) -cf 10/29/92 */
static int
view_pick( DView *dv, int x, int y, Pick *pick )
{
  Transform V, T, Tnet, Tt, Tmodel, Tnorm, oldTw, Tworld;
  int i;
  int chosen = NOID;
  float xpick, ypick;
  DGeom *dg;
  Appearance *ap;
  WnPosition wp;
  
  if(dv == NULL)
    return NOID;

  if(dv->stereo == NO_KEYWORD) {
    wp = drawerstate.winpos;
    mousemap(x, y, &xpick, &ypick, &wp);
  } else {
    /* Map screen -> view position in a stereo window.
     */
    for(i = 0; i < 2; i++) {
      wp.xmin = drawerstate.winpos.xmin + dv->vp[i].xmin;
      wp.xmax = wp.xmin + dv->vp[i].xmax - dv->vp[i].xmin;
      wp.ymin = drawerstate.winpos.ymin + dv->vp[i].ymin;
      wp.ymax = wp.ymin + dv->vp[i].ymax - dv->vp[i].ymin;
      mousemap(x, y, &xpick, &ypick, &wp);
      if(fabs(xpick) <= 1 && fabs(ypick) <= 1)
	break;
    }
  }

  { /* Hacks for setting up transforms so INST ... location / origin works.
     * This'll be unnecessary if we switch over to using an mg "pick" device.
     * slevy 96.10.26.
     */
    Transform Tc2n, Tw2n, Ts2n;
    CamViewProjection( dv->cam, Tc2n );
    CamView( dv->cam, Tw2n );
    TmTranslate(Ts2n, -1.0, -1.0, 0.0);
    CtmScale(Ts2n, 2.0/(wp.xmax-wp.xmin+1), 2.0/(wp.ymax-wp.ymin+1), 1.0);
    PickSet( pick, PA_TC2N, Tc2n, PA_TW2N, Tw2n, PA_TS2N, Ts2n, PA_END );
  }

  if (drawerstate.NDim > 0) {
    TransformN *V, *W, *Tnet, *Tmodel, *Tworld;
    
    V = drawer_ND_CamView(dv, NULL);
    
    if (dv->Item != drawerstate.universe) {
      /* Picking in a window with a dedicated Scene */
      /* We yield results in the Scene's coordinate system */
      if (GeomMousePick(dv->Item, pick, (Appearance *)NULL,
			NULL, V, dv->NDPerm, xpick, ypick)) {
	chosen = dv->id;
      }
      return chosen;
    }

    /* Picking in the real world */
    Tworld = drawer_get_ND_transform(WORLDGEOM, UNIVERSE);
    TmNConcat(Tworld, V, V); /* world -> screen */
    TmNDelete(Tworld);
    
    /*  Now V contains the world -> screen projection */
    LOOPSOMEGEOMS(i,dg,ORDINARY) {
      if (dg->pickable) {
	Geom *g = NULL;
	int id = GEOMID(i);
	
	if (dg->Lgeom) {
	  Tmodel = drawer_get_ND_transform(id, WORLDGEOM);
	  Tnet = TmNConcat(Tmodel, V, NULL); /* Now Tnet geom -> screen */
	  GeomGet(dg->Lgeom, CR_GEOM, &g);
	  ap = drawer_get_ap(dg->id);
	  if (GeomMousePick(g, pick, ap, NULL,
			    Tnet, dv->NDPerm, xpick, ypick)) {
	    chosen = id;
	    /* Arrange for things to be in world coords not Dgeom coords. */
	    pick->TwN = TmNConcat(pick->TwN, Tmodel, pick->TwN);
	    W = drawer_get_ND_transform(WORLDGEOM, id);
	    pick->TselfN = TmNConcat(pick->TwN, W, pick->TselfN);
	    TmNDelete(W);
	  }
	  TmNDelete(Tmodel);
	  TmNDelete(Tnet);
	  ApDelete(ap);
	}
      }
    }

    TmNDelete(V);

    return chosen;
  }

  CamView( dv->cam, V );	/* V = camera-to-screen matrix
				 * cH: wrong. V = universe-to-screen matrix
				 */

  if(dv->Item != drawerstate.universe) {
    /* Picking in a window with a dedicated Scene */
    /* We yield results in the Scene's coordinate system */
    /* Is this really correct? Why should we call GeomPosition() here? 
     * dv->Item is just a normal geometry, only by chance a
     * single-element INST.
     */
#if 0
    GeomPosition( dv->Item, T );
    TmConcat(T,V, T);		/* T = Scene to screen projection */
    if(GeomMousePick( dv->Item, pick, (Appearance *)NULL, T, xpick, ypick )) {
      chosen = dv->id;
    }
#else
    if (GeomMousePick( dv->Item, pick, (Appearance *)NULL,
		       V, NULL, NULL, xpick, ypick )) {
      chosen = dv->id;
    }
#endif
    return chosen;
  }

  /* Picking in the real world */
  GeomPosition(drawerstate.world, Tworld); /* world -> universe */
  TmConcat(Tworld, V, T); /* world -> screen */
  /*
   * We now assume the complete screen -> DGeom transform is in T.
   * This is true only if we have just a single level of DGeom's in the world.
   *
   * cH: this is wrong, T is the World -> screen projection, not the
   * other way round. (Transforms operate from the right!).
   */
  LOOPSOMEGEOMS(i,dg,ORDINARY) {
    if (dg->pickable) {
      Geom *g = NULL;
      int id = GEOMID(i);

      if (dg->Lgeom) {
	GeomPosition( dg->Item, Tmodel );
	GeomPosition( dg->Inorm, Tnorm );
	TmConcat( Tnorm, Tmodel, Tt );
	TmConcat( Tt, T, Tnet ); /* Now Tnet = complete geom-to-screen proj'n */
	GeomGet( dg->Lgeom, CR_GEOM, &g );
	ap = drawer_get_ap(dg->id);
	if (GeomMousePick( g, pick, ap, Tnet, NULL, NULL, xpick, ypick )) {
	  chosen = id;
	  /* We remember oldTw to print out info below for debugging only */
	  TmCopy(pick->Tw, oldTw);
	  /* This is necessary!  Arranges for things to be in world
	   * coords not Dgeom coords. Tt is the dgeom-to-world
	   * transform, Tw is (more or less) screen -> dgeom
	   */
	  TmConcat(pick->Tw, Tt, pick->Tw);
	  drawer_get_transform(WORLDGEOM, pick->Tself, id);
	  TmConcat(pick->Tw, pick->Tself, pick->Tself);
	}
	ApDelete(ap);
      }
    }
  }

  /* Ok, everything below is just debugging stuff */
  if (chosen == NOID) {
    /*    printf("Picked nothing.\n"); */
  } else {
    /*    printf("Picked dgeom #%d\n", INDEXOF(chosen)); */
    
    /* pick->got is in mouse coords.
       wgot is world coords.
       old world is really dgeom coords. (maybe...)
       got is raw object coords. (the kind of numbers in geom data file!)
    
    */
    if (pick && getenv("VERBOSE_PICK")) {
      Point3 got, v, e[2], wgot, wv, we[2], owgot, owv, owe[2];
	
      Pt3Transform(pick->Tmirp, &(pick->got), &got);
      Pt3Transform(pick->Tw, &(pick->got), &wgot);
      Pt3Transform(oldTw, &(pick->got), &owgot);
	
      printf("pick->\n");
      printf("  got = (%f %f %f)\n",
	     pick->got.x, pick->got.y, pick->got.z);
      if (pick->found&PW_VERT)
	printf("    v = (%f %f %f)\n",
	       pick->v.x, pick->v.y, pick->v.z);
      if (pick->found&PW_EDGE) {
	printf(" e[0] = (%f %f %f)\n",
	       pick->e[0].x, pick->e[0].y, pick->e[0].z);
	printf(" e[1] = (%f %f %f)\n",
	       pick->e[1].x, pick->e[1].y, pick->e[1].z);
      }
	
      printf("Transformed pick [raw]->\n");
      printf("  got = (%f %f %f)\n", got.x, got.y, got.z);
      if (pick->found&PW_VERT) {
	HPt3TransPt3(pick->Tmirp, &(pick->v), &v);
	printf("    v = (%f %f %f)\n", v.x, v.y, v.z);
      }
      if (pick->found&PW_EDGE) {
	HPt3TransPt3(pick->Tmirp, &(pick->e[0]), &(e[0]));
	HPt3TransPt3(pick->Tmirp, &(pick->e[1]), &(e[1]));
	printf(" e[0] = (%f %f %f)\n", e[0].x, e[0].y, e[0].z);
	printf(" e[1] = (%f %f %f)\n", e[1].x, e[1].y, e[1].z);
      }
	
      printf("Transformed pick [old world]->\n");
      printf("  got = (%f %f %f)\n", owgot.x, owgot.y, owgot.z);
      if (pick->found&PW_VERT) {
	HPt3TransPt3(oldTw, &(pick->v), &owv);
	printf("    v = (%f %f %f)\n", owv.x, owv.y, owv.z);
      }
      if (pick->found&PW_EDGE) {
	HPt3TransPt3(oldTw, &(pick->e[0]), &(owe[0]));
	HPt3TransPt3(oldTw, &(pick->e[1]), &(owe[1]));
	printf(" e[0] = (%f %f %f)\n", owe[0].x, owe[0].y, owe[0].z);
	printf(" e[1] = (%f %f %f)\n", owe[1].x, owe[1].y, owe[1].z);

	printf("Transformed pick [world]->\n");
	printf("  got = (%f %f %f)\n", wgot.x, wgot.y, wgot.z);
	if (pick->found&PW_VERT) {
	  HPt3TransPt3(pick->Tw, &(pick->v), &wv);
	  printf("    v = (%f %f %f)\n", wv.x, wv.y, wv.z);
	}
	if (pick->found&PW_EDGE) {
	  HPt3TransPt3(pick->Tw, &(pick->e[0]), &(we[0]));
	  HPt3TransPt3(pick->Tw, &(pick->e[1]), &(we[1]));
	  printf(" e[0] = (%f %f %f)\n", we[0].x, we[0].y, we[0].z);
	  printf(" e[1] = (%f %f %f)\n", we[1].x, we[1].y, we[1].z);
	}


      }
    }
  }
  return chosen;
}

LDEFINE(rawpick, LINT,
	"(rawpick CAMID X Y)\n\
	Process a pick event in camera CAMID at location (X,Y) given in\n\
	integer pixel coordinates.  This is a low-level procedure not\n\
	intended for external use.")
{
  int pickedid, id, x, y;
  Pick *pick;

  LDECLARE(("rawpick", LBEGIN,
	    LID, &id,
	    LINT, &x,
	    LINT, &y,
	    LEND));
  if (TYPEOF(id) != T_CAM) {
    fprintf(stderr, "rawpick: first arg must be a camera id\n");
    return Lnil;
  }

  pick = PickSet(NULL, PA_WANT, PW_EDGE|PW_VERT|PW_FACE, PA_END);
  pickedid= view_pick( (DView *)drawer_get_object(id), x, y, pick );

  if (pickedid != NOID) {
    emit_pick(pickedid, pick);
  }
  PickDelete(pick);
  return LNew(LINT, &pickedid);
}

void
emit_pick(int pickedid, Pick *pick)
{
  /* Variables for total hack */
  vvec done;
  char donebits[512];

  LInterest *interest = LInterestList("pick");

  VVINIT(done, char, 128);
  if(interest) {
    vvuse(&done, donebits, COUNT(donebits));
    vvzero(&done);
  }

#define DONEID(id) *VVINDEX(done, char, id-CAMID(-20))

  for ( ; interest != NULL; interest = interest->next) {
    int coordsysid;

    /* extract the coord system to use from the interest filter;
       if none given, use world */
    if (interest->filter
	&& interest->filter->car
	&& (LFILTERVAL(interest->filter->car)->flag == VAL)) {
      if (!LFROMOBJ(LID)(LFILTERVAL(interest->filter->car)->value,
			 &coordsysid)) {
	OOGLError(0,"emit_pick: bad coord sys filter type");
	continue;
      }
    } else {
      coordsysid = WORLDGEOM;
    }

    if (drawerstate.NDim > 0) {
      /* In an ND-context, we report the 3d-quantities picked in the
       * 3d sub-space of the camera where the pick occurred. The
       * interested parties still have access to the ND-co-ordinates
       * by means of the vertex/edge/face indices.
       *
       * We use TmNMap() to map the 3d vertices to the full Nd-space.
       */
      int dim = drawerstate.NDim;
      TransformN *T, *tmp;
      HPointN *got = NULL;
      HPointN *v = NULL;
      VARARRAY(e, HPtNCoord, 2*dim);
      VARARRAY(f, HPtNCoord, pick->found & PW_FACE ? pick->fn * dim : 0);
      int gn, vn, vi, en, ei[2], ein, fn, fi;

      /*  T = transform converting to the coord system of coordsysid */
      /* This section does the setup for the total hack */

      /* Total hack gigantic if statement */
      if (!DONEID(coordsysid)) {
	DONEID(coordsysid) = 1;
	switch(coordsysid) {
	case WORLDGEOM:
	  T = TmNMap(pick->TwN, pick->axes, NULL);
	  break;
	case PRIMITIVE:
	  T = TmNMap(pick->TmirpN, pick->axes, NULL);
	  break;
	case SELF:
	  T = TmNMap(pick->TselfN, pick->axes, NULL);
	  break;
	default:
	  tmp = drawer_get_ND_transform(WORLDGEOM, coordsysid);
	  T = TmNConcat(pick->TwN, tmp, NULL);
	  TmNDelete(tmp);
	  TmNMap(T, pick->axes, T);
	  break;
	}

	if (pickedid != NOID) {
	  HPoint3 gothpt4;

	  Pt3ToHPt3(&pick->got, &gothpt4, 1);
	  got = HPt3NTransform(T, &gothpt4, NULL);
	  gn = got->dim;
#if 1
	  {
	    HPointN *gotn;
	    
	    gotn = HPt3ToHPtN(&gothpt4, pick->axes, NULL);
	    HPtNTransform(pick->TwN, gotn, gotn);
	    HPtNDelete(gotn);
	  }
#endif
	} else {
	  got = NULL;
	  gn = 0;
	}
      
	if (pick->found & PW_VERT) {
	  v = HPt3NTransform(T, &pick->v, NULL);
	  vn = v->dim;
	  vi = pick->vi;
	} else {
	  v = NULL;
	  vn = 0;
	  vi = -1;
	}
      
	if (pick->found & PW_EDGE) {
	  HPointN tmp;
	  
	  tmp.dim   = T->odim;
	  tmp.flags = 0;

	  tmp.v = e+0;   HPt3NTransform(T, &pick->e[0], &tmp);
	  tmp.v = e+dim; HPt3NTransform(T, &pick->e[1], &tmp);
	  en = 2*dim;
	  ei[0] = pick->ei[0];
	  ei[1] = pick->ei[1];
	  ein = 2;
	} else {
	  en = 0;
	  ein = 0;
	}
	if (pick->found & PW_FACE) {
	  HPointN tmp;
	  int i;
	  
	  tmp.dim   = T->odim;
	  tmp.flags = 0;
	  for (i = 0; i < pick->fn; i++) {
	    tmp.v = &f[i*dim];
	    HPt3NTransform(T, pick->f+i, &tmp);
	  }
	  fi = pick->fi;
	  fn = pick->fn * dim;
	} else {
	  fn = 0;
	  fi = -1;
	}
      
	/* Cause of total hack.
	 * This CANNOT be called once for every interested party - otherwise
	 * every interested party will hear about it numerous times. */
	gv_pick(coordsysid, pickedid,
		got ? got->v : NULL, gn,
		v ? v->v : NULL, vn,
		e, en,
		f, fn,
		VVEC(pick->gpath, int), VVCOUNT(pick->gpath),
		vi,
		ei, ein,
		fi);

	HPtNDelete(got);
	HPtNDelete(v);
	TmNDelete(T);

      } /* End of total hack if statement */

    } else {
      /*  T = transform converting to the coord system of coordsysid */
      /* This section does the setup for the total hack */    
      Transform T;
      HPoint3 got;
      HPoint3 v, e[2];
      HPoint3 *f;
      int gn, vn, vi, en, ei[2], ein, fn, fi;

      /* Total hack gigantic if statement */
      if (!DONEID(coordsysid)) {
	DONEID(coordsysid) = 1;
	switch(coordsysid) {
	case WORLDGEOM:
	  TmCopy(pick->Tw, T); break;
	case PRIMITIVE:
	  TmCopy(pick->Tmirp, T); break;
	case SELF:
	  TmCopy(pick->Tself, T); break;
	default:
	  drawer_get_transform(WORLDGEOM, T, coordsysid);
	  TmConcat(pick->Tw, T, T);
	  break;
	}

	if (pickedid != NOID) {
	  Pt3Transform(T, &pick->got, HPoint3Point3(&got));
	  got.w = 1;
	  gn = 4;
	} else {
	  gn = 0;
	}
      
	if (pick->found & PW_VERT) {
	  HPt3Transform(T, &(pick->v), &v);
	  vn = 4;
	  vi = pick->vi;
	} else {
	  vn = 0;
	  vi = -1;
	}
      
	if (pick->found & PW_EDGE) {
	  HPt3TransformN(T, pick->e, &e[0], 2);
	  en = 8;
	  ei[0] = pick->ei[0];
	  ei[1] = pick->ei[1];
	  ein = 2;
	} else {
	  en = 0;
	  ein = 0;
	}
	if (pick->found & PW_FACE) {
	  f = OOGLNewNE(HPoint3, pick->fn, "rawpick");
	  HPt3TransformN(T, pick->f, f, pick->fn);
	  fi = pick->fi;
	  fn = pick->fn * 4;
	} else {
	  f = NULL;
	  fn = 0;
	  fi = -1;
	}
      
	/* Cause of total hack.
	 * This CANNOT be called once for every interested party - otherwise
	 * every interested party will hear about it numerous times. */
	gv_pick(coordsysid, pickedid,
		HPoint3Data(&got), gn,
		&v.x, vn,
		&e[0].x, en,
		(float *)f, fn, 	/* f, fn, */
		VVEC(pick->gpath, int), VVCOUNT(pick->gpath),
		vi,
		ei, ein,
		fi);

	if (f != NULL) OOGLFree(f);

      } /* End of total hack if statement */
      
    } /* End of 3d case */
  }
  vvfree(&done);
}

LDEFINE(pick, LVOID,
	"(pick COORDSYS GEOMID G V E F P VI EI FI)\n"
	"The pick command is executed internally in response to pick\n"
	"events (right mouse double click).\n"
	"\n\n"
	"COORDSYS = coordinate system in which coordinates of the following\n"
	"arguments are specified. This can be:"
	"\n\n\tworld: world coord sys"
	"\n\n\tself:  coord sys of the picked geom (GEOMID)"
	"\n\n\tprimitive: coord sys of the actual primitive within"
	"\n\n\t\tthe picked geom where the pick occurred."
	"\n\n\n\n"
	"GEOMID = id of picked geom"
	"\n\n\n\n"
	"G = picked point (actual intersection of pick ray with object)"
	"\n\n\n\n"
	"V = picked vertex, if any"
	"\n\n\n\n"
	"E = picked edge, if any"
	"\n\n\n\n"
	"F = picked face"
	"\n\n\n\n"
	"P = path to picked primitive [0 or more]"
	"\n\n\n\n"
	"VI = index of picked vertex in primitive"
	"\n\n\n\n"
	"EI = list of indices of endpoints of picked edge, if any"
	"\n\n\n\n"
	"FI = index of picked face"
	"\n\n\n\n"
	"External modules can find out about pick events by registering\n"
	"interest in calls to \"pick\" via the \"interest\" command."
	"\n\n\n\n"
	"In the ND-viewing context the co-ordinates are actually ND-points.\n"
	"They correspond to the 3D points of the pick relative to the\n"
	"sub-space defined by the viewport of the camera where the pick\n"
	"occurred. The co-ordinates are then padded with zeroes and\n"
	"transformed back to the co-ordinate system defined by\n"
	"\"COORDSYS\".")
{
  float *got = NULL, *v = NULL, *e = NULL, *f = NULL;
  int vi, ei[2], fi, *p = NULL;
  int gn, vn, en, fn, pn;
  int ein = 2;
  int id, coordsys;

  /* NOTE: If you change the lisp syntax of this function (which you
     shouldn't do), you must also update the DEFPICKFUNC macro in the
     file "pickfunc.h", which external modules use. */
  LDECLARE(("pick", LBEGIN,
	    LID, &coordsys,
	    LID, &id,
	    LHOLD, LVARARRAY, LFLOAT, &got, &gn,
	    LHOLD, LVARARRAY, LFLOAT, &v, &vn,
	    LHOLD, LVARARRAY, LFLOAT, &e, &en,
	    LHOLD, LVARARRAY, LFLOAT, &f, &fn,
	    LHOLD, LVARARRAY, LINT, &p, &pn,
	    LINT, &vi,
	    LHOLD, LARRAY, LINT, ei, &ein,
	    LINT, &fi,
	    LEND));

  if (got) OOGLFree(got);
  if (v) OOGLFree(v);
  if (e) OOGLFree(e);
  if (f) OOGLFree(f);
  if (p) OOGLFree(p);

  return Lt;
}

/*****************************************************************************/

LDEFINE(event_keys, LVOID,
	"(event-keys {on|off})\n\
         Turn keyboard events on or off to enable/disable keyboard shortcuts.")
{
  int on;

  LDECLARE(("event-keys", LBEGIN,
	    LKEYWORD, &on,
	    LEND));

  if (on == ON_KEYWORD)
    keyshorts = 1;
  else
    if (on == OFF_KEYWORD)
      keyshorts = 0;
    else {
      OOGLError(0, "event-keys: expected \"on\" or \"off\" keyword");
      return Lnil;
    }

  return Lt;

}

/*****************************************************************************/

LDEFINE(event_pick, LVOID,
	"(event-pick {on|off})\n\
         Turn picking on or off.")
{
  int on;

  LDECLARE(("event-pick", LBEGIN,
	    LKEYWORD, &on,
	    LEND));

  if (on == ON_KEYWORD)
    pickon = 1;
  else
    if (on == OFF_KEYWORD)
      pickon = 0;
    else {
      OOGLError(0, "event-pick: expected \"on\" or \"off\" keyword");
      return Lnil;
    }

  return Lt;
}

/*****************************************************************************/

LDEFINE(dither, LVOID,
        "(dither  CAM-ID {on|off|toggle})\n\
         Turn dithering on or off in that camera.")
{
  DView *dv;
  int id, dither, i, on = -1;

  LDECLARE(("dither", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &on,
	    LEND));

  MAYBE_LOOP(id, i, T_CAM, DView, dv) {
    if (dv->mgctx) {
      mgctxselect(dv->mgctx);
      mgctxget(MG_DITHER, &dither);
    }
    if (on == TOGGLE_KEYWORD)
      dither = !dither;
    else
      if (on == ON_KEYWORD)
	dither = 1;
      else
	if (on == OFF_KEYWORD)
	  dither = 0;
	else {
	  OOGLError(0, "dither: expected \"on\", \"off\" or \"toggle\" keyword");
	  return Lnil;
	}

    if (dv->mgctx) {
      mgctxset(MG_DITHER, dither, MG_END);
      gv_redraw(dv->id);
    }
    ui_maybe_refresh(dv->id);
  }
  return Lt;

}

/*****************************************************************************/

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
