/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h> /* for decl of 'free'; mbp Tue May 16 19:21:59 2000 */
#include "handleP.h"	/* Includes stdio.h, etc. too */
#include "freelist.h"

#if defined(unix) || defined(__unix)
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
#elif defined(_WIN32)
# include <winsock.h>
#endif

#ifndef S_IFMT
# define S_IFMT  _S_IFMT
# define S_IFIFO _S_IFIFO
#endif

#ifdef _WIN32
#undef FD_ISSET
int FD_ISSET(SOCKET fd, fd_set *fds) {
  int i = fds->fd_count;
  while(--i >= 0) {
    if (fds->fd_array[i] == fd) return 1;
  }
  return 0;
}

void gettimeofday(struct timeval *tv, struct timezone *tz)
{
  fprintf(stderr, "Call to stub gettimeofday()\n");
  tv->tv_sec = tv->tv_usec = 0;
}
#endif /*_WIN32*/

#include <errno.h>

static const int o_nonblock =
#ifdef O_NONBLOCK
    O_NONBLOCK |
#endif
#ifdef O_NDELAY
    O_NDELAY |
#endif
#ifdef FNONBLK
    FNONBLK |
#endif
#ifdef FNDELAY
    FNDELAY |
#endif
    0;

#if !defined(O_NONBLOCK) && !defined(O_NDELAY) \
 && !defined(FNONBLK) && !defined(FNDELAY)
# error Do not know how to achieve non-blocking IO
#endif

static DBLLIST(AllPools);
static DEF_FREELIST(Pool);

static fd_set poolwatchfds;
static int poolmaxfd = 0;
static fd_set poolreadyfds;
static int poolnready = 0;

#define	FOREVER  ((((unsigned)1)<<31)-1)
static struct timeval nexttowake = { FOREVER, };

static Pool *newPool(char *name);

/*
 * Does the whole job of handling stream references to named objects.
 * Interprets strings of the forms:
 *	name:file
 *		referring to a specific named object to be read from a
 *		specific file, or
 *	file
 *		referring to the nameless content of that file, or
 *	name:
 *		referring to a specific named object from any source.
 *
 * In the first two cases, we open (as a Pool) the specified file and
 * attempt to read it using the functions in *ops.
 *
 * The caller of this function owns the returned handle and should
 * call HandleDelete() when it doesn't need the return value.
 */
Handle *
HandleReferringTo(int prefixch, char *str, HandleOps *ops, Handle **hp)
{
    Pool *p = NULL;
    Handle *h = NULL, *ph = NULL;
    Handle *hknown = NULL;
    char *sep;
    char *fname;
    char *name;
    char nb[128];

    if (str == NULL || ops == NULL)
	return 0;

    sep = strrchr(str, ':');
    if (prefixch == ':') {	/*   :  name   -- take 'name' from anywhere */
	name = str;
	fname = NULL;
    } else if (sep == NULL) {	/*   <  file   -- read from file 'name' */
	fname = str;
	name = NULL;
    } else {			/*   <  file:name */
	name = sep+1;
	fname = nb;
	if ((size_t)(sep-str) >= sizeof(nb)) {
	    sep = &str[sizeof(nb)-1];
	}
	memcpy(fname, str, sep-str);
	fname[sep-str] = '\0';
	/* The ':' introduces an ambiguity: at least on MS win it can
	 * also mean a drive letter, and on other systems it could
	 * also be a legal part of the name. We only hack around the
	 * drive-letter stuff: if we have a one-letter file-name, then
	 * we assume it is a drive letter.
	 */
	if (fname[1] == '\0' && findfile(NULL, fname) == NULL) {
	  fname = str;
	}
    }

    if (fname != NULL && *fname != '\0') {
	p = PoolStreamOpen(fname, NULL, 0, ops);
	hknown = HandleCreate(fname, ops);
    }

    if (p && ((p->flags & (PF_ANY|PF_REREAD)) != PF_ANY || hknown != NULL)) {
	ph  = PoolIn(p);	/* Try reading one item. */
    }

    if (name) {
	/* Accessing a handle via ':' makes the handle global */
	h = HandleCreateGlobal(name, ops);
    }

    if (ph) {
	if (h) {
	    /* If we were told to assign to a specific named handle
	     * then do so and get rid of the handle returned by
	     * PoolIn().
	     */
	    HandleSetObject(h, HandleObject(ph));
	    HandleDelete(ph);
	} else {
	    h = ph; /* otherwise we return the handle returned by PoolIn(). */
	}
	HandleDelete(hknown);
    } else if (p) {
	/* If we have a pool but not handle, then generate one for
	 * this pool; callers of this functinos treat handle == NULL
	 * as error case.
	 */
	REFGET(Handle, hknown);
	HandleSetObject(hknown, NULL);
	h = hknown;
	if (h->whence) {
	    if (h->whence != p) {
		/* steal the pool pointer */
		DblListDelete(&h->poolnode);
		h->whence = p;
		DblListAdd(&p->handles, &h->poolnode);
	    }
	    REFPUT(h); /* no need to call HandleDelete() */
	} else {
	    h->whence = p;
	    DblListAdd(&p->handles, &h->poolnode);
	}
    }

    if (hp) {
	if (*hp) {
	    if (*hp != h) {
		HandlePDelete(hp);
	    } else {
		HandleDelete(*hp);
	    }
	}
	*hp = h;
    }

    return h;
}

