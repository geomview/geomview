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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "mg.h"
#include "mouse.h"
#include "drawer.h"
#include "event.h"
#include "ui.h"
#include "motion.h"
#include "transform.h"
#include "space.h"
#include "comm.h"
#include "lang.h"

typedef int (*ControlFunc)(int action, float x,float y,float t, float dx,float dy,float dt);



typedef struct boundctl {
  char *buttons;
  ControlFunc function;
  float rate;
} boundctl;

/* These macros are passed to DTC and DSC as "functions".  As a special
   case you can pass expressions like ".5 * X" which works because it
   expands properly.  Be careful! */

#undef X
#undef Y
#undef Z

#define	ANGULAR 0
#define	LINEAR  1

#define X mot.x
#define Y mot.y
#define	Z mot.z
#define ZERO  0

#define	FAST 1.0
#define SLOW 0.1


static int minterp_switch(Event *event, boundctl *ctls);

/* Define Transform Control */
#define DTC(name, interp, type, moving, center, frame, fx, fy, fz)	\
int name(int action, float x, float y, float t, float dx, float dy, float dt) \
{									\
  Point3 mot;								\
  int useframe = interp(action, center, frame, dx, dy, &mot);		\
									\
  D1PRINT(("%s: action=%3d x=%8f y=%8f t=%8f dx=%8f dy=%8f dt=%8f\n",	\
	   #name, action, x, y, t, dx, dy, dt));			\
									\
  (uistate.inertia ? gv_transform_incr : gv_transform)			\
    ( moving, center, useframe, type, fx, fy, fz, dt, NO_KEYWORD );	\
  return 1;								\
}

/* Define Scale Control */
#define DSC(name, func, id, f)						      \
int name(int action, float x, float y, float t, float dx, float dy, float dt) \
{									      \
  if (action > 0) /* button went down */				      \
    dx = dy = 0;							      \
  func( id,  f(dx,dy) );                                                      \
  return 1;								      \
}

static void maybe_scale(int id, float s)
{
  /* If ND-viewing is active, then a scaling motion is very confusing
     because it only applies to one 3d sub-space. Instead, I change
     the behaviour of the "scale" button to directly change the
     ND-transform of "id". FIXME. We should handle this case as motion,
     but that would require quite some hacks. cH.
   */
  if (drawerstate.NDim > 0) {
    gv_scale(id, s, s, s);
  } else {
    /* only allow mouse scaling in Euclidean space */
    if (spaceof(id) == TM_EUCLIDEAN) {
      gv_transform(id, CENTERID, FOCUSID,
		   SCALE_KEYWORD, s, s, s, 0, NO_KEYWORD);
    }
    return;
  }
}

static float scaleexp(float dx, float dy)
{
  return exp((dx+dy)/2);
}

/*
 * Construct "mot" motion-scaling vector:
 *   mot->z scales motions into the screen: is the frame camera's focal length.
 *   mot->x and mot->y scale motions in the screen plane:
 *	if 'linear', they're spatial distances (for translations);
 *	otherwise, they're tangents of half-angles (for rotations).
 */
static float getcaminfo(int frame, int linear, Point3 *mot)
{
  float aspect = 1;
  float focallen = 3;
  int perspective = 1;
  float halfyfield = .5;
  DView *dv = (DView *)drawer_get_object(frame);
  if(dv != NULL && ISCAM(dv->id)) {
    CamGet(dv->cam, CAM_ASPECT, &aspect);
    CamGet(dv->cam, CAM_FOCUS, &focallen);
    CamGet(dv->cam, CAM_PERSPECTIVE, &perspective); 
    CamGet(dv->cam, CAM_HALFYFIELD, &halfyfield);
  }
  if(perspective && linear) halfyfield *= focallen;
  else if(!perspective && !linear) halfyfield /= focallen;
  mot->z = focallen;
  mot->y = halfyfield;
  mot->x = halfyfield * aspect;
  return aspect;
}

/*
 * Apply constraints to motion.
 * Change sign if we're moving in the object's own system.
 * Return the appropriate frame of motion (the camera or SELF).
 */
static int constrain(Point3 *mot, int framecamera)
{
  if(uistate.constrained) {
    if(fabs(mot->x) < fabs(mot->y)) mot->x = 0;
    else mot->y = 0;
  }
  if(uistate.ownmotion)
    return SELF;
  return framecamera;
}

static int
ROTATION(int action, int center, int frame, float dx, float dy, Point3 *mot)
{
  if (action > 0) /* button went down */
    mot->x = mot->y = mot->z = 0;
  else {
    float aspect = getcaminfo(frame, ANGULAR, mot);
    if(drawer_idmatch(center, frame)) {
	/* If we're rotating about the camera itself (e.g. in fly mode), try to
	 * track the mouse: scale rotation according to angular field of view.
	 */
	mot->x *= -dy / aspect;
	mot->y *= dx * aspect;
    } else {
	/* Otherwise, just adjust for non-square windows & hope for the best. */
	mot->x = -dy;
	mot->y = dx * aspect;
    }
    mot->z = .5*M_PI*(dx+dy);	/* Scale Z-axis motion according to mouse x+y */
				/* One full window width = half rotation */
  }
  return constrain(mot, frame);
}

static int
TRANSLATION(int action, int center, int frame, float dx, float dy, Point3 *mot)
{
  if(action > 0)  /* button went down */
    mot->x = mot->y = mot->z = 0;
  else {
    getcaminfo(frame, LINEAR, mot);
    mot->x *= dx;
    mot->y *= dy;
    if(spaceof(frame) != TM_EUCLIDEAN)
	mot->z = 1;	/* This is arbitrary; avoid moving too fast. */
    mot->z *= .125 * (dx + dy);
  }
  return constrain(mot, frame);
}

static int
STRANSLATION(int action, int center, int frame, float dx, float dy, Point3 *mot)
{
  if(action > 0)  /* button went down */
    mot->x = mot->y = mot->z = 0;
  else {
    getcaminfo(frame, LINEAR, mot);
    mot->x *= dx;
    mot->y *= dy;
    mot->z = .125 * sinh(dx + dy);
  }
  return constrain(mot, frame);
}

#if 0
static int
SCALING(int action, int center, int frame, float dx, float dy, Point3 *mot)
{
  float scale;
  if(action > 0)  /* button went down */
    mot->x = mot->y = mot->z = 1;
  else {
    scale = .5*(dx + dy);
    mot->x = mot->y = mot->z = 1+scale;
  }
  return uistate.ownmotion ? SELF : frame;
}
#endif

/*
Kludge to make mouse-driven hyperbolic translations work correctly.
If the selected center is literally "target", then translations
are done with respect to the universe origin, rather than the object's
origin.  This is harmless in euclidean mode and  important in hyperbolic mode
so that wild spinning behavior does not occur. This change was originally 
in 1.70.1.1, and got lost in the shuffle. (Stuart and Tamara 5/14/93)

Thus, Use TCENTERID as center for all translations.
*/

#define TCENTERID (uistate.centerid == TARGETID && \
		    drawerstate.space == TM_HYPERBOLIC ? UNIVERSE : CENTERID)

/*
 * CONTROLS:
 * A control is a function which interprets a single mouse button-combination
 * action.  Controls don't know which button was pressed.
 * Eventually, picking should be a control.
*/

/*
 * PLEASE, DO NOT CHANGE the controls without reading these naming conventions.
 *
 * TEST any changes made in hyperbolic and spherical space before installation
 *
 * the naming convention for controls:
 * controls start with 'ctl_'
 * next comes a letter describing the kind of motion,
 *  e.g. 'r' for rotation or 't' for translation
 * next comes letters describing which axes are involved.
 * next comes three letters describing which objects are involved.
 * 'f' stands for FOCUSID, 'c' stands for CENTERID, 't' stands for TARGETID,
 * 'g' stands for GEOMID(uistate.targetgeom), and
 * 'C' stands for CAMID(uistate.targetcam). (there are none of these now)
 * finally come letters describing how the input is transformed into the
 * movement:
 * 'l' stands for linear
 */
DTC(ctl_rxy_tcf_l, ROTATION, ROTATE_KEYWORD, TARGETID, CENTERID, FOCUSID,
    X, Y, ZERO)

DTC(ctl_rz_tcf_l, ROTATION, ROTATE_KEYWORD, TARGETID, CENTERID, FOCUSID,
    ZERO, ZERO, Z)

DTC(ctl_txy_tcf_l, TRANSLATION, TRANSLATE_KEYWORD, TARGETID, TCENTERID, FOCUSID,
    X, Y, ZERO)

DTC(ctl_tz_tcf_l, TRANSLATION, TRANSLATE_KEYWORD, TARGETID, TCENTERID, FOCUSID,
    ZERO, ZERO, Z)

DTC(ctl_tsxy_tcf_l, STRANSLATION, TRANSLATE_SCALED_KEYWORD, TARGETID, TCENTERID, FOCUSID,
    X, Y, ZERO)

DTC(ctl_tsz_tcf_l, STRANSLATION, TRANSLATE_SCALED_KEYWORD, TARGETID, TCENTERID, FOCUSID,
    ZERO, ZERO, Z)

DTC(ctl_rxy_fff_l, ROTATION, ROTATE_KEYWORD, FOCUSID, FOCUSID, FOCUSID,
    X, Y, ZERO)

DTC(ctl_tz_fff_l, TRANSLATION, TRANSLATE_KEYWORD, FOCUSID, FOCUSID, FOCUSID,
    ZERO, ZERO, -Z)

DTC(ctl_tsz_fcf_l, STRANSLATION, TRANSLATE_SCALED_KEYWORD, FOCUSID, TCENTERID, FOCUSID,
    ZERO, ZERO, -Z)

DTC(ctl_tswz_fff_l, TRANSLATION, TRANSLATE_KEYWORD,FOCUSID,FOCUSID,FOCUSID,
    ZERO, ZERO, -Z)

DTC(ctl_rxy_fcf_l, ROTATION, ROTATE_KEYWORD, FOCUSID, CENTERID, FOCUSID,
    -X, -Y, ZERO)

DSC(ctl_z_l, gv_zoom, FOCUSID, scaleexp)

   /* maybe_scale(), defined above, prevents scaling in non-Euclidean spaces */
DSC(ctl_s_l, maybe_scale, TARGETGEOMID, scaleexp)


/*
 * Bindings:
 * Controls are bound together in useful functional groups (motion modes).
 * This binding is really a part of the user interface.
 */

boundctl bound_rotations[] = {
  {"leftmouse", ctl_rxy_tcf_l, FAST},
  {"middlemouse", ctl_rz_tcf_l, FAST},
  {"shift leftmouse", ctl_rxy_tcf_l, SLOW},
  {"shift middlemouse", ctl_rz_tcf_l, SLOW},
  {NULL, NULL}
};

boundctl bound_translations[] = {
  {"leftmouse", ctl_txy_tcf_l, FAST},
  {"middlemouse", ctl_tz_tcf_l, FAST},
  {"shift leftmouse", ctl_txy_tcf_l, SLOW},
  {"shift middlemouse", ctl_tz_tcf_l, SLOW},
  {NULL, NULL}
};

boundctl bound_stranslations[] = {	/* Not used now - slevy 10/93 */
  {"leftmouse", ctl_tsxy_tcf_l, FAST},
  {"middlemouse", ctl_tsz_tcf_l, FAST},
  {"shift leftmouse", ctl_tsxy_tcf_l, SLOW},
  {"shift middlemouse", ctl_tsz_tcf_l, SLOW},
  {NULL, NULL}
};

boundctl bound_fly[] = {
  {"leftmouse", ctl_rxy_fff_l, FAST},
  {"middlemouse", ctl_tswz_fff_l, FAST},
  {"shift leftmouse", ctl_rxy_fff_l, SLOW},
  {"shift middlemouse", ctl_tswz_fff_l, SLOW},
  {NULL, NULL}
};

boundctl bound_orbit[] = {
  {"leftmouse", ctl_rxy_fcf_l, FAST},
  {"middlemouse", ctl_tsz_fcf_l, FAST},
  {"shift leftmouse", ctl_rxy_fcf_l, SLOW},
  {"shift middlemouse", ctl_tsz_fcf_l, SLOW},
  {NULL, NULL}
};

boundctl bound_zoom[] = {
  {"leftmouse", ctl_z_l, FAST},
  {NULL, NULL}
};

boundctl bound_scale[] = {
  {"leftmouse", ctl_s_l, FAST},
  {NULL, NULL}
};

int minterp_rotate(Event *event)
{ return minterp_switch(event, bound_rotations);}

int minterp_translate(Event *event) /* not currently used */
{return minterp_switch(event, bound_translations);}

int minterp_stranslate(Event *event)
{return minterp_switch(event, bound_stranslations);}

int minterp_fly(Event *event)
{return minterp_switch(event, bound_fly);}

int minterp_orbit(Event *event)
{return minterp_switch(event, bound_orbit);}

int minterp_zoom(Event *event)
{return minterp_switch(event, bound_zoom);}

int minterp_scale(Event *event)
{return minterp_switch(event, bound_scale);}
      
#if EXPERIMENTAL_MOTION_AVERAGING
/* Ring buffer for motion averaging.
 * For estimating speed for inertial motion when we release the mouse.
 * Just extrapolating the last mouse delta may fail if we're redrawing
 * very quickly.  So we keep more samples, and extrapolate motion over the
 * last redraw cycle or last human-reasonable time (.1 second),
 * whichever is longer.
 */
#define NMOTE  8		/* Must be a power of two */
#define MODMOTE(x) ((x) & (NMOTE-1))
static struct mote {
        float dx, dy;
	long dt;
} mote[NMOTE];
static int mo;
#endif


static int minterp_switch(Event *event, boundctl *ctls)
{
  int i;
  float x, y, dx, dy;
  long dt;
  char buttonsdown[100];

PRINT_EVENT(("minterp_switch", event));
D1PRINT(("interp_switch: ctls->buttons = %s\n", ctls->buttons));

  strcpy(buttonsdown, "");
  mousedisp(event, &dx, &dy, (unsigned long *)&dt, &drawerstate.winpos);
D1PRINT(("minterp_switch: mousedisp returned : dx = %10f, dy = %10f, dt = %ld\n", dx, dy, dt));
  if(dt <= 0 && !(dt == 0 && dx == 0 && dy == 0)) {
     /* Discredit it if --
      *  time is flowing backwards, or
      *  we're being asked for finite motion in zero time.
      */
     return 0;
  }
#if EXPERIMENTAL_MOTION_AVERAGING
  mo = MODMOTE(mo+1);
  mote[mo].dx = dx;  mote[mo].dy = dy;  mote[mo].dt = dt;
  if(event->val == 0) {
	/* Mouse release: use recently-averaged dt to extrapolate
	 * continuing motion.
	 */
	for(i = 1; i < NMOTE; i++) {
	    struct mote *m = &mote[MODMOTE(mo-i)];
	    
	    if(dt + m->dt > uistate.cursor_still)
		break;
	    dx += m->dx;
	    dy += m->dy;
	    dt += m->dt;
	}
	i;	/* For debugging */
  }
#endif
  mousemap(event->x, event->y, &x, &y, &drawerstate.winpos);
  if (button.shift) strcat(buttonsdown, "shift ");
  if (button.ctrl) strcat(buttonsdown, "ctrl ");
  if (button.left) strcat(buttonsdown, "leftmouse ");
  if (button.middle) strcat(buttonsdown, "middlemouse ");
  if(buttonsdown[0] != '\0') buttonsdown[strlen(buttonsdown)-1] = '\0';

  for (i = 0; ctls[i].buttons != NULL; i++) {
    if (!strcmp(ctls[i].buttons, buttonsdown)) {
      return (ctls[i].function)(event->val, x,y,event->t, dx,dy,
		dt*.001/ctls[i].rate);
    }
  }
  return 0;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
