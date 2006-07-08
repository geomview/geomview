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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef WINDOWDEF
#define WINDOWDEF

#include <stdio.h>
#include <stdarg.h>
#include "reference.h"

#include "ooglutil.h"
#include "streampool.h"

typedef struct {
	int xmin, xmax, ymin, ymax;
} WnPosition;

typedef struct WnWindow WnWindow;

/* Public WnWindow methods: */

extern WnWindow *WnCreate(int firstattr, ... /*, WN_END */);
extern WnWindow *WnSet(WnWindow *, int firstattr, ... /*, WN_END */);
extern int	 WnGet(WnWindow *, int attr, void *value);
extern void	 WnDelete(WnWindow *);
extern WnWindow *WnCopy(WnWindow *);
extern WnWindow *WnMerge(WnWindow *src, WnWindow *dst);

extern WnWindow *_WnSet(WnWindow *win, int firstattr, va_list *alist);


extern int WnStreamOut( Pool *, Handle *, WnWindow * );
extern int WnStreamIn( Pool *p, Handle **hp, WnWindow **wp );

/*
 * Attributes for WnCreate(), WnSet(), WnGet().
 *
 * Supplying a NULL pointer, or -1 value, for one of these
 * unsets the corresponding value.
 * 
 * Initially, all values are unset.
 *
 * WnGet()'s returned value
 *	on an invalid token is -1
 *	on an unset value is 0
 *	on a  set value is 1
 */
				/* Set/Create type	Get type	*/
#define	WN_END		900	/* ---------------	--------	*/
#define WN_XSIZE	901	/* int			int *		*/
#define WN_YSIZE	902	/* int			int *		*/
#define WN_PREFPOS	903	/* WnPosition *		WnPosition *	*/
#define	WN_VIEWPORT	904	/* WnPosition *		WnPosition *	*/
#define	WN_CURPOS	905	/* WnPosition *		WnPosition *	*/
#define	WN_NAME		906	/* char *		char **		*/
/*
 * The following attributes always have a value (may not be "unset"):
 */
#define WN_ENLARGE	907	/* int			int *		*/
#define WN_SHRINK	908	/* int			int *		*/
#define WN_NOBORDER	909	/* int			int *		*/
#define WN_ASPECT	910	/* --- unsettable ---	float *		*/
#define	WN_PIXELASPECT	911	/* float		float *		*/

#define WN_ABLOCK	912	/* void **ablock	--ungettable--	*/
/*
  WN_END:	end of attribute list
  WN_XSIZE:	desired window width in pixels
  WN_YSIZE:	desired window height in pixels
  WN_PREFPOS:	desired absolute position of window (pixels)
  WN_VIEWPORT:	viewport relative to window (pixels)
  WN_CURPOS:	window's real position (pixels)
  WN_NAME:	window name
  WN_ENLARGE:	window enlargable from pref size? (1=yes, 0=no) (default:1)
  WN_SHRINK:	window shrinkable from pref size? (1=yes, 0=no) (default:1)
  WN_NOBORDER:	draw window without border? (1=yes, 0=no)	(default:0)
  WN_ASPECT:	the window's current aspect ratio; the window package updates
		  this automatically when you set the CURPOS attribute.
  WN_PIXELASPECT: pixel aspect ratio, i.e. screen X pixel size / Y pixel size.
		Default is 1.0, but may change e.g. for stereo hardware.
		Taken into account by WN_ASPECT */

#endif /* WINDOWDEF */