char *
PoolName(Pool *p)
{
    return p ? p->poolname : NULL;
}

Pool *
PoolByName(char *fname, HandleOps *ops)
{
    Pool *p;

    DblListIterateNoDelete(&AllPools, Pool, node, p) {
	if ((ops == NULL || p->ops == ops) && strcmp(fname, p->poolname) == 0) {
	    return p;
	}
    }
    
    return NULL;
}

void pool_dump(void)
{
    Pool *p;
    Handle *h;

    OOGLWarn("Active Pools:");
    DblListIterateNoDelete(&AllPools, Pool, node, p) {
	OOGLWarn("  %s[%s]%p",
		 p->ops ? p->ops->prefix : "none",
		 p->poolname, (void *)p);
	OOGLWarn("    Attached Handles:");
	DblListIterateNoDelete(&p->handles, Handle, poolnode, h) {
	    OOGLWarn("    %s", h->name);
	}
    }
}

static
void watchfd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE || FD_ISSET(fd, &poolwatchfds))
	return;

    FD_SET(fd, &poolwatchfds);
    if (poolmaxfd <= fd)
	poolmaxfd = fd+1;
}

static
void unwatchfd(int fd)
{
    int i;

    if (fd < 0 || fd >= FD_SETSIZE)
	return;

    if (FD_ISSET(fd, &poolwatchfds)) {
	FD_CLR(fd, &poolwatchfds);
    }
    if (fd+1 >= poolmaxfd) {
	for(i = poolmaxfd; --i >= 0 && !FD_ISSET(i, &poolwatchfds); )
	    ;
	poolmaxfd = i+1;
    }
    if (FD_ISSET(fd, &poolreadyfds)) {
	FD_CLR(fd, &poolreadyfds);
	poolnready--;
    }
}

static Pool *
newPool(char *name)
{
    Pool *p;

    FREELIST_NEW(Pool, p);
    memset(p, 0, sizeof(Pool));
    DblListInit(&p->node);
    DblListInit(&p->handles);
    p->poolname = strdup(name);
    return p;
}
    
