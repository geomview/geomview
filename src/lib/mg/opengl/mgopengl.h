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

/*
 * MG context attributes specific to GL
 *
 * MG_GLBORN may only be gotten.  Not set.
 */


#define MG_GLWINID	101	/* window id (returned by winopen) */
#define MG_GLBORN	102	/* born flag (has window been created)? */
#define MG_GLZMAX	103	/* max z-buffer value */
#define	MG_GLXDISPLAY	104	/* X11 Display *  (for mixed model) */
#define	MG_GLXSINGLEWIN	105	/* X Window id of single-buffered window */
#define	MG_GLXDOUBLEWIN	106	/* X Window id of double-buffered window */
#define	MG_GLXSINGLECTX	107	/* X11 context for single-buffered window */
#define	MG_GLXDOUBLECTX	108	/* X11 context for double-buffered window */

#define	MG_GLXSHARECTX	109	/* 'Get' another GLXcontext (or None) for sharing */

#define	MG_BGIMAGEFILE	110	/* Name of file containing background image */

extern mgcontext *mgopengl_findctx(int glwinid);
