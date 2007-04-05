/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
 * Copyright (C) 2007 Claus-Justus Heine
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
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/* Author: Timothy Rowley */

#include <stdio.h>
#include <stdlib.h>
#include <tk.h>

#include "geom.h"
#include "color.h"
#include "callbacks.h"
#include "ui.h"
#include "crayola.h"

static Tcl_Interp *magic;
static Color rgb, hsv;

void
uiFreeze(void)
{
}

void
uiThaw(void)
{
}

static int
checkOperation(char *op)
{
  const char *value = Tcl_GetVar(magic, "operation", 0);

  if (!strcmp(value, op))
    return 1;
  else
    return 0;
}

int
uiGet(void)
{
  return checkOperation("get");
}

int
uiSet(void)
{
  return checkOperation("set");
}

int
uiSetAll(void)
{
  return checkOperation("setall");
}

int
uiEliminateColor(void)
{
  return checkOperation("eliminate");
}

#if 0
static char cscl[] = ".crayola.c";
#endif

static void
setsliders(char *which, Color *c)
{
  char s[200];
  sprintf(s, "setsliders %s {%g %g %g}", which, c->r, c->g, c->b);
  Tcl_Eval(magic, s);
}

void
uiChangeColor(ColorA *color)
{
  rgb = *(Color *)color;
  rgb2hsv(&rgb, &hsv);
  setsliders("rgb", &rgb);
  setsliders("hsv", &hsv);
  Tcl_Eval(magic, "newColor");
}

void
uiCurrentColor(ColorA *color)
{
  *(Color *)color = rgb;
  color->a = 1.0;
}

int
uiQuery(char *ques1, char *ques2, char *ques3, char *res1, char *res2)
{
  Tcl_VarEval(magic, "tk_dialog .question Query {", ques1, " ", ques2, " ",
	      ques3, "} {} -1 \"", res1, "\" \"", res2, "\"", NULL);
  return atoi(magic->result);
}

int
undoCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv)
{
  undo();
  return TCL_OK;
}

int
setColorCmd(ClientData clientData, Tcl_Interp *interp,
	    int argc, const char **argv)
{
  Color *c = &rgb;

  if(argc > 1 && strcmp(argv[1], "-hsv") == 0) {
    c = &hsv;
    argc--;
    argv++;
  }
	
  if (argc != 4)
    {
      Tcl_SetResult(interp, "usage: crayolaSetColor ?-hsv? r g b", TCL_STATIC);
      return TCL_ERROR;
    }
  c->r = atof(argv[1]);
  c->g = atof(argv[2]);
  c->b = atof(argv[3]);
  if(c == &hsv) {
    hsv2rgb(&hsv, &rgb);
    setsliders("rgb", &rgb);
  } else {
    rgb2hsv(&rgb, &hsv);
    setsliders("hsv", &hsv);
  }
  return TCL_OK;
}

int
quitCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv)
{
  quit();
  return TCL_OK;
}

static void
handler(ClientData clientData/*, int mask*/)
{
#if HAVE_TCL_CREATEFILEHANDLER
  dopipes();
#else
  checkpipes();
  Tk_CreateTimerHandler(100, handler, NULL);
#endif
}

int
crayola_init(Tcl_Interp *interp)
{
  Tk_Window mainw = Tk_MainWindow(interp);

  magic = interp;
  Tcl_CreateCommand(interp, "crayolaSetColor", setColorCmd, 
		    (ClientData) mainw, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "crayolaUndo", undoCmd, (ClientData) mainw,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "crayolaQuit", quitCmd, (ClientData) mainw,
		    (Tcl_CmdDeleteProc *) NULL);

  init();
  uiThaw();
#if HAVE_TCL_CREATEFILEHANDLER
  Tk_CreateFileHandler(0, TK_READABLE, handler, 0);
#else
  Tk_CreateTimerHandler(100, handler, NULL);
#endif
  return TCL_OK;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
