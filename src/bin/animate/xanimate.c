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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#include <unistd.h>
#include "mibload.h"
#include "animate.h"
#include "xanimate.h"
#include "interface/animate.mib"
#include "interface/info.mib"
#include "interface/command.mib"


/*****************************************************************************/
/* static global vars */

static XtAppContext	App;
static Widget		TopLevel, AnimList, FileText, CommandText,
			RangeToggle, OnceToggle, BounceToggle, SpeedScale;
static int		playing = 0;
static long		speed = 0L;


#define NULL_XtIntervalId ((XtIntervalId)NULL)

static XtIntervalId	lasttime = NULL_XtIntervalId;


/*****************************************************************************/

static void close_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void info_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void bstep_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void fstep_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void play_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void stop_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void load_callback(Widget , XtPointer, XmAnyCallbackStruct *);
static void script_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void command_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);
static void typecommand_cback(Widget , XtPointer, XmAnyCallbackStruct *);
static void frameselect_callback(Widget, XtPointer, XmListCallbackStruct *);
static void range_callback(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void once_callback(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void bounce_callback(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void speed_callback(Widget, XtPointer, XmScaleCallbackStruct *);


/*****************************************************************************/

void keep_playing(XtPointer);

/*****************************************************************************/

static void cinfo_callback(Widget , XtPointer, XmPushButtonCallbackStruct *);

void UIloadinterface()
{
  Display	*dpy;
  Widget	MainWindow, InfoWindow, CommandWindow,
		InfoButton, CommandButton, PlayButton,
		StopButton, BStepButton, FStepButton,
		LoadButton, ScriptButton, CloseButton,
		AnimateTitle, InfoTitle, InfoText, InfoClose;
  XFontStruct  *font;
  XmFontList    fontlist;
  Arg		args[10];
  int		n, oldstderr;
  int		argcblah = 1;
  char 	       *argvblah[1] = { "Animate" };

  unsigned char *item;

  mib_Widget   *MainForm, *InfoForm, *CommandForm;
  String	fallbacks[] = {

#ifdef dark_ui
	"*Foreground:			gray20",
	"*Background:			gray70",
#endif
#ifdef light_ui
	"*Foreground:			#000000000000",
	"*Background:			#afafafafafaf",
#endif
	"*BorderWidth:			0",
	"*XmToggleButton.selectColor:    yellow",
	"*XmToggleButton.indicatorSize:    16",
	"*XmToggleButtonGadget.selectColor:    yellow",
	"*XmToggleButtonGadget.indicatorSize:    16",
	"*XmTextField.background:	        #cc7777",
	"*fontList:\
	-adobe-helvetica-medium-r-normal--14-100-100-100-p-76-iso8859-1",
	NULL};


  /* initialize application top level widget */

  TopLevel = XtVaAppInitialize(&App, "animate", NULL, 0,
	&argcblah, argvblah, fallbacks, NULL);

  dpy = XtDisplay(TopLevel);

  oldstderr = dup(fileno(stderr));
  freopen("/dev/null", "w", stderr);

  /* configure resize policy of window */

  XtVaSetValues(TopLevel, XmNminWidth, 272, XmNminHeight, 319,
			XmNkeyboardFocusPolicy, XmPOINTER, NULL);

  /* create the application windows */

  MainWindow = XtCreateManagedWidget("MainWindow",
	xmMainWindowWidgetClass, TopLevel, NULL, 0);

/*****************************************************************************/

  n = 0;
  XtSetArg(args[n], XmNminWidth, 359); n++;
  XtSetArg(args[n], XmNminHeight, 197); n++;
  XtSetArg(args[n], XmNtitle, "Info"); n++;

  InfoWindow = XmCreateFormDialog(MainWindow, "Info", args, n);

/*****************************************************************************/

  n = 0;
  XtSetArg(args[n], XmNminWidth, 365); n++;
  XtSetArg(args[n], XmNminHeight, 64); n++;
  XtSetArg(args[n], XmNmaxHeight, 64); n++;
  XtSetArg(args[n], XmNtitle, "Command"); n++;
  XtSetArg(args[n], XmNkeyboardFocusPolicy, XmPOINTER); n++;

  CommandWindow = XmCreateFormDialog(MainWindow, "Command", args, n);

/*****************************************************************************/

  /* load the interface via the mib library */

/*
  MainForm = mib_load_interface(MainWindow,
	"interface/animate.mib", MI_FROMFILE);
  InfoForm = mib_load_interface(InfoWindow,
	"interface/info.mib", MI_FROMFILE);
  CommandForm = mib_load_interface(CommandWindow,
	"interface/command.mib", MI_FROMFILE);
*/
  MainForm = mib_load_interface(MainWindow, AnimRoot, MI_FROMSTRING);
  InfoForm = mib_load_interface(InfoWindow, InfoRoot, MI_FROMSTRING);
  CommandForm = mib_load_interface(CommandWindow, CommRoot, MI_FROMSTRING);


  /* Get widget pointers from interface */

  AnimateTitle = mib_find_name(MainForm, "AnimateTitle")->me;
  CloseButton = mib_find_name(MainForm, "CloseButton")->me;
  InfoButton = mib_find_name(MainForm, "InfoButton")->me;
  CommandButton = mib_find_name(MainForm, "CommandButton")->me;
  ScriptButton = mib_find_name(MainForm, "Script")->me;
  LoadButton = mib_find_name(MainForm, "Load")->me;
  PlayButton = mib_find_name(MainForm, "Play")->me;
  StopButton = mib_find_name(MainForm, "Stop")->me;
  BStepButton = mib_find_name(MainForm, "BStep")->me;
  FStepButton = mib_find_name(MainForm, "FStep")->me;
  SpeedScale = mib_find_name(MainForm, "SpeedScale")->me;
  FileText = mib_find_name(MainForm, "FileText")->me;
  AnimList = mib_find_name(MainForm, "AnimList")->me;
  RangeToggle = mib_find_name(MainForm, "Range")->me;
  OnceToggle = mib_find_name(MainForm, "Once")->me;
  BounceToggle = mib_find_name(MainForm, "Bounce")->me;

  InfoTitle = mib_find_name(InfoForm, "Title")->me;
  InfoText = mib_find_name(InfoForm, "TextBig")->me;
  InfoClose = mib_find_name(InfoForm, "Close")->me;

  CommandText = mib_find_name(CommandForm, "TextBox")->me;

  /* Set large font for titles */

  font = XLoadQueryFont(dpy,
        "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1");
  fontlist = XmFontListCreate(font, "bigger");
  XtVaSetValues(AnimateTitle, XmNfontList, fontlist, NULL);
  XtVaSetValues(InfoTitle, XmNfontList, fontlist, NULL);

  /* Set various resources and values */

  XtVaSetValues(InfoText, XmNcursorPositionVisible, False,
                           XmNeditable, False, NULL);
  XmTextSetString(InfoText, get_info());

  XtVaSetValues(AnimList, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);

  XmScaleSetValue(SpeedScale, 100);

  /* Set callbacks - stupid @#$@&%!@# boiler plate code */

  XtAddCallback(CloseButton,
		XmNactivateCallback,
		(XtCallbackProc)close_callback,
		(XtPointer)NULL
		);

  XtAddCallback(InfoButton,
		XmNactivateCallback,
		(XtCallbackProc)info_callback,
		(XtPointer)InfoWindow
		);

  XtAddCallback(InfoClose,
		XmNactivateCallback,
		(XtCallbackProc)cinfo_callback,
		(XtPointer)InfoWindow
		);

  XtAddCallback(BStepButton,
		XmNactivateCallback,
		(XtCallbackProc)bstep_callback,
		(XtPointer)NULL
		);

  XtAddCallback(FStepButton,
		XmNactivateCallback,
		(XtCallbackProc)fstep_callback,
		(XtPointer)NULL
		);

  XtAddCallback(PlayButton,
		XmNactivateCallback,
		(XtCallbackProc)play_callback,
		(XtPointer)NULL
		);

  XtAddCallback(StopButton,
		XmNactivateCallback,
		(XtCallbackProc)stop_callback,
		(XtPointer)NULL
		);

  XtAddCallback(LoadButton,
		XmNactivateCallback,
		(XtCallbackProc)load_callback,
		(XtPointer)FileText
		);


  XtAddCallback(FileText,
		XmNactivateCallback,
		(XtCallbackProc)load_callback,
		(XtPointer)FileText
		);

  XtAddCallback(ScriptButton,
		XmNactivateCallback,
		(XtCallbackProc)script_callback,
		(XtPointer)FileText
		);

  XtAddCallback(CommandButton,
		XmNactivateCallback,
		(XtCallbackProc)command_callback,
		(XtPointer)CommandWindow
		);

  XtAddCallback(AnimList,
		XmNdefaultActionCallback,
		(XtCallbackProc)frameselect_callback,
		(XtPointer)NULL
		);

  XtAddCallback(RangeToggle,
		XmNvalueChangedCallback,
		(XtCallbackProc)range_callback,
		(XtPointer)NULL
		);

  XtAddCallback(OnceToggle,
		XmNvalueChangedCallback,
		(XtCallbackProc)once_callback,
		(XtPointer)NULL
		);

  XtAddCallback(BounceToggle,
		XmNvalueChangedCallback,
		(XtCallbackProc)bounce_callback,
		(XtPointer)NULL
		);

  XtAddCallback(CommandText,
		XmNactivateCallback,
		(XtCallbackProc)typecommand_cback,
		(XtPointer)CommandWindow
		);

  XtAddCallback(SpeedScale,
		XmNdragCallback,
		(XtCallbackProc)speed_callback,
		(XtPointer)NULL
		);

  freopen("/dev/null", "w", stderr);
  dup2(oldstderr, fileno(stderr));
  close(oldstderr);

  /* Bring the application window up on the screen. */
}

void UIshowinterface()
{
  XtRealizeWidget(TopLevel);
}

void UImainloop()
{
  /* Begin main Intrinsics event loop */

  XtAppMainLoop (App);

}

/*****************************************************************************/

void UIclearlist()
{
  XmListDeleteAllItems(AnimList);
}

/*****************************************************************************/

void UIaddlist(char *line)
{
  XmString item = XmStringCreateSimple(line);

  XmListAddItem(AnimList, item, 0); /* add item to the end of the list */

  XmStringFree(item);
}

/*****************************************************************************/

void UIsetframe(char *line)
{
  XmTextFieldSetString(FileText, line);
}

/*****************************************************************************/

int UIgetselected(int **pos_list, int *pos_count)
{
  return (int)XmListGetSelectedPos(AnimList, pos_list, pos_count);
}

/*****************************************************************************/

void UIstartplay()
{
  if (playing)
    return;

  lasttime = XtAppAddTimeOut(App, speed, (XtTimerCallbackProc)keep_playing, (XtPointer) NULL);
  playing = 1;
}

void UIstopplay()
{
  playing = 0;
  if (lasttime != NULL_XtIntervalId)
  {
    XtRemoveTimeOut(lasttime);
    lasttime = NULL_XtIntervalId;
  }
}

/*****************************************************************************/

void keep_playing(XtPointer data)
{
  XEvent xev;

  lasttime = NULL_XtIntervalId;

  if (playing)
  {
    while (XtAppPending(App))		/* clear out event queue */
    {					/* before adding another timeout. */
      XtAppNextEvent(App, &xev);
      XtDispatchEvent(&xev);
    }

    if (lasttime == NULL_XtIntervalId)
      lasttime = XtAppAddTimeOut(App, speed, (XtTimerCallbackProc)keep_playing, (XtPointer) NULL);

    anim_playing();
  }
}

/*****************************************************************************/

void UIsetRange(int val)
{
  XmToggleButtonSetState(RangeToggle, val, False);
}

/*****************************************************************************/

void UIsetOnce(int val)
{
  XmToggleButtonSetState(OnceToggle, val, False);
}

/*****************************************************************************/

void UIsetBounce(int val)
{
  XmToggleButtonSetState(BounceToggle, val, False);
}

/*****************************************************************************/

void UIsetSpeed(int val)
{

  XmScaleSetValue(SpeedScale, val);
  if (val == 0)
    speed = 60000L;
  else
    if (val > 0)
      speed = (long) ((100.0 - (float)val) *
                        (100.0 - (float)val) *
                          (100.0 - (float)val) / 100.0) + 30L;

}

/*****************************************************************************/

static void close_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  anim_close();
}

static void info_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  XtManageChild((Widget)data);
}

static void cinfo_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  XtUnmanageChild((Widget)data);
}

