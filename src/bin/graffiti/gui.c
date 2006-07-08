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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include "ooglutil.h"
#include "3d.h"
#include "forms.h"
#include "xforms-compat.h"

#include "panel.h"

#include "graffiti.h"

extern void Input(void);
extern void EraseHeadAndTail(void);
extern void EraseHeadTail(char *xformname);
extern void EraseHead(void);
extern void StartNewVector(void);
extern void LangInit(FILE *fp);
extern void progn(void);
extern void ShowTailAt(Point3 *p);
extern void ShowHeadAt(Point3 *p, Point3 *prev);
extern void Geometry(void);
extern void endprogn(void);
extern void gui_init(void);
extern void gui_main_loop(void);

void gui_init(void)
{
#ifdef XFORMS
  FL_INITIALIZE("Stereo");
#else
  fl_init();
  foreground();
#endif

  create_the_forms();
  fl_show_form(MainForm, FL_PLACE_SIZE, TRUE, "Graffiti");
}

void gui_main_loop(void)
{
  fd_set fdmask;
  struct timeval timeout;

  while (1) {

    FD_ZERO(&fdmask);
    FD_SET(fileno(stdin), &fdmask);
    timeout.tv_sec = 0;  timeout.tv_usec = 200000;
    select(fileno(stdin)+1, &fdmask, NULL, NULL, &timeout);

    if (async_fnextc(stdin,0) != NODATA) {
      Input();
    }
    fl_check_forms();
  }
}

void QuitProc(FL_OBJECT *obj, long val)
{
  DeleteHeadAndTail();
  exit(0);
}

void UndoButtonProc(FL_OBJECT *obj, long val)
{
  RemoveVertex();
}

void DrawButtonProc(FL_OBJECT *obj, long val)
{
  NewLine();
}

void VertexButtonProc(FL_OBJECT *obj, long val)
{
  onlyverts = fl_get_button(obj);
}

void PushDrawButtonUp(void)
{
  fl_set_button(DrawButton, FALSE);
}

void PushDrawButtonDown(void)
{
  fl_set_button(DrawButton, TRUE);
}

void DisplayPickInfoPanel(void)
{
  fl_show_form(PickInfoForm, FL_PLACE_MOUSE, TRUE, "Graffiti Info");
}

void DoneButtonProc(FL_OBJECT *obj, long val)
{
  fl_hide_form(PickInfoForm);
}
