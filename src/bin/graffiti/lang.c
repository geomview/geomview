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
#include "3d.h"
#include "geom.h"
#include "vect.h"
#include "color.h"
#include "graffiti.h"
#include "lang.h"
#include "handleP.h"
#include "ooglutil.h"
#include "pickfunc.h"

static Lake *lake;

void handle_pick(char *name, HPoint3 *got, 
		 int vert, HPoint3 *v, int gotpoint);
void defhead(float size);
void deftail(float size);
void EraseHeadTail(char *xformname);

FILE *outf = NULL;

static char *tailgeom = "\
OFF\n\
8 6 12\n\
-1 -1 -1  # 0\n\
 1 -1 -1  # 1\n\
 1  1 -1  # 2\n\
-1  1 -1  # 3\n\
-1 -1  1  # 4\n\
 1 -1  1  # 5\n\
 1  1  1  # 6\n\
-1  1  1  # 7\n\
4 0 1 2 3\n\
4 4 5 6 7\n\
4 2 3 7 6\n\
4 0 1 5 4\n\
4 0 4 7 3\n\
4 1 2 6 5\n";

static char *headgeom = "\
OFF\n\
5 5 8\n\
-1 -1 -1   # 0\n\
 1 -1 -1   # 1\n\
 1  1 -1   # 2\n\
-1  1 -1   # 3\n\
 0  0  1   # 4\n\
3 0 1 4\n\
3 1 2 4\n\
3 2 3 4\n\
3 3 0 4\n\
4 0 1 2 3\n";


void
handle_pick(char *name, HPoint3 *got, int vert, HPoint3 *v, int gotpoint)
{
  Point3 new;
  HPoint3 *use;
  static int nopick = 0;
  static int notified = 0;
  
  if (gotpoint) {
    if (strcmp(name, tailname)==0) {
      /* user picked the pen's tail --- close up this loop */
      Close();
      NewLine();
    } else {
      use = got;
      if (onlyverts) {
	if (vert)
	  use = v;
	else 
	  use = NULL;
      }
      if (use) {
	HPt3ToPt3(use, &new);
	AddVertex(&new);
      }
    }
    nopick = 0;
  } else {
    /* Nothing was picked. Keep track of the number of times this
       happens; the first time it happens 3 times in a row, pop
       up a message panel telling the user that they must pick
       on an object in the geomview window. */
    if (!notified)
      if (++nopick >= 3) {
	DisplayPickInfoPanel();
	notified = 1;
      }
  }
}


/* This macro is defined in pickfunc.h. It does the LDEFINE/LDECLARE for us */

DEFPICKFUNC("(pick COORDSYS GEOMID G V E F P VI EI FI)",
	    coordsys,
	    id,
	    point, pn,
	    vertex, vn,
	    edge, en,
	    face, fn,
	    ppath, ppn,
	    vi,
	    ei, ein,
	    fi,
	    {
	      handle_pick(id, &point, vn, &vertex, pn);
	      return Lt;
	    },
	    /* ND-stuff */
)
     


void
LangInit(IOBFILE *inf, FILE *fp)
{
  lake = LakeDefine(inf, fp, NULL);

  LInit();
  LDefun("pick", Lpick, Hpick);

  outf = fp;

  /*
    We could do just "(interest (pick))" which would give us all
    info, in world coordinates, about all picks.  We do the following
    instead to save bandwidth (the nil's prevent the corresponding
    arguments from being passed back), since all we care about is 
    the picked point and whether a it's a vertex.
  */
  progn();
  fprintf(outf, "(interest (pick world * * * nil nil nil nil nil))\n");
  defhead(0.02);
  deftail(0.02);
  endprogn();
}


void Input(void)
{
  LObject *lit, *val;
  lit = LSexpr(lake);
  val = LEval(lit);
  LFree(lit);
  LFree(val);
}

