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
#include <stdlib.h>
#include "geom.h"
#include "streampool.h"
#include "transobj.h"
#include "origin.common.h"

static char targetname[255];

static int neutral;

static Pool *infile;

/* Return ui to state where there are no axes visible */
static void NeutralState() {
  neutral = 1;
  uiDisableDoneCancel();
  uiEnableShow();
  uiSetInstructions("Click on the \"Show Origin\"",
		    "button to display the",
		    "coordinate axes of the",
		    "named object.");
		    
}

/* Set ui to state where there are axes visible */
static void NonNeutralState() {
  neutral = 0;
  uiEnableDoneCancel();
  uiDisableShow();
  uiSetInstructions("Now move the axis around in",
		    "Geomview to move the origin of",
		    "the named object.  Then",
		    "click the \"Done\" button.");
		   
}

void internalsInit() {
  uiSetTargetname("targetgeom");

  infile = PoolStreamTemp(NULL, stdin, 0, NULL); 

  NeutralState();
}

void internalsShow() 
{
  Transform T;

  uiGetTargetname(targetname);

  printf("(echo \"{ \")");
  printf("(write transform - %s world)", targetname);
  printf("(echo } )");
  fflush(stdout);
  if (!TransStreamIn(infile, NULL, T)) {
    uiError("Unable to find transform of", "named item.  Perhaps",
	    "the object does not exist.");
    return;
  }

  printf("(progn ");
  printf("(geometry axes.%s < axes.list) ", targetname);
  printf("(normalization axes.%s none) ", targetname);
  printf("(xform-set axes.%s", targetname);
  fputtransform(stdout, 1, &T[0][0], 0);
  printf(") ");
  printf(")");
  fflush(stdout);
  NonNeutralState();
}

static void DeleteAxes() {
  printf("(delete axes.%s)", targetname);
  fflush(stdout);
}

void internalsDone() {
  Geom *g;
  Transform w2axes, w2target, axes2w, axes2target;
 
  printf("(echo \"{ \")");
  printf("(write transform - axes.%s world)", targetname);
  printf("(echo } )");
  fflush(stdout);
  if (!TransStreamIn(infile, NULL, w2axes)) {
    uiError("Unable to find axes.  Perhaps", "they were deleted in",
	    "Geomview.");
    NeutralState();
    return;
  }

  printf("(echo \"{ \")");
  printf("(write geometry - %s self)", targetname);
  printf("(echo } )");
  fflush(stdout);
  g = GeomFLoad(stdin, NULL);
  if (g == NULL) 
    uiError("Unable to find", "target object.  Perhaps", 
	    "it has been deleted.");
  else {
    printf("(echo \"{ \")");
    printf("(write transform - %s world)", targetname);
    printf("(echo } )");
    fflush(stdout);
    if (!TransStreamIn(infile, NULL, w2target)) {
      uiError("Unable to find transform of", "named item.  Perhaps",
	      "the object does not exist.");
      return;
    }
    TmInvert(w2axes, axes2w);
    TmConcat(axes2w, w2target, axes2target);
    GeomTransform(g, axes2target);

    printf("(progn ");
    printf("(geometry %s ", targetname);
    GeomFSave(g, stdout, NULL);
    printf(")");
    printf("(xform-set %s ", targetname);
    fputtransform(stdout, 1, &w2axes[0][0], 0);
    printf(")");
    printf(")");
    fflush(stdout);
  }

  DeleteAxes();
  NeutralState();
}

void internalsCancel() {
  DeleteAxes();
  NeutralState();
}

void internalsQuit() {
  PoolDelete(infile);
  exit(0);
}
  
