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

#ifndef DGFLAGDEFS
#define DGFLAGDEFS

#define DG_HYPERBOLIC	1
#define DG_EUCLIDEAN	2
#define DG_SPHERICAL	4
#define DG_FINITE	8
#define DG_TRANSPOSED	16

#define DG_CONFORMALBALL	32
#define DG_UPPERHALFSPACE	64
#define DG_PROJECTIVEMODEL	128
#define DG_NUM_ATTR	8	/* the above are the attributes */

#define DG_METRIC_BITS		(DG_HYPERBOLIC | DG_EUCLIDEAN | DG_SPHERICAL)
#define DG_MODEL_BITS		(DG_CONFORMALBALL | DG_UPPERHALFSPACE | DG_PROJECTIVE)

/* for the matrix group entries */
#define DG_GENERAL	0x000	/* general invertible group: default */
#define DG_SPECIAL	0x100	/* determinant +/- 1 */
#define DG_ORTHOGONAL	0x200	/* M.Transpose(M) = d.I */
#define DG_PROJECTIVE	0x400	/* M.Transpose(M) = d.I */
#define DG_REAL		0x000	/* default is real */
#define DG_COMPLEX	0x800	/* default is real */

#define DG_TMP		0x10000

/* flag */
#define DG_CENTERCAM	1
#define DG_NEWDIRDOM	2	/* recompute dirdom */
#define DG_DRAWDIRDOM	4
#define DG_ZCULL	8	/* don't draw those behind the camera */
#define DG_DRAWCAM	0x10		
#define DG_DDBEAM	0x20	/* compute beamed dirichlet domain */
#define DG_DDSLICE	0x40	/* slice off cusps in dirichlet domain */
#define DG_DEBUG	0x100
#define DG_SAVEBIGLIST	0x200	/* does DiscGrpSave write out the group elements? */
#define DG_SAVEDIRDOM	0x400	/* does DiscGrpSave write out the dirichlet domain? */
#define DG_DRAWGEOM	0x800	/* draw other geometry? */
#define DG_NUM_DSPYATTR	5	/* zcull, cenercam, drawcam, drawdirdom, and drawgeom are externally settable */

/* constraint return values (only DG_CONSTRAINT_VALID is required) */
#define DG_CONSTRAINT_PRINT	1
#define DG_CONSTRAINT_TOOFAR	2
#define DG_CONSTRAINT_STORE	4
#define DG_CONSTRAINT_LONG	8
#define DG_CONSTRAINT_NEW	16
#define DG_CONSTRAINT_MAXLEN	32

/* bits in DiscGrpEl.attributes */
#define DGEL_IS_IDENTITY	0x1	/* need to mark the identity matrix */
#endif /*DGFLAGDEFS*/
