/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

/*
 * GeExDir.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#include	<tcl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tk.h>
#include	"geom.h"
#include	"geomclass.h"
#include	"hpointn.h"
#include	"transformn.h"
#include	"GeomExtentDirection.h"

extern IOBFILE *infile;

/*The following is C code meant to be compiled with a tcl/tk interpreter to
create two new tcl functions that communicate with Geomview.
The first expects four string arguments: 1. a vector (of dimension no more than 100)
2. The canonical name of a geom,
3. the canonical name of a coordinate system (e.g world, universe, etc.)
4. a boolean operator
It returns the scaled extent of the object in the given direction (considered in
the given coordinate system) in the following manner:
If the object does not exist it returns the string "NoObj"
If it could not figure out the objects extent it returns "Error"
If it encountered no problems it returns a string representation of the maximum and
minimum separated by a space
If the direction and object were of different dimensions it pads the smaller with
0's and returns the maximum and minimum as before except that the string begins with
the word "Pad"
If the boolean was true (i.e 1) then the results are exact.  If it was false (i.e 0)
it returns an approximate answer.


The second function verifies that the currently specified camera is a 
valid camera name*/

int GeomExDir(ClientData clientdata, Tcl_Interp * interp,
	      int argc, const char **argv)
{
  float *results, *direction;
  Geom *g;
  static char str[128];
  char *ptr[100];
  int n = 0, dimdir;
  TransformN *t;
  if (argc != 5) {
    interp->result = "wrong number of args";
    return TCL_ERROR;
  }

/*The following section of code parses the direction which was sent as a
string into an array of floats*/

  ptr[n] = strtok((char *) argv[1], " ");
  while (ptr[n] != NULL) {
    n++;
    ptr[n] = strtok(NULL, " ");
  }
  dimdir = n;
  direction = (float *) malloc((sizeof *direction) * dimdir);
  for (n = 0; n < dimdir; n++)
    direction[n] = atof(ptr[n]);

/*Ask geomview for data on the specified object.  If the object does not
exist return "NoObj"*/

  printf("(if (real-id %s) (write geometry - %s self) (echo \"nada\"))\n",
	 argv[2], argv[2]);
  fflush(stdout);
  g = GeomFLoad(infile, "stdin");
  if (g == NULL) {
    interp->result = "NoObj";
    iobfrewind(infile);
    return TCL_OK;
  }

/*Get the transform that takes the object to the world/universe*/

  printf("(echo(ND-xform-get %s %s))\n", argv[2], argv[3]);
  fflush(stdout);
  t = TmNRead(infile, 0);

/*Invert the transform so that it takes the world/universe to the object*/

/*	TmNInvert(t,t);*/

/*Get the extent of the object.  If there is a problem return error.
Otherwise, place the results in a string*/

  results = GeomExtentDir(g, t, dimdir, direction, atoi(argv[4]));
  if (results == NULL) {
    interp->result = "Error";
    return TCL_OK;
  }
  if (results[2] == 0.0)
    sprintf(str, "Pad %f %f", results[0], results[1]);
  else
    sprintf(str, "%f %f", results[0], results[1]);
  free(results);
  free(direction);
  interp->result = str;
  return TCL_OK;
}

/*verifies that the currently specified object is a valid object name*/
int ObjExistCheck(ClientData clientdata, Tcl_Interp * interp,
		  int argc, const char **argv)
{
  char *str;

  iobfrewind(infile);
  printf("(if (real-id %s) (echo yes\\n) (echo no\\n))\n", argv[1]);
  fflush(stdout);
  str = iobftoken(infile, 0);
  if (strcmp(str, "yes") == 0) {
    interp->result = "yes";
  } else if (strcmp(str, "no") == 0) {
    interp->result = "no";
  } else {
    static char msg[1024];

    sprintf(msg, "obj: \"%s\", answer: \"%s\"", argv[1], str);

    interp->result = msg /*"Error" */ ;
    return TCL_ERROR;
  }
  return TCL_OK;
}
