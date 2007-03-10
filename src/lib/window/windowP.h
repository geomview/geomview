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

#ifndef WINDOWPDEF
#define WINDOWPDEF

#include <stdarg.h>
#include "window.h"

/* Private definitions for Window package */

/*
 * Private flag bits:
 * "flag" indicates whether corresponding field valid,
 * "changed" indicates whether changed since someone last unset "changed".
 */
#define	WNF_ENLARGE	0x01	/* Allow window to be enlarged from pref size*/
#define	WNF_SHRINK	0x02	/* Allow window to be shrunk */
#define WNF_NOBORDER	0x04	/* Draw window without frame */

#define WNF_HASPREF	0x10	/* Requested position */
#define WNF_HASSIZE	0x20	/* Requested size */
#define	WNF_HASVP	0x40	/* Drawing viewport within window */
#define	WNF_HASCUR	0x80	/* Window's "current position" */
#define	WNF_HASNAME	0x100	/* Window's title */
#define	WNF_HASPIXASPECT 0x200	/* pixel aspect ratio? */

struct WnWindow {
    REFERENCEFIELDS;
    int		flag;		/* Which fields are set?	 */
    int		xsize, ysize;	/* Window size			 */
    WnPosition	pref;		/* Requested abs win position	 */
    WnPosition	cur;		/* Current position		 */
    WnPosition	viewport;	/* Relative viewport in win	 */
    char	*win_name;	/* Window title			 */
    float	aspect;		/* X/Y Aspect ratio (of cur pos) */
    float	pixaspect;	/* Aspect ratio of hardware pixels */
    int		changed;	/* These fields changed		 */
};

extern WnWindow *_WnSet(WnWindow *, int firstattr, va_list *alist);

#define	WINDOWMAGIC	OOGLMagic('w', 1)

#endif	/* WINDOWPDEF */

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
