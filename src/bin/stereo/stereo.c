/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

#include <stdio.h>
#include "forms.h"
#include "xforms-compat.h"

#ifdef XFORMS
	/* Essential GL stubs */
extern int fl_screen;	/* XFORMS internal */

enum gdesc { GD_XPMAX, GD_YPMAX, GD_XMMAX, GD_YMMAX };
enum monmode { HZ72, STR_RECT };
enum qdevices { WINQUIT, WINSHUT };
extern int getgdesc(int);
extern void setmonitor(int);
extern int getmonitor();
extern void fl_qdevice(int);
extern void unqdevice(int);

#else
#include <gl/gl.h>
#include <gl/device.h>
#include <gl/get.h>
#endif

#include <ooglutil.h>
#include <streampool.h>
#include <camera.h>
#include <window.h>
#include <lisp.h>

#include <sys/signal.h>

#include "sterui.h"

extern HandleOps CamOps, WindowOps;
static void monitor(int mode);

#define	CURRENT		0
#define	ST_MONO		1
#define	ST_CROSSEYED	2
#define	ST_HARDWARE	3
#define	ST_COLORED	4

#define	FOR_SOMETHING	0
#define	FOR_FOCUS	1
#define	FOR_REDRAW	2


#ifndef TRUE
#define TRUE 1
#endif


Camera *cam;
WnWindow *win;
int camwinseq;
int expectredraw;
Pool *io;
Lake *lake;
long mainwin = -1, helpwin = -1, morewin = -1;	/* GL window id's */

			/* Values cached from current camera & window */
int	isstereo = 0;
float	pixelaspect = 1;	/* pixel aspect */
float	camaspect = 1.3;	/* Camera aspect ratio, xsize/ysize */
float	halffield = .4;		/* half linear field (= tan(fov/2)) */
float	halfyfield = .4;	/* half Y-axis linear field */
float	focallen = 3;		/* focal length */
char	*camname = "focus";	/* Use this camera */
int	smode = ST_MONO;	/* Last known mode */
WnPosition curpos;
int	xsize, ysize;

    /* Stereo convergence angle to an object in the nominal plane of view */
float	halfconv = .1;		/* tan(current convergence angle/2) */
float	halfmaxconv = .22;	/* tan(max allowable convergence/2) */
int	swapeyes = 1;

    /* Hardware parameters */
float	pixperinch;
int	xscreen, yscreen;
float	screenwidth;

    /* Warmware parameter? */
float	ocularsep = 1.15;	/* 2*ocularsep inches between human eyes */

current_mode()
{
    if(cam == NULL || win == NULL) {
	fprintf(stderr, "Hey? -- stereo sleeping...\n");
	unqdevice(WINSHUT);
	unqdevice(WINQUIT);
	pause();
	return ST_MONO;
    }
    CamGet(cam, CAM_STEREO, &isstereo);
    CamGet(cam, CAM_HALFYFIELD, &halfyfield);
    CamGet(cam, CAM_HALFFIELD, &halffield);
    CamGet(cam, CAM_FOCUS, &focallen);
    CamGet(cam, CAM_ASPECT, &camaspect);
    WnGet(win, WN_PIXELASPECT, &pixelaspect);
    WnGet(win, WN_CURPOS, &curpos);
    xsize = curpos.xmax - curpos.xmin + 1;
    ysize = curpos.ymax - curpos.ymin + 1;
    return smode;
}

