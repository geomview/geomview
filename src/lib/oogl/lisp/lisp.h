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
  void *p;
  int i;
  unsigned long l;
  float f;
  double d;
} LCell;

struct LType
{
  /* name of type */
  const char *name;

  /* size of corresponding C type */
  int size;

  /* extract cell value from obj */
  bool (*fromobj)(/* LObject *obj, void *x */);

  /* create a new LObject of this type */
  LObject *(*toobj)(/* void *x */);

  /* free a cell of this type */
  void (*free)(/* void *x */);

  /* write a cell value to a stream */
  void (*write)(/* FILE *fp, void *x */);

  /* test equality of two cells of this type */
  bool (*match)(/* void *a, void *b */);

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

typedef struct Lake {
  IOBFILE *streamin;
  FILE *streamout;
  Pool *river;
  int   timing_interests;	/* Are we time-stamping interest reports? */
  float deltatime;		/* delta time between timestamps */
  float nexttime;		/* Pool time when next timestamp'll be needed */
  const char *initial, *prefix, *suffix; /* printf format strings */
} Lake;

#define POOL(lake)  ((lake)->river)

/* Return true if the next character starts a new S-expr, i.e. if the
 * next character is an opening parenthesis.
 */
static inline bool LakeNewSexpr(Lake *lake)
{
  return (iobfnextc(lake->streamin, 0) == '(');
}

/* Return true if the next token is NOT a closing parenthesis. */
static inline bool LakeMore(Lake *lake)
{
  int c;
  
  return (c = iobfnextc(lake->streamin,0)) != ')' && c != EOF;
}

/* Return the next token from LAKE or NULL.  If the token was quoted
 * then store the quote character in *QUOTE.
 */
static inline const char *LakeNextToken(Lake *lake, int *quote)
{
  return iobfquotedelimtok("()", lake->streamin, 0, quote);
}

/************************ end of lake stuff ***********************************/

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

/* A symbol is just a string which can be bound to a value in a lambda
 * expression. Symbols can be parsed into strings.
 */
extern LType LSymbolp;
#define LSYMBOL (&LSymbolp)
#define LSYMBOLVAL(obj) ((char*)((obj)->cell.p))

/* A string is just a symbol which cannot be bound to a value in a
 * lambda expression. Strings can be parsed into symbols, however.
 */
extern LType LStringp;
#define LSTRING (&LStringp)
#define LSTRINGVAL(obj) ((char*)((obj)->cell.p))

extern LType LIntp;
#define LINT (&LIntp)
#define LINTVAL(obj) ((obj)->cell.i)

extern LType LLongp;
#define LLONG (&LLongp)
#define LLONGVAL(obj) ((obj)->cell.l)

extern LType LFloatp;
#define LFLOAT (&LFloatp)
#define LFLOATVAL(obj) ((obj)->cell.f)

extern LType LDoublep;
#define LDOUBLE (&LDoublep)
#define LDOUBLEVAL(obj) ((obj)->cell.d)

extern LType LListp;
#define LLIST (&LListp)
#define LLISTVAL(obj) ((LList*)((obj)->cell.p))

#define LLAKEVAL(obj) ((Lake*)(obj->cell.p))
extern LType LLakep;
#define LLAKE (&LLakep)

extern LType LObjectp;
#define LLOBJECT (&LObjectp)

/* Convenience functions for generating objects */
static inline LObject *LLISTTOOBJ(LList *list)
{
  return LTOOBJ(LLIST)(&list);
}
static inline LObject *LSYMBOLTOOBJ(const char *string)
{
  return LTOOBJ(LSYMBOL)(&string);
}
static inline LObject *LSTRINGTOOBJ(const char *string)
{
  return LTOOBJ(LSTRING)(&string);
}
static inline LObject *LINTTOOBJ(int value)
{
  return LTOOBJ(LINT)(&value);
}
static inline LObject *LLONGTOOBJ(long value)
{
  return LTOOBJ(LLONG)(&value);
}
static inline LObject *LFLOATTOOBJ(float value)
{
  return LTOOBJ(LFLOAT)(&value);
}
static inline LObject *LDOUBLETOOBJ(double value)
{
  return LTOOBJ(LDOUBLE)(&value);
}

static inline bool LSTRINGFROMOBJ(LObject *obj, char **str)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    *str = LSTRINGVAL(obj);
    return true;
  }
  return false;
}

