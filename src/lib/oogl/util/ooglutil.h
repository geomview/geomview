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

#ifndef OOGLUTILDEF
#define OOGLUTILDEF

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
# define M_PI 3.14159265358979323
extern int finite(double v);
extern int strcasecmp(char *s1, char *s2);
#endif


/*
 * Definition so that (obsolete Irix 3) SGIs won't barf on member function prototypes.
 */
#ifdef no_prototypes
#define P(foo) ()
#else
#define P(foo) foo
#endif


#define	COUNT(array)	(sizeof(array)/sizeof(array[0]))

/*
 * Public definitions for miscellaneous useful OOGL internal stuff.
 */

#define	OOGLMagic(key, vers)    (0x9c800000 | (((key)&0x7f)<<16) | ((vers)&0xffff) )
#define OOGLIsMagic(magic)	(((magic) & 0xff800000) == 0x9c800000)

/*
 * Memory allocation
 */
#if defined(sgi) || AIX
extern void *(*OOG_NewP)(size_t);
extern void *(*OOG_RenewP)(void *, size_t);
#else
extern void *(*OOG_NewP)(size_t);
extern void *(*OOG_RenewP)(void *, size_t);
#endif

extern void (*OOGLFree)(void *);
extern void *OOG_NewE(int, char *);
extern void *OOG_RenewE(void *, int, char *);

#define	OOGLNew(t)		(t *)(*OOG_NewP)(sizeof(t))
#define	OOGLNewN(t,N)		(t *)(*OOG_NewP)(sizeof(t)*(N))
extern void (*OOGLFree)(void *);
#define	OOGLRenewN(t,p,N)	(t *)(*OOG_RenewP)(p, sizeof(t)*(N))
#define	OOGLRealloc(t,p)	(t *)(*OOG_RenewP)(p, sizeof(t))

#define	OOGLNewE(t, errmsg)	(t *)OOG_NewE(sizeof(t), errmsg)
#define	OOGLNewNE(t,N, errmsg)	(t *)OOG_NewE(sizeof(t)*(N), errmsg)
#define	OOGLRenewNE(t,p,N, errmsg) (t *)OOG_RenewE(p, sizeof(t)*(N), errmsg)

/*
 * Backward compatibility
 */
#define	GeomNew(t)		OOGLNew(t)
#define	GeomNewN(t,N)		OOGLNewN(t,N)
#define	GeomFree(p)		OOGLFree(p)
#define	GeomError  		OOGLError

/*
 * Variable-sized arrays ("vectors").  Manipulates variables of type "vvec".
 * Maintains the data they point to, but doesn't allocate the vvec's themselves;
 * typical use might be
 *   vvec myvec;
 *   VVINIT(myvec, float, 10);
 *   while( ... ) {
 *	*VVAPPEND(myvec, float) = something;
 *   }
 *   for(i = 0; i < VVSIZE(myvec); i++)
 *	... VVEC(myvec, float)[i] ...
 * 
 */
typedef struct vvec {
	char *base;	/* The real data */
	int count;	/* Number of elements in use (indices 0..count-1) */
	int allocated;	/* Number of elements allocated */
	int elsize;	/* sizeof(element type) */
	char dozero;	/* Zero-fill all new elements */
	char malloced;	/* "base" has been malloced */
	char spare1, spare2; /* for future extensions */
} vvec;

/*
 * Macros take 'vvec' arguments, while functions take addresses of vvec's.
 * VVINIT() : initialize new empty array; first malloc will grab >= 'minelems'.
 * VVZERO() : request that all newly allocated elements be cleared to zeros
 * VVINDEX() : return address of index'th element, ensuring that it exists.
 * VVAPPEND() : increments array size, returning address of new last element
 * VVEC()   : returns address of array base; VVEC()[i] is the i'th element
 *		assuming that its existence was ensured by earlier call to
 *		VVAPPEND() or VVINDEX() or vvneeds().
 * VVCOUNT() : number of valid elements in array.  Maintained by user.
 */
