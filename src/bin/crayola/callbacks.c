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

#include <stdio.h>
#include <string.h>
#include "geom.h"
#include "lisp.h"
#include "pickfunc.h"
#include "crayola.h"
#include "ui.h"

IOBFILE *infile;

Lake *lake;

static Geom *undoGeom = NULL;
static char *undoID = NULL;

static char msg[] = "callbacks.c";

#define PICK_YES		"(interest (pick self))"
#define PICK_NO			"(uninterest (pick self))"

void SetUndo(Geom *g, char *name)
{
  if (undoGeom != NULL) GeomDelete(undoGeom);
  if (undoID != NULL) OOGLFree(undoID);
  undoGeom = GeomCopy(g);
  undoID = OOGLNewNE(char, strlen(name) + 1, msg);
  strcpy(undoID, name);
}

Geom *GetObject(char *name)
{
  printf("(echo \"{\")");
  printf("(write geometry - %s bare)", name);
  printf("(echo \"}\")");
  fflush(stdout);
  return GeomFLoad(infile, NULL);
}

Geom *GetTarget(void)
{
  return GetObject("targetgeom");
}

void ReplaceObject(Geom *g, char *name)
{
  if (g == NULL) return;
  printf("(geometry %s ", name);
  GeomFSave(g, stdout, NULL);
  printf(")");
  printf("(redraw allcams)");
  fflush(stdout);
}

void ReplaceTarget(Geom *g)
{
  ReplaceObject(g, "targetgeom");
}

void freeze(void)
{
  uiFreeze();
  printf(PICK_NO);
  fflush(stdout);
}

void thaw(void)
{
  printf("(echo \"(ack)\\n\")");
  fflush(stdout);
}

LDEFINE(ack, LVOID,
	"(ack)\n\
Command sent back from Geomview indicating that everything has been\n\
processed")
{
  LDECLARE(("ack", LBEGIN,
	    LEND));
  printf(PICK_YES);
  fflush(stdout);
  uiThaw();
  return Lt;
}

DEFPICKFUNC("(pick COORDSYS GEOMID G V E F P VI EI FI)",
	    coordsys,
	    id,
	    point, pn,
	    vertex, vn,
	    edge, en,
	    face, fn, 10,
	    ppath, ppn, 50,
	    vi,
	    ei, ein,
	    fi,
{
  Geom *g;
  ColorA color;
  bool setcolor = false;
  bool vcolor;
  bool fcolor;  

  if (!pn) {
    return Lt;
  }
  freeze();
  g = GetObject(id);
  if (uiSet() || uiSetAll()) {
    if (crayHasColor(g, ppath)) {
      setcolor = true; 
      vcolor = fcolor = false;
    } else {
      vcolor = crayCanUseVColor(g, ppath) != 0;
      fcolor = crayCanUseFColor(g, ppath) != 0;
      if ((vcolor || fcolor) && 
	  !uiQuery("Object currently has no color", 
		   "information.  Would you",
		   "like to add color information to it?", "yes", "no")) {
	if (vcolor && fcolor) {
	  vcolor = uiQuery("Would you like to color the",
			   "object by face or", "by vertex?", 
			   "By Face", "By Vertex");
	  fcolor = vcolor ? false : true;
	}
	setcolor = true;
      }
    }
    if (setcolor) {
      uiCurrentColor(&color);
      SetUndo(g, id);
      if (vcolor) {
	crayUseVColor(g, &crayDefColor, ppath);
      } else if (fcolor) {
	crayUseFColor(g, &crayDefColor, ppath);
      } if (uiSetAll()) {
	craySetColorAll(g, &color, ppath);
      } else {
	craySetColorAt(g, &color, vi, fi, ei, ppath, &vertex);
      }
      ReplaceObject(g, id);
    }
  } else if (uiGet()) {
    if (crayGetColorAt(g, &color, vi, fi, ei, ppath, &vertex)) {
      uiChangeColor(&color);
    }
  } else if (uiEliminateColor()) {
    SetUndo(g, id);
    if (crayEliminateColor(g, ppath)) {
      ReplaceObject(g, id);
    }
  }
  GeomDelete(g);
  thaw();
  return Lt;
})

void init(void)
{       
  crayolaInit();
     
  infile = iobfileopen(stdin);

  LInit();
  LDefun("pick", Lpick, Hpick);
  LDefun("ack", Lack, Hack);
  lake = LakeDefine(infile, stdout, NULL);
  printf(PICK_YES);
  fflush(stdout);

  uiChangeColor(&crayDefColor);
}

void dopipes(void)
{
  LObject *lit, *val;
  lit = LSexpr(lake);
  val = LEval(lit);
  LFree(lit);
  LFree(val);
}

void checkpipes(void)
{
  if (async_iobfnextc(infile, 0) != NODATA) dopipes();
#if 0
  else {
    static struct timeval tenth = { 0, 100000 };
    select(0, NULL, NULL, NULL, &tenth);
  }
#endif
}

int undo(void)
{
  if (undoGeom != NULL) {
    ReplaceObject(undoGeom, undoID);
    return 1;
  } else return 0;
}
     

void quit(void)
{
  if (undoGeom != NULL) GeomDelete(undoGeom);
  if (undoID != NULL) OOGLFree(undoID);
  exit(0);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
