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

#ifndef _HANDLEPOOLP_
#define _HANDLEPOOLP_
/*
 * Communications -- Private definitions for Pools and Handles.
 */
#include <sys/time.h>
#include "ooglutil.h"
#include "streampool.h"
/*#include "sm.h"*/

#define	HANDLEMAGIC	OOGLMagic('h',1)

typedef struct HRef {
	DblListNode node;
	Handle	**hp;
	Ref	*parentobj;
	void	*info;
	void	(*update) P((Handle **, Ref *, void *));
} HRef;

struct Handle {
	REFERENCEFIELDS;
	HandleOps *ops;		/* Comm-related operations on our datatype */
	char	*name;		/* Char-string name */
	Ref	*object;	/* Current object value if any */
	DblListNode opsnode;    /* node in list of all handles with given ops */
	DblListNode poolnode;   /* node in list of all handles with given pool*/
	DblListNode objnode;    /* node in list of all handles pointing to a
				   given object */
	Pool	*whence;	/* Where did this handle's value come from? */

	DblListNode refs;       /* list of references to this Handle,
				 * which we update when the Handle's
				 * object changes.
				 */
	bool	permanent;	/* Retain even when last reference goes away? */
	bool    obj_saved;      /* Set during saving of objects when
				 * the handle has been dumped as
				 * reference to indicate that its
				 * objects also already has been
				 * saved.
				 */
	/*
	 * Pool-type-specific state
	 */
#if 0
    	SMSym	*sym;		/* Address of our SM symbol */
    	int	version;	/* Version number of shared-memory symbol */
    	Pool	*smpool;	/* Pool in which our symbol lies */
    	Handle	*samepool;	/* Link in list of handles on this pool */
#endif
};

#define	P_SM	1
#define	P_STREAM 2

struct Pool {
	DblListNode node;       /* Link in list of all Pools (or free pools) */
	int	type;		/* P_SM or P_STREAM */
	char	*poolname;	/* Name of this pool: typically a filename */
	DblListNode handles;	/* All handles using this Pool */
	HandleOps *ops;		/* I/O operations */

	long	await;		/* Unix time until which we should wait */
	int	(*resyncing)();	/* We're resyncing, call this ... if non-NULL */

	/*
	 * State for P_STREAM pools.
	 */

	char	otype;		/* PO_HANDLES, PO_DATA, PO_ALL */
	char	mode;		/* read/write status: 0, 1, 2 as with open() */
	char	seekable;	/* 1 for plain file, 0 for pipe/socket */
	char	softEOF;	/* Can we hope to read more after EOF?
				 * 1 for tty or named pipe, 0 otherwise.
				 */
	IOBFILE	*inf;
	int     infd;
	FILE	*outf;

	short	flags;		/* Miscellaneous internal flags: */
#define	  PF_TEMP	1	/*   "Temporary pool" -- not in AllPools list */
#define	  PF_ANY	2	/*   any objects read from this Pool? */
#define	  PF_REREAD	4	/*   actually re-read on "<" */
#define	  PF_CLOSING	0x10	/* Internal flag to avoid PoolClose() recursion */
#define	  PF_ASLEEP	0x20	/* PoolSleep() called on this Pool. */
#define	  PF_DELETED	0x40	/* Pool is on free list - don't touch! */
#define	  PF_NOPREFETCH	0x80	/* Don't let PoolIn() prefetch the first char */

	short	level;		/* {} Bracket counter */

	long	inf_mtime;	/* modification time of p->inf file */
				/* A second explicit reference to the same
				 * file can cause it to be re-read if it's
				 * been changed since last time, or if it's
				 * a stream (not seekable).
				 */

	struct timeval awaken;	/* Resume reading at this time */
	struct timeval timebase; /* Basis for our clock */

	/*
	 * State for P_SM pools.
	 */
#if 0
    	SMRegion *sm;		/* for shared-memory Pools */
#endif

	/*
	 * client data pointer, used by clients for whatever they want
	 */
	void *client_data;

};

#endif /*_HANDLEPOOLP_*/