void
set_mode(int mode, int why)
{
    char *layout = "no";
    int stereo = 1;
    int gap = 0;
    int dx, hvpwidth;
    float halfxfield, newxfield, newyfield, newfocallen, newaspect, focalscale;
    int got;
    char winstuff[80];
    char camstuff[80];
    char advice[120];
    static int oldxsize = -1, oldysize;

    request_camwin();
    got = camwinseq;
    do {
	LFree( LEvalSexpr(lake) );
    } while(got == camwinseq);	/* Keep reading until we see a camera/window. */
    got = current_mode();
    if(mode == 0)
	mode = got;

    if(why == FOR_REDRAW && oldxsize == xsize && oldysize == ysize) {
	expectredraw = 0;
	return;
    }

#ifdef notdef
fprintf(stderr, "# halfyfield %.3f  halffield %.3f fov %.3f\r\n",
	halfyfield, halffield, 2*DEGREES(atan(halffield)));
#endif

    winstuff[0] = '\0';
    camstuff[0] = '\0';
    hvpwidth = xsize/2;
    newyfield = halfyfield;
    focalscale = 1;

    printf("(progn\n");

    switch(mode) {
    case ST_MONO:
	monitor(HZ72);
	pixelaspect = 1;
	layout = "no";
	stereo = 0;
	camaspect = (double)xsize / ysize;
	newxfield = camaspect>1 ? camaspect*halffield : halffield;
	break;

    case ST_CROSSEYED:
	monitor(HZ72);
	hvpwidth = xsize/4;
	dx = xsize/2;
	pixelaspect = 1;
	layout = "horizontal";
	camaspect = .5*xsize / ysize;

	/* Field-of-view is related to convergence angle in crosseyed view:
	 *  tan(xfov/2) = tan(conv/2) / (1 + ocularsep/(viewportwidth/2)).
	 * Push the camera back so as to frame the same object, assuming its
	 * "focus" distance correctly reflects the object of interest.
	 */

	halfxfield = camaspect>1 ? halffield*camaspect : halffield;
	newxfield = halfconv / (1 + ocularsep*pixperinch / hvpwidth);
	focalscale = halfxfield / newxfield;
	break;

    case ST_HARDWARE:
	monitor(STR_RECT);
	gap = 40;
	dx = 0;
	pixelaspect = .5;
	layout = "vertical";
	sprintf(winstuff, "position %d %d %d %d noborder\n",
		curpos.xmin, curpos.xmax, 0, yscreen-1);

	camaspect = (double)xsize / ((yscreen - gap) / 2);
	halfxfield = camaspect>1 ? halffield*camaspect : halffield;
	newxfield = halfconv / (pixelaspect * ocularsep*pixperinch / hvpwidth);
	focalscale = halfxfield / newxfield;
	break;

    case ST_COLORED:
	monitor(HZ72);
	gap = 0;
	dx = 0;
	pixelaspect = 1;
	layout = "colored";
	camaspect = (double)xsize / ysize;
	halfxfield = halfyfield*camaspect;
	newxfield = halfconv / (ocularsep*pixperinch / hvpwidth);
	focalscale = halfxfield / newxfield;
	break;

    default:
	fprintf(stderr, "set_mode(%d)?\n", mode);
    }

    halfyfield = newxfield/camaspect;
    halffield = newxfield < halfyfield ? newxfield : halfyfield;
    newfocallen = focallen * focalscale;
#ifdef notdef
fprintf(stderr, "# NEW halfxfield %.3f halfyfield %.3f  aspect %.3f focus %.2f  fov %.3f  hconv %.1f\r\n",
	    halfxfield, halfyfield, camaspect, newfocallen,
	    2*DEGREES(atan(halffield)), 2*DEGREES(atan(halfconv)));
#endif

    if(fabs(focalscale - 1) > .02 && why == FOR_SOMETHING && !fl_get_button(FixedCamButton)) {
	float near, far;
	CamGet(cam, CAM_NEAR, &near);
	CamGet(cam, CAM_FAR, &far);
	printf("(transform %s self self translate 0 0 %g)\n",
	    camname, newfocallen - focallen);
	sprintf(camstuff, "focus %g near %g far %g\n",
		newfocallen, near*focalscale, far*focalscale);
	focallen = newfocallen;
    }

    printf("(stereowin %s %s %d)\n", camname, layout, gap);
    printf("(merge window %s { pixelaspect %g %s })\n",
	camname, pixelaspect, winstuff);
    printf("(merge camera %s {\n\
	%s perspective 1 frameaspect %g halfyfield %g\n\
	stereyes\n\
	transform { 1 0 0 0  0 1 0 0  %g 0 1 0  %g 0 0 1 }\n\
	transform { 1 0 0 0  0 1 0 0  %g 0 1 0  %g 0 0 1 }\n\
}))",
	camname, camstuff, camaspect, halfyfield,
	-swapeyes*halfconv, -swapeyes*halfconv*focallen,
	swapeyes*halfconv, swapeyes*halfconv*focallen);
    fflush(stdout);

    /* Advise viewing from distance such that the field-of-view is correct. */
    halfxfield = halffield * (camaspect>1 ? camaspect : 1);
    sprintf(advice, "%.1f degree field of view\nBest view ~%.0f\" from screen",
	2*DEGREES(atan(halffield)), hvpwidth/pixperinch / halfxfield);

    fl_set_object_label(BestViewText, advice);

    isstereo = stereo;
    expectredraw = 1;
    oldxsize = xsize;
    oldysize = ysize;

    if(stereo)
	fl_show_object(SwapButton);
    else
	fl_hide_object(SwapButton);
    if(fl_get_browser(StereoBrowser) != mode)
	fl_select_browser_line(StereoBrowser, mode);
}

