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

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h> /* for decl of 'free'; mbp Tue May 16 19:21:59 2000 */
#include "handleP.h"	/* Includes stdio.h, etc. too */

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
    if(fds->fd_array[i] == fd) return 1;
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

#ifndef O_NONBLOCK
# ifdef FNONBLK
#  define O_NONBLOCK FNONBLK
# else
#  ifdef O_NDELAY
#   define O_NONBLOCK O_NDELAY
#  else
#   define O_NONBLOCK 0
#  endif
# endif
#endif

#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

#ifndef FNONBLK
#define FNONBLK O_NONBLOCK
#endif

#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif


static Pool *AllPools = NULL;
static Pool *FreePools = NULL;

static fd_set poolwatchfds;
static int poolmaxfd = 0;
static fd_set poolreadyfds;
static int poolnready = 0;

#define	FOREVER  ((((unsigned)1)<<31)-1)
static struct timeval nexttowake = { FOREVER };

static Pool *newPool(char *name);

extern Handle *AllHandles;	/* From handle.c */

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
 */
Handle *
HandleReferringTo(int prefixch, char *str, HandleOps *ops, Handle **hp)
{
    Pool *p = NULL;
    Handle *h = NULL;
    Handle *hknown;
    char *sep;
    char *fname;
    char *name;
    char nb[128];

    if(str == NULL || ops == NULL)
	return 0;

    sep = strrchr(str, ':');
    if(prefixch == ':') {	/*   :  name   -- take 'name' from anywhere */
	name = str;
	fname = NULL;
    } else if(sep == NULL) {	/*   <  file   -- read from file 'name' */
	fname = str;
	name = str;
    } else {			/*   <  file:name */
	name = sep+1;
	fname = nb;
	if(sep-str >= sizeof(nb))
	    sep = &str[sizeof(nb)-1];
	memcpy(fname, str, sep-str);
	fname[sep-str] = '\0';
    }
    h = hknown = HandleByName(name, ops);

    if(fname != NULL)
	p = PoolStreamOpen(fname, NULL, 0, ops);

    if((p || prefixch == ':') && h == NULL) {
	h = HandleCreate(name, ops);
	if(h == NULL) {
	    OOGLError(1, "Can't make handle for '%s'", str);
	    return NULL;
	}
    }
    if(p && (((p->flags & (PF_ANY|PF_REREAD)) != PF_ANY) || hknown != NULL)) {
	Handle *th = PoolIn(p);	/* Try reading one item. */
	if(th) {		/* Read anything? */
	   h = th;		/* Return that if so */
	   h->whence = p;
	}
    }
    if(hknown == NULL && prefixch == '<' && h != NULL && p && p->seekable)
	h->permanent = 0;
    if(hp && h && h->permanent) {
	if(*hp && *hp != h)
	    HandlePDelete(hp);
	*hp = h;
    }
    return h;
}

char *
PoolName(Pool *p)
{ return p ? p->poolname : NULL;
}

Pool *
PoolByName(char *fname)
{
    register Pool *p;

    for(p = AllPools; p != NULL; p = p->next)
	if(strcmp(fname, p->poolname) == 0)
	    return p;
    return NULL;
}


static
void watchfd(int fd)
{
    if(fd < 0 || fd >= FD_SETSIZE || FD_ISSET(fd, &poolwatchfds))
	return;

    FD_SET(fd, &poolwatchfds);
    if(poolmaxfd <= fd)
	poolmaxfd = fd+1;
}

static
void unwatchfd(int fd)
{
    register Pool *p;
    register int i;

    if(fd < 0 || fd >= FD_SETSIZE)
	return;

    if(FD_ISSET(fd, &poolwatchfds)) {
	FD_CLR(fd, &poolwatchfds);
    }
    if(fd+1 >= poolmaxfd) {
	for(i = poolmaxfd; --i >= 0 && !FD_ISSET(i, &poolwatchfds); )
	    ;
	poolmaxfd = i+1;
    }
    if(FD_ISSET(fd, &poolreadyfds)) {
	FD_CLR(fd, &poolreadyfds);
	poolnready--;
    }
}