#define	VVINIT(vv,type,minelems)  vvinit(&(vv), sizeof(type), minelems)
#define	VVEC(vv, type)		((type *)((vv).base))
#define	VVCOUNT(vv)		(vv).count
#define	VVINDEX(vv,type,index)	((type *)vvindex(&(vv), (index)))
#define	VVAPPEND(vv, type)  	VVINDEX(vv, type, (vv).count++)

/* Functions take addresses of vvec's.  Besides the above, there are
 * vvtrim() : trim allocated but unused space (beyond VVCOUNT()'th element).
 * vvfree() : free malloced array.
 * vvneeds(): ensure that at least this many elements are allocated
 * vvzero() : all newly allocated elements will be filled with binary zeros
 * vvuse()  : Use the given non-malloced buffer until more room is needed;
 *		call vvuse() after vvinit() but before allocating anything.
 * vvcopy() : copies one vvec into another, allocating space in the 
 *              destination to accommodate the data and copying everything.
 */
extern void vvinit(vvec *v, int elsize, int minelems);
extern void vvuse(vvec *v, void *buf, int allocated);
extern void vvtrim(vvec *v);		/* Trim allocated but unused data */
extern void vvfree(vvec *v);		/* Free all malloced data */
extern void vvneeds(register vvec *v, int needed);
extern void *vvindex(vvec *v, int index);
extern void vvzero(vvec *v);
extern void vvcopy(vvec *src, vvec *dest);



/*
 * Error handling
 */
extern char *_GFILE;		/* Name of file where error is found */
extern int _GLINE;		/* Line number in file where error is found */
extern int OOGL_Errorcode;	/* Unique integer error code */
extern void OOGLWarn (char *fmt, ...);
extern const char *sperrno(unsigned int);
extern const char *sperror(void);

/* Kludge for obtaining file name and line number of error: */
#define OOGLError (_GFILE= __FILE__, _GLINE=__LINE__,0)?0:_OOGLError

extern int _OOGLError(int, char *fmt, ...);

extern void OOGLSyntax(FILE *, char *fmt, ...); 

	/* Bit fields in error codes */
#define	OE_VERBOSE	0x1
#define	OE_FATAL	0x2


/*
 * File-I/O utility routines
 */
extern int fnextc(FILE *, int flags);
extern int fexpectstr(FILE *, char *string);
extern int fexpecttoken(FILE *, char *string);
extern char *ftoken(FILE *, int fnextc_flags);
extern char *fdelimtok(char *delims, FILE *f, int flags);

extern int fgetnf(FILE *, int nfloats, float *floatp, int binary);
extern int fputnf(FILE *, int nfloats, float *floatp, int binary);
extern int fgetni(FILE *, int nints, int *intp, int binary);
extern int fgetns(FILE *, int nshorts, short *shortp, int binary);
extern int fgettransform(FILE *, int ntrans, float *transforms, int binary);
extern int fputtransform(FILE *, int ntrans, float *transforms, int binary);
extern FILE *fstropen(char *buf, int buflen, char *mode);
extern int fhasdata(FILE *);

/*
 * fcontext(f) returns a string indicating the current position of file f,
 * or the empty string if no data are available.
 */
extern char *fcontext(FILE *f);


#define	NODATA	-2	/* async_fnextc() and async_getc() return NODATA if
			 * none is immediately available
			 */

extern int async_fnextc(FILE *, int flags);
extern int async_getc(FILE *);

extern struct stdio_mark *stdio_setmark(struct stdio_mark *, FILE *);
extern int		  stdio_seekmark(struct stdio_mark *);
extern void		  stdio_freemark(struct stdio_mark *);