Pool *
PoolStreamTemp(char *name, IOBFILE *inf, FILE *outf, int rw, HandleOps *ops)
{
    Pool *p;
    char dummy[3+sizeof(unsigned long)*2+1];
    FILE *f = NULL;
    
    if (name==NULL) {
	sprintf(name=dummy, "_p@%lx",
		(unsigned long)(inf ? (void *)inf : (void *)outf));
    }

    if (inf == NULL && outf == NULL && name != NULL) {
	f = fopen(name, rw ? (rw>1 ? "w+b":"wb") : "rb");
	if (f == NULL) {
	    OOGLError(0, "Can't open %s: %s", name, sperror());
	    return NULL;
	}
    } 

    if (f == NULL && inf == NULL && outf == NULL) {
	OOGLError(0, "PoolStreamTemp(%s): file == NULL\n", name);
	return NULL;
    }

    if (f) {
	switch(rw) {
	case 0:
	    inf  = iobfileopen(f);
	    outf = NULL;
	    break;
	case 1:
	    outf = f;
	    inf  = NULL;
	    break;
	case 2:
	    inf  = iobfileopen(f);
	    outf = fdopen(dup(fileno(f)), "wb");
	    break;
	}
    } else if (rw != 1 && inf == NULL) {
	inf  = iobfileopen(fdopen(dup(fileno(outf)), "rb")); 
    } else if (rw != 0 && outf == NULL) {
	outf = fdopen(dup(iobfileno(inf)), "wb");
    }

    if ((rw != 1 && inf == NULL) || (rw != 0 && outf == NULL)) {
	OOGLError(0, "PoolStreamTemp(%s): file == NULL\n", name);
	return NULL;
    }
    
    p = newPool(name);
    p->ops = ops;
    p->type = P_STREAM;

    p->outf = outf;
    p->inf  = inf;
    p->infd = p->inf ? iobfileno(p->inf) : -1;

    /*p->handles = NULL;*/
    p->resyncing = NULL;
    p->otype = PO_ALL;
    p->mode = inf && outf ? 2 : (outf ? 1 : 0);
    p->seekable = (p->inf && lseek(iobfileno(p->inf),0,SEEK_CUR) != -1 &&
		   !isatty(iobfileno(p->inf)));
    p->softEOF = !p->seekable;
    p->level = (p->outf && lseek(fileno(p->outf),0,SEEK_CUR) != -1 &&
			!isatty(fileno(p->outf)))
		? 0 : 1;
    p->flags = PF_TEMP;
    p->client_data = NULL;

#if HAVE_FCNTL
    if (p->inf && p->infd >= 0) {
	fcntl(p->infd, F_SETFL, fcntl(p->infd, F_GETFL) & ~o_nonblock);
    }
    if (p->outf && fileno(p->outf) >= 0) {
	int fd = fileno(p->outf);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~o_nonblock);
    }
#endif /*unix*/

    return p;
}

