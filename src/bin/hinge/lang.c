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
#include "hinge.h"
#include "hui.h"
#include "lang.h"
#include "ooglutil.h"
#include "pickfunc.h"

static Lake *inlake;

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
  handle_pick(id, &point, vn>0, &vertex, en>0, edge);
  return Lt;
})

#if 0
LObject *Lpicked(Lake *lake, LList *args)
/* (picked name (gotx goty gotz) (vx vy vz) (e0x e0y e0z e1z e1y e1z))
   all arguments are optional;  v and e lists may be empty */
{
  LObject *arg;
  char *name = NULL;
  Point3 g,v,e[2];
  int gn=3, vn=3, en=6, picked;

  LDECLARE(("picked", lake, args,
	    LARG_OPTIONAL,
	    LARG_STRING, &name,
	    LARG_HOLD, LARG_FLOAT_ARRAY, &g, &gn,
	    LARG_HOLD, LARG_FLOAT_ARRAY, &v, &vn,
	    LARG_HOLD, LARG_FLOAT_ARRAY, e, &en,
	    LARG_END));

  handle_pick(name, &g, vn>0, &v, en>0, e);
  return Lt;
}
#endif

void
handle_pick(char *name, HPoint3 *got,
	    int vert, HPoint3 *v, int edge, HPoint3 e[])
{
  int n;
  
  if (name) {
    if (sscanf(name, "geom%d", &n) != 1) return;
    TmCopy(TT[n], BaseT);
    if (edge) {
      DefineAxis(&e[0], &e[1]);
    }
    ShowAxis();
  }
  
  if (haveaxis) {
    int faceno, edgetest;
    char buf[80];
    
    
    faceno = -1;
    if (!edge && !vert) {
      faceno = WhichFace(got, TT[n], pl);
      if (faceno >= 0) {
	edgetest = PolyContainsEdge( currentedge,
				    TT[n], &(pl->p[faceno]) );
	if (edgetest) {
	  TmCopy(TT[n], BaseT);
	  HingeIt(edgetest);
	}
      }
      
    }
    
    if (faceno >= 0) {
      sprintf(buf, "face %1d of geom %1d; edgetest->%1d\n",
	      faceno, n, edgetest);
    } else {
      sprintf(buf, "no face\n");
    }
    hui_message(buf);
  }
}


void LangInit(IOBFILE *fromgv)
{
  inlake = LakeDefine(fromgv, stdout, NULL);

  LInit();
  LDefun("pick",  Lpick, Hpick);
}


void
Input()
{
  LObject *lit, *val;
  lit = LSexpr(inlake);
  val = LEval(lit);
  LFree(lit);
  LFree(val);
}
