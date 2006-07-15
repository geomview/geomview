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

/*
 * Routines for operating on References -- pretty trivial, huh.
 */

#include "reference.h"
#include "ooglutil.h"

#ifndef NULL
# define NULL 0
#endif

void
RefInit(Ref *ref, long magic)
{
    ref->ref_count = 1;
    ref->magic = magic;
    ref->handle = NULL;
}

int
RefCount(Ref *ref)
{
    return ref ? ref->ref_count : 0;
}

Ref *
RefIncr(Ref *ref)
{
    if(ref) ++ref->ref_count;
    return ref;
}

int
RefDecr(Ref *ref)
{
    if(ref == NULL) return 0;
    if(--ref->ref_count < 0) {
	OOGLError(1, "RefDecr: ref %x count %d < 0!", ref, ref->ref_count);
    }
    return ref->ref_count;
}

int
RefMagic(Ref *ref) { return ref->magic; }

