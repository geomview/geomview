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
#include <math.h>
#include "graffiti.h"

#define MAXVERTS 1000

int newvect = 1;
int wrap = 0;
short nverts = 0;
Point3 verts[MAXVERTS];
int onlyverts = 0;
char *headname = "graffiti-head";
char *tailname = "graffiti-tail";
char *headxformname = "graffiti-head-xform";
char *tailxformname = "graffiti-tail-xform";
char *graffitiname = "graffiti";

main(int argc, char *argv[])
{
  Initialize();
  gui_init();
  gui_main_loop();
}

NewPLine()
{
  nverts = 0;
}

NewLine()
{
  if (!newvect) {
    EraseHeadAndTail();
    StartNewVector();
    NewPLine();
    newvect = 1;
    wrap = 0;
  }
}

Initialize()
{
  LangInit(stdout);
  NewPLine();
}

void
AddVertex(Point3 *p)
{
  progn();

  /* if this is the first vertex of this pline, draw the tail here */
  if (nverts == 0) {
    ShowTailAt(p);
  } else {
    /* otherwise draw the head here, pointing along last segment */
    ShowHeadAt(p, &verts[nverts-1]);
  }

  if (nverts < MAXVERTS) {
    verts[nverts] = *p;
    ++nverts;
  }
  Geometry();
  endprogn();
}

RemoveVertex()
{
  if (!nverts) return;
  --nverts;

  progn();
  Geometry();
  if (nverts == 0) { /* if we are down to 0 vertices, nuke head and tail */
    EraseHeadAndTail();
  } else if (nverts == 1) { /* if just 1 vertex nuke head */
    EraseHead();
  } else if (nverts > 1) { /* if > 1 vertex, move head to last one */
    ShowHeadAt(&verts[nverts-1], &verts[nverts-2]);
  }
  endprogn();
}

Close()
{
  wrap = 1;
  Geometry();
}