static Pool *
newPool(char *name)
{
    register Pool *p;

    if((p = FreePools) != NULL)
	FreePools = p->next;
    else
	p = OOGLNewE(Pool, "Pool");
    memset(p, 0, sizeof(Pool));
    p->poolname = strdup(name);
    return p;
}
    
Pool *
PoolStreamTemp(char *name, FILE *f, int rw, HandleOps *ops)
{
    register Pool *p;
    char dummy[12];

    if(name==NULL) sprintf(name=dummy, "_p%x", (unsigned int)f);
    p = newPool(name);
    p->ops = ops;
    p->type = P_STREAM;
    if(f == NULL && name != NULL) {
	f = fopen(name, rw ? (rw>1 ? "w+b":"wb") : "rb");
	if(f == NULL) {
	    OOGLError(0, "Can't open %s: %s", name, sperror());
	    OOGLFree(p);
	    return NULL;
	}
    }
    p->inf  = p->outf = f;
    p->sinf = NULL;
    p->infd = -1;
    if(f) {
	switch(rw) {
	    case 0: p->outf = NULL; break;
	    case 1: p->inf = NULL; break;
	    case 2: p->outf = fdopen(dup(fileno(f)), "wb");
	}
    }
    p->handles = NULL;
    p->resyncing = NULL;
    p->otype = PO_ALL;
    p->next = NULL;
    p->mode = rw;
    p->seekable = (p->inf && lseek(fileno(p->inf),0,SEEK_CUR) != -1 &&
			!isatty(fileno(p->inf)));
    p->softEOF = !p->seekable;
    p->level = (p->outf && lseek(fileno(p->outf),0,SEEK_CUR) != -1 &&
			!isatty(fileno(p->outf)))
		? 0 : 1;
    p->flags = PF_TEMP;
    p->client_data = NULL;

#if USE_SEEKPIPE
    if (p->inf) {
	p->infd = fileno(p->inf);
	if (!p->seekable && p->infd >= 0) {
	    /* leave seekable and fake files alone */
	    extern FILE *seekpipe_open(int fd);
	    setbuf(p->inf, NULL);
	    p->sinf = p->inf;
	    p->inf  = seekpipe_open(p->infd);
	}
    }
#else
    if (p->inf) {
	setvbuf(p->inf, NULL, _IOFBF, (1 << 16));
    }
#endif
    return p;
}

