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

#include <stdio.h>
#include "origin.common.h"
#include "forms.h"
#include "xforms-compat.h"
#include "origin.panel.h"

/*
 * This file should contain ONLY the user interface and main()
 * to make splitting off the NeXT version easy
 */

main() {

#ifdef XFORMS
  FL_INITIALIZE("Origin");
#else
  fl_init();
  foreground();
#endif
  
  create_the_forms();
 
  internalsInit();

  fl_show_form(MainForm, FL_PLACE_SIZE, TRUE, "Move Origin");
 
  while(1) fl_do_forms();
}

void ShowButtonProc(FL_OBJECT *obj, long val) {
  internalsShow();
}

void DoneButtonProc(FL_OBJECT *obj, long val) {
  internalsDone();
}

void CancelButtonProc(FL_OBJECT *obj, long val) {
  internalsCancel();
}

void QuitButtonProc(FL_OBJECT *obj, long val) {
  internalsQuit();
}

void OkButtonProc(FL_OBJECT *obj, long val) {
  fl_hide_form(ErrorForm);
  fl_activate_form(MainForm);
}

/* 
 * These are routines that common code (origin.common.c) will call
 */

void uiDisableShow() {
  ShowButton->active = 0;
  fl_set_object_boxtype(ShowButton, FL_FRAME_BOX);
}

void uiEnableShow() {
  ShowButton->active = 1;
  fl_set_object_boxtype(ShowButton, FL_UP_BOX);
}

void uiDisableDoneCancel() {
  DoneButton->active = 0;
  CancelButton->active = 0;
  fl_set_object_boxtype(DoneButton, FL_FRAME_BOX);
  fl_set_object_boxtype(CancelButton, FL_FRAME_BOX);
}

void uiEnableDoneCancel() {
  DoneButton->active = 1;
  CancelButton->active = 1;
  fl_set_object_boxtype(DoneButton, FL_UP_BOX);
  fl_set_object_boxtype(CancelButton, FL_UP_BOX);
}

void uiSetInstructions(char *str1, char *str2, char *str3, char *str4) 
{
  fl_set_object_label(Instruc1, str1);
  fl_set_object_label(Instruc2, str2);
  fl_set_object_label(Instruc3, str3);
  fl_set_object_label(Instruc4, str4);
  fl_redraw_form(MainForm);
}

void uiGetTargetname(char *name) {
  strcpy(name, fl_get_input(TargetInput));
}
	 

void uiSetTargetname(char *name) {
  fl_set_input(TargetInput, name);
}


void uiError(char *str1, char *str2, char *str3) {
  fl_deactivate_form(MainForm);

  fl_set_object_label(Error1, str1);
  fl_set_object_label(Error2, str2);
  fl_set_object_label(Error3, str3);

  fl_show_form(ErrorForm, FL_PLACE_MOUSE, TRUE, "");
  while (fl_do_forms() != OkButton);

  fl_hide_form(ErrorForm);
  fl_activate_form(MainForm);

}

