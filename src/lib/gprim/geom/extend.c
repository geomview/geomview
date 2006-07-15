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
#include <stdarg.h>

static struct extmethods {
    char *methodname;
    GeomExtFunc *defaultfunc;
} *extmethods = NULL;

static int n_extmethods = 1, max_extmethods = 0;

int
GeomMethodSel(char *methodname)
{
    struct extmethods *m;
    int i;

    for(i = 1; i < n_extmethods; i++)
      if((m = &extmethods[i])->methodname != NULL &&
		strcmp(m->methodname, methodname) == 0)
	return i;
    return 0;
}

int
GeomNewMethod(char *methodname, GeomExtFunc *defaultfunc)
{
    int sel;
    int oldmax = max_extmethods;

    sel = GeomMethodSel(methodname);
    if(sel > 0)
	return 0;
    sel = n_extmethods++;
    if(sel >= oldmax) {
	extmethods = (oldmax == 0)
	    ? OOGLNewNE(struct extmethods, (max_extmethods = 7),
		"Extension methods")
	    : OOGLRenewNE(struct extmethods, extmethods, (max_extmethods *= 2),
		"Extension methods");
	memset(&extmethods[oldmax], 0,
		(max_extmethods - oldmax) * sizeof(struct extmethods));
    }
    extmethods[sel].defaultfunc = defaultfunc;
    extmethods[sel].methodname = strdup(methodname);
    return sel;
}

char *
GeomMethodName(int sel)
{
    return (sel <= 0 || sel >= n_extmethods)
	? NULL : extmethods[sel].methodname;
}

GeomExtFunc *
GeomSpecifyMethod( int sel, GeomClass *Class, GeomExtFunc *func )
{
    int oldmax;
    int need;
    GeomExtFunc *oldfunc;

    if(Class == NULL || sel <= 0 || sel >= n_extmethods)
	return NULL;

    oldmax = Class->n_extensions;
    if(sel >= oldmax) {
	need = (oldmax == 0) ? 7 : oldmax*2;
	if(need <= sel) need = sel+1;
	Class->extensions = (oldmax == 0)
		? OOGLNewNE(GeomExtFunc *,
			need, "Extension func vector")
		: OOGLRenewNE(GeomExtFunc *, Class->extensions,
			need, "Extension func vector"); 
	Class->n_extensions = need;
	memset(&Class->extensions[oldmax], 0,
		(need - oldmax) * sizeof(GeomExtFunc *));
    }
    oldfunc = Class->extensions[sel];
    Class->extensions[sel] = func;
    return oldfunc;
}

void *
GeomCall(int sel, Geom *geom, ...)
{
    GeomClass *C;
    GeomExtFunc *ext = NULL;
    void *result = NULL;
    va_list args;

    if(geom == NULL || sel <= 0 || sel >= n_extmethods)
	return NULL;

    C = geom->Class;
    while(sel >= C->n_extensions || (ext = C->extensions[sel]) == NULL) {
	if((C = C->super) == NULL) {
	    ext = extmethods[sel].defaultfunc;
	    break;
	}
    }
    if(ext) {
	va_start(args, geom);
	result = (*ext)(sel, geom, &args);
	va_end(args);
    }
    return result;
}

void *
GeomCallV(int sel, Geom *geom, va_list *args)
{
    GeomClass *C;
    GeomExtFunc *ext = NULL;

    if(geom == NULL || sel <= 0 || sel >= n_extmethods)
	return NULL;

    C = geom->Class;
    while(sel >= C->n_extensions || (ext = C->extensions[sel]) == NULL) {
	if((C = C->super) == NULL) {
	    ext = extmethods[sel].defaultfunc;
	    break;
	}
    }
    return ext ? (*ext)(sel, geom, args) : NULL;
}