Pool *
PoolStreamOpen(char *name, FILE *f, int rw, HandleOps *ops)
{
    register Pool *p;
    struct stat st;

    p = PoolByName(name);

    if(p == NULL) {
	p = newPool(name);
	p->ops = ops;
	p->type = P_STREAM;
	p->inf = p->sinf = p->outf = NULL;
	p->infd = -1;
	p->mode = rw;
	p->handles = NULL;
	p->resyncing = NULL;
	p->otype = PO_ALL;
	p->level = 0;
	p->flags = 0;
	p->next = AllPools;
	p->client_data = NULL;
    } else {
	if(rw == 0 && p->mode == 0 && p->inf != NULL
		&& p->softEOF == 0
		&& (p->flags & PF_REREAD) == 0
		&& stat(name, &st) == 0
		&& st.st_mtime == p->inf_mtime) {
	    rewind(p->inf);
	    return p;
	}

	/*
	 * Combine modes.  Allows e.g. adding write stream to read-only pool.
	 */
	p->mode = ((p->mode+1) | (rw+1)) - 1;
	if(p->inf && rw != 1) {
	    if(p->sinf) fclose(p->sinf);
	    if(p->inf != stdin) fclose(p->inf);
	    p->inf  = NULL;
	    p->infd = -1;
	}
    }

    if(f == NULL || f == (FILE *)-1) {
	if(rw != 1) {
	    if(strcmp(name, "-") == 0) {
		p->inf  = stdin;
	    } else {
		/* Try opening read/write first in case it's a Linux named pipe */

		int fd;
#if 1 || defined(notdef)
		/* BTW, this is not Linux, but common Unix
		 * behaviour. Reading from a pipe with no writers will
		 * just return.
		 */

		/* Linux 2.0 is said to prefer that someone always has
		 * a named pipe open for writing as well as reading.
		 * But if we do that, we seem to lose the ENXIO error given
		 * to a process which opens that pipe for non-blocking write.
		 * Let's not do this now, even on Linux.
		 * Note that we do use PoolSleepFor() to slow down polling
		 * of pipes which claim to have data ready, but actually don't;
		 * I think that's what O_RDWR mode is intended to fix.
		 */
		fd = open(name, O_RDWR | O_NDELAY);

		/* Read-only file, or named pipe which doesn't allow RDWR? */
		if(fd < 0)
#endif
		    fd = open(name, O_RDONLY | O_NDELAY);


#if defined(unix) || defined(__unix)
		/* Unix-domain socket? */
		if(fd < 0 && errno == EOPNOTSUPP) {
		    struct sockaddr_un sa;
		    sa.sun_family = AF_UNIX;
		    strncpy(sa.sun_path, name, sizeof(sa.sun_path));
		    fd = socket(AF_UNIX, SOCK_STREAM, 0);
		    if(connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			close(fd);
			fd = -1;
		    }
		}
#endif /*unix*/

		if(fd < 0)
		    OOGLError(0, "Cannot open file \"%s\": %s", name, sperror());
		else {
#if defined(unix) || defined(__unix)
# ifdef FNONBLK
		    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~(FNDELAY|FNONBLK));
# else
		    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~FNDELAY);
# endif
#endif /*unix*/
		    p->inf  = fdopen(fd, "rb");
		}
	    }
	}
	if(rw > 0) {
	    if(strcmp(name, "-") == 0)
		p->outf = stdout;
	    else if((p->outf = fopen(name, "wb")) == NULL)
		OOGLError(0, "Cannot create \"%s\": %s", name, sperror());
	}
    } else {
	if(rw != 1)
	    p->inf = f;
	if(rw > 0)
	    p->outf = (rw == 2) ? fdopen(dup(fileno(f)), "wb") : f;
    }

    if(p->inf == NULL && p->outf == NULL) {
	PoolDelete(p);
	return NULL;
    }

	/* We're committed now. */
    if(p->next == AllPools)
	AllPools = p;
    p->seekable = 0;
    p->softEOF = 0;
    if(p->inf != NULL) {
	if (p->infd == -1 && p->sinf == NULL) {
	    p->infd = fileno(p->inf);
	}
	if (p->infd != -1) {
	    if(isatty(p->infd)) {
		p->softEOF = 1;
	    } else if(lseek(p->infd,0,SEEK_CUR) != -1) {
		p->seekable = 1;
	    }
	    if(fstat(p->infd, &st) < 0 || (st.st_mode & S_IFMT) == S_IFIFO)
		p->softEOF = 1;
	    p->inf_mtime = st.st_mtime;
	    watchfd(p->infd);
	}
    }
    if(p->level == 0 && p->outf &&
	  (lseek(fileno(p->outf),0,SEEK_CUR) == -1 || isatty(fileno(p->outf))))
	p->level = 1;
#if USE_SEEKPIPE
    if (p->inf != NULL && p->sinf == NULL && !p->seekable && p->infd >= 0) {
	/* leave seekable and fake files alone */
	extern FILE *seekpipe_open(int fd);
	p->sinf = p->inf;
	setbuf(p->inf, NULL);
	p->inf  = seekpipe_open(p->infd);
	p->infd = fileno(p->sinf);
    }
#else
    if (p->inf) {
	setvbuf(p->inf, NULL, _IOFBF, (1 << 16));
    }
#endif
    return p;
}

int
PoolIncLevel(Pool *p, int incr)
{
    return (p->level += incr);
}

int
PoolOType(Pool *p, int otype)
{
    return p->otype;
}

void
PoolSetOType(Pool *p, int otype)
{
    p->otype = otype;
}

FILE *
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

void PoolClose(register Pool *p)
{
    if(p->ops->close && !(p->flags & PF_CLOSING)) {
	p->flags |= PF_CLOSING;
	if((*p->ops->close)(p))
	    return;
    }

    if(p->type == P_STREAM) {
	if (p->sinf != NULL) {
	    fclose(p->inf);
	    p->inf = p->sinf;
	    p->sinf = NULL;
	}
	if(p->inf != NULL) {
	    unwatchfd(fileno(p->inf));
	    if(p->inf != stdin) fclose(p->inf);
	    p->inf = NULL;
	}
	if(p->outf != NULL) {
	    if(p->outf != stdout) fclose(p->outf);
	    p->outf = NULL;
	}
    }
}

