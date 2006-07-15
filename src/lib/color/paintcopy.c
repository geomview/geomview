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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "color.h"

void
PaintCopy( f, b, t, m )
    ColorA *f, *b, *t;
    float *m;
{
    float a = 1.0 - *m;

    t->r = *m * f->r + a * b->r;
    t->g = *m * f->g + a * b->g;
    t->b = *m * f->b + a * b->b;
    t->a = *m * f->a + a * b->a;
}

void
PaintCopyN( f, b, t, m, n )
    ColorA *f, *b, *t;
    float *m;
    int n;
{
    float a;
    
    while( n-- ) {
	a = 1.0 - *m;
	t->r = *m * f->r + a * b->r;
	t->g = *m * f->g + a * b->g;
	t->b = *m * f->b + a * b->b;
	t->a = *m * f->a + a * b->a;
	f++;
	b++;
	t++;
	m++;
    }
}