Pool *
PoolStreamOpen(char *name, FILE *f, int rw, HandleOps *ops)
{
    Pool *p;
    struct stat st;

    p = PoolByName(name, ops);

    if (p == NULL) {
	p = newPool(name);
	p->ops = ops;
	p->type = P_STREAM;
	p->inf  = NULL;
	p->infd = -1;
	p->outf = NULL;
	p->mode = rw;
	/*p->handles = NULL;*/
	p->resyncing = NULL;
	p->otype = PO_ALL;
	p->level = 0;
	p->flags = 0;
	p->client_data = NULL;
    } else {
	if (rw == 0 && p->mode == 0 && p->inf != NULL
	    && p->softEOF == 0 && (p->flags & PF_REREAD) == 0
	    && stat(name, &st) == 0 && st.st_mtime == p->inf_mtime) {
	    iobfrewind(p->inf);
	    return p;
	}

	/*
	 * Combine modes.  Allows e.g. adding write stream to read-only pool.
	 */
	p->mode = ((p->mode+1) | (rw+1)) - 1;
	if (p->inf && rw != 1) {
	    if (iobfile(p->inf) == stdin) {
		iobfileclose(p->inf); /* leaves stdin open */
	    } else {
		iobfclose(p->inf);
	    }
	    p->inf  = NULL;
	}
    }

    if (f == NULL || f == (FILE *)-1) {
	if (rw != 1) {
	    if (strcmp(name, "-") == 0) {
		f = stdin;
	    } else {
		/* Try opening read/write first in case it's a Linux named pipe */
		int fd;
		/* Linux 2.0 is said to prefer that someone always has
		 * a named pipe open for writing as well as reading.
		 * But if we do that, we seem to lose the ENXIO error given
		 * to a process which opens that pipe for non-blocking write.
		 * Let's not do this now, even on Linux.
		 * Note that we do use PoolSleepFor() to slow down polling
		 * of pipes which claim to have data ready, but actually don't;
		 * I think that's what O_RDWR mode is intended to fix.
		 */
		/* BTW, this is not Linux, but common Unix
		 * behaviour. Reading from a pipe with no writers will
		 * just return.
		 *
		 * cH: In principle we would want to avoid that
		 * O_NONBLOCK; but then the call to open() may
		 * actually block. Using O_RDONLY, however, seems to
		 * render the select stuff unusable.
		 *
		 * cH: update: at least on Linux, O_RDWR means that we
		 * always have a writer for this pipe (ourselves). It
		 * is probably more poratble to use a second file
		 * descriptor in write mode. However, leave it as is
		 * for now.
		 */
		fd = open(name, O_RDWR | o_nonblock);
		if (fd < 0)
		    fd = open(name, O_RDONLY | o_nonblock);

#if HAVE_UNIX_SOCKETS
		/* Unix-domain socket? */
		if (fd < 0 && errno == EOPNOTSUPP) {
		    struct sockaddr_un sa;
		    sa.sun_family = AF_UNIX;
		    strncpy(sa.sun_path, name, sizeof(sa.sun_path));
		    fd = socket(PF_UNIX, SOCK_STREAM, 0);
		    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			close(fd);
			fd = -1;
		    }
		}
#endif /* HAVE_UNIX_SOCKETS */

		if (fd < 0)
 		    OOGLError(0, "Cannot open file \"%s\": %s",
			      name, sperror());
		else
		    f  = fdopen(fd, "rb");	    
	    }
	    p->inf = iobfileopen(f);
	}
	if (rw > 0) {
	    if (strcmp(name, "-") == 0)
		p->outf = stdout;
	    else if ((p->outf = fopen(name, "wb")) == NULL)
		OOGLError(0, "Cannot create \"%s\": %s", name, sperror());
	}
    } else {
	if (rw != 1) {
	    p->inf = iobfileopen(f);
	}
	if (rw > 0) {
	    p->outf = (rw == 2) ? fdopen(dup(fileno(f)), "wb") : f;
	}
    }

    if (p->inf == NULL && p->outf == NULL) {
	PoolDelete(p);
	return NULL;
    }

	/* We're committed now. */
    if (DblListEmpty(&p->node)) {
	DblListAddTail(&AllPools, &p->node);
    }
    p->seekable = false;
    p->softEOF = false;
    if (p->inf != NULL) {
	p->infd = iobfileno(p->inf);
	if (p->infd != -1) {
	    if (isatty(p->infd)) {
		p->softEOF = true;
	    } else if (lseek(p->infd,0,SEEK_CUR) != -1) {
		p->seekable = true;
	    }
	    if (fstat(p->infd, &st) < 0 || (st.st_mode & S_IFMT) == S_IFIFO) {
		p->softEOF = true;
	    }
	    p->inf_mtime = st.st_mtime;
	    watchfd(p->infd);
#if HAVE_FCNTL
	    fcntl(p->infd, F_SETFL, fcntl(p->infd, F_GETFL) & ~o_nonblock);
#endif /* HAVE_FCNTL */
	}
    }
#if HAVE_FCNTL
    if (p->outf && fileno(p->outf) >= 0) {
	int fd = fileno(p->outf);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~o_nonblock);
    }
#endif /* HAVE_FCNTL */
    if (p->level == 0 && p->outf &&
	  (lseek(fileno(p->outf),0,SEEK_CUR) == -1 || isatty(fileno(p->outf))))
	p->level = 1;

    return p;
}

/* Return "true" if succesful */
int PoolSetMark(Pool *p)
{
    return iobfsetmark(PoolInputFile(p)) == 0;
}