void Geometry(void)
{
  Geom *g;
  short dummy = 0;
  short vectc;

/*
       CR_NVECT,	# polylines
       CR_VECTC,	# verts in each polyline; array of NVECT shorts
       CR_COLRC,	# colors in each polyline; array of NVECT shorts
       CR_NVERT,	total # vertices
       CR_POINT,	the vertices
       CR_NCOLR,	total # colors
       CR_COLOR,	the colors
*/

  vectc = (wrap) ? -nverts : nverts;
  g = GeomCreate("vect",
		 CR_NVECT,	(nverts) ? 1 : 0,
		 CR_NVERT,	nverts,
		 CR_VECTC,	&vectc,
		 CR_POINT,	verts,
		 CR_NCOLR,	0,
		 CR_COLRC,	&dummy,
		 CR_END);
  fprintf(outf, "(geometry %s { ", graffitiname);
  GeomFSave(g, outf, "graffiti output pipe");
  GeomDelete(g);
  fprintf(outf, " } )\n");
  if (newvect) {
    fprintf(outf, "(normalization %s none)\n", graffitiname);
    fprintf(outf, "(bbox-draw %s off)\n", graffitiname);
    newvect = 0;
  }
  fflush(outf);
}

void
defheadtail(char *name, char *xformname, char *geom, float size)
{
  fprintf(outf,"\
(geometry \"%s\" {\n\
	appearance { +edge material { diffuse 0 1 0 } }\n\
	INST transforms :%s\n\
        geom\n\
	{ INST\n\
	  transform %f 0 0 0\n\
		    0 %f 0 0\n\
		    0 0 %f 0\n\
		    0 0 0  1\n\
	  geom { \n", name, xformname, size, size, size);
  fprintf(outf,"%s", geom);
  fprintf(outf,"} } } )\n");
  fprintf(outf,"(normalization %s none)\n", name);
  fprintf(outf,"(bbox-draw %s off)\n", name);
  EraseHeadTail(xformname);
}

void
defhead(float size)
{
  progn();
  defheadtail(headname, headxformname, headgeom, size);
  fprintf(outf, "(pickable %s no)\n", headname);
  endprogn();
}

void
deftail(float size)
{
  progn();
  defheadtail(tailname, tailxformname, tailgeom, size);
  endprogn();
}

void
ShowTailAt(Point3 *p)
{
  fprintf(outf, "(read geometry { define %s ", tailxformname);
  fprintf(outf, "{ TLIST 1 0 0 0  0 1 0 0  0 0 1 0  %f %f %f 1 } } )\n",
	  p->x, p->y, p->z);
}

void
ShowHeadAt(Point3 *p, Point3 *prev)
{
  HPoint3 dir;		/* Should be a Point3, but TmRotateTowardZ()
			 * wants HPoint3, even though it ignores w component.
			 */
  Transform R,T,S;

  Pt3Sub(p, prev, (Point3 *)(void *)&dir);
  if (Pt3Length((Point3 *)(void *)&dir) == 0.0) {
    dir.x = dir.y = 0;
    dir.z = 1;
  } else {
    Pt3Unit((Point3 *)(void *)&dir);
  }

  TmRotateTowardZ(R, &dir);
  TmInvert(R,R);
  TmTranslate(T, p->x, p->y, p->z);
  TmConcat(R, T, S);

  fprintf(outf, "(read geometry { define %s { TLIST\n", headxformname);
  fputtransform(outf, 1, (float *)S, 0);
  fprintf(outf, "} } )\n");
}

void
EraseHeadTail(char *xformname)
{
  fprintf(outf,  "(read geometry { define %s { LIST } } )\n", xformname);
}

void EraseHead(void)
{
  EraseHeadTail(headxformname);
}

void EraseTail()
{
  EraseHeadTail(tailxformname);
}

void EraseHeadAndTail(void)
{
  progn();
  EraseHead();
  EraseTail();
  endprogn();
}

void DeleteHeadAndTail(void)
{
  progn();
  fprintf(outf, "(delete %s)\n", headname);
  fprintf(outf, "(delete %s)\n", tailname);
  endprogn();
}

/* Rename the old vector (so that different vectors can have different
   appearances) */

void StartNewVector(void)
{
  static int which = 0;
  progn();
  fprintf(outf, "(name-object graffiti graffiti%d)", ++which);
  endprogn();
}

void progn(void)
{
  fprintf(outf, "(progn\n");
}

void endprogn(void)
{
  fprintf(outf,")\n");
  fflush(outf);
}