/*
 * Function definition stuff:
 */

enum lparseresult {
  LASSIGN_GOOD,
  LASSIGN_BAD,
  LPARSE_GOOD,
  LPARSE_BAD
};

typedef enum lparseresult LParseResult;

#define LPARSEMODE (lake != NULL)
#define LEVALMODE  (!LPARSEMODE)

#define LDECLARE(stuff) \
  switch (LParseArgs stuff) { \
  case LASSIGN_BAD: case LPARSE_BAD: return Lnil; \
  case LPARSE_GOOD: return Lt; \
  default: case LASSIGN_GOOD: break; \
  }

extern LType Larray;
#define LARRAY (&Larray)

extern LType Lvararray;
#define LVARARRAY (&Lvararray)

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

void            RemoveLakeInterests(Lake *lake);
void            LInit();
Lake *          LakeDefine(IOBFILE *streamin, FILE *streamout, void *river);
void            LakeFree(Lake *lake);
LObject *       LNew(LType *type, void *cell);
/* LObject *    LRefIncr(LObject *obj); */
/* void         LRefDecr(LObject *obj); */
void            LWrite(FILE *fp, LObject *obj);
/* void         LFree(LObject *obj); */
/* LObject      *LCopy(LObject *obj); */
LObject *       LSexpr(Lake *lake);
LObject *       LLiteral(Lake *lake);
LObject *       LEvalSexpr(Lake *lake);
LObject *       LEval(LObject *obj);
LList   *       LListNew();
LList   *       LListAppend(LList *list, LObject *obj);
void            LListFree(LList *list);
LList *         LListCopy(LList *list);
LObject *       LListEntry(LList *list, int n);
int             LListLength(LList *list);
LParseResult    LParseArgs(const char *name, Lake *lake, LList *args, ...);
bool            LDefun(const char *name, LObjectFunc func, const char *help);
void            LListWrite(FILE *fp, LList *list);
LInterest *     LInterestList(const char *funcname);
LObject *       LEvalFunc(const char *name, ...);
bool            LArgClassValid(LType *type);
void            LHelpDef(const char *key, const char *message);
const char *    LakeName(Lake *lake);
const char *    LSummarize(LObject *obj);
const char *    LListSummarize(LList *list);
LObject *       LMakeArray(LType *basetype, char *data, int count);

static inline LObject *LRefIncr(LObject *obj)
{
  ++(obj->ref);
  return obj;
}

static inline int LRefDecr(LObject *obj)
{
  return --(obj->ref);
}

static inline void LFree(LObject *obj)
{
  extern void _LFree(LObject *);
  if (obj == NULL || obj == Lnil || obj == Lt) return;
  if (LRefDecr(obj) == 0) {
    _LFree(obj);
  }
}

static inline LObject *LCopy(LObject *obj)
{
  if (obj == Lnil) return Lnil;
  if (obj == Lt) return Lt;
  return LTOOBJ(obj->type)(&(obj->cell));
}

void LShow(LObject *obj);	/* for debugging; writes obj to stderr */
void LListShow(LList *list);
void LWriteFile(const char *fname, LObject *obj);

#include "clisp.h"

/*
  LDEFINE(name, ltype, doc) is the header used to declare a lisp
  function.  It should be followed by the body of a function
  (beginning with '{' and ending with '}').  LDEFINE delcares the
  function's name to be "Lname" (the "name" argument with "L"
  prepended to it).  It also defines a string named "Hname"
  initialized to "doc".  The "ltype" argument gives the lisp object
  type returned by the function (all functions defined via DEFILE
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

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
