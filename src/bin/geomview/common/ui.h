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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#ifndef UI_H
#define UI_H

#include "common.h"

#define MAXMODES 16		/* Max built-in motion modes */
#define	MAXMORE	10
#define MAXTOOLS 10 		/* Max built-in tools */
#define MAXACTIONS 10 		/* Max built-in actions */

typedef struct _emodule {
	char *name;
	void (*func)();
	char *text;
	int pid;
	Pool *link;
	void *data;
	char *dir;
} emodule;
	
typedef struct {
  int	targetgeom;		/* index of target Geom for user actions   */
  int	targetcam;		/* index of target Camera for user actions */
  int	targettype;		/* T_GEOM, T_CAM, or T_NONE		   */
  int   targetid;		/* id of target (geom or cam) (moving obj) */
  int   centerid;		/* id of center of motion)		   */
  int   bbox_center;		/* use center of bbox of centerid as origin */
  int	mousefocus;		/* index of camera (window) where mouse is */
  int   cam_wm_focus;           /* true: activate cam-win on
				 * FocusChange events, false: activate
				 * cam-win on mouse-cross events.
				 */

  int   savewhat;		/* What in the world did we want to save?
					(NOID = all) */
  /*
   * Motion modes are no longer separated into two arrays of geoms and cams. 
   * Everything is in one array.
   */
  PFI	modes[MAXMODES];		/* motion procs */
  char *modenames[MAXMODES];		/* motion names */
  int   modetype[MAXMODES];		/* type of motion: 
					   T_NONE if could be either, 
					   T_CAM or T_GEOM if specific*/
  int	mode_count;			/* number of entries in above arrays */
  int	mode_current;			/* current mode */

  vvec  emod;				/* External module table */
  int	emod_changed;			/* Set if browser changed since drawn */
  int	emod_check;			/* Set if some emodule died */
  char *emod_dir;			/* if non-NULL, pathname of emodule's dir */
  int   lights_shown;			/* are lights drawn? */	
  int	current_light;		/* currently selected light for editing */

  int   cursor_on;			/* cursor visible? */
  int	clamp_cursor;			/* cursor clamped to center of window */
  int 	cursor_still;			/* microseconds in which cursor must
					   not move to as holding
					   still */
  int	cursor_twitch;			/* distance cursor must not move 
					   (in either x or y) to register
					   as holding still. */
  float	longwhile;			/* Max credible real time (in seconds)
					 * between redraws.  If dt > longwhile,
					 * clamp it to longwhile to avoid huge
					 * jumps in motion. */

  Color pickedbboxcolor;		/* Color of picked objects' bboxes */
  int   backface;			/* Cull back-facing polygons */
  int	apoverride;			/* -1 if appearance controls should
					 * override, else 0 */
  int   ownmotion;			/* Interactive motions with respect to
					 * object's own coordinates, not camera.
					 */
  int	constrained;			/* Constrain interactive motions to 
					 * just one (X/Y) axis, whichever is
					 * greater.
					 */
  int	inertia;			/* Motions have inertia */
  int	freeze;				/* Whether to freeze UI panels */
  int	pick_invisible;			/* Should picks be sensitive to invisible objects? */
} UIState;

extern UIState uistate;

/* shading */
#define CONSTANTSHADE	APF_CONSTANT
#define FLATSHADE	APF_FLAT
#define SMOOTHSHADE	APF_SMOOTH
extern char *shades[5];

/* translucency */
#define BSPTREE_BLENDING APF_ALPHA_BLENDING
#define SCREEN_DOOR      APF_SCREEN_DOOR
#define SIMPLE_BLENDING  APF_NAIVE_BLENDING
extern char *translucencies[3];

/* normalization --- these must be consective beginning with 0 ---
   they are used as indices into the "norm" array !!! */
typedef enum { NONE = 0, EACH, ALL, KEEP } Normalization;
extern char *norm[4];

/* projection */
typedef enum { ORTHOGRAPHIC = 0, PERSPECTIVE } Projection;
extern char *proj[2];

/* UI space */
/* NOTE:  These are now strictly UI values.  Internally space is recorded
   using TM_EUCLIDEAN, etc from transform3.h. */
#define EUCLIDEAN 0
#define HYPERBOLIC 1
#define SPHERICAL 2
extern char *spc[3];
extern char *mdl[3];

