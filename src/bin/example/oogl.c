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

/* oogl.c */
/* geomview communication code */
/* Charlie Gunn & Tamara Munzner */
/* 9/92 */

#include <stdio.h>
#include "geom.h"
#include "meshflag.h"

FILE *f = NULL;
static char *getline(char *s);

Begin_OOGL()
{
  if (f==NULL) { f = stdout; }
  fprintf(f, "(geometry example { : exhandle })\n");
  fprintf(f, "(echo 'caughtup\n')");
  fflush(f);
}

UpdateOOGL(x_size, y_size, gridunit, data)
     int x_size, y_size;
     float gridunit;
     float data[];
{
        register int    x,y,k;
        Point3  *points;
        Geom *mesh;
	char line[80];
	char *caughtup;

	if (f==NULL) { f = stdout; }

	if (caughtup = fgets(line, 10, stdin)) {
	  points = OOGLNewN(Point3, x_size*y_size);
	  
	  
	  for (k = 0, y=0; y<y_size; ++y)
	    {
	      for (x=0; x<x_size; ++x, ++k)
                {
		  points[k].x = x*gridunit;
		  points[k].y = y*gridunit;
		  points[k].z = data[k];
                }
	    }
	  mesh = GeomCreate("mesh", 
			    CR_NOCOPY, /* don't copy the points */
			    CR_FLAG, MESH_Z, 
			    CR_NU, x_size, 
			    CR_NV, y_size, 
			    CR_POINT, points,
			    CR_END);
	  
	  fprintf(f, "(read geometry { define exhandle \n");
	  GeomFSave(mesh, f, NULL);
	  fprintf(f, "})\n");
	  fprintf(f, "(echo 'caughtup\n')");
	  fflush(f);
	  OOGLFree(mesh);
	}
}



/* Stolen from Mark Phillips' Hinge module */
static char *getline(char *s)
{
  static char *p;
  char *first;

  if (s != NULL) {
    p = s;
  } else {
    *p = '\n';
  }
  ++p;
  first = p;
  while (*p != '\n' && *p != '\0') ++p;
  if (*p == '\n') {
    *p = '\0';
    return first;
  }
  return NULL;
}
