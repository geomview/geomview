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
#include <stdio.h>
#include "mg.h"
#include "mgP.h"
#include "mgrib.h"
#include "transform.h"
#include "bezierP.h"

typedef struct mgribcontext {
  struct mgcontext mgctx;	/* The mgcontext */
  int born;			/* Has window been displayed on the screen? */
  int win;			/* window ID, or 0 */

  int render_device;	/* Device Options */
#define RMD_ASCII	0x1
#define RMD_BINARY	0x2


/* Which line drawing technique?
 * MG_RIBPOLYGON for creating RIB tokens true to OOGL/geomview, but may appear
 *    incorrect when viewed dynamically
 * MG_RIBCYLINDER for creating RIB tokens which may be dynamically viewed, but
 *    may not be true to OOGL/geomview
 * MG_RIBPRMANLINE for NeXT Renderer - May Cause Incompatibilities!
 * Note: Future versions of renderman should support line drawing, NeXT does
 * now.
 */
  int line_mode;

  FILE *rib;
  char filepath[256];
#define DEFAULT_RIB_FILE "geom.rib"

  int display;		/* MG_RIBFRAME: render to framebuffer (screen window)
			   MG_RIBFILE: render to file  */
  char displayname[256];/* if display == MG_RIFILE, name of the file */
			   
  int backing;		/* MG_RIBDOBG: simulate colored background w/ polygon
  			   MG_RIBNOBG: no background simulation (defualt) */
  int shader;		/* MG_RIBSTDSHADE: uses standard shader
  			   MG_RIBEXTSHADE: uses extended shaders (eplastic,
			   	heplastic, hplastic) */
  char *shadepath;	/* path to extended shaders or user shaders */
  char ribscene[128];	/* scene name for RIB 1.0 file comments */
  char ribcreator[128]; /* creator field for RIB 1.0 file comments */
  			/* defaults to "mgrib driver" */
  char ribfor[128];	/* for(user) field for RIB 1.0 file comments */
  			/* defaults to user account name */
  char ribdate[128];	/* creation date, defualts to today's date */
} mgribcontext;

/* Make some convenient defines */
#define RiDEV  _mgribc->render_device
#define NotImplemented(name) fprintf(stderr,"%s is not yet implemented.\n",name);

#define _mgribc		((mgribcontext*)_mgc)
#define RI_INFINITY 1.0e38

void mgrib_printmatrix(Transform T);
void mgrib_drawline(HPoint3 *p1, HPoint3 *p2);
void mgrib_drawpoint(HPoint3 *p);
void mgrib_drawnormal(HPoint3 *p, Point3 *n);
