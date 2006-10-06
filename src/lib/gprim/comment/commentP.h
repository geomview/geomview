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


/*
 * Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Nathaniel Thurston
 */

#ifndef COMMENTPDEFS
#define COMMENTPDEFS

/*
 * Private definitions for Comments.
 */

#include "comment.h"
#include "bboxP.h"	/* includes geomclass.h */

#define COMMENTMAGIC	GeomMagic('c', 1)

struct Comment {
    GEOMFIELDS;
    char	*name;
    char	*type;
    int		length;		/* 0 if data is ASCII */
    char	*data;
};

extern GeomClass *CommentClass;		/* Public read-only; valid only
					 * after first call to CommentMethods()
					 */

extern Comment *CommentCreate( Comment *exist, GeomClass *Class, va_list *attrs );
extern void  CommentDelete( Comment * );
extern Comment *CommentCopy( Comment * );
extern Comment *CommentDice( Comment *, int nu, int nv );

#endif /*COMMENTPDEFS*/
