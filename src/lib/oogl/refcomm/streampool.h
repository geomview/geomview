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

#ifndef _STREAMPOOL_
#define	_STREAMPOOL_

#if defined(HAVE_CONFIG_H) && ! defined(CONFIG_H_INCLUDED)
# include "config.h"
#endif

#ifdef AIX
# include <sys/select.h>  /* RS-6000 needs this */
#endif

#include <sys/types.h>

#if defined(unix) || defined(__unix)
# include <sys/time.h>
#elif defined(_WIN32)
# include <winsock.h>
#endif

#include <stdarg.h>

#include "handle.h"


	/*
	 * Common stream syntax
	 */

#define	CBRA	'{'
#define	CKET	'}'


	/*
	 * Open a stream-type Pool.
	 * Uses stream f if != NULL, otherwise opens the named file.
	 * The name is required anyway, to uniquely identify the pool.
	 */
extern Pool *PoolStreamOpen(char *name, FILE *f, int rw, HandleOps *ops);

	/*
	 * Open a temporary Pool.  Like PoolStreamOpen, but:
	 *   - the opened Pool is not listed as a input source for PoolInputFDs
	 *   - it is not added to the table of known Pools, so
	 *	it won't conflict with other Pools known by the same name.
	 * Uses stream f if != NULL, otherwise opens the named file.
	 * The name is not required -- it may be NULL.
	 */
extern Pool *PoolStreamTemp(char *name, IOBFILE *inf, FILE *outf, int rw,
			    HandleOps *ops);

	/*
	 * Return the set of file descriptors pool file descriptors we'd like
	 * polled for input.  Fills *maxfd with the highest+1 file descriptor.
	 * Returns 0 if some pool holds buffered input (i.e. don't let select()
	 * block), otherwise the time in seconds until the next sleeping Pool
	 * awakens.  If no Pools are asleep, returns a large positive number.
	 */
extern float PoolInputFDs( fd_set *fds, int *maxfd );

	/* Try to read something from the pool, in case of success a
	 * handle is returned, possibly with an object attached to it.
	 * The caller of this function owns the handle.
	 */
extern Handle *PoolIn(Pool *p);

	/*
	 * Given the fd mask returned by select(), call import() on
	 * all pools which have input.  Removes used fd's from *fds;
	 * decrements count for each fd used.
	 * Returns 1 if anything read from any pool, 0 otherwise.
	 */
extern int PoolInAll( fd_set *fds, int *count );

	/*
	 * Utility routine for XXXOut functions.
	 * Writes relevant text of h to the Pool based on p's output setup;
	 * returns 1 if associated data should be written too, 0 if not.
	 */
extern int PoolStreamOutHandle(Pool *p, Handle *h, int havedata);

	/*
	 * Set a pool's client data pointer.  Pool users may use
	 * this for whatever they want.
	 */
void PoolSetClientData(Pool *p, void *data);

	/*
	 * Return a pool's client data pointer.
	 */
void *PoolClientData(Pool *p);

extern void PoolDelete(Pool *p);

				/* How much to export when we do so? */
				/* Pass these values to PoolSetOType(). */
#define	  PO_ALL	0	/*   - use all handles and all data */
#define	  PO_HANDLES	1	/*   - use handles wherever possible, avoid data */
#define	  PO_DATA	2	/*   - data, no external refs */

extern IOBFILE *PoolInputFile(Pool *);
extern FILE *PoolOutputFile(Pool *);
extern void  PoolDoReread(Pool *);
extern void PoolClose(Pool *p);
extern int   PoolOType(Pool *, int otype);
extern void  PoolSetOType(Pool *, int otype);
extern Pool *PoolByName(char *name, HandleOps *ops);
extern char *PoolName(Pool *);

extern void PoolSleepFor(Pool *, double seconds);
extern void PoolSleepUntil(Pool *, double seconds);
extern void PoolSetTime(Pool *, struct timeval *basis, double time_at_basis);
extern double PoolTimeAt(Pool *, struct timeval *at_this_time);
extern void PoolAwaken(Pool *);
extern bool PoolASleep(Pool *p);

extern int PoolIncLevel(Pool *p, int incr);

extern int PoolSetMark(Pool *p);
extern int PoolSeekMark(Pool *p);
extern int PoolClearMark(Pool *p);

extern void PoolFPrint(Pool *p, FILE *f, const char *format, ...);
extern void PoolPrint(Pool *p, const char *format, ...);

extern void PoolDetach(Pool *p);
extern void PoolReattach(Pool *p);

#endif /*_STREAMPOOL_*/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
