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

#include <math.h>
#include <stdlib.h>
#include "geom.h"
#include "transform.h"
#include "transform3.h"
#include "forms.h"
#include "xforms-compat.h"
#include "panel.h"

#ifdef XFORMS
# define  FL_SHORTCUT(obj, string)	fl_set_object_shortcut(obj, string, 1)
#else
# define  FL_SHORTCUT(obj, string)	fl_set_object_shortcut(obj, string)
#endif

Transform T;
FL_OBJECT *t[4][4];

void set_input_float(FL_OBJECT *obj, float val);
float get_input_float(FL_OBJECT *obj);

int main(int argc, char *argv[]) {
#ifdef XFORMS
  FL_INITIALIZE("Transformer");
#else
  foreground();
  fl_init();
#endif

  create_the_forms();

  t[0][0] = t00;
  t[0][1] = t01;
  t[0][2] = t02;
  t[0][3] = t03;
  t[1][0] = t10;
  t[1][1] = t11;
  t[1][2] = t12;
  t[1][3] = t13;
  t[2][0] = t20;
  t[2][1] = t21;
  t[2][2] = t22;
  t[2][3] = t23;
  t[3][0] = t30;
  t[3][1] = t31;
  t[3][2] = t32;
  t[3][3] = t33;

  FL_SHORTCUT(RotateButton, "R");
  FL_SHORTCUT(TranslateButton, "T");
  FL_SHORTCUT(ScaleButton, "S");
  FL_SHORTCUT(ApplyButton, "a ");
  FL_SHORTCUT(SetButton, "s");
  FL_SHORTCUT(ReadButton, "r");
  FL_SHORTCUT(IdentityButton, "i");
  FL_SHORTCUT(DoneButton, "q");

  FL_SHORTCUT(CameraButton, "C");
  FL_SHORTCUT(SelfButton, "E");
  FL_SHORTCUT(WorldButton, "W");

  fl_set_button(actGeom, 1);

  fl_set_button(CameraButton, 1);

  set_input_float(RotationMeasure, 10);
  fl_set_button(RotationAxisX, 1);
  fl_set_button(RotationMeasure, 1);

  set_input_float(TranslationMeasure, 1);
  fl_set_button(TranslationAxisX, 1);

  fl_set_button(ScaleX, 1);
  fl_set_button(ScaleY, 1);
  fl_set_button(ScaleZ, 1);
  set_input_float(ScaleMeasure, 2);

  fl_set_button(RotateModeButton, 1);
  fl_show_object(RotateGroup);
  fl_hide_object(TranslateGroup);
  fl_hide_object(ScaleGroup);

  fl_show_form(TransForm, FL_PLACE_SIZE, TRUE, "Transformer");
  ReadProc(NULL, 0);
  while (1) fl_do_forms();
  
}


void set_input_float(FL_OBJECT *obj, float val) {
  char buf[255];
  sprintf(buf, "%.2f", val);
  fl_set_input(obj, buf);
}

float get_input_float(FL_OBJECT *obj) {
  float f;
  sscanf(fl_get_input(obj), "%f", &f);
  return f;
}


void SetPanel(Transform T) {
  int i, j;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) 
      set_input_float(t[i][j], T[i][j]);
}

void GetPanel(Transform T) {
  int i, j;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) 
      T[i][j] = get_input_float(t[i][j]);

}

void CoordSysProc(FL_OBJECT *o, long val)
{
  fl_freeze_form(TransForm);
  fl_set_button(CameraButton, 0);
  fl_set_button(SelfButton, 0);
  fl_set_button(WorldButton, 0);
  fl_set_button(o, 1);
  fl_unfreeze_form(TransForm);
}

char *frame()
{
  if (fl_get_button(SelfButton)) return "self";
  if (fl_get_button(WorldButton)) return "World";
  return "targetcam";
}


