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

#include	<limits.h>
#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tk.h>

static Tk_PhotoHandle pp;
static Tk_PhotoImageBlock blk;


int parts;
double colors[20][3];
double heights[20];
double slopes[20][3];
double max, min;

double *
colorSingle(double z)
{
    double r = 0, g = 0, b = 0;
    int i;
    double *c;

    for (i=parts-1; i>=0; i--)
    {
	if (z == heights[i])
	{
	    r = colors[i][0];
	    g = colors[i][1];
	    b = colors[i][2];
	    break;
	}
	if (z > heights[i])
	{
	    r = (slopes[i][0]*(z-heights[i+1])) + colors[i+1][0];
	    g = (slopes[i][1]*(z-heights[i+1])) + colors[i+1][1];
	    b = (slopes[i][2]*(z-heights[i+1])) + colors[i+1][2];
	    break;
	}
    }
    c = (double *)malloc(3*sizeof(double));
    c[0] = r; c[1] = g; c[2] = b;
    return c;
}

int
setphotoCmd(ClientData data, Tcl_Interp *interp,
	    int argc, const char **argv)
{
  if (argc != 2)
  {
    Tcl_AppendResult(interp, argv[0],
		     ": argument should be <photo>", (char *) NULL);
    return TCL_ERROR;
  }
  pp = Tk_FindPhoto(interp, argv[1]);
  if (pp == NULL)
  {
    Tcl_AppendResult(interp, argv[0], ": photo window ", argv[1],
		     " not found", (char *) NULL);
    return TCL_ERROR;
  }
  blk.width = 432;
  blk.height = 20;
  blk.pixelSize = 3;
  blk.pitch = 3*432;
  blk.offset[0] = 0;
  blk.offset[1] = 1;
  blk.offset[2] = 2;
  blk.pixelPtr = malloc(432*20*3);
  return TCL_OK;
}

int
colorsCmd(ClientData data, Tcl_Interp *interp, int argc, const char **argv)
{
    int i, j, v;
    double z, r = 0, g = 0, b = 0;
    int red, green, blue;
    int sub = 0;
    unsigned char *buf=blk.pixelPtr;

    if (argc==5)
	return TCL_OK;
    if (argc<5)
    {
	Tcl_AppendResult(interp,
			 "wrong # args: should be <colors>\n",
			 (char *) NULL);
	return TCL_ERROR;
    }
    if ((argc-1)%4)
    {
	Tcl_AppendResult(interp,
			 "wrong # args: should be <colors>\n",
			 (char *) NULL);
	return TCL_ERROR;
    }

    argc-=1;
    argv+=1;
    parts=argc/4;
    for (i=0, j=0; i<parts; i++)
    {
        heights[j] = atof(argv[4*i]);
	if ((j!=0) && (heights[j] == heights[j-1]))
	{
	    sub++;
	    continue;
	}
	colors[j][0] = atof(argv[4*i+1])/255.0;
	colors[j][1] = atof(argv[4*i+2])/255.0;
	colors[j][2] = atof(argv[4*i+3])/255.0;
	j++;
    }
    parts -= sub;
    heights[0] = min;
    heights[parts-1] = max;
    for (i=0; i<parts-1; i++)
	for (j=0; j<3; j++)
	    slopes[i][j] = (colors[i+1][j]-colors[i][j])/
		(heights[i+1]-heights[i]);
    for (j=0; j<216; j++)
    {
	z = j*(max-min)/215.0+min;
	for (i=parts-1; i>=0; i--)
	{
	    if (z == heights[i])
	    {
		r = colors[i][0];
		g = colors[i][1];
		b = colors[i][2];
		break;
	    }
	    if (z > heights[i])
	    {
		r = (slopes[i][0]*(z-heights[i+1])) + colors[i+1][0];
		g = (slopes[i][1]*(z-heights[i+1])) + colors[i+1][1];
		b = (slopes[i][2]*(z-heights[i+1])) + colors[i+1][2];
		break;
	    }
	}
	red = (unsigned)(0xFFU*r);
	green = (unsigned)(0xFFU*g);
	blue = (unsigned)(0xFFU*b);
	for (v=0; v<20; v++)
	{
	    buf[j*2*3 + v*432*3] = red;
	    buf[j*2*3 + v*432*3 + 1] = green;
	    buf[j*2*3 + v*432*3 + 2] = blue;
	    buf[(j*2+1)*3 + v*432*3] = red;
	    buf[(j*2+1)*3 + v*432*3 + 1] = green;
	    buf[(j*2+1)*3 + v*432*3 + 2] = blue;
	}
    }
    Tk_PhotoPutBlock(pp, &blk, 0, 0, 432, 20, TK_PHOTO_COMPOSITE_OVERLAY);
    return TCL_OK;
}

int
minCmd(ClientData data, Tcl_Interp *interp, int argc, const char **argv)
{
    float f;
    if (argc!=2)
    {
	Tcl_AppendResult(interp, "invalid min value\n", (char *) NULL);
	return TCL_ERROR;
    }
    sscanf(argv[1], "%f", &f);
    min = f;
    return TCL_OK;
}

int
maxCmd(ClientData data, Tcl_Interp *interp, int argc, const char **argv)
{
    float f;
    if (argc!=2)
    {
	Tcl_AppendResult(interp, "invalid max value\n", (char *) NULL);
	return TCL_ERROR;
    }
    sscanf(argv[1], "%f", &f);
    max = f;
    return TCL_OK;
}

