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

#include "string.h"
#include "geomclass.h"

static struct classtable {
    struct classtable *next;
    char *classname;
    GeomClass *Class;
} *table = NULL;

GeomClass *
GeomClassLookup( name )
    char *name;
{
    static char done = 0;
    struct classtable *cp;

    if(!done) {
	done = 1;
	GeomKnownClassInit();
    }
    for(cp = table; cp != NULL; cp = cp->next)
	if( strcmp( cp->classname, name ) == 0 )
	    return cp->Class;
    return NULL;
}

static void
GeomClassInstall( name, Class )
    char *name;
    GeomClass *Class;
{
    struct classtable *cp;

    cp = OOGLNewE(struct classtable, "GeomClass table");
    cp->next = table;
    table = cp;
    cp->classname = strdup(name);
    cp->Class = Class;
}

GeomClass *
GeomSubClassCreate( classname, subclassname )
    char *classname, *subclassname;
{
    GeomClass *Class, *subclass;

    Class = GeomClassLookup( classname );
    if( !Class ) {
	Class = OOGLNewE(GeomClass, "GeomClass");
	memset( (char *)Class, 0, sizeof(GeomClass) );
	GeomClassInstall( classname, Class );
    }
    subclass = OOGLNewE(GeomClass, "GeomClass");
    memcpy( (char *)subclass, (char *)Class, sizeof(GeomClass) );
    subclass->super = Class;
    GeomClassInstall( subclassname, subclass );

    return subclass;
}

GeomClass *
GeomClassCreate( name )
    char *name;
{
    return GeomSubClassCreate( "geom", name );
}

GeomClass *
GeomMethods( object )
Geom *object;
{
    return object ? object->Class : NULL;
}

/*
 * Call a function for all known classes.
 */
void *
GeomClassIterate( )
{
    return (void *)table;
}

GeomClass *
GeomNextClass( it )
    void *it;
{
    GeomClass *c;
#define  iter  ((struct classtable **)it)

    if(*iter == NULL)
	return NULL;
    c = (*iter)->Class;
    *iter = (*iter)->next;
    return c;

#undef iter
}
