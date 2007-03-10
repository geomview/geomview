/* Copyright (C) 2006 Claus-Justus Heine
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


#ifndef _NTRANSOBJ_H_
#define _NTRANSOBJ_H_

#ifndef _HANDLE_
#  include "handle.h"
#endif /*_HANDLE_*/

extern void NTransPosition(TransformN *tobj, TransformN *Tinto);
extern void NTransTransformTo(TransformN *tobj, TransformN *Tfrom);
extern void NTransDelete(TransformN *ntobj);

extern int NTransStreamIn(Pool *, Handle **, TransformN **);
extern int NTransStreamOut(Pool *, Handle *, TransformN *);
extern TransformN *NTransCreate(TransformN *);



#endif /*_TRANSOBJ_*/