void
ConvProc(FL_OBJECT *obj, long arg)
{
    float v = fl_get_slider_value(obj);

    halfconv = tan(RADIANS(v/2));
    if(isstereo)
	set_mode(CURRENT, FOR_SOMETHING);
}

void
StereoProc(FL_OBJECT *obj, long arg)
{
    set_mode(fl_get_browser(obj), FOR_SOMETHING);
}

void
SwapProc(FL_OBJECT *obj, long arg)
{
    swapeyes = fl_get_button(obj) ? -1 : 1;
    set_mode(CURRENT, FOR_SOMETHING);
}


LDEFINE(stereowin, LVOID, "")
{
    char *cam = NULL, *mode = NULL;
    LDECLARE(("stereowin", LBEGIN,
	LSTRING, &cam,
	LSTRING, &mode,
	LREST, NULL,
	LEND));
    if(mode == NULL)
	return Lnil;
    switch(mode[0]) {
    case 'n': smode = ST_MONO; break;		/* "no" */
    case 'h': smode = ST_CROSSEYED; break;	/* "horizontal" */
    case 'v': smode = ST_HARDWARE; break;	/* "vertical" */
    case 'c': smode = ST_COLORED; break;	/* "colored" */
    default:
	fprintf(stderr,
	  "stereo: Can't understand  '(stereowin %s %s)' from geomview.\n",
		cam, mode);
    }
    return Lt;
}


LDEFINE(redraw, LVOID, "")
{
    LDECLARE(("redraw", LBEGIN,
	LREST, NULL,
	LEND));

    if(expectredraw-- <= 0) {
	expectredraw = 1;		/* Prevent reentrant calls */
	set_mode(CURRENT, FOR_REDRAW);
    }
    return Lt;
}


static char simplepick[] = "(pick %s nil * nil nil nil nil nil nil nil)";

LDEFINE(pick, LVOID, "")
{
    float z;
    HPoint3 pt;
    char *me, *it;
    int npt = 4;
    static int picking = 0;

    LDECLARE(("pick", LBEGIN,
	LSTRING, &me,
	LSTRING, &it,
	LHOLD, LARRAY, LFLOAT, &pt, &npt,
	LREST, NULL,
	LEND));

    fl_set_object_boxtype(FocalButton, FL_UP_BOX);
    if(picking) return Lnil;
    picking = 1;

    printf("(progn (uninterest\n");
    printf(simplepick, camname);
    printf(")");
    if(npt == 4) {
	printf("(merge camera %s { focus %g }))\n", camname, -pt.z / pt.w);
	fflush(stdout);
	set_mode(CURRENT, FOR_FOCUS);
    } else {
	printf(")\n");	/* Close the progn */
	fflush(stdout);
    }
    picking = 0;
    return Lt;
}

static char simplemerge[] = "(merge camera %s *)";

LDEFINE(merge, LVOID,
       "(merge camera CAM-ID  { CAMERA ... } )")
{
  char *opsname = NULL;
  int c, id;
  LObject *kw = NULL, *idarg = NULL;
  float newfocallen = focallen;

  if (lake == NULL)
    return Lt;

  /* All the work of this function is done at parse time. */

  /* parse first arg [ops]: */
  if (! LakeMore(lake,c) || (kw = LSexpr(lake)) == Lnil ||
      !LFROMOBJ(LSTRING)(kw, &opsname) || strcmp(opsname, "camera") != 0) {
    OOGLSyntax(stdin, "merge: expected \"camera\", got \"%s\"", opsname);
    goto parsefail;
  }

  /* parse 2nd arg; it's a string (id) */
  if (! LakeMore(lake,c) || (idarg = LEvalSexpr(lake)) == Lnil) {
    OOGLSyntax(stdin,"\"merge\": expected CAM-ID");
    goto parsefail;
  }


  cam = NULL;
  if(CamOps.strmin(POOL(lake), NULL, (Ref **)&cam) == 0) {
    OOGLSyntax(stdin, "\"merge\": error reading camera");
    goto parsefail;
  }

  CamGet(cam, CAM_FOCUS, &focallen);
  if(fabs(focallen - newfocallen) > .01) {
    focallen = newfocallen;
    if(!expectredraw)
	set_mode(CURRENT, FOR_FOCUS);
  }
 parsefail:
  LFree(kw);
  LFree(idarg);
  return Lt;
}

