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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "geomclass.h"
#include "handleP.h"

int PoolDoCacheFiles;

void
GeomDelete( object )
    Geom *object;
{
    if (object == NULL)
	return;

    if (!GeomIsMagic(object->magic)) {
	OOGLWarn("Internal warning: GeomDelete of non-Geom %x (%x !~ %xxxxx)",
	    object, object->magic, (GeomMagic(0,0)>>16)&0xFFFF);
	return;
    }
    /* If we're not caching contents of files, and this object was loaded
     * from a file, and the sole reference to it is from the Handle,
     * delete it now (and delete the handle and possibly close the file).
     */
    switch (RefDecr((Ref *)object)) {
    case 1:
	if(object->handle && object->handle->whence
		&& object->handle->object == (Ref *)object
		&& !PoolDoCacheFiles) {

	    HandleDelete(object->handle);
	}
	return;
    default:
	if(object->ref_count < 0 || object->ref_count > 100000) { /* XXX debug */
	    OOGLError(1, "GeomDelete(%x) -- ref count %d?", object,
		object->ref_count);
	    return;
	}
	return;
    case 0:
	/* Actually delete it */;
    }

    if(object->ap) {
	ApDelete(object->ap);
	object->ap = NULL;
    }
    if(object->aphandle)
	HandlePDelete(&object->aphandle);
    if(object->Class->Delete) {
	(*object->Class->Delete)(object);
    }
    if(object->handle && HandleObject(object->handle) == (Ref *)object)
	HandleDelete(object->handle);
    object->magic ^= 0x80000000;
    OOGLFree(object);
}
