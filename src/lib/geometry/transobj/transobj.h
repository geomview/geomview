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

#ifndef _GV_TRANSOBJ_H_
#define _GV_TRANSOBJ_H_

#include "reference.h"
#include "handle.h"
#include "transform.h"

#define	TRANSMAGIC	OOGLMagic('t', 1)

typedef struct TransObj {
	REFERENCEFIELDS;
	Transform T;
} TransObj;

extern void TransPosition(TransObj *tobj, Transform Tinto);
extern void TransTransformTo(TransObj *tobj, Transform Tfrom);
extern void TransDelete(TransObj *tobj);

extern int TransStreamIn(Pool *, Handle **, Transform T);
extern int TransStreamOut(Pool *, Handle *, Transform T);
extern TransformPtr TransFSave(Transform T, FILE *outf, char *fname);
extern int TransObjStreamIn(Pool *p, Handle **hp, TransObj **tobjp);
extern int TransObjStreamOut(Pool *, Handle *, TransObj *tobj);
extern TransObj *TransObjFSave(TransObj *t, FILE *outf, char *fname);
extern void TransUpdate(Handle **hp, Ref *ignored, Transform Tfixme);

extern TransObj *TransCreate(Transform);

#endif /*_TRANSOBJ_*/
