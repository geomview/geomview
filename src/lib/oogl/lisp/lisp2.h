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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef LISP_H
#define LISP_H

#include <stdarg.h>
#include "ooglutil.h"
#include "fsa.h"

#include "streampool.h"

typedef struct LType LType;
typedef struct LObject LObject;


typedef union {
  int i;
  float f;
  void *p;
} LCell;

struct LType {

  /* name of type */
  char *name;

  /* size of corresponding C type */
  int size;

  /* extract cell value from obj */
  int (*fromobj)(/* LObject *obj, void *x */);

  /* create a new LObject of this type */
  LObject *(*toobj)(/* void *x */);

  /* free a cell of this type */
  void (*free)(/* void *x */);

  /* write a cell value to a stream */
  void (*write)(/* FILE *fp, void *x */);

  /* test equality of two cells of this type */
  int (*match)(/* void *a, void *b */);

  /* pull a cell value from a va_list */
  void (*pull)(/* va_list *a_list, void *x */);

  /* parse an object of this type */
  LObject *(*parse)(/* Lake *lake */);

  /* magic number; always set to LTypeMagic */
  int magic;

};

#define LTypeMagic 314159

#define LNAME(type)	(type->name)
#define LSIZE(type)	(type->size)
#define LFROMOBJ(type)	(*(type->fromobj))
#define LTOOBJ(type)	(*(type->toobj))
#define LFREE(type)	(*(type->free))
#define LMATCH(type)	(*(type->match))
#define LWRITE(type)	(*(type->write))
#define LPULL(type)	(*(type->pull))
#define LPARSE(type)	(*(type->parse))

struct LObject {
  LType *type;
  int ref;
  LCell cell;
};

typedef struct {
  FILE *streamin;
  FILE *streamout;
  Pool *river;
  int   timing_interests;	/* Are we time-stamping interest reports? */
  float deltatime;		/* delta time between timestamps */
  float nexttime;		/* Pool time when next timestamp'll be needed */
  char *initial, *prefix, *suffix; /* printf format strings */
} Lake;


#define LakeMore(lake,c) ((c=fnextc(lake->streamin,0)) != ')' && c != EOF)
#define POOL(lake)  ((lake)->river)

typedef struct LList {
  LObject * 	car;
  struct LList *cdr;
} LList;

typedef LObject *(*LObjectFunc)();

typedef struct LInterest {
  Lake *lake;
  LList *filter;
  struct LInterest *next;
} LInterest;

typedef struct {
  enum { ANY,			/* match anything */
	 VAL,			/* match only our value */
	 NIL			/* match anything but report nil */
	 } flag;
  LObject *value;
} LFilter;

#define LFILTERVAL(lobject) ((LFilter *)(lobject->cell.p))
extern LType LFilterp;
#define LFILTER (&LFilterp)

/*
 * Built-in objects: Lnil and Lt:
 */
extern LObject *Lnil, *Lt;

/*
 * Built-in object types: string, list, and function.  Function type
 *  is only used internally.  See lisp.c for the code that initializes
 *  these type pointers.
 */
extern LType LStringp;
#define LSTRING (&LStringp)
#define LSTRINGVAL(obj) ((char*)((obj)->cell.p))

extern LType LIntp;
#define LINT (&LIntp)
#define LINTVAL(obj) ((obj)->cell.i)

extern LType LFloatp;
#define LFLOAT (&LFloatp)
#define LFLOATVAL(obj) ((obj)->cell.f)

extern LType LListp;
#define LLIST (&LListp)
#define LLISTVAL(obj) ((LList*)((obj)->cell.p))

#define LLAKEVAL(obj) ((Lake*)(obj->cell.p))
extern LType LLakep;
#define LLAKE (&LLakep)

extern LType LObjectp;
#define LLOBJECT (&LObjectp)

/*
 * Function definition stuff:
 */

#define LASSIGN_GOOD 1
#define LASSIGN_BAD  2
#define LPARSE_GOOD  3
#define LPARSE_BAD   4

