/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/*
 * Random.c
 * author: Brian T. Luense
 * date: August 18,1994
 */


#include 	<stdlib.h>

/* Why can't SunOS define RAND_MAX like everyone else? *sigh* */

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

/*Returns a psuedo-random integer between i1 and i2.  Assumes srand has already
been seeded.*/

int irand(int i1, int i2)
{
     return (int)(((double)(i2)-(double)(i1))*((double)rand()/RAND_MAX)+i1);
}