request_camwin()
{
    printf("\
(progn (echo (stereowin %s)) (echo \"(camwin \") (write camera - %s) (write window - %s) (echo \"\\n)\\n\") )\n",
	camname, camname, camname);
    fflush(stdout);
}

LDEFINE(camwin, LVOID, "")
{
    if(lake == NULL) return Lt;
    camwinseq++;
    if(CamStreamIn(POOL(lake), NULL, &cam) <= 0) {
	OOGLSyntax(stdin, "stereo: couldn't read camera");
	return Lnil;
    }
    if(WnStreamIn(POOL(lake), NULL, &win) <= 0) {
	OOGLSyntax(stdin, "stereo: couldn't read window");
	return Lnil;
    }
    return Lt;
}

void
FocalProc(FL_OBJECT *obj, long arg)
{
    printf("(interest ");
    printf(simplepick, camname);
    printf(")\n");
    fflush(stdout);
    fl_set_object_label(BestViewText, "Click right mouse on point");
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
}

static char *HelpText[] = {
#include "help.h"
};

void HelpProc(FL_OBJECT *obj, long arg)
{ helpwin = fl_show_form(Help, FL_PLACE_SIZE, TRUE, "Stereo Help"); }

int
validinput(FL_OBJECT *obj, float *val)
{
  char *s = (char *)fl_get_input(obj);
  char *s0 = s;
  float v;
  v = strtod(s, &s);
  if(s == s0) return 0;
  if(*val == v) return -1;
  *val = v;
  return 1;
}

void updateMore()
{
  char s[80];
  fl_freeze_form(More);
  sprintf(s, "%.2f", screenwidth);
  fl_set_input(ScreenWidthInput, s);
  sprintf(s, "%.2f", 2*ocularsep);
  fl_set_input(OcularInput, s);
  fl_unfreeze_form(More);
}

void
OcularSepProc(FL_OBJECT *obj, long arg)
{
  if(validinput(obj, &ocularsep) > 0) {
    ocularsep *= .5;
    set_mode(CURRENT, FOR_SOMETHING);
  }
  updateMore();
}

void
ScreenWidthProc(FL_OBJECT *obj, long arg)
{
  if(validinput(obj, &screenwidth) > 0 && screenwidth > 0) {
    pixperinch = xscreen / screenwidth;
    set_mode(CURRENT, FOR_SOMETHING);
  }
  updateMore();
}

void
CamNameProc(FL_OBJECT *obj, long arg)
{
  char *is = (char *)fl_get_input(CamNameInput);
  if(strcmp(camname, is) && is[0] != '\0') {
    camname = is;
    request_camwin();
  }
}

void MoreProc(FL_OBJECT *obj, long arg)
{
  updateMore();
  morewin = fl_show_form(More, FL_PLACE_SIZE, TRUE, "Stereo Parameters");
}

static void
monitor(int mode)
{
    if((mode == STR_RECT) != (getmonitor() == STR_RECT) && !getenv("NOMON"))
	setmonitor(mode);
}

void
die(int sig)
{
    monitor(HZ72);
    exit(0);
}

void
QuitProc(FL_OBJECT *obj, long arg)
{
    die(0);
}

void
DoneProc(FL_OBJECT *obj, long arg)
{
   fl_hide_form(obj->form);
}

