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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "polylistP.h"
	static int BlockSize, array_size;
	static Poly *mystack, *stackptr;
   	static int count = 0, debug = 0;

int initstack(void)
{
    	array_size = 1;
    	BlockSize = 1024;
	count = 0;
    	if ((mystack = OOGLNewN (Poly, BlockSize )) == (Poly *) NULL) return(0);
	stackptr = mystack;
	return(1);
}
	
int
push(pp)
Poly *pp;
{
    int i;
    if (stackptr >= &mystack[BlockSize*array_size])	{ 
	if (debug) 
	    fprintf(stderr,"allocating again: size is now %d\n",array_size*BlockSize);
	array_size = array_size*2;
        if ((mystack = OOGLRenewN(Poly,mystack, array_size*BlockSize)) == (Poly *) NULL) return (0);
	stackptr = &mystack[count];
	}
    *stackptr = *pp;
    if ((stackptr->v = OOGLNewN(Vertex *, pp->n_vertices)) == (Vertex **) NULL)
	return(0);
    for (i=0; i<pp->n_vertices; ++i)  stackptr->v[i] = pp->v[i];
    stackptr++;
    count++;
    return(1);
}
    
int
getsize(void)
{
    return(count);
}

Poly *
getstack()
{
    return(mystack);
}
