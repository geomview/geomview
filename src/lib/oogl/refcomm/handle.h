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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _HANDLE_
#define _HANDLE_
/*
 * Communications -- Public definitions for Handle package.
 */

#include "ooglutil.h"
#include "reference.h"
#include "dbllist.h"


typedef struct Handle	Handle;
typedef struct Pool	Pool;




int	 HandleSetObject( Handle *, Ref *object );
Ref	*HandleObject( Handle * );
void	 HandlePDelete(Handle **hp);
char	*HandleName( Handle * );
void	 HandleDelete( Handle * );
Pool	*HandlePool( Handle * );
Handle  *HandleRefIterate(Ref *r, Handle *pos);
Handle  *HandlePoolIterate(Pool *p, Handle *pos);

	/* set obj_saved flag of all handles */
void     HandlesSetObjSaved(bool obj_saved);

#if 0
/* stuff which is nowhere defined nor referenced */
#define	IO_HANDLES	1
#define	IO_COMPLETE	2		

int	 HandleExport( Handle *, Pool * ); /* Export */
int	 HandleUnExport( Handle * );	/* Cancel exported object if possible */
int	 HandleImport( Handle * );	/* Import now (possibly blocking) */

int	 PoolOutMode( int poflags );	/* Returns old IO mode */

#endif


	/*
	 * Data-specific operations for (stream-type) Pools (files, pipes).
	 */
typedef struct HandleOps {
    char *prefix;			/* Prefix for unique external naming */
    int (*strmin) P((Pool *, Handle **, Ref **)); /* Read from stream */
    int (*strmout) P((Pool *, Handle *, Ref *));	/* Write */
    void (*delete) P((Ref *));		/* Delete object */
    int	(*resync) P((Pool *));		/* Resync pool on import err */
    int	(*close) P((Pool *));		/* Close pool */
    DblListNode handles;                /* list of all handles with these ops */
    DblListNode node;                   /* node into list of all ops */
} HandleOps;

	/*
	 * Given a string using the reference syntax,
	 * e.g. ``name:file'' or ``name:'',
	 * do whatever's necessary to create and return a handle to it.
	 * Might entail a lookup of an existing handle,
	 * or opening and possibly reading a file (as a stream-type Pool).
	 *
	 * The caller of this functions owns the returned handle and
	 * must call HandleDelete() to get rid of the returned handle.
	 */
extern Handle *HandleReferringTo(int prefixch, char *str, HandleOps *ops, Handle **hp);


	/*
	 * Search for ops matching prefix.
	 */
extern HandleOps *HandleOpsByName(char *prefix);

	/*
 	 * Given a Handle's name and type (as given by HandleOps),
	 * return it or NULL if it doesn't exist.
	 *
	 * The caller owns the returned handle, i.e. the reference
	 * count of the handle is incremented by this function.
	 */
extern Handle *HandleByName( char *, HandleOps * );

	/*
	 * If a Handle with this name already exists, return it;
	 * otherwise, create a new one with NULL value.
	 *
	 * If the handle already exists then its reference count is
	 * incremented by this function.
	 */
extern Handle *HandleCreate( char *name, HandleOps *ops );

	/*
	 * Unconditionally create a new Handle with NULL value.
	 */
extern Handle *HandleDoCreate( char *name, HandleOps *ops );

	/*
	 * Same as HandleCreate() for an existing global handle, but
	 * if the handle does not already exist increment the
	 * ref-count twice s.t. the handle cannot be deleted.
	 */
extern Handle *HandleCreateGlobal( char *name, HandleOps *ops);

	/*
	 * Set a Handle's object to obj, creating the Handle if necessary.
	 * Update all registered references to the Handle's value.
	 * Return the (possibly new) Handle.
	 *
	 * If the handle already exists then its reference count is
	 * incremented by this function.
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
	 * Un(all callbacks on) a reference to a Handle's value.
	 * It's important to do this when deleting an object,
	 * lest dangling pointers cause trouble.
	 * It's harmless to remove a reference which was never registered.
	 */
extern void HandleUnregister(Handle **hp);

	/* Unjust one callback on a reference to a Handle's value.
	 * HandleUnregister() is the one to use when deleting an object.
	 * The viewer needs this to keep track of callbacks it plants
	 * throughout Geom trees, which trigger e.g. BBox recomputation.
	 */
extern void HandleUnregisterJust(Handle **hp, Ref *par, void *info, void (*upd)());

	/*
	 * Unall callbacks matching a given pattern, no matter
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

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