void PoolDelete(Pool *p)
{
    register Pool **pp;
    register Handle *h;

    if(p == NULL || p->flags & PF_DELETED) return;
    p->flags |= PF_DELETED;

    if((p->flags & PF_TEMP) == 0) {
	for(pp = &AllPools; *pp != NULL; pp = &(*pp)->next) {
	    if(*pp == p) {
		*pp = p->next;
		for(h = AllHandles; h != NULL; h = h->next)
		    if(h->whence == p)
			h->whence = NULL;
		break;
	    }
	}
    }

    free(p->poolname);
    p->next = FreePools;
    FreePools = p;
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
    if(p->infd >= 0) {
	watchfd(p->infd);
	if(fhasdata(p->inf) && !FD_ISSET(p->infd, &poolreadyfds)) {
	   FD_SET(p->infd, &poolreadyfds);
	   poolnready++;
	}
    }
}

static void
awaken_until(struct timeval *until)
{
    register Pool *p;

    nexttowake.tv_sec = FOREVER;
    for(p = AllPools; p; p = p->next) {
	if(p->flags & PF_ASLEEP) {
	    if(timercmp(&p->awaken, until, <)) {
		awaken(p);
	    } else if(p->inf != NULL && timercmp(&p->awaken, &nexttowake, <)) {
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
    if(nexttowake.tv_sec != FOREVER) {
	struct timeval now;
	gettimeofday(&now, NULL);
	if(timercmp(&nexttowake, &now, <))
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
PoolInAll(register fd_set *fds, int *nfds)
{
    register Pool *p, *nextp;
    int got = 0;
    struct timeval now;
    int gotnow = 0;

    for(p = AllPools; p != NULL; p = nextp) {
	nextp = p->next;	/* Grab it now, in case we PoolDelete(p) */
	if(p->type != P_STREAM || p->inf == NULL || p->infd < 0)
	    continue;

	if(FD_ISSET(p->infd, &poolreadyfds)) {
	    FD_CLR(p->infd, &poolreadyfds);
	    poolnready--;
	    if(PoolIn(p))
		got++;
	} else if(FD_ISSET(p->infd, fds)) {
	    FD_CLR(p->infd, fds);
	    (*nfds)--;
	    if(PoolIn(p))
		got++;
	}
    }
    return got;
}

/*
 * Handle NULL or uninitialized times.
 */
static struct timeval *
timeof(struct timeval *when)
{
   static struct timeval now;
   if((when == NULL && (when = &now)) || !timerisset(when))
	gettimeofday(when, NULL);
   return when;
}
    
static void
addtime(struct timeval *base, double offset, struct timeval *result)
{
    double osec = floor(offset);
    result->tv_sec = base->tv_sec + osec;
    result->tv_usec = base->tv_usec + (int)((offset - osec)*1000000);
    while(result->tv_usec >= 1000000) {
	result->tv_sec++;
	result->tv_usec -= 1000000;
    }
}

static void
asleep(Pool *p, struct timeval *base, double offset)
{
    struct timeval until;

    base = timeof(base);
    if(p->inf != NULL) {
	p->flags |= PF_ASLEEP;
	addtime(base, offset, &until);
	if(timercmp(&until, &nexttowake, <))
	    nexttowake = until;
	if (p->infd >= 0) {
	    unwatchfd(p->infd);
	    if(FD_ISSET(p->infd, &poolreadyfds)) {
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
    addtime(base, -time_at_base, &p->timebase);
}

double
PoolTimeAt(Pool *p, struct timeval *then)
{
    if(p->timebase.tv_sec == 0) timeof(&p->timebase);
    then = timeof(then);
    return then->tv_sec - p->timebase.tv_sec +
		.000001*(then->tv_usec - p->timebase.tv_usec);
}

void
PoolAwaken(Pool *p)
{
    awaken(p);
    if(timercmp(&p->awaken, &nexttowake, <=))
	awaken_until(&nexttowake);
}

static void
poolresync(Pool *p, int (*resync)())
{ /* XXX implement this */
}


#define	CBRA	'{'
#define	CKET	'}'

Handle *
PoolIn(Pool *p)
{
    register int c = 0;
    int fd;
    Handle *h = NULL;
    Ref *r = NULL;
    int ok;

    if(p->type != P_STREAM)
	return NULL;		/* Do shared memory here someday XXX */
    if(p->inf == NULL || p->ops == NULL || p->ops->strmin == NULL)
	return NULL;		/* No way to read */

    if((p->flags & PF_NOPREFETCH) ||
       ((c = async_fnextc_fd(p->inf, 3, p->infd)) != NODATA && c != EOF)) {
	/* Kludge.  The interface to TransStreamIn really needs to change. */

	if((*p->ops->strmin)(p, &h,
			(strcmp(p->ops->prefix,"trans")==0) ? NULL : &r)) {

	    /*
	     * Attach nameless objects to a handle named for the Pool.
	     * Putting this code here in PoolIn() ensures we just bind names to
	     * top-level objects, not those nested inside a hierarchy.
	     */
	    if(h == NULL && r != NULL) {
		h = HandleAssign(p->poolname, p->ops, r);
		    /* Decrement reference count since we're handing
		     * ownership of the object to the Handle.
		     */
		RefDecr(r);
	    }


	    /* Remember whether we've read (PF_ANY) at least one and
	     * (PF_REREAD) at least two objects from this pool.
	     * There'll be a nontrivial side effect of rereading a file
	     * containing multiple objects, so we actually do reread if asked.
	     */
	    p->flags |= (p->flags & PF_ANY) ? PF_REREAD : PF_ANY;
	} else {
	    if(p->flags & PF_DELETED)
		return NULL;
	    if(p->ops->resync) {
		(*p->ops->resync)(p);
	    } else if(p->softEOF) {
		rewind(p->inf);
	    } else if(p->inf != NULL) {	/* Careful lest already PoolClose()d */
		if (p->infd >= 0) {
		    if(FD_ISSET(p->infd, &poolreadyfds)) {
			FD_CLR(p->infd, &poolreadyfds);
			poolnready--;
		    }
		}
		PoolClose(p);
		return NULL;
	    }
	}
	if(p->seekable && p->inf != NULL)
	    c = fnextc(p->inf, 0);	/* Notice EOF if appropriate */
    }
    if(c == EOF && feof(p->inf)) {
	if(p->softEOF) {
	    rewind(p->inf);
	    PoolSleepFor(p, 1.0);	/* Give us a rest */
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
	    return h;
	}
    }

    if(p->inf && !(p->flags & PF_ASLEEP) && p->infd >= 0) {
	/*
	 * Anything left in stdio buffer?  If so,
	 * remember to try reading next time without waiting for select().
	 */
	if(fhasdata(p->inf)) {
	    if(!FD_ISSET(p->infd, &poolreadyfds)) {
		FD_SET(p->infd, &poolreadyfds);
		poolnready++;
	    }
	} else {
	    if(FD_ISSET(p->infd, &poolreadyfds)) {
		FD_CLR(p->infd, &poolreadyfds);
		poolnready--;
	    }
	}
    }
    return h;
}

/*
 * Support routine for writing {handle, object} pairs to Pools.
 * Checks the Pool's output type (PO_HANDLES/PO_DATA/PO_ALL).
 * If appropriate, writes something to the Pool representing the given Handle.
 * Returns nonzero if the associated object should also be written literally.
 */
int
PoolStreamOutHandle(Pool *p, Handle *h, int havedata)
{
    if(p == NULL || p->outf == NULL)
	return 0;

    if(h == NULL || p->otype&PO_DATA ||
				(p->otype&PO_HANDLES && havedata))
	return havedata;

    if(h->whence != NULL && h->whence->seekable) {
	fprintf(p->outf, " < \"");
	if(strcmp(h->name, p->poolname) == 0) {
	    fprintf(p->outf, "%s\"\n", h->whence->poolname);
	} else {
	    fprintf(p->outf, "%s:%s\"\n", h->whence->poolname, h->name);
	}
    } else {
	fprintf(p->outf, ": \"%s\"\n", h->name);
    }
    return ((p->otype & (PO_DATA|PO_HANDLES)) == PO_ALL);
}