/* Return "true" if succesful */
int PoolSeekMark(Pool *p)
{
    return iobfseekmark(PoolInputFile(p)) == 0;
}

/* Return "true" if succesful. BUT WHO CARES. Grin. */
int PoolClearMark(Pool *p)
{
    return iobfclearmark(PoolInputFile(p)) == 0;
}

int
PoolIncLevel(Pool *p, int incr)
{
    if (p) {
	p->level += incr;
	if (p->level < 0) {
	    OOGLError(0, "PoolIncLevel(): negative level.\n");
	}
	return p->level;
    } else {
	return incr;
    }
}

int
PoolOType(Pool *p, int otype)
{
    (void)otype;
    return p->otype;
}

void
PoolSetOType(Pool *p, int otype)
{
    p->otype = otype;
}

IOBFILE *
PoolInputFile(Pool *p)
{
    return (p && p->type == P_STREAM) ? p->inf : NULL;
}

FILE *
PoolOutputFile(Pool *p)
{
    return (p && p->type == P_STREAM) ? p->outf : NULL;
}

void *
PoolClientData(Pool *p)
{
    return (p ? p->client_data : NULL);
}

void
PoolSetClientData(Pool *p, void *data)
{
    p->client_data = data;
}

void
PoolDoReread(Pool *p)
{
    p->flags |= PF_REREAD;	/* Reread when asked */
}

void PoolClose(Pool *p)
{
    if (p->ops->close && (p->flags & PF_CLOSING) == 0) {
	p->flags |= PF_CLOSING;
	if ((*p->ops->close)(p)) {
	    return;
	}
    }

    if (p->type == P_STREAM) {
	if (p->inf != NULL) {
	    unwatchfd(iobfileno(p->inf));
	    if (iobfile(p->inf) == stdin) {
		iobfileclose(p->inf);
	    } else {
		iobfclose(p->inf);
	    }
	    p->inf = NULL;
	    p->infd = -1;
	}
	if (p->outf != NULL) {
	    if (p->outf != stdout) fclose(p->outf);
	    p->outf = NULL;
	}
	/* PoolDelete(p); */ /* Shouldn't we delete the Pool? */
    }
}

void PoolDelete(Pool *p)
{
    Handle *h, *hn;

    if (p == NULL || (p->flags & PF_DELETED) != 0) {
	return;
    }

    p->flags |= PF_DELETED;

    if ((p->flags & PF_TEMP) == 0) {
	DblListDelete(&p->node);
	DblListIterate(&p->handles, Handle, poolnode, h, hn) {
	    h->whence = NULL;
	    DblListDelete(&h->poolnode);
	    HandleDelete(h);
	}
    }

    free(p->poolname);
    FREELIST_FREE(Pool, p);
}

void PoolDetach(Pool *p)
{
    if ((p->flags & PF_TEMP) == 0) {
	DblListDelete(&p->node);
    }
}

void PoolReattach(Pool *p)
{
    if ((p->flags & PF_TEMP) == 0) {
	if (DblListEmpty(&p->node)) {
	    DblListAddTail(&AllPools, &p->node);
	}
    }
}

/*
 * Marks a Pool as being awake.  Doesn't update nexttowake;
 * so this should only be called where awaken_until() gets a chance to run.
 */
static void
awaken(Pool *p)
{
    p->flags &= ~PF_ASLEEP;
    timerclear(&p->awaken);    
    if (p->infd >= 0) {
	watchfd(p->infd);
	if (iobfhasdata(p->inf) && !FD_ISSET(p->infd, &poolreadyfds)) {
	   FD_SET(p->infd, &poolreadyfds);
	   poolnready++;
	}
    }
}

static void
awaken_until(struct timeval *until)
{
    Pool *p;

    nexttowake.tv_sec = FOREVER;
    DblListIterateNoDelete(&AllPools, Pool, node, p) {
	if (p->flags & PF_ASLEEP) {
	    if (timercmp(&p->awaken, until, <)) {
		awaken(p);
	    } else if (p->inf != NULL && timercmp(&p->awaken, &nexttowake, <)) {
		nexttowake = p->awaken;
	    }
	}
    }
}

