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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "enum.h"
#include "ooglutil.h"
#define CHUNKSIZE 10000

	typedef struct entry {char word[MAXDEPTH]; } entry;
	static entry *stack = NULL, *old, *oldtop, *oldbase, *new;
	static int numchunks = 1;
	/* new points to the next empty space in the stack */
	/* old points to the first non-empty space in the stack */

void	
init_stack()
{
	if (stack) OOGLFree(stack);
	stack = OOGLNewN(entry, CHUNKSIZE);
	if (stack == NULL) exit(fprintf(stderr,"init_stack: no space\n"));
	old =  oldbase = new = stack;
	oldtop = stack - 1;
}

void
make_new_old()
{
	/* oldbase and oldtop are inclusive limits if non-empty words */
	oldbase = oldtop+1;
	old = oldtop = new-1;
}

char *
pop_old_stack()
{
	if (old >= oldbase)	return((old--)->word);
	else return(NULL);
}

int
push_new_stack(word)
char *word;
{
	entry * oldstack;
	if (new >= stack + numchunks * CHUNKSIZE)	
	    {
	    oldstack = stack;
	    numchunks *= 2;
            if ((stack = OOGLRenewN(entry, stack, numchunks * CHUNKSIZE )) == (entry *) NULL) return (0);
	    new = (entry *) stack + (new - oldstack);
	    old = (entry *) stack + (old - oldstack);
	    oldtop = (entry *) stack + (oldtop - oldstack);
	    oldbase = (entry *) stack + (oldbase - oldstack);
	    if (stack == NULL)
		exit(fprintf(stderr,"push_new_stack: no space\n"));
	    }

 	strcpy(new->word, word);
	new++;
	return 0;
}	

