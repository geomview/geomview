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
#include <X11/Xlib.h>
#define MG_X11WINID	101     /* for passing an X window id */
#define MG_X11PARENT    102	/* for passing an X parent window id */
#define MG_X11DISPLAY   103	/* for passing an X display id */
#define MG_X11VISIBLE   104	/* window visible */
#define MG_X11SIZELOCK  105	/* for locking size of window */
#define MG_X11DELETE    106	/* callback for window deletion */
#define MG_X11PIXID     107	/* for passing an X pixmap id */
#define MG_X11VISUAL    108     /* for visual */
#define MG_X11EXPOSE	109	/* expose event */
#define MG_X11COLORMAP	110	/* for passing colormap */

extern mgcontext *mgx11_findctx( Window );
extern unsigned long mgx11_RGB(int, int, int);
extern unsigned long mgx11_setRGB(int, int, int);

extern int mgx11_getvisual( Display *,  Visual **, Colormap *, int * );

/* return values from mgx11_getvisual: */
#define MG_X11VISFAIL     0	/* Failed to find a reasonable visual */
#define MG_X11VISSUCCEED  1	/* Succeeded in finding a reasonable visual */
#define MG_X11VISPRIVATE  2	/* Succeeded + Visual needs a priv colormap */
