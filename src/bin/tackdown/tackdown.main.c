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
#include "geom.h"
#include "streampool.h"
#include "forms.h"

#include "xforms-compat.h"
#include "panel.h"

static Pool *standardIn, *standardOut;

main() {

#ifdef XFORMS
  FL_INITIALIZE("Tackdown");
#else
  fl_init();
  foreground();
#endif

  standardIn = PoolStreamTemp(NULL, stdin, 0, NULL);
  standardOut = PoolStreamTemp(NULL, stdout, 1, NULL);

  create_the_forms();
  fl_set_button(worldButton, 1);
  fl_show_form(MainForm, FL_PLACE_SIZE, TRUE, "Tack Down");
  while(1) fl_do_forms();

}


void TransButtonProc(FL_OBJECT *o, long val) {
  char buf[256];
  Geom *g;
  Transform t;

  printf("(write geometry - targetgeom self)");
  printf("(write transform - targetgeom %s)", 
	 fl_get_button(worldButton) ? "world" : "universe");
  fflush(stdout);
  
  g = GeomFLoad(stdin, NULL);
 
  TransStreamIn(standardIn, NULL, t);

  GeomTransform(g, t);

  printf("(freeze allcams)");
  printf("(geometry targetgeom {");
  GeomFSave(g, stdout, NULL);
  printf("})");
  printf("(xform-set targetgeom ");
  fputtransform(stdout, 1, &TM_IDENTITY[0][0], 0);
  printf(")");
  printf("(redraw allcams)");
  fflush(stdout);
  fl_set_button(TransButton, 0);

  GeomDelete(g); 
 
}


void QuitButtonProc(FL_OBJECT *o, long val) {
  exit(0);
}
