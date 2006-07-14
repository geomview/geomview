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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "stdio.h"
#include "transform3.h"

# define MAXSTACK 32
static int maxstack = 0;
static Transform3 stack[MAXSTACK];

void
Tm3Push( T )
    Transform3 T;
{
    if( maxstack < MAXSTACK )
        Tm3Copy( T, stack[maxstack++] );
    else
	fprintf( stderr, "Stack Overflow at 32\n" );
}

void
Tm3Pop( T )
    Transform3 T;
{
    if( maxstack > 0 ) 
        Tm3Copy( stack[--maxstack], T );
    else
	fprintf( stderr, "Stack Underflow\n" );
}