static void command_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  XtManageChild((Widget)data);
}

static void bstep_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  anim_stop();
  anim_stepb();
}

static void fstep_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  anim_stop();
  anim_stepf();
}

static void play_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  anim_play();
}

static void stop_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  anim_stop();
}

static void load_callback(	Widget w,
				XtPointer data,
				XmAnyCallbackStruct *cbs)
{
  char *str;

  str = XmTextFieldGetString((Widget)data);
  anim_load(str);
  XtFree(str);
}

static void script_callback(	Widget w,
				XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  char *str;

  str = XmTextFieldGetString((Widget)data);
  anim_loadscript(str);
  XtFree(str);
}

static void frameselect_callback(	Widget w,
					XtPointer data,
					XmListCallbackStruct *cbs)
{
  anim_stop();
  anim_goframe(cbs->item_position - 1);
}

static void range_callback(	Widget w,
				XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  anim_range(cbs->set);
}

static void once_callback(	Widget w,
				XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  anim_once(cbs->set);
}

static void bounce_callback(	Widget w,
				XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  anim_bounce(cbs->set);
}

static void typecommand_cback(	Widget w,
				XtPointer data,
				XmAnyCallbackStruct *cbs)
{
  char *str;

  str = XmTextFieldGetString(CommandText);
  anim_setcommand(str);
  XtFree(str);
  XtUnmanageChild((Widget)data);
}

static void speed_callback(	Widget w,
				XtPointer data,
				XmScaleCallbackStruct *cbs)
{
  int val = cbs->value;
  int rmv = 0;

  if (lasttime != NULL_XtIntervalId)		/* get rid of old timeout */
  {
    XtRemoveTimeOut(lasttime);
    lasttime = NULL_XtIntervalId;
    rmv = 1;			/* removed timeout */
  }

  if (cbs->value == 0)
    speed = 60000L;
  else
    if (cbs->value > 0)
      speed = (long) ((100.0 - (float)cbs->value) *
			(100.0 - (float)cbs->value) *
			  (100.0 - (float)cbs->value) / 100.0) + 30L;

  if (rmv)
    lasttime = XtAppAddTimeOut(App, speed, (XtTimerCallbackProc)keep_playing, (XtPointer) NULL);

}
