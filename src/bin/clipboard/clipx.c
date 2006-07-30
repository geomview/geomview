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

#include <stdio.h>
#include <math.h>
#include "mg.h"
#include "geom.h"
#include "mibload.h"
#include "mgx11.h"
#include "clip.h"

void winexpose(Widget, XtPointer,
	XmDrawingAreaCallbackStruct *);

extern Geom *theObject;
extern Visual *visual;
extern Colormap colormap;
extern int bitdepth;

ColorA white = {1.0, 1.0, 1.0, 1.0};
ColorA red = {1.0, 0, 0, 1.0};
ColorA cameragray = {1.0/3.0, 1.0/3.0, 1.0/3.0, 1.0};
ColorA darkgray = {0.2, 0.2, 0.2, 1.0};
ColorA darkblue = {0, 0, .2, 1.0};
Point lightpos1 = {1, .3, .7, 0.0};
Point lightpos2 = {-0.5, 1.0, 0.5, 0.0};
ColorA lightcol2 = {0.6, 0.6, 0.6, 1.0};
Point lightpos3 = {0.0, -2.0, 1.0, 0.0};
ColorA lightcol3 = {0.4, 0.4, 0.4, 1.0};


Camera		*theCamera;
Appearance	*theAppearance;
LmLighting	*lightingModel;
LtLight		*theLight;
mgcontext	*mgctx;

void clipboard_init(Widget parent)
{
  Widget	MG_DrawArea;

  MG_DrawArea = XtNameToWidget(parent, "DrawingArea");

  mgdevice_X11();

  theObject = NULL;

  theCamera = CamCreate( CAM_FOV, 40.0, CAM_END );

  lightingModel = LmCreate( LM_AMBIENT, &darkgray,
			    LM_REPLACELIGHTS, 1,
			    LM_END);

  theLight = LtCreate( LT_COLOR, &white,
		       LT_POSITION, &lightpos1,
		       LT_INTENSITY, 1.0, LT_END);

  LmAddLight(lightingModel, theLight);

  theLight = LtCreate( LT_COLOR, &lightcol2,
		       LT_POSITION, &lightpos2,
		       LT_INTENSITY, 1.0, LT_END);

  LmAddLight(lightingModel, theLight);

  theLight = LtCreate( LT_COLOR, &lightcol3,
		       LT_INTENSITY, 1.0, LT_END);

  LmAddLight(lightingModel, theLight);

  theAppearance = ApCreate( AP_DO, APF_FACEDRAW | APF_SHADING | APF_EDGEDRAW |
			    APF_VECTDRAW, AP_MtSet, MT_Kd, 1.0, MT_DIFFUSE,
			    &white, MT_END, AP_LGT, lightingModel,
			    AP_SHADING, APF_FLAT, AP_END);

  mgctx = mgctxcreate(MG_X11DISPLAY, XtDisplay(MG_DrawArea),
		      MG_X11COLORMAP, colormap,
		      MG_BITDEPTH, bitdepth,
		      MG_X11VISUAL, visual,
		      MG_X11WINID, XtWindow(MG_DrawArea),
		      MG_CAMERA, theCamera,
		      MG_BACKGROUND, &cameragray /*&darkblue */, 
		      MG_APPEAR, theAppearance, MG_END);

  XtAddCallback(MG_DrawArea, XmNexposeCallback, (XtCallbackProc)winexpose, 0);
  XtVaSetValues(MG_DrawArea, XmNbackground,
		BlackPixel(XtDisplay(MG_DrawArea),
		DefaultScreen(XtDisplay(MG_DrawArea))), NULL);

}

void redraw(Widget w)
{
  mgreshapeviewport();
  mgworldbegin();
  GeomDraw(theObject);
  mgworldend();
}


void winexpose(Widget w, XtPointer stuff, XmDrawingAreaCallbackStruct *data)
{
  mgreshapeviewport();
  mgworldbegin();
  GeomDraw(theObject);
  mgworldend();
}