void RotateButtonProc(FL_OBJECT *o, long val) {
  float f;

  f = get_input_float(RotationMeasure) * 2.0 * M_PI / 360.0;
  printf("(transform %s %s rotate %f %f %f)",
	 (fl_get_button(actGeom) ? 
	  "targetgeom targetgeom" : "targetcam targetcam"),
	 frame(),
	 (fl_get_button(RotationAxisX) ? f : 0.0),
	 (fl_get_button(RotationAxisY) ? f : 0.0),
	 (fl_get_button(RotationAxisZ) ? f : 0.0));
  fflush(stdout);

  ReadProc(NULL, 0);

}


void TranslateButtonProc(FL_OBJECT *o, long val) {
  float f;

  f = get_input_float(TranslationMeasure);
  printf("(transform %s %s translate %f %f %f)",
	 (fl_get_button(actGeom) ? 
	  "targetgeom targetgeom" : "targetcam targetcam"),
	 frame(),
	 (fl_get_button(TranslationAxisX) ? f : 0.0),
	 (fl_get_button(TranslationAxisY) ? f : 0.0),
	 (fl_get_button(TranslationAxisZ) ? f : 0.0));
  fflush(stdout);

  ReadProc(NULL, 0);
    
}


void ScaleButtonProc(FL_OBJECT *o, long val) {
  float f;

  f = get_input_float(ScaleMeasure);
  if (!fl_get_button(actGeom)) return;
  printf("(scale targetgeom %f %f %f)", 
	 fl_get_button(ScaleX) ? f : 1.0,
	 fl_get_button(ScaleY) ? f : 1.0,
	 fl_get_button(ScaleZ) ? f : 1.0);
  fflush(stdout);
  ReadProc(NULL, 0);
}


void ApplyProc(FL_OBJECT *o, long val) {
  Transform T;
  GetPanel(T);
  printf("(xform %s { ", fl_get_button(actGeom) ? "targetgeom" : "targetcam" );
  fputtransform(stdout, 1, &T[0][0], 0);
  printf(" } )");
  fflush(stdout);
}


void SetProc(FL_OBJECT *o, long val) {
  Transform T;
  GetPanel(T);
  printf("(xform-set %s { ",
	 fl_get_button(actGeom) ? "targetgeom" : "targetcam" );
  fputtransform(stdout, 1, &T[0][0], 0);
  printf(" } ) ");
  fflush(stdout);
}


void ReadProc(FL_OBJECT *o, long val) {
  char buf[255];
  Transform T;

  printf("(if (= (real-id target) (real-id World)) ");
  printf("(write transform - %s universe) ",
	 fl_get_button(actGeom) ? "targetgeom" : "targetcam" );
  printf("(write transform - %s world))\n",
	 fl_get_button(actGeom) ? "targetgeom" : "targetcam" );
  fflush(stdout);
  scanf("%s {", buf);
  fgettransform(stdin, 1, &T[0][0], 0);
  scanf("%s", buf);
  SetPanel(T);
}


void IdentityProc(FL_OBJECT *o, long val) {
  
  SetPanel(TM_IDENTITY);

}


void DoneProc(FL_OBJECT *o, long val) {
  exit(0);
}



void RotateMode(FL_OBJECT *o, long val) {
  fl_freeze_form(TransForm);
  fl_hide_object(TranslateGroup);
  fl_hide_object(ScaleGroup);
  fl_show_object(RotateGroup);
  fl_unfreeze_form(TransForm);
}

void TranslateMode(FL_OBJECT *o, long val) {
  fl_freeze_form(TransForm);
  fl_hide_object(RotateGroup);
  fl_hide_object(ScaleGroup);
  fl_show_object(TranslateGroup);
  fl_unfreeze_form(TransForm);
}

void ScaleMode(FL_OBJECT *o, long val) {
  fl_freeze_form(TransForm);
  fl_hide_object(RotateGroup);
  fl_hide_object(TranslateGroup);
  fl_show_object(ScaleGroup);
  fl_unfreeze_form(TransForm);
}
