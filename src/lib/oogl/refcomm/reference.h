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

#ifndef REFCOMMDEF
#define REFCOMMDEF

/*
 * Public definitions for Reference & Communications package
 */

#define REFERENCEFIELDS	\
  int magic;		\
  int ref_count;	\
  struct Handle *handle

typedef struct Ref  {
  REFERENCEFIELDS;
} Ref ;

#define	REFINCR(type, obj)	(type *)RefIncr((Ref *)(obj))
#define	REFDECR(obj)		RefDecr((Ref *)(obj))

extern void RefInit( Ref *, int magic );/* Initializes a Reference */
extern int  RefCount( Ref * );		/* Returns current ref count */
extern Ref *RefIncr( Ref * );		/* Increments count, returns object */
extern int  RefDecr( Ref * );		/* Decrements count, returns it  */
extern int  RefMagic( Ref * );		/* Returns magic number */

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
