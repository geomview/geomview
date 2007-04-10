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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tcl.h>
#include	<tk.h>
#include 	"TransformGeom.h"
#include	"WithColor.h"
#include	"geom.h"
#include	"geomclass.h"
#include	"hpointn.h"
#include	"transformn.h"

extern IOBFILE *infile;

int DoProjection(ClientData clientdata, Tcl_Interp * interp,
		 int argc, const char **argv)
{
  char scratch[16];
  TransformN *ObjUniv, *UnivCam;
  Geom *g;
  int axes[4];
  FILE *fileptr;
  if (argc != 5) {
    interp->result = "Wrong number of arguments";
    return TCL_ERROR;
  }
  printf("(if (real-id \"%s\") (write geometry - \"%s\" self) (echo \"nada\"))\n",
	 argv[1], argv[1]);
  fflush(stdout);
  g = GeomFLoad(infile, "stdin");
  if (g == NULL) {
    interp->result = "NoObj";
    iobfrewind(infile);
    return TCL_OK;
  }
  printf("(echo(ND-xform-get \"%s\" universe))\n", argv[1]);
  fflush(stdout);
  ObjUniv = TmNRead(infile, 0);
  printf("(echo(ND-xform-get universe \"%s\"))\n", argv[2]);
  fflush(stdout);
  UnivCam = TmNRead(infile, 0);
  iobfrewind(infile);
  printf("(echo(ND-axes \"%s\")\\n)\n", argv[2]);
  fflush(stdout);
  iobftoken(infile, 0);
  if (iobfgetni(infile, 4, axes, 0) != 4) {
    iobfrewind(infile);
    interp->result = "Wrong reply to ND-axes";
    return TCL_ERROR;
  }
  /* consume the remainder of the answer */
  iobfgets(scratch, sizeof(scratch), infile);
  iobfrewind(infile);
  if (atoi(argv[4]))
    g = GeomProjCamWC(g, ObjUniv, UnivCam, axes,
		      (char *) argv[1], (char *) argv[2]);
  else
    g = GeomProjCam(g, ObjUniv, UnivCam, axes);
  if (g == NULL) {
    interp->result = "Error";
    return TCL_OK;
  }
  fileptr = fopen(argv[3], "w");
  GeomFSave(g, fileptr, (char *) argv[3]);
  fclose(fileptr);
  interp->result = "yes";
  return TCL_OK;
}
