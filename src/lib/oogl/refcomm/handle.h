/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _HANDLE_
#define _HANDLE_
/*
 * Communications -- Public definitions for Handle package.
 */

#include "ooglutil.h"
#include "reference.h"


typedef struct Handle	Handle;
typedef struct Pool	Pool;




int	 HandleSetObject( Handle *, Ref *object );
Ref	*HandleObject( Handle * );
void	 HandlePDelete(Handle **hp);
char	*HandleName( Handle * );
void	 HandleDelete( Handle * );
Pool	*HandlePool( Handle * );

#define	IO_HANDLES	1
#define	IO_COMPLETE	2		

int	 HandleExport( Handle *, Pool * ); /* Export */
int	 HandleUnExport( Handle * );	/* Cancel exported object if possible */
int	 HandleImport( Handle * );	/* Import now (possibly blocking) */

Handle *PoolIn(Pool *p);
int	 PoolOutMode( int poflags );	/* Returns old IO mode */


	/*
	 * Data-specific operations for (stream-type) Pools (files, pipes).
	 */
typedef struct HandleOps {
    char *prefix;			/* Prefix for unique external naming */
    int (*strmin) P((Pool *, Handle **, Ref **)); /* Read from stream */
    int (*strmout) P((Pool *, Handle *, Ref *));	/* Write */
    void (*Delete) P((Ref *));		/* Delete object */
    int	(*resync) P((Pool *));		/* Resync pool on import err */
    int	(*close) P((Pool *));		/* Close pool */
} HandleOps;

	/*
	 * Given a string using the reference syntax,
	 * e.g. ``name:file'' or ``name:'',
	 * do whatever's necessary to create and return a handle to it.
	 * Might entail a lookup of an existing handle,
	 * or opening and possibly reading a file (as a stream-type Pool).
	 */
extern Handle *HandleReferringTo(int prefixch, char *str, HandleOps *ops, Handle **hp);


	/*
 	 * Given a Handle's name and type (as given by HandleOps),
	 * return it or NULL if it doesn't exist.
	 */
extern Handle *HandleByName( char *, HandleOps * );

	/*
	 * If a Handle with this name already exists, return it;
	 * otherwise, create a new one with NULL value.
	 */
extern Handle *HandleCreate( char *name, HandleOps *ops );

	/*
	 * Set a Handle's object to obj, creating the Handle if necessary.
	 * Update all registered references to the Handle's value.
	 * Return the (possibly new) Handle.
	 */
extern Handle *HandleAssign( char *name, HandleOps *ops, Ref *obj );

	/*
	 * Register a reference to a Handle's value.
	 * Whenever the value changes, a call is made to:
	 *	(*update)(Handle **hp, Ref *parentobject, void *info)
	 * It's intended that "info" will be a key used by the update
	 * procedure to know which reference to update.
	 * The update procedure is automatically called once when
	 * registration is done.
	 */
extern int HandleRegister( Handle **hp, Ref *parent, void *info, void (*update)());

	/*
	 * Unregister (all callbacks on) a reference to a Handle's value.
	 * It's important to do this when deleting an object,
	 * lest dangling pointers cause trouble.
	 * It's harmless to remove a reference which was never registered.
	 */
extern void HandleUnregister(Handle **hp);

	/* Unregister just one callback on a reference to a Handle's value.
	 * HandleUnregister() is the one to use when deleting an object.
	 * The viewer needs this to keep track of callbacks it plants
	 * throughout Geom trees, which trigger e.g. BBox recomputation.
	 */
extern void HandleUnregisterJust(Handle **hp, Ref *par, void *info, void (*upd)());

	/*
	 * Unregister all callbacks matching a given pattern, no matter
	 * which Handle they're associated with.  Another routine needed
	 * for the viewer, called whenever a Geom tree is replaced.
	 */
extern void HandleUnregisterAll(Ref *par, void *info, void (*upd)());

	/*
	 * Suitable update() func for HandleRegister in many cases.
	 * Just deletes *objp then assigns *objp = (*hp)->object when called.
	 */
extern void HandleUpdRef(Handle **hp, Ref *parent, Ref **objp);

#endif /*_HANDLE_*/
