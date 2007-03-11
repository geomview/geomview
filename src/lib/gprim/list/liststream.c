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

#include <stdio.h>
#include "inst.h"
#include "streampool.h"
#include "transobj.h"
#include "handleP.h"
#include "listP.h"


List *
ListImport( Pool *p )
{
    List *list = NULL;
    List *new, **tailp = &list;
    int c, prevc = 0;
    int brack = 0;
    int any = 0;
    IOBFILE *file;

    if(p == NULL || (file = PoolInputFile(p)) == NULL)
	return NULL;

    if(strcmp(GeomToken(file), "LIST") != 0)
	return NULL;

    while((c = iobfnextc(file, 0)) != EOF) {
	switch(c) {
	case ';':
	case ')':
	    goto done;
	case CBRA:
	    brack++;
	    iobfgetc(file);
	    continue;
	case CKET:
	    if(--brack < 0) goto done;
	    iobfgetc(file);
	    if(prevc == CBRA) {
		/* Interpret "{ }" as a NULL entry in a list.  Leave it
		 * there as a placeholder -- could be deliberate.
		 */
		new = (List *)GeomCCreate(NULL, ListMethods(), CR_END);
		*tailp = new;
		tailp = &new->cdr;
		any = 1;
	    }
	    break;
	default:
	    new = (List *)GeomCCreate(NULL, ListMethods(), CR_END);
	    if(GeomStreamIn(p, &new->carhandle, &new->car)) {
		*tailp = new;
		tailp = &new->cdr;
		if(new->carhandle)
		    HandleRegister(&new->carhandle,
				   (Ref *)new, (Ref **)(void *)&new->car,
				   HandleUpdRef);
	    } else {
		OOGLSyntax(file,
		 "Couldn't read LIST in \"%s\": error reading embedded object",
		 p->poolname);
		GeomDelete((Geom *)new);
		GeomDelete((Geom *)list);
		return NULL;
	    }
	    any = 1;
	    break;
	}
	prevc = c;
    }

    /*
     * If no list members appeared at all, call it a valid, empty List.
     * But if there were list members but none were readable, fail.
     * Hence the "any" flag.
     */
  done:
    if(brack > 0)
	OOGLSyntax(file, "Couldn't read LIST in \"%s\": too many %c's",
		p->poolname, CBRA);
    if(!any)
	return (List *)GeomCCreate(NULL, ListMethods(), CR_END); /* null LIST */
    return list;
}

int
ListExport(List *l, Pool *p)
{
    if(p == NULL || p->outf == NULL) {
	return 0;
    }

    PoolFPrint(p, p->outf, "LIST\n");
    while(l != NULL) {
	PoolFPrint(p, p->outf, "");
	if(!GeomStreamOut( p, l->carhandle, l->car )) {
	    return 0;
	}
	l = l->cdr;
    }
    return 1;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
	