/*
 * PoolInputFDs(fds, maxfd)
 * supplies a list of file descriptors for select() to watch for input.
 * Sets *fds to the set of pool input file descriptors
 * Sets *maxfd to the max+1 file descriptor in fds.
 * Returns max time to sleep in seconds; 0 if some Pools have input immediately
 * available (so select() shouldn't block).
 */
float
PoolInputFDs(fd_set *fds, int *maxfd)
{
    float timeleft = FOREVER;

#if defined(unix) || defined(__unix)
    if (nexttowake.tv_sec != FOREVER) {
	struct timeval now;
	gettimeofday(&now, NULL);
	if (timercmp(&nexttowake, &now, <))
	   awaken_until(&now);
	timeleft = (nexttowake.tv_sec - now.tv_sec)
			+ .000001 * (nexttowake.tv_usec - now.tv_usec);
    }
#endif

    *fds = poolwatchfds;
    *maxfd = poolmaxfd;
    return poolnready != 0 || timeleft < 0 ? 0 : timeleft;
}

/*
 * PoolInAll(fd_set *fds, int nfds)
 * Read from all available pools, given a set of file descriptors
 * which claim to have some input.  We also import from all those which
 * already have buffered input.
 * nfds is an advisory upper bound on the number of fd's in *fds;
 * if the caller doesn't keep the return value from select(), just use
 * FD_SETSIZE or other huge number.
 * Each fd used is removed from *fds, and *nfds is decremented.
 * The return value is 1 if anything was read from any pool, 0 otherwise.
 */

int
PoolInAll(fd_set *fds, int *nfds)
{
    Pool *p;
    int got = 0;

    /* We use DblListIterateNoDelete() _NOT_ because PoolIn() could
     * not delete the current Pool p, because PoolIn() can delete
     * _ANY_ pool so we need to work harder to make this
     * PoolInAll()-loop failsafe.
     */

    DblListIterateNoDelete(&AllPools, Pool, node, p) {
	if (p->type != P_STREAM || p->inf == NULL || p->infd < 0) {
	    continue;
	}

	if (FD_ISSET(p->infd, &poolreadyfds)) {
	    FD_CLR(p->infd, &poolreadyfds);
	    poolnready--;
	    if (PoolIn(p)) {
		got++;
	    }
	} else if (FD_ISSET(p->infd, fds)) {
	    FD_CLR(p->infd, fds);
	    (*nfds)--;
	    if (PoolIn(p)) {
		got++;
	    }
	}
	/* We need to be very careful here: PoolIn() can have _ANY_
	 * sort of side effect, in particular it might delete _ANY_
	 * pool. Luckily we do not really delete pools; if "p" is
	 * actually in the deleted state, then it has its PF_DELETED
	 * flag set. In this case we simply restart the loop.
	 *
	 * NOTE: Just using &AllPools triggers a strict aliasing
	 * warning with gcc. Of course, I don't know if the funny
	 * "next->prev" stuff simply confuses the compiler such that
	 * it does no longer emit the warning, but still emits wrong
	 * code. In principle the construct should be ok. "next->prev"
	 * should always point back to &AllPools. Of course, the
	 * compiler cannot know this. Maybe this is the difference.
	 */
	if (p->flags & PF_DELETED) {
	    p = DblListContainer(AllPools.next->prev, Pool, node);
	}
    }
    return got;
}

static void
asleep(Pool *p, struct timeval *base, double offset)
{
    struct timeval until;

    base = timeof(base);
    if (p->inf != NULL) {
	p->flags |= PF_ASLEEP;
	addtime(&until, base, offset);
	if (timercmp(&until, &nexttowake, <)) {
	    nexttowake = until;
	}
	p->awaken = until;
	if (p->infd >= 0) {
	    unwatchfd(p->infd);
	    if (FD_ISSET(p->infd, &poolreadyfds)) {
		FD_CLR(p->infd, &poolreadyfds);
		poolnready--;
	    }
	}
    }
}

