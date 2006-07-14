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

extern void Input(void);
extern void EraseHeadAndTail(void);
extern void EraseHeadTail(char *xformname);
extern void EraseHead(void);
extern void StartNewVector(void);
extern void LangInit(IOBFILE *inf, FILE *outf);
extern void progn(void);
extern void ShowTailAt(Point3 *p);
extern void ShowHeadAt(Point3 *p, Point3 *prev);
extern void Geometry(void);
extern void endprogn(void);
extern void gui_init(void);
extern void gui_main_loop(IOBFILE *inf);

void NewPLine(void)
{
  nverts = 0;
}

void NewLine(void)
{
  if (!newvect) {
    EraseHeadAndTail();
    StartNewVector();
    NewPLine();
    newvect = 1;
    wrap = 0;
  }
}

IOBFILE *Initialize(void)
{
  IOBFILE *iobf = iobfileopen(stdin);

  LangInit(iobf, stdout);
  NewPLine();

  return iobf;
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

void RemoveVertex(void)
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

void Close(void)
{
  wrap = 1;
  Geometry();
}

int main(int argc, char *argv[])
{
  IOBFILE *iobf;
  
  iobf = Initialize();
  gui_init();
  gui_main_loop(iobf);
  return 0;
}