/* motion mode names */
extern char OBJROTATE[];		/* Rotate */
extern char OBJTRANSLATE[];		/* Translate */
extern char OBJSCALE[];			/* Scale */
extern char OBJZOOM[];			/* Zoom -- FOV! */

extern char OBJFLY[];			/* Cam Fly */
extern char OBJORBIT[];			/* Cam Orbit */

extern char LIGHTEDIT[];		/* Edit Lights */

#define IMMEDIATE   1
#define NOIMMEDIATE 0

/* Tools */

/* These MUST match the order declared in cui_init!! */
#define TOOL_ROT	0
#define TOOL_ZOOM	1
#define TOOL_TRANS	2
#define TOOL_FLY	3
#define TOOL_ORBIT	4
#define TOOL_SCALE	5

/* Actions */
#define ACTION_STOP	0
#define ACTION_PAUSE	1
#define ACTION_CENTER	2
#define ACTION_RESET	3
#define ACTION_LOOK	4

#define UI_ADD		0
#define UI_REPLACE	1
#define UI_ADDCAMERA	2

#define UI_TARGET	0
#define UI_CENTER	1

/* Cursor */
#define UI_CURSOR_STILL	100
#define UI_CURSOR_TWITCH	5


/* in ui.c */

extern void ui_install_mode(char *name, PFI proc, int type);
extern void ui_reinstall_mode(char *name, PFI proc, int type, int index);
extern void ui_uninstall_mode(char *name);
extern int  ui_mode_index(char *name);
extern emodule *ui_emodule_install(int after, char *external, PFI func);
extern int  ui_emodule_index(char *external, emodule **emp);
extern void ui_emodule_uninstall(int index);
extern void set_ui_target(int type, int index);
extern void set_ui_center(int id);
extern void set_ui_target_id(int id);
extern void set_ui_center_origin(int use_bbox_center);
extern void set_ui_wm_focus(int cam_wm_focus);
extern void ui_cleanup();

/* in glui.c */

extern int ui_panelshown(int index);
extern void ui_showpanel( int index, int show );
extern void ui_init();
extern void env_init();
extern void ui_lights_changed();
extern void set_light_display(int lightno);
extern void ui_keyboard(int ch);
extern void timing(int interval);
extern void ui_add_mode(char *name, PFI proc, int type);
extern void ui_remove_mode(char *name);
extern void ui_replace_mode(char *name, PFI proc, int type, int index);
extern void ui_event_mode(char *mode);
extern void ui_action(int val);
extern void ui_popup_message(char *s);


extern void ui_mousefocus(int index);
extern void ui_objectchange();
extern void ui_maybe_refresh(int id);
extern void set_light_intensity(float intens);
extern void lights_changed_check();
extern void ui_light_button();
extern void ui_changed_lights();

			/* ui_fileio() pops up a file browser/input panel.
			 * It loads the resulting file if dosave==0,
			 * saves state into it otherwise.
			 */
extern void ui_fileio( int dosave );

			/* ui_pickcolor() pops up a color picker,
			 * sets the color associated with the given code,
			 * e.g. DRAWER_EDGECOLOR or DRAWER_BACKCOLOR.
			 */
extern void ui_pickcolor( int drawer_code );

			/* ui_curson() toggles cursor visibility.
			 * 1 : enable, 0 : disable, -1 : toggle.
			 */
extern void ui_curson( int toggle );

			/* ui_name2panel() looks up the name of a UI panel
			 * and returns a UI-specific index if found,
			 * otherwise zero.
			 */
extern int  ui_name2panel( char *name );

			/* ui_showpanel() makes the given UI panel visible
			 * or invisible, according to 'show'
			 */
extern void ui_showpanel( int index, int show );

			/* ui_manual_browser() tries to spawn an external
			 * html or pdf browser to give access to the manual.
			 * "type" may be either "html" or "pdf", other values
			 * are silently ignored.
			 */
extern void ui_manual_browser(const char *type);

			/* ui_panelwindow() specifies positioning for the
			 * given UI panel from the given WnWindow.
			 */
extern void ui_panelwindow( int index, WindowStruct *ws );
extern void cui_init();

extern int uispace(int space);
extern void ui_windowWillOpen(DView *dv);
extern void ui_windowDidOpen(DView *dv);
#endif /* ! UI_H */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