void
PoolSleepUntil(Pool *p, double until)
{
    asleep(p, &p->timebase, until);
}

void
PoolSleepFor(Pool *p, double naptime)
{
    asleep(p, NULL, naptime);
}

void
PoolSetTime(Pool *p, struct timeval *base, double time_at_base)
{
    base = timeof(base);
    addtime(&p->timebase, base, -time_at_base);
}

double
PoolTimeAt(Pool *p, struct timeval *then)
{
    if (p->timebase.tv_sec == 0) timeof(&p->timebase);
    then = timeof(then);
    return then->tv_sec - p->timebase.tv_sec +
		.000001*(then->tv_usec - p->timebase.tv_usec);
}

void
PoolAwaken(Pool *p)
{
    awaken(p);
    if (timercmp(&p->awaken, &nexttowake, <=))
	awaken_until(&nexttowake);
}

bool
PoolASleep(Pool *p)
{
    if (p->flags & PF_ASLEEP) {
	struct timeval now;
	gettimeofday(&now, NULL);
	if (timercmp(&p->awaken, &now, >)) {
	    return true;
	}
	awaken(p);
    }
    return false;
}

#if 0
static void
poolresync(Pool *p, int (*resync)())
{ /* XXX implement this */
}
#endif

#define	CBRA	'{'
#define	CKET	'}'

/* The caller of this functions owns the returned handle and must call
 * HandleDelete() to get rid of the handle.
 */
Handle *PoolIn(Pool *p)
{
    int c = 0;
    Handle *h = NULL;
    Ref *r = NULL;

    if (p->type != P_STREAM) {
	return NULL;		/* Do shared memory here someday XXX */
    }
    if (p->inf == NULL || p->ops == NULL || p->ops->strmin == NULL) {
	return NULL;		/* No way to read */
    }

    if ((p->flags & PF_NOPREFETCH) ||
       ((c = async_iobfnextc(p->inf, 3)) != NODATA && c != EOF)) {
	if ((*p->ops->strmin)(p, &h, &r)) {
	    /* Attach nameless objects to a handle named for the Pool.
	     * Putting this code here in PoolIn() ensures we just bind
	     * names to top-level objects, not those nested inside a
	     * hierarchy.
	     */
	    if (h == NULL) {
		h = HandleCreate(p->poolname, p->ops);
		if (r != NULL) {
		    HandleSetObject(h, r);
		    /* Decrement reference count since we're handing
		     * ownership of the object to the Handle.
		     */
		    REFPUT(r);
		    if (h->whence == NULL) {
			/* Increment the reference count lest PoolDelete()
			 * will also consume the attached object.
			 */
			REFGET(Handle, h);
		    }
		}
	    } else {
		/* Increment the count such that the calling function
		 * can safely HandleDelete(retval) without destroying a
		 * handle explicitly declared in the pool (with define
		 * or hdefine).
		 */
		REFGET(Handle, h);
	    }
	    if (h->whence) {
		if (h->whence != p) {
		    /* steal the pool pointer */
		    DblListDelete(&h->poolnode);
		    h->whence = p;
		    DblListAdd(&p->handles, &h->poolnode);
		}
		REFPUT(h); /* no need to call HandleDelete() */
	    } else {
		h->whence = p;
		DblListAdd(&p->handles, &h->poolnode);
	    }
	    
	    /* Remember whether we've read (PF_ANY) at least one and
	     * (PF_REREAD) at least two objects from this pool.
	     * There'll be a nontrivial side effect of rereading a file
	     * containing multiple objects, so we actually do reread if asked.
	     */
	    p->flags |= (p->flags & PF_ANY) ? PF_REREAD : PF_ANY;

	    /* handle_dump(); */

	} else {
	    if (p->flags & PF_DELETED)
		return NULL;
	    if (p->ops->resync) {
		(*p->ops->resync)(p);
	    } else if (p->softEOF) {
		iobfrewind(p->inf);
	    } else if (p->inf != NULL) { /* Careful lest already PoolClose()d */
		if (p->infd >= 0) {
		    if (FD_ISSET(p->infd, &poolreadyfds)) {
			FD_CLR(p->infd, &poolreadyfds);
			poolnready--;
		    }
		}
		PoolClose(p);
		return NULL;
	    }
	}
	if (p->seekable && p->inf != NULL)
	    c = iobfnextc(p->inf, 0);	/* Notice EOF if appropriate */
    }
    if (c == EOF && iobfeof(p->inf)) {
	if (p->softEOF) {
	    iobfrewind(p->inf);
	    /* cH: opening O_RDWR should fix this, as this leaves one
	     * active writer attached to the FIFO: ourselves.
	     */
	    /*PoolSleepFor(p, 1.0);*/	/* Give us a rest */
		/* SVR4 poll() doesn't allow us to
		 * wait quietly when the sender closes a named pipe;
		 * any poll() for reading returns immediately with POLLHUP.
		 * Then, attempts to read return EOF.  We loop at full speed.
		 * Can't see any better way to throttle this than to
		 * quit listening for a while, at the cost of being slow to
		 * respond if another sender connects.  We wait 1 second.
		 */
	} else {
	    PoolClose(p);
	    REFGET(Handle, h); /* PoolDelete() deletes also the handle. */
	    PoolDelete(p);
	    return h;
	}
    }

    if (p->inf && !(p->flags & PF_ASLEEP) && p->infd >= 0) {
	/*
	 * Anything left in stdio buffer?  If so,
	 * remember to try reading next time without waiting for select().
	 */
	if (iobfhasdata(p->inf)) {
	    if (!FD_ISSET(p->infd, &poolreadyfds)) {
		FD_SET(p->infd, &poolreadyfds);
		poolnready++;
	    }
	} else {
	    if (FD_ISSET(p->infd, &poolreadyfds)) {
		FD_CLR(p->infd, &poolreadyfds);
		poolnready--;
	    }
	}
    }
    return h;
}