#define LDECLARE(stuff) \
  switch (LParseArgs stuff) { \
  case LASSIGN_BAD: case LPARSE_BAD: return Lnil; \
  case LPARSE_GOOD: return Lt; \
  default: case LASSIGN_GOOD: break; \
  }

extern LType Larray;
#define LARRAY (&Larray)

extern LType Lend;
#define LEND (&Lend)

extern LType Lhold;
#define LHOLD (&Lhold)

extern LType Lliteral;
#define LLITERAL (&Lliteral)

extern LType Loptional;
#define LOPTIONAL (&Loptional)

extern LType Lrest;
#define	LREST (&Lrest)

/*
 * Function prototypes:
 */

void 	RemoveLakeInterests(Lake *lake);
void	  	LInit();
Lake *    	LakeDefine(FILE *streamin, FILE *streamout, void *river);
void	  	LakeFree(Lake *lake);
LObject * 	_LNew(LType *type, LCell *cell);
#define   	LNew(type,cell) _LNew(type,(LCell*)cell)
LObject * 	LRefIncr(LObject *obj);
void	  	LRefDecr(LObject *obj);
void	  	LWrite(FILE *fp, LObject *obj);
void	  	LFree(LObject *obj);
LObject	* 	LCopy(LObject *obj);
LObject * 	LSexpr(Lake *lake);
LObject *	LEvalSexpr(Lake *lake);
LObject * 	LEval(LObject *obj);
LList	* 	LListNew();
LList	* 	LListAppend(LList *list, LObject *obj);
void	  	LListFree(LList *list);
LList *	  	LListCopy(LList *list);
LObject * 	LListEntry(LList *list, int n);
int	  	LListLength(LList *list);
int	  	LParseArgs(char *name, Lake *lake, LList *args, ...);
int	  	LDefun(char *name, LObjectFunc func, char *help);
void		LListWrite(FILE *fp, LList *list);
LInterest *	LInterestList(char *funcname);
LObject *	LEvalFunc(char *name, ...);
int		LArgClassValid(LType *type);
void		LHelpDef(char *key, char *message);
void		LHelpRedef(char *key, char *newmessage);
char *		LakeName(Lake *lake);
char *		LSummarize(LObject *obj);
LObject *	LMakeArray(LType *basetype, char *data, int count);

void LShow(LObject *obj);	/* for debugging; writes obj to stderr */
void LListShow(LList *list);
void LWriteFile(char *fname, LObject *obj);

#include "clisp.h"

/*
  LDEFINE(name, ltype, doc) is the header used to declare a lisp
  function.  It should be followed by the body of a function
  (beginning with '{' and ending with '}').  LDEFINE delcares the
  function's name to be "Lname" (the "name" argument with "L"
  prepended to it).  It also defines a string named "Hname"
  initialized to "doc".  The "ltype" argument gives the lisp object
  type returned by the function (all functions defined via LDEFINE
  *must* return a lisp object.)  LDEFINE actually ignores this but
  read the next paragraph.

  LDEFINE is intended for use in conjunction with the "lisp2c" shell
  script which searches for calls to LDEFINE and builds a C language
  interface to the functions so defined.  This script makes use of all
  3 arguments to LDEFINE, plus the use of the LDECLARE macro in the
  function body that follows, to build the files clang.c and clang.h.
  It makes certain assumptions about the way LDEFINE and LDECLARE
  are used.  See the comments at the top of lisp2c for details.
  (I don't want to write the assumptions down here because if they
  change I'll forget to update this comment!).
*/

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define LDEFINE( name, ltype, doc ) \
  char H##name[] = doc ; LObject *L##name(Lake *lake, LList *args)
#else
#define LDEFINE( name, ltype, doc ) \
  char H/**/name[] = doc ; LObject *L/**/name(Lake *lake, LList *args)
#endif

#define LBEGIN lake, args

#endif /* ! LISP_H */

