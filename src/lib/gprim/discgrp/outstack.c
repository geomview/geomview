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

/* This file is for managing the output stack of group elements */

#include "discgrpP.h"

	static int BlockSize, array_size;
	static DiscGrpEl *mystack = NULL, *stackptr = NULL;
   	static int count = 0, debug = 0;

int
init_out_stack()
{
    	array_size = 1;
    	BlockSize = 1024;
	count = 0;
    	if ((mystack = OOGLNewN (DiscGrpEl, BlockSize )) == (DiscGrpEl *) NULL) return(0);
	stackptr = mystack;
	return(1);
}
	
int
enumpush(pp)
DiscGrpEl *pp;
{
    if (stackptr >= &mystack[BlockSize*array_size])	{ 
	if (debug) 
	    fprintf(stderr,"allocating again: size is now %d\n",array_size*BlockSize);
	array_size = array_size*2;
        if ((mystack = OOGLRenewN(DiscGrpEl,mystack, array_size*BlockSize)) == (DiscGrpEl *) NULL) return (0);
	stackptr = &mystack[count];
	}
    *stackptr = *pp;
    TmCopy(pp->tform, stackptr->tform); 
    stackptr++;
    count++;
    return(1);
}
    
int
enumgetsize()
{
    return(count);
}

DiscGrpEl *
enumgetstack()
{
    DiscGrpEl *thisptr;
    thisptr = OOGLNewN (DiscGrpEl, count );
    if (thisptr == NULL) return ( (DiscGrpEl *) NULL);
    memcpy(thisptr, mystack, sizeof(DiscGrpEl) * count);
    OOGLFree(mystack);
    return(thisptr);
}
