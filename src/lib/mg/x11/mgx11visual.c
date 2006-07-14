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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "mgP.h"
#include "mgx11P.h"

int mgx11_getvisual( Display *dpy, Visual **mgvis,
				Colormap *mgcol, int *mgdepth)
{
  static XVisualInfo vTemplate;
  static XVisualInfo *visualList;
  unsigned int nplanes = 0;
  unsigned long planemasks[1];
  unsigned long testcols[30];
  int	      mgpriv = 0;
  int         visualsMatched;


  vTemplate.screen = DefaultScreen(dpy);
  vTemplate.class = TrueColor;
  *mgdepth = vTemplate.depth = 24;
  visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask |
                              VisualDepthMask, &vTemplate, &visualsMatched);
  if (visualsMatched == 0)
  {
	*mgdepth = vTemplate.depth = 16;
	visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask |
				 VisualDepthMask, &vTemplate, &visualsMatched);
  }

  if (visualsMatched == 0) {
	vTemplate.depth = 15;  /* But leave mgdepth == 16 (2-byte data) */
        visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask |
				 VisualDepthMask, &vTemplate, &visualsMatched);
  }
  if (visualsMatched == 0) {
	vTemplate.depth = 12;  /* Ditto, leave mgdepth == 16 for 2-byte data */
        visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask |
				 VisualDepthMask, &vTemplate, &visualsMatched);
  }
  if (visualsMatched == 0)
  {
	vTemplate.class = PseudoColor;
	*mgdepth = vTemplate.depth = 8;
	visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask |
			     VisualDepthMask, &vTemplate, &visualsMatched);
  }
  if (visualsMatched == 0)
  {
	 *mgdepth = vTemplate.depth = 1;
	 visualList = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask,
					  &vTemplate, &visualsMatched);
  }
  if (visualsMatched == 0)
    return MG_X11VISFAIL; /* Didn't find a useful visual! */

  *mgvis = visualList[0].visual;

  if (*mgdepth == 8)
  {
    if (!XAllocColorCells(dpy,
		DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy)),
		False, planemasks, nplanes, testcols, (unsigned int)30))
      /* there aren't enough colors so use private colormap instead */
      mgpriv = 1;
    else
    {
      /* there *are* enough colors in shared colormap */
      mgpriv = 0;
      XFreeColors(dpy, DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy)),
		testcols, 30, nplanes);
    }
  }

  

  if (*mgdepth == 16 || *mgdepth == 24)
  {
    *mgcol = XCreateColormap(dpy, RootWindow(dpy, DefaultScreen(dpy)),
                                *mgvis, AllocNone);
  }
  else
    if (*mgdepth == 1)
      *mgcol = DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy));
    else
      if (*mgdepth == 8) {
        if (!mgpriv)
          *mgcol = DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy));
        else
          *mgcol = XCreateColormap(dpy,
                RootWindow(dpy, DefaultScreen(dpy)), *mgvis, AllocNone);
      }

  if (mgpriv)
    return MG_X11VISPRIVATE;
  else
    return MG_X11VISSUCCEED;

}

/*****************************************************************************/
