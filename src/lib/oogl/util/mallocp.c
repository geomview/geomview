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

#include <stdlib.h>	/* Declare malloc() */
#include "ooglutil.h"	/* Declare OOGLError */

#undef malloc
#undef free
#undef realloc
#undef calloc
#undef OOG_NewE
#undef OOG_RenewE
#undef OOGLFree

/*
 * Pointers needed for OOGLNew(), etc.  Placed in a separate module so
 * non-shared-memory users needn't link with that code.
 */
void *OOG_CurSM		= NULL;

void *(*OOG_NewP)(size_t)		= malloc;
void  (*OOGLFree)(void *)		= free;
void *(*OOG_RenewP)(void *, size_t)	= realloc;

void *
OOG_NewE(int n, char *msg)
{
    void *p;

    p = (*OOG_NewP)(n);
    if(p == NULL && n != 0) {
	OOGLError(1/*Fatal mem alloc*/,
	    "OOGLNew: couldn't allocate %d bytes: %s: %s", n, msg, sperror());
	exit(1);
    }
    return p;
}

void *
OOG_RenewE(void *p, int n, char *msg)
{
    p = (*OOG_RenewP)(p, n);
    if(p == NULL && n != 0) {
	OOGLError(1/*Fatal mem alloc*/,
		"OOGLRenewN: couldn't reallocate %d bytes (from %x): %s: %s",
		n, p, msg, sperror());
	exit(1);
    }
    return p;
}
