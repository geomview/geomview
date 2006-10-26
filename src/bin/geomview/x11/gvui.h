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
extern XtAppContext App;
extern int YScrnSize;
extern int      gvbitdepth;
extern Visual  *gvvisual;
extern Colormap gvcolormap;
extern int	gvpriv;
extern XtAccelerators KeyAccels;
extern Time     last_event_time;


void load_interface(unsigned int, char **);
void main_loop();

void panel_input(Widget, XtPointer, XEvent *, Boolean *);
void key_action(Widget, XEvent *, String *, Cardinal *);
void cam_input(Widget, XtPointer, XmDrawingAreaCallbackStruct *);
void cam_resize(Widget, XtPointer, XmDrawingAreaCallbackStruct *);
void cam_mouse(Widget, XtPointer , XPointerMovedEvent *, Boolean *);
void cam_mousecross(Widget, XtPointer , XEnterWindowEvent *, Boolean *);
void cam_focus(Widget w, XtPointer data, XFocusChangeEvent *event,
	       Boolean *cont);
void cam_expose(Widget w, XtPointer id, XEvent *ev, Boolean *junk);

void ui_build_browser(Widget, int, char *[]);
void ui_set_ftext(Widget, float);
void ui_set_itext(Widget, int);
void ui_showpanel(int, int);
void ui_final_init();
void ui_select(int);
void ui_update();
void silence_errors();
void resume_errors();
void ui_init_snaphelp();


void ui_hide(Widget, XtPointer panelname, XmAnyCallbackStruct *cbs);
void ui_show(Widget, XtPointer panelname, XmAnyCallbackStruct *cbs);
Widget ui_make_panel_and_form(char *title, char *rootstr,
			      Boolean catchkeys, Boolean resizable,
			      Widget *form);

/* Panel Defines */
/*****************************************************************************/

#define P_NULL          0
#define P_MAIN          1
#define P_TOOL          2
#define P_APPEARANCE    3
#define P_CAMERA        4
#define P_LIGHTS        5
#define P_COMMANDS      6
#define P_MATERIAL      7
#define P_CREDITS       8
#define P_SAVE          9
#define P_FILEBROWSER   10
#define P_INPUT         11
#define P_COLOR         12
#define P_MAX           13

/* Camera Methods */
/*****************************************************************************/

Widget ui_create_camera(Widget, DView *);
void cam_mgdevice();
Pixel ui_RGB(Colormap cmap, int permanent, float r, float g, float b);
void update_x11_depth(DView *dv, mib_Widget *DepthMenu, Widget DitherToggle,
			int *depth_state, int *dither_state);
void ui_raise_window( int camid );

/* Main Panel Methods */
/*****************************************************************************/

void   ui_load_mainpanel();

void   ui_refresh_mainpanel();
void   ui_adjustpicklist(int);
void   ui_adjustemodulelist(int);

/* Tool Panel Methods */
/*****************************************************************************/

void   ui_load_toolpanel();
void   ui_select_tool(int);

void   ui_tool_targetset(char *);
void   ui_tool_centerset(char *);
void   ui_tool_center_origin_set(int use_bbox_center);

/* Appearance Panel Methods */
/*****************************************************************************/

void   ui_load_appearancepanel();
void   ui_show_appearancepanel();

void   ui_target_appearancepanel(int);

/* Material Panel Methods */
/*****************************************************************************/

void   ui_load_materialpanel();
void   ui_show_materialpanel();

void   ui_target_materialpanel(int);

/* Credits Panel Methods */
/*****************************************************************************/

void   ui_load_creditspanel();
void   ui_show_creditspanel();

/* Save Panel Methods */
/*****************************************************************************/

void   ui_load_savepanel();
void   ui_show_savepanel();

/* Files Panel Methods */
/*****************************************************************************/

void   ui_load_filespanel();
void   ui_show_filespanel();

/* Load Panel Methods */
/*****************************************************************************/

void   ui_load_loadpanel();
void   ui_show_loadpanel();

/* Command Panel Methods */
/*****************************************************************************/

void   ui_load_commandspanel();

/* Lights Panel Methods */
/*****************************************************************************/

void   ui_load_lightspanel();
void   ui_show_lightspanel();

/* Cameras Panel Methods */
/*****************************************************************************/

void   ui_load_cameraspanel();

void   ui_target_cameraspanel(int);

/* Color Panel Methods */
/*****************************************************************************/

void   ui_load_colorpanel();
void   ui_show_colorpanel();

void ui_config_colorpanel(char *, Color *, int, int, int);

/*****************************************************************************/
