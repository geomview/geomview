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
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include "forms.h"
#include "geom.h"
#include "point3.h"
#include "xforms-compat.h"
#include "panel.h"
#include "sweep.h"

void
flx_set_input_printf(FL_OBJECT *obj, char *fmt, ...)
{
  char stuff[10240];
  va_list args;
  va_start(args, fmt);
  vsprintf(stuff, fmt, args);
  fl_set_input(obj, stuff);
  va_end(args);
}

void
flx_set_input_int(FL_OBJECT *obj, int v)
{
  flx_set_input_printf(obj, "%d", v);
}

void
flx_set_input_float(FL_OBJECT *obj, float v)
{
  flx_set_input_printf(obj, "%g", v);
}

int
flx_get_input_int(FL_OBJECT *obj)
{
  return atoi(fl_get_input(obj));
}

float
flx_get_input_float(FL_OBJECT *obj)
{
  float foo;
  const char *str = fl_get_input(obj);
  foo = atof(str);
  return foo;
  
}

int main(int argc, char *argv[]) {

#ifdef XFORMS
  FL_INITIALIZE("Sweep");
#else
  foreground();
  fl_init();
#endif

  create_the_forms();
  fl_set_button(TransSweep, 1);
  fl_set_button(RadButton, 1);

  flx_set_input_printf(Transxdir, "%.2f", 0.0);
  flx_set_input_printf(Transydir, "%.2f", 0.0);
  flx_set_input_printf(Transzdir, "%.2f", 1.0);
  flx_set_input_printf(Translength, "%.2f", 1.0);
  
  flx_set_input_printf(Rotlength, "%.2f", 2.0 * M_PI);
  flx_set_input_int(Rotdivisions, 10);
  flx_set_input_float(Rotendx, 0.0);
  flx_set_input_float(Rotendy, 0.0);
  flx_set_input_float(Rotendz, 0.0);
  flx_set_input_float(Rotdirx, 0.0);
  flx_set_input_float(Rotdiry, 1.0);
  flx_set_input_float(Rotdirz, 0.0);

  fl_hide_object(RotGroup);

  fl_show_form(MainForm, FL_PLACE_SIZE, TRUE, "Sweep");
  while(1) fl_do_forms();

  return 0;
}


void RotSweepProc(FL_OBJECT *obj, long val) {
  fl_hide_object(TransGroup);
  fl_show_object(RotGroup);
}


void TransSweepProc(FL_OBJECT *obj, long val) {
  fl_hide_object(RotGroup);
  fl_show_object(TransGroup);
}


void GoButtonProc(FL_OBJECT *obj, long val) {
  IOBFILE *iobf;
  Geom *g, *s;
  Point3 pt1, pt2;

  printf("(write geometry - targetgeom bare)");
  fflush(stdout);
  iobf = iobfileopen(stdin);
  g = GeomFLoad(iobf, NULL);
  iobfileclose(iobf);

  if (fl_get_button(RotSweep)) {
    pt1.x = flx_get_input_float(Rotendx);
    pt1.y = flx_get_input_float(Rotendy);
    pt1.z = flx_get_input_float(Rotendz);
    pt2.x = flx_get_input_float(Rotdirx);
    pt2.y = flx_get_input_float(Rotdiry);
    pt2.z = flx_get_input_float(Rotdirz);
    s = RotationSweep(flx_get_input_float(Rotlength) * 
		      (fl_get_button(DegButton) ? M_PI / 180.0 : 1.0), 
		      &pt1, &pt2, 
		      flx_get_input_int(Rotdivisions), g);
  } else {
    pt1.x = flx_get_input_float(Transxdir);
    pt1.y = flx_get_input_float(Transydir);
    pt1.z = flx_get_input_float(Transzdir);
    s = TranslationSweep(flx_get_input_float(Translength), &pt1, g);
  }

  if (s != NULL) {
    printf("(geometry sweep { ");
    GeomFSave(s, stdout, NULL);
    printf(" } ) ");
    fflush(stdout);
  }
  GeomDelete(g);
  GeomDelete(s);
		       
}

void QuitButtonProc(FL_OBJECT *obj, long val) {
  exit(0);
}
