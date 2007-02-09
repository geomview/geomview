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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _TRANSOBJ_
#define _TRANSOBJ_

#ifndef _HANDLE_
#  include "handle.h"
#endif /*_HANDLE_*/

#define	TRANSMAGIC	OOGLMagic('t', 1)

typedef struct TransObj {
	REFERENCEFIELDS;
	Transform T;
} TransObj;

extern void TransPosition( TransObj *tobj, Transform Tinto );
extern void TransTransformTo( TransObj *tobj, Transform Tfrom );
extern void TransDelete( TransObj *tobj );

extern int TransStreamIn( Pool *, Handle **, Transform );
extern int TransStreamOut( Pool *, Handle *, Transform );
extern TransObj *TransCreate( Transform );

extern void TransUpdate( Handle **transhandle, Ref *ignored, Transform );


#endif /*_TRANSOBJ_*/