/*
 * Support routine for writing {handle, object} pairs to Pools.
 * Checks the Pool's output type (PO_HANDLES/PO_DATA/PO_ALL).  If
 * appropriate, writes something to the Pool representing the given
 * Handle.  Returns nonzero if the associated object should also be
 * written literally.
 *
 * For global handles: also emit a "define" statement.
 */
int
PoolStreamOutHandle(Pool *p, Handle *h, int havedata)
{
    if (p == NULL || p->outf == NULL) {
	return 0;
    }

    if (h == NULL || (p->otype & PO_DATA)) {
	return havedata;
    }

    if (havedata && !h->obj_saved) {
	h->obj_saved = true;
	PoolFPrint(p, p->outf, "define \"%s\"\n", h->name);
	return true;
    }

    if (h->whence != NULL && h->whence->seekable) {
	PoolFPrint(p, p->outf, " < \"");
	if (strcmp(h->name, p->poolname) == 0) {
	    fprintf(p->outf, "%s\"\n", h->whence->poolname);
	} else {
	    fprintf(p->outf, "%s:%s\"\n", h->whence->poolname, h->name);
	}
    } else {
	PoolFPrint(p, p->outf, ": \"%s\"\n", h->name);
    }
    
    return havedata && !h->obj_saved &&
	(p->otype & (PO_DATA|PO_HANDLES)) == PO_ALL;
}

void PoolFPrint(Pool *p, FILE *f, const char *format, ...)
{
  va_list alist;

  if (p) {
      fprintf(f, "%*s", p->level*2, "");
  }
  va_start(alist, format);
  vfprintf(f, format, alist);
  va_end(alist);
}

void PoolPrint(Pool *p, const char *format, ...)
{
  va_list alist;
  
  if (p) {
      fprintf(PoolOutputFile(p), "%*s", p->level*2, "");
  }
  va_start(alist, format);
  vfprintf(PoolOutputFile(p), format, alist);
  va_end(alist);
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