main(int argc, char *argv[])
{
    int i, c;

    io = PoolStreamOpen("stdio", stdin, 0, &CamOps);
    PoolStreamOpen("stdio", stdout, 1, &CamOps);
    lake = LakeDefine(stdin, stdout, io);

    LInit();
    LDefun("pick", Lpick, Hpick);
    LDefun("redraw", Lredraw, Hredraw);
    LDefun("merge", Lmerge, Hmerge);
    LDefun("camwin", Lcamwin, Hcamwin);
    LDefun("stereowin", Lstereowin, Hstereowin);

#ifdef XFORMS
    FL_INITIALIZE("Stereo");
#else
    foreground();
    fl_init();
#endif
    create_the_forms();


    signal(SIGHUP, die);
    signal(SIGINT, die);

	/* This order must match that of the ST_* define's */
    fl_add_browser_line(StereoBrowser, "Monoscopic");
    fl_add_browser_line(StereoBrowser, "Crosseyed");
    fl_add_browser_line(StereoBrowser, "Hardware");
    fl_add_browser_line(StereoBrowser, "Red/Cyan");

    fl_set_slider_bounds(ConvSlider, 1.5, DEGREES(atan(halfmaxconv)));
    fl_set_slider_value(ConvSlider, 2*DEGREES(atan(halfconv)));
    mainwin = fl_show_form(stereo, FL_PLACE_SIZE, TRUE, "Stereo View");
    fl_qdevice(WINSHUT);
    fl_qdevice(WINQUIT);

    fl_set_input(CamNameInput, camname);

    for(i = 0; i < COUNT(HelpText); i++)
	fl_add_browser_line(HelpBrowser, HelpText[i]);

	/* Screen pixel density */
    xscreen = getgdesc(GD_XPMAX);
    yscreen = getgdesc(GD_YPMAX);
    screenwidth = getgdesc(GD_XMMAX) * .03937;
    pixperinch = xscreen / screenwidth;

    printf("(interest (redraw))\n");
    printf("(interest (merge camera))\n");
    request_camwin();
    for(i = 1; i < argc; i++) {
	if(!strcmp(argv[i], "-fixed")) {
	    fl_set_button(FixedCamButton, 1);
	} else if(!strcmp(argv[i], "-unfixed")) {
	    fl_set_button(FixedCamButton, 0);
	} else if(!strncmp(argv[i], "-cam", 4)) {	/* -cam or -camera */
	    fl_set_input(CamNameInput, argv[++i]);
	    CamNameProc(CamNameInput, 0);
	} else if(!strcmp(argv[i], "-mode")) {
	    /* ... */
	}
    }

    for(;;) {
	if((c = async_fnextc(stdin, 0)) == NODATA) {
	    static struct timeval tenth = { 0, 100000 }; /* 0.1 sec */
	    select(0, NULL, NULL, NULL, &tenth);
	} else {
	    /* Got a Lisp expression */
	    c = camwinseq;
	    LFree(LEvalSexpr(lake));
	    /* If it was a camera/window pair, ... */
	    if(c != camwinseq)
		set_mode(current_mode(), FOR_REDRAW);
	}
	fl_check_forms();

#ifndef XFORMS
	if(fl_qtest()) {
	    short v;

	    switch( fl_qread(&v) ) {
	    case WINQUIT:
		die(0);

	    case WINSHUT:
		if(v == mainwin) die(0);
		else if(v == helpwin) fl_hide_form(Help), helpwin = -1;
		else if(v == morewin) fl_hide_form(More), morewin = -1;
	    }
	}
#endif

    }
}

#ifdef XFORMS

int getgdesc(int what) {
    Screen *s = ScreenOfDisplay(fl_display, fl_screen);

    switch(what) {
	case GD_XPMAX: return WidthOfScreen(s);
	case GD_YPMAX: return HeightOfScreen(s);
	case GD_XMMAX: return WidthMMOfScreen(s);
	case GD_YMMAX: return HeightMMOfScreen(s);
	default: return 0;
    }
}

void fl_qdevice(int dev) { /* Ignored */; }
void unqdevice(int dev)  { /* Ignored */; }

static int currentmonitor = -1;	/* Initially unknown */

int getmonitor(type) { return currentmonitor; }

void setmonitor(int mode) {
    char cmd[128];
    if(mode != currentmonitor) {
	sprintf(cmd, "/usr/gfx/setmon -n %s >&2",
			mode==STR_RECT ? "STR_RECT" : "72HZ");
	system(cmd);
	currentmonitor = mode;
    }
}
#endif