/*
 * int fnextc(FILE *f, int flags)
 *	Advances f to the next "interesting" character and
 *	returns it.  The returned char is ungetc'ed so the next getc()
 *	will yield the same value.
 *	Interesting depends on flags:
 *	  0 : Skip blanks, tabs, newlines, and comments (#...\n).
 *	  1 : Skip blanks, tabs, and comments, but newlines are interesting
 *		(including the \n that terminates a comment).
 *	  2 : Skip blanks, tabs, and newlines, but stop at #.
 *	  3 : Skip blanks and tabs but stop at # or \n.
 *
 * int
 * fexpectstr(FILE *file, char *string)
 *	Expect the given string to appear immediately on file.
 *	Return 0 if the complete string is found,
 *	else the offset+1 of the last matched char within string.
 *	The first unmatched char is ungetc'd.
 *
 * int
 * fexpecttoken(FILE *file, char *string)
 *	Expect the given string to appear on the file, possibly after
 *	skipping some white space and comments.
 *	Return 0 if found, else the offset+1 of last matched char in string.
 *	The first unmatched char is ungetc'd.
 *
 * char *ftoken(FILE *f, int flags)
 *	Skips uninteresting characters with fnextc(f, flags),
 *	then returns a "token" - string of consecutive interesting characters.
 *	Returns NULL if EOF is reached with no token, or if
 *	flags specifies stopping at end-of-line and this is encountered with
 *	no token found.
 *	The token is effectively statically allocated and will be
 *	overwritten by the next ftoken() call.
 */
/*
 * int fgetnf(file, nfloats, floatp, binary)
 *	Read an array of floats from a file in "ascii" or "binary" format.
 *	Returns number of floats successfully read, should = nfloats.
 *	"Binary" means "IEEE 32-bit floating-point" format.
 *
 * int fgetni(FILE *file, int nints, int *intsp, int binary)
 *	Read an array of ints from a file in "ascii" or "binary" format.
 *	Returns number of ints successfully read, should = nints.
 *	"Binary" means "32-bit big-endian" integer format.
 *
 * int fgetns(FILE *file, int nshorts, short *intsp, int binary)
 *	Read an array of shorts from a file in "ascii" or "binary" format.
 *	Returns number of shorts successfully read, should = nints.
 *	"Binary" means "16-bit big-endian" integer format.
 *
 * int fgettransform(FILE *f, int ntransforms, float *transforms, int binary)
 *	Reads 4x4 matrices from FILE.  Returns the number of matrices found,
 *	up to ntransforms.  Returns 0 if no numbers are found.
 *	On finding incomplete matrices (not a multiple of 16 floats)
 *	returns -1, regardless of whether any whole matrices were found.
 *	In ASCII (binary==0) mode, matrices are read in conventional order
 *	which is the transpose of the internal form.  Binary mode reads
 *	32-bit IEEE floats in internal order.
 *
 * int fputtransform(FILE *f, int ntransforms, float *transforms, int binary)
 *	Writes 4x4 matrices to FILE.  Returns the number written, i.e.
 *	ntransforms unless an error occurs.  See fgettransform() for format.
 *
 * FILE *fstropen(str, len, mode)
 *	Opens a string (buffer) as a "file".
 *	Mode is the usual "r", "w", etc. stuff.
 *	Reads should return EOF on encountering end-of-string,
 *	writes past end-of-string should also yield an error return.
 *	fclose() should be used to free the FILE after use.
 */

extern char *findfile(char *superfile, char *file);
extern void filedirs(char *dirs[]);
extern char **getfiledirs();
char *envexpand(char *s);

extern char **ooglglob(char *s);
extern void ooglblkfree(char **av0);


/*
 * macro for grabbing the next value from either a region of
 * memory or a va_list.
 * al should be the address of the va_list
 * p should be the address of a poiner to the region of memory,
 *   or NULL, which means use the va_list instead.
 */
#define OOGL_VA_ARG(type,al,p) (p ? ((type*)((*p)+=sizeof(type)))[-1] \
    : (va_arg (*al, type)))


#include "porting.h"


#endif
