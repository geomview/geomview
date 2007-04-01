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

#include <ctype.h>
#include "../../../../config.h"

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * lisp.c: minimal (but less minimal than before) lisp interpreter
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "lisp.h"
#include "clisp.c"
#include "freelist.h"

#define MAXPAT 10
#define MAXPATLEN 128

typedef struct _pattern {
    int n;
    char p0[MAXPATLEN];
    char *pat[MAXPAT];
    int len[MAXPAT];
} pattern;

static int match(char *str, pattern *p);
static void compile(char *str, pattern *p);
static int LCompare(char *name, LObject *expr1, LObject *expr2);

typedef struct Help {
    char *key;
    char *message;
    struct Help *next;
} Help;

static Help *helps = NULL;

static char nomatch[] = "No commands match \"%s\"; see \"(? *)\" for a list.\n";

static int FilterArgMatch(LList *filter, LList *args);
static void InterestOutput(char *name, LList *args, LInterest *interest);

static LObject *LFAny, *LFNil;
static LFilter FAny = {ANY, NULL};
static LFilter FNil = {NIL, NULL};

static int obj2array(LObject *obj, LType *type, char *x, int *n);
LObject *LMakeArray(LType *basetype, char *array, int count);

static char *delims = "()";
#define NEXTTOKEN(tok,fp) tok = iobfdelimtok( delims, fp, 0 )

	/* Use -1 as the item size of special type markers
	 * for quick detection in LParseArgs()/AssignArgs().
	 */
LType Larray = { NULL, -1 };
LType Lend = { NULL, -1 };
LType Lrest = { NULL, -1 };
LType Lhold = { NULL, -1 };
LType Lliteral = { NULL, -1 };
LType Loptional = { NULL, -1 };

#define REJECT -1

typedef struct {
  LObjectFunc fptr;
  char *name;
  LInterest *interested;
} LFunction;

extern LType LFuncp;
#define LFUNC (&LFuncp)
#define LFUNCVAL(obj) ((int)((obj)->cell.i))

vvec funcvvec;
#define functable VVEC(funcvvec,LFunction)

static Fsa func_fsa;

/*
 * function prototypes
 */

static int AssignArgs(char *name, LList *args, va_list a_list);
static int funcindex(char *name);

static LObject *LSexpr0(Lake *lake, int listhow);
#define	LIST_LITERAL	0
#define	LIST_FUNCTION	1
#define	LIST_EVAL	2	/* Parse with intention to evaluate */

LObject *Linterest(Lake *lake, LList *args);
LObject *Luninterest(Lake *lake, LList *args);
LObject *Lregtable(Lake *lake, LList *args);
static LObject *do_interest(Lake *lake, LList *call, char *action);

static void RemoveInterests(LInterest **interest, Lake *lake,
			    int usefilter, LList *filter);
static int InterestMatch(LInterest *interest, Lake *lake,
			 int usefilter, LList *filter);
static int FilterMatch(LFilter *f1, LFilter *f2);
static void DeleteInterest(LInterest *interest);
static LInterest *NewInterest();
static void AppendInterest(LInterest **head, LInterest *new);
static LList *FilterList(LList *args);



/*
 * nil object implementation
 */

static void nilwrite(FILE *fp, void *value)
{
  fprintf(fp, "nil");
}

static LCell nullcell;

static LType niltype = {
  "nil",
  sizeof(int),
  NULL,
  NULL,
  NULL,
  nilwrite,
  NULL,
  NULL,
  NULL,
  LTypeMagic
  };
static LObject nil; /* = {&niltype, 1, nullcell }; */
LObject *Lnil = &nil;

/*
 * t object implementation
 */

static void twrite(FILE *fp, void *value)
{
  fprintf(fp,"t");
}

static LType ttype = {
  "t",
  sizeof(int),
  NULL,
  NULL,
  NULL,
  twrite,
  NULL,
  NULL,
  NULL,
  LTypeMagic
  } ;
static LObject t; /* = {&ttype, 1, nullcell }; */
LObject *Lt = &t;


/*
 * int object implementation
 */

static int intfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    if (cp[0]=='n' && cp[1]=='i' && cp[2]=='l' && cp[3]=='\0')
      *x = 0;
    else {
      *x = strtol(cp, &cp, 0);
      return cp != LSTRINGVAL(obj) ? 1 : 0;
    }
  } else if (obj->type == LINT) {
    *x = LINTVAL(obj);
  } else return 0;
  return 1;
}

static LObject *int2obj(x)
    int *x;
{
  return LNew( LINT, x );
}

static void intfree(x)
    int *x;
{}

static int intmatch(a, b)
    int *a,*b;
{
  return *a == *b;
}

static void intwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "%1d", *x);
}

static void intpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LObject *intparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string, promote it to an int, otherwise
     leave it as it is */
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    int val = strtol(cp, &cp, 0);
    if(cp != LSTRINGVAL(obj)) {		/* if valid int */
	free(LSTRINGVAL(obj));
	obj->type = LINT;
	obj->cell.i = val;
    }
  }
  return obj;
}

LType LIntp = {
  "int",
  sizeof(int),
  intfromobj,
  int2obj,
  intfree,
  intwrite,
  intmatch,
  intpull,
  intparse,
  LTypeMagic
  };

/*
 * float object implementation
 */

int floatfromobj(obj, x)
    LObject *obj;
    float *x;
{
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    *x = strtod(cp, &cp);
    return cp != LSTRINGVAL(obj) ? 1 : 0;
  } else if (obj->type == LFLOAT) {
    *x = LFLOATVAL(obj);
  } else return 0;
  return 1;
}

LObject *float2obj(x)
    float *x;
{
  return LNew( LFLOAT, x );
}

void floatfree(x)
    float *x;
{}

int floatmatch(a, b)
    float *a,*b;
{
  return *a == *b;
}

void floatwrite(fp, x)
    FILE *fp;
    float *x;
{
  fprintf(fp, "%1g", *x);
}

void floatpull(a_list, x)
    va_list *a_list;
    float *x;
{
  *x = va_arg(*a_list, double);
}

LObject *floatparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string or int, promote it to a float, otherwise
     leave it as it is */
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    float val = strtod(cp, &cp);
    if(cp != LSTRINGVAL(obj)) {
	free(LSTRINGVAL(obj));
	obj->type = LFLOAT;
	obj->cell.f = val;
    }
  } else if (obj->type == LINT) {
    float val = LINTVAL(obj);
    obj->type = LFLOAT;
    obj->cell.f = val;
  }
  return obj;
}

LType LFloatp = {
  "float",
  sizeof(float),
  floatfromobj,
  float2obj,
  floatfree,
  floatwrite,
  floatmatch,
  floatpull,
  floatparse,
  LTypeMagic
  };

/*
 * string object implementation
 */

int stringfromobj(obj, x)
    LObject *obj;
    char * *x;
{
  if (obj->type != LSTRING) return 0;
  *x = LSTRINGVAL(obj);
  return 1;
}

LObject *string2obj(x)
    char * *x;
{
  char *copy = *x ? strdup(*x) : NULL;
  return LNew( LSTRING, &copy );
}

void stringfree(x)
    char * *x;
{
  if (*x) free(*x);
}

int stringmatch(a, b)
    char **a,**b;
{
  if (!*a) return *b==NULL;
  if (!*b) return *a==NULL;
  return strcmp(*a,*b)==0 ;
}

void stringwrite(fp, x)
    FILE *fp;
    char * *x;
{
  fprintf(fp, "\"%s\"", *x);
}

void stringpull(a_list, x)
    va_list *a_list;
    char * *x;
{
  *x = va_arg(*a_list, char *);
}

LType LStringp = {
  "string",
  sizeof(char *),
  stringfromobj,
  string2obj,
  stringfree,
  stringwrite,
  stringmatch,
  stringpull,
  LSexpr,
  LTypeMagic
  };

/*
 * list implementation
 */
static DEF_FREELIST(LList);

LList *LListNew(void)
{
  LList *new;

  FREELIST_NEW(LList, new);
  new->cdr = NULL;
  return new;
}

LList *LListCopy(LList *list)
{
  LList *new;

  if (! list) return NULL;
  new = LListNew();
  if (list->car)
    new->car = LCopy(list->car);
  new->cdr = LListCopy(list->cdr);
  return (void*)new;
}

void LListFree(LList *list)
{
  if (!list) {
    return;
  }
  if (list->cdr) {
    LListFree(list->cdr);
  }
  LFree(list->car);
  FREELIST_FREE(LList, list);
}

void LListWrite(FILE *fp, LList *list)
{
  int first = 1;
  fprintf(fp,"(");
  while (list != NULL) {
    if (!first) {
      fprintf(fp," ");
    }
    first = 0;
    LWrite(fp, list->car);
    list = list->cdr;
  }
  fprintf(fp,")");
}

/**********************************************************************/

int listfromobj(LObject *obj, LList * *x)
{
  if (obj->type != LLIST) return 0;
  *x = LLISTVAL(obj);
  return 1;
}

LObject *list2obj(LList * *x)
{
  LList *list = *x ? LListCopy(*x) : NULL;
  return LNew( LLIST, &list );
}

void listfree(LList * *x)
{
  if (*x) LListFree(*x);
}

int listmatch(LList **a, LList **b)
{
  return *a == *b;
}

void listwrite(FILE *fp, LList * *x)
{
  LListWrite(fp, *x);
}

void listpull(va_list *a_list, LList * *x)
{
  *x = va_arg(*a_list, LList *);
}

LType LListp = {
  "list",
  sizeof(LList *),
  listfromobj,
  list2obj,
  listfree,
  listwrite,
  listmatch,
  listpull,
  LSexpr,
  LTypeMagic
  };

int objfromobj(obj, x)
    LObject *obj;
    LObject * *x;
{
  *x = LRefIncr(obj);
  return 1;
}

LObject *obj2obj(x)
    LObject * *x;
{
  if (*x) LRefIncr(*x);
  return *x;
}

void objpull(a_list, x)
    va_list *a_list;
    LObject * *x;
{
  *x = va_arg(*a_list, LObject *);
}

int objmatch(a, b)
    LObject **a,**b;
{
  return *a == *b;
}

LType LObjectp = {
  "lisp object",
  sizeof(LObject *),
  objfromobj,
  obj2obj,
  NULL,
  NULL,
  objmatch,
  objpull,
  LSexpr,
  LTypeMagic
  };

/*
 * Lake implementation
 */

Lake *LakeDefine(IOBFILE *streamin, FILE *streamout, void *river)
{
  Lake *lake = OOGLNewE(Lake, "new Lake");
  lake->streamin = streamin;
  lake->streamout = streamout;
  lake->river = river;
  lake->timing_interests = 0;
  return lake;
}

void LakeFree(Lake *lake)
{
  OOGLFree(lake);
}


/*
 * Lake object implementation
 *   ( Not the same as the Lake itself; the lake object is a lisp
 *     object type whose value is a Lake pointer. )
 */


int lakefromobj(obj, x)
    LObject *obj;
    Lake * *x;
{
  *x = LLAKEVAL(obj);
  return 1;
}

LObject *lake2obj(x)
    Lake * *x;
{
  return LNew( LLAKE, x );
}

void lakefree(x)
    Lake * *x;
{}

void lakewrite(fp, x)
    FILE *fp;
    Lake * *x;
{
  fprintf(fp,"-lake-");
}


LType LLakep = {
  "lake",
  sizeof(Lake *),
  lakefromobj,
  lake2obj,
  lakefree,
  lakewrite,
  NULL,
  NULL,
  NULL,
  LTypeMagic
  };

/*
 * function object implementation
 */

int funcfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    *x = funcindex(LSTRINGVAL(obj));
    if (*x == REJECT) return 0;
  } else if (obj->type == LFUNC) {
    *x = LFUNCVAL(obj);
  } else return 0;
  return 1;
}

LObject *func2obj(x)
    int *x;
{
  return LNew( LFUNC, x );
}

void funcfree(x)
    int *x;
{}

int funcmatch(a, b)
    int *a,*b;
{
  return *a == *b;
}

void funcwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "%s", functable[*x].name);
}

void funcpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LType LFuncp = {
  "lisp function",
  sizeof(int),
  funcfromobj,
  func2obj,
  funcfree,
  funcwrite,
  funcmatch,
  funcpull,
  LSexpr,
  LTypeMagic
  };

/**********************************************************************/

void LInit()
{
  VVINIT(funcvvec, LFunction, 30);
  func_fsa = fsa_initialize( NULL, (void*)REJECT );

  nullcell.p = NULL;
  nil.type = &niltype;
  nil.cell = nullcell;
  t.type = &ttype;
  t.cell = nullcell;

  {
    LCell cell;
    cell.p = (void*)(&FAny);
    LFAny = LNew(LFILTER, &cell);
    cell.p = (void*)(&FNil);
    LFNil = LNew(LFILTER, &cell);
  }

  {
    extern LObject *Lhelp(Lake *lake, LList *args);
    extern LObject *Lmorehelp(Lake *lake, LList *args);
    LDefun("?", Lhelp,
	   "(?  [command])\n\
	Command may include \"*\"s as wildcards; see also \"??\"\n\
	One-line command help; lists names only if multiple commands match.\n\
	? is a synonym for \"help\"");
    LDefun("??", Lmorehelp,
	"(?? command)  \"command\" may include \"*\" wildcards\n\
	Prints more info than \"(? command)\".  ?? is a synonym\n\
	for \"morehelp\"");
  }

  clisp_init();

  LHelpDef("STATEMENT",
	   "STATEMENT represents a function call.  Function calls have\n\
	   the form \"( func arg1 arg2 ... )\", where func is the name\n\
	   of the function and arg1, arg2, ... are the arguments.");

}
  
LDEFINE(quote, LLOBJECT,
       "(quote EXPR)\n\
	returns the symbolic lisp expression EXPR without evaluating it.")
{
  LObject *arg;

  LDECLARE(("quote", lake, args,
	    LLITERAL, LLOBJECT, &arg,
	    LEND));
  LRefIncr(arg);
  return arg;
}

LDEFINE(if, LLOBJECT,
	"(if TEST EXPR1 [EXPR2])\n\
	Evaluates TEST; if TEST returns a non-nil value, returns the\n\
	value of EXPR1.  If TEST returns nil, returns the value of\n\
	EXPR2 if EXPR2 is present, otherwise returns nil.")
{
  LObject *test, *tclause, *fclause=NULL;
  LDECLARE(("if", LBEGIN,
	    LLOBJECT, &test,
	    LHOLD, LLOBJECT, &tclause,
	    LOPTIONAL,
	    LHOLD, LLOBJECT, &fclause,
	    LEND));
  if (test != Lnil) {
    return LEval(tclause);
  } else if (fclause) {
    return LEval(fclause);
  } else {
    return Lnil;
  }
}

LDEFINE(not, LINT,
	"(not EXPR)\n\
	Evaluates EXPR; if EXPR returns a non-nil value, returns nil,\n\
	if EXPR returns nil, return t.")
{
  LObject *expr;

  LDECLARE(("not", LBEGIN,
	    LLOBJECT, &expr,
	    LEND));
  if (expr != Lnil) {
    return Lnil;
  } else {
    return Lt;
  }
}

LDEFINE(or, LLOBJECT,
	"(or EXPR1 EXPR2\n\
	Evaluates EXPR1; if EXPR1 returns non-nil, return its value,\n\
        if EXPR1 returns nil, evaluate EXPR2 and return its value.")
{
  LObject *expr1, *expr2;
  LDECLARE(("or", LBEGIN,
	    LLOBJECT, &expr1,
	    LHOLD, LLOBJECT, &expr2,
	    LEND));
  if (expr1 != Lnil) {
    /* arguments do not survive the life-time of a function */
    return LRefIncr(expr1);
  } else {
    return LEval(expr2);
  }
}

LDEFINE(and, LINT,
	"(and EXPR1 EXPR2\n\
	Evaluate EXPR1 and EXPR2 and return t if both return non-nil,\n\
	otherwise return nil.")
{
  LObject *expr1, *expr2;
  LDECLARE(("and", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  return (expr1 != Lnil && expr2 != Lnil) ? Lt : Lnil;
}

LDEFINE(greater, LINT,
	"(> EXPR1 EXPR2)\n\
	Returns t if EXPR1 is greater than EXPR2.  EXPR1 and EXPR2 should\n\
	be either both integers or floats, or both strings.")
{
  LObject *expr1, *expr2;
  LDECLARE((">", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  if (LCompare(">", expr1, expr2)==1) return Lt;
  else return Lnil;
}

LDEFINE(less, LINT,
	"(< EXPR1 EXPR2)\n\
	Returns t if EXPR1 is less than EXPR2.  EXPR1 and EXPR2 should\n\
	be either both integers or floats, or both strings.")
{
  LObject *expr1, *expr2;
  LDECLARE(("<", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  if (LCompare("<", expr1, expr2)==-1) return Lt;
  else return Lnil;
}

LDEFINE(equal, LINT,
	"(= EXPR1 EXPR2)\n\
	Returns t if EXPR1 is equal to EXPR2.  EXPR1 and EXPR2 should\n\
	be either both integers or floats, or both strings.")
{
  LObject *expr1, *expr2;
  LDECLARE(("=", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  if (LCompare("=", expr1, expr2)==0) return Lt;
  else return Lnil;
}

static int LCompare(char *name, LObject *expr1, LObject *expr2)
{
  char *s1, *s2;
  float e1, e2;
  if (expr1->type == LSTRING && expr2->type == LSTRING) {
    s1 = LSTRINGVAL(expr1);
    s2 = LSTRINGVAL(expr2);
    return -strcmp(s1,s2);
  }
  if (expr1->type == LINT) e1 = LINTVAL(expr1);
  else if (expr1->type == LFLOAT) e1 = LFLOATVAL(expr1);
  else {
    OOGLError(0, "%s: arg 1 must be int, float, or string\n", name);
    return -2;
  }
  if (expr2->type == LINT) e2 = LINTVAL(expr2);
  else if (expr2->type == LFLOAT) e2 = LFLOATVAL(expr2);
  else {
    OOGLError(0, "%s: arg 2 must be int, float, or string\n", name);
    return -2;
  }
  if (e1 == e2) return 0;
  else if (e1 > e2) return 1;
  else return -1;
}

LDEFINE(sgi, LINT,
	"(sgi)\n\
	Returns t if running on an sgi machine, nil if not")
{
  LDECLARE(("sgi", LBEGIN,
	    LEND));
  if (strcmp(MACHTYPE,"sgi")==0) return Lt;
  return Lnil;
}

LDEFINE(NeXT, LINT,
	"(NeXT)\n\
	Returns t if running on a NeXT, nil if not")
{
  LDECLARE(("NeXT", LBEGIN,
	    LEND));
  if (strcmp(MACHTYPE,"next")==0) return Lt;
  return Lnil;
}



LDEFINE(progn, LLOBJECT,
       "(progn STATEMENT [ ... ])\n\
	evaluates each STATEMENT in order and returns the value of the\n\
	last one.  Use progn to group a collection of commands together,\n\
	forcing them to be treated as a single command.")
{
  LObject *val=NULL;
  LList *arglist = NULL;

  LDECLARE(("progn", LBEGIN,
	LHOLD,
	LREST, &arglist,
	LEND));
  for( ; arglist != NULL; arglist = arglist->cdr) {
    LFree(val);
    val = LEval(arglist->car);
  }
  return val;
}

void LListShow(LList *list)
{
  LListWrite(stderr,list);
}

/*
 * Lisp object implementation
 */
static DEF_FREELIST(LObject);

LObject *_LNew(LType *type, LCell *cell)
{
  LObject *obj;

  FREELIST_NEW(LObject, obj);

  obj->type = type;
  obj->ref = 1;
  if (!cell) {
    obj->cell.p = NULL;
  } else if(sizeof(int) < sizeof(void *)) { /* Really want "alignof(int)" */
    int *unalignedcell = (int *)cell;
    memcpy((void *)&obj->cell, unalignedcell, sizeof(LCell));
  } else {
    obj->cell = *(LCell *)cell;
  }
  return obj;
}

void LWrite(FILE *fp, LObject *obj)
{
  (*obj->type->write)(fp, &(obj->cell));
}

void LWriteFile(char *fname, LObject *obj)
{
  FILE *fp = fopen(fname, "w");
  if (fp != NULL) {
    LWrite(fp, obj);
    fclose(fp);
  } else {
    OOGLError(0, "LWriteFile: can't create file %s",fname);
  }
}

void LShow(LObject *obj)
{
  LWrite(stderr, obj);
}

void LFree(LObject *obj)
{
  if (obj == NULL || obj == Lnil || obj == Lt) return;
  LRefDecr(obj);
  if ( obj->ref == 0 ) {
    (*obj->type->free)(&(obj->cell));
    FREELIST_FREE(LObject, obj);
  }
}

LObject *LCopy(LObject *obj)
{
  if (obj == Lnil) return Lnil;
  if (obj == Lt) return Lt;
  return LTOOBJ(obj->type)(&(obj->cell));
}

LObject *LRefIncr(LObject *obj)
{
  ++(obj->ref);
  return obj;
}

void LRefDecr(LObject *obj)
{
  --(obj->ref);
}

/* LSexpr() uses special parsing on lists; changes function names to
   function pointers, and calls the function to parse the arguments */
LObject *LSexpr(Lake *lake)
{
  return LSexpr0(lake, LIST_FUNCTION);
}
  
/* LLiteral() uses literal parsing; lists are not interpreted
   as function calls */
LObject *LLiteral(Lake *lake)
{
  return LSexpr0(lake, LIST_LITERAL);
}

/* LEvalSexpr() both parses and evaluates the requested expression. */
LObject *LEvalSexpr(Lake *lake)
{
  LObject *args, *val;

  args = LSexpr0(lake, LIST_EVAL);
  val = LEval(args);
  LFree(args);
  return val;
}


/* LSexpr0() does the work of both LSexpr() and LLiteral();
   special says whether to interpret lists specially */
static LObject *LSexpr0(Lake *lake, int listhow)
{
  LObject *obj, *head;
  char *tok;
  int i, c;
  
  NEXTTOKEN(tok,lake->streamin);
  if(tok == NULL)
    return Lnil;
  if (*tok == '(' && tok[1] == '\0') {
    obj = LNew(LLIST, NULL);
    if(listhow == LIST_LITERAL) {
	while ( LakeMore(lake,c) )
	    obj->cell.p = (void*) LListAppend((LList*)(obj->cell.p),
					 LSexpr0(lake,LIST_LITERAL));
    } else if ( LakeMore(lake,c) ) {
      /* if we have a non-empty list ... */
	/* ... get the first element and see if it's a function name */
	head = LEvalSexpr(lake);
	if (funcfromobj(head, &i)) {
	  /* It's a function name.  Enter the function as the first element
	     of our list, and then call the function in parse mode to
	     construct the rest of the list (arguments to the function) */
	    if(head->type != LFUNC) {
		LFree(head);
		head = LNew(LFUNC, &i);
	    }
	    obj->cell.p = (void*) LListAppend(LLISTVAL(obj), head);
	    if ( (*functable[i].fptr)(lake, LLISTVAL(obj)) == Lnil ) {
		LFree(obj);
		obj = Lnil;
	    }
	} else {
	  /* It's not a function name.  Probably this part will only
	     be called in error, because plain lists should always be
	     quoted.  This should probably be replaced by more robust
	     error detection and recovery code.  For now, just parse
	     as a plain list.  LEval() will emit an error message if
	     this list is ever evaluated. */
	  if(listhow == LIST_EVAL)
	     OOGLSyntax(lake->streamin, "Reading \"%s\": call to unknown function \"%s\"",
		LakeName(lake), LSummarize(head));
	  obj->cell.p = (void*) LListAppend(LLISTVAL(obj), head);
	  while ( LakeMore(lake,c) )
	    obj->cell.p = (void*) LListAppend(LLISTVAL(obj),
					     LSexpr0(lake,listhow));
	}
    }
    NEXTTOKEN(tok,lake->streamin);
  } else {
    obj = LNew(LSTRING, NULL);
    obj->cell.p = strdup(tok);
  }
  return obj;
}

LObject *LEval(LObject *obj)
{
  LObject *ans;
  LList *list, *args;
  LInterest *interest;
  LFunction *fentry;

  /* all non-list objects evaluate to themselves */
  if (obj->type != LLIST) {
    LRefIncr(obj);
    return obj;
  }

  list = LLISTVAL(obj);

  /* the empty list evaluates to itself */
  if (list == NULL || list->car == NULL) return obj;

  /* a nonempty list corresponds to a function call;
     the list's value is the value returned by the function */
  if (list->car->type == LFUNC) {
    fentry = &functable[LFUNCVAL(list->car)];
    args = LLISTVAL(obj)->cdr;

    /* deal with any interests in the function first */
    if ((interest=fentry->interested) != NULL) {
      while (interest) {
	if (FilterArgMatch(interest->filter, args)) {
	  InterestOutput(fentry->name, args, interest);
	}
	interest = interest->next;
      }
    }

    /* then call the function */
    ans = (*(fentry->fptr))( NULL, args );
    return ans;
  } else {
    OOGLError(0, "lisp error: call to unknown function %s", LSummarize(list->car));
    return Lnil;
  }
}

LList *LListAppend(LList *list, LObject *obj)
{
  LList *l, *new = LListNew();

  new->car = obj;
  l = list;
  if (l) {
    while (l->cdr) l = l->cdr;
    l->cdr = new;
    return list;
  }
  return new;
}

int LListLength(LList *list)
{
  int n=0;
  while (list) {
    ++n;
    list = list->cdr;
  }
  return n;
}

LObject *LListEntry(LList *list, int n)
{
  if (n < 0) n = LListLength(list) + 1 + n;
  while (list && --n) list = list->cdr;
  if (list) return list->car;
  else return NULL;
}

LDEFINE(car, LLOBJECT,
	"(car LIST)\n\
	returns the first element of LIST.")
{
  LList *list;
  LDECLARE(("car", LBEGIN,
	    LLIST, &list,
	    LEND));
  if (list && list->car) {
    return LCopy(list->car);
  }
  return Lnil;
}

LDEFINE(cdr, LLOBJECT,
	"(cdr LIST)\n\
	returns the list obtained by removing the first element of LIST.")
{
  LList *list;
  LDECLARE(("cdr", LBEGIN,
	    LLIST, &list,
	    LEND));
  if (list && list->cdr) {
    LList *copy = LListCopy(list->cdr);
    return LNew(LLIST, &copy);
  }
  return Lnil;
}

/*
 * function definition implementation
 */

int LDefun(char *name, LObjectFunc func, char *help)
{
  int index = VVCOUNT(funcvvec)++;
  LFunction *lfunction = VVINDEX(funcvvec, LFunction, index);
  lfunction->fptr = func;
  lfunction->name = strdup(name);
  lfunction->interested = NULL;
  fsa_install( func_fsa, name, (void *)(long)index );
  if (help) LHelpDef(name, help);
  return 1;
}

/* Function is called in one of three modes:
   lake != NULL, args != NULL: parse mode
     In this mode, upon entry args is a list containing one element,
     the function object itself.  We parse arguments from lake,
     appending them to the args list.  We return Lt if the parsing was
     successful, Lnil if not.
   lake == NULL: evaluate mode
     In this mode, upon entry args is a list containing the arguments
     to the function.  We return the function's value on the arguments.
*/

static int funcindex(char *name)
{
  return (int)(long)fsa_parse( func_fsa, name );
}

/*
 * The LDECLARE() macro calls this function.
 */
int LParseArgs(char *name, Lake *lake, LList *args, ...)
{
  int c, moreargspecs=1, argsgot=0, argsrequired= -1;
  LType *argclass;
  int argspecs=0, literal=0, hold = 0;
  LObject *arg;
  va_list a_list;

  va_start(a_list, args);
  
  if (lake == NULL) {
    int val = AssignArgs(name, args, a_list);
    va_end(a_list);
    return val;
  }
  
  while (moreargspecs) {
    argclass=va_arg(a_list, LType *);
    if (argclass->size < 0) {
	if (argclass == LEND) {
	  moreargspecs = 0;
	} else if (argclass == LOPTIONAL) {
	  argsrequired = argspecs;
	} else if (argclass == LHOLD) {
	  hold = 1;
	} else if (argclass == LLITERAL) {
	  /* literal affects the way an argument is parsed (as well as
	     implying "hold" in the assignment stage).  It should only be
	     used on LLOBJECT or LLIST.  It means parse the
	     argument literally.  In non-literal parsing, lists are
	     treated as function calls and the function is called to parse
	     the arguments.  In literal parsing, we don't treat lists as
	     function calls.  Just parse them as lists.  */
	  literal = 1;
	} else if (argclass == LARRAY) {
	  /* special case for this because it takes 3 args: the base type,
	     the array itself, and a count */
	    (void)va_arg(a_list, LType *);
	    (void)va_arg(a_list, void *);
	    (void)va_arg(a_list, int *);

	    ++argspecs;
	    if (LakeMore(lake,c)) {
		LListAppend(args, LSexpr(lake));
		++argsgot;
	    }
	} else if(argclass == LREST) {
	    /*
	     * Gather up any remaining arguments into an LList.
	     * If the caller provides a NULL pointer, discard them;
	     * otherwise store the list there.  Note that we yield an LList,
	     * not an LLIST-typed LObject.
	     */
	    LList **restp = va_arg(a_list, LList **);

	    (void)restp;

	    while(LakeMore(lake,c)) {
		arg = hold||literal ? LSexpr(lake) : LEvalSexpr(lake);
		LListAppend(args, arg);	/* Stash args for AssignArgs to grab */
	    }
	    moreargspecs = 0;
	}
    } else if(argclass == LLAKE) {
	(void)va_arg(a_list, Lake **);
	LListAppend(args, LTOOBJ(LLAKE)(&lake));
    } else {
      ++argspecs;
      (void)va_arg(a_list, void *);
      if (LakeMore(lake,c)) {
	LObject *arg;

	if (literal) {
	  /* literal should only be used on LLOBJECT or LLIST
	     types, both of which use the LSexpr() parse method; in
	     the literal case, we use LLiteral() instead. */
	  arg = LLiteral(lake);
	  literal=0;
	} else {
	  LObject *parsed = arg = LPARSE(argclass)(lake);
	  if(!hold && parsed->type == LLIST) {
	    arg = LEval(parsed);
	    LFree(parsed);
	  }
	}
	LListAppend(args, arg);
	++argsgot;
      }
    }
  }
  if (argsrequired<0) argsrequired = argspecs;
  va_end(a_list);
  if (argsgot < argsrequired) {
    OOGLSyntax(lake->streamin, "Reading from \"%s\": %s requires %d args, got %d",
	PoolName(POOL(lake)),name,argsrequired,argsgot);
    return LPARSE_BAD;
  }
  if (LakeMore(lake,c)) {
    OOGLSyntax(lake->streamin, "In \"%s\": %s: ignoring additional arguments (expected %1d)\n",
	    PoolName((Pool *)(lake->river)), name, argsgot);
    while (LakeMore(lake,c)) LFree(LSexpr(lake));
  }
  return LPARSE_GOOD;
}

static int obj2array(LObject *obj, LType *type, char *x, int *n)
{
  int max= abs(*n);
  LList *list;

  *n = 0;

  /* interpret the nil object as an empty list */
  if (   (obj == Lnil)
      || (obj->type==LSTRING && strcmp(LSTRINGVAL(obj),"nil")==0) ) {
    return 1;
  }
  
  list = LLISTVAL(obj);
  if (obj->type != LLIST) return 0;
  while (list && list->car && *n<max) {
    if (!LFROMOBJ(type)(list->car, (void*)(x + (*n)*LSIZE(type)))) return 0;
    (*n)++;
    list = list->cdr;
  }
  return 1;
}

LObject *LMakeArray(LType *basetype, char *array, int count)
{
  int i;
  LList *list = NULL;
  LObject *obj;

  for (i=0; i<count; ++i) {
    obj = LTOOBJ(basetype)((void*)(array + i*LSIZE(basetype)));
    list = LListAppend(list, obj);
  }
  return LNew(LLIST, &list);
}

static int AssignArgs(char *name, LList *args, va_list a_list)
{
  LObject *arg;
  int moreargspecs=1, argsgot=0, argsrequired= -1, hold=0;
  LType *argtype;
  int argspecs=0, convok;
  
  while (moreargspecs) {
    argtype=va_arg(a_list, LType *);
    if (argtype->size < 0) {
	if (argtype == LEND) {
	  moreargspecs = 0;
	} else if (argtype == LOPTIONAL) {
	  argsrequired = argspecs;
	} else if (argtype == LHOLD) {
	  hold=1;
	} else if (argtype == LLITERAL) {
	  /* in the assignment stage, literal means the same as hold */
	  hold=1;
	} else if (argtype == LLAKE) {
	  if (args) {
	    arg = args->car;
	    *va_arg(a_list, Lake **) = LLAKEVAL(arg);
	    args = args->cdr;
	  } else {
	    OOGLError(0,"%s: internal lake assignment out of whack.  Please\n\
    report this error!",name);
	    goto bad;
	  }
	} else if (argtype == LARRAY) {
	  /* get the base type of the array */
	  argtype=va_arg(a_list, LType *);
	  ++argspecs;
	  if (args) {
	    void *array = va_arg(a_list, void*);
	    int *count = va_arg(a_list, int*);
	    int origcount = abs(*count);
	    if (hold) {
	      arg = LRefIncr(args->car);
	    } else {
	      arg = LEval(args->car);
	    }
	    ++argsgot;
	    convok = obj2array(arg, argtype, array, count);
	    if (!convok) {
	      OOGLError(0, "%s: array of at most %1d %ss expected in\n\
     arg position %1d (got %s)\n", name,origcount, argtype->name, argsgot,
		      LSummarize(arg));
	    }
	    args = args->cdr;
	    hold = 0;
	  } else {
	    (void)va_arg(a_list, void *);
	    (void)va_arg(a_list, void *);
	  }
	} else if(argtype == LREST) {
	    LList **restp = va_arg(a_list, LList **);
	    if(restp)
		*restp = args;
	    moreargspecs = 0;
	    args = NULL;		/* Don't complain of excess args */
	}
    } else {
      ++argspecs;
      if (args) {
	if (hold) {
	  arg = LRefIncr(args->car);
	} else {
	  arg = LEval(args->car);
	}
	if (argtype == LLOBJECT && arg == args->car) {
	  LFree(arg);
	}
	++argsgot;
	convok = LFROMOBJ(argtype)(arg, va_arg(a_list, void *));
	if (!convok) {
	  OOGLError(0,"%s: %s expected in arg position %1d (got %s)\n",
		    name,LNAME(argtype),argsgot,LSummarize(arg));
	  LFree(arg);
	  goto bad;
	}
	LFree(arg);
	args = args->cdr;
	hold = 0;
      } else
	(void)va_arg(a_list, void *);
    }
  }
  if (argsrequired<0) argsrequired = argspecs;
  /*va_end(a_list);*/
  if (argsgot < argsrequired) {
    OOGLError(0,"%s: internal argument list deficit; require %1d, got %1d\n\
Please report this error!", name, argsrequired, argsgot);
    goto bad;
  }
  if (args) {
    OOGLError(1,"%s: internal argument list excess\n\
Please report this error!", name);
    goto bad;
  }
  return LASSIGN_GOOD;
 bad:
  /*va_end(a_list);*/
  return LASSIGN_BAD;
}

int LArgClassValid(LType *type)
{
  return (type->magic == LTypeMagic);
}

LObject *LEvalFunc(char *name, ...)
{
  va_list a_list;
  int laked = 0;
  LList *list = NULL;
  LObject *obj, *val;
  int i;
  LType *a;
  LCell cell;

  if ( (i=funcindex(name)) != REJECT ) {
    list = LListAppend(list, LNew( LFUNC, &i ));
  } else {
    char *copy = strdup(name);
    list = LListAppend(list, LNew( LSTRING, &copy ));
  }

  va_start(a_list, name);
  while ( (a=va_arg(a_list, LType *)) != LEND ) {
    if (a==LHOLD
	|| a==LLITERAL
	|| a==LOPTIONAL
	) {
      /* do nothing */
    } else if (a==LLAKE) {
      laked=1;
    } else if (a==LARRAY) {
      LType *basetype=va_arg(a_list, LType *);
      void *array = va_arg(a_list, void *);
      int count = abs(va_arg(a_list, int));
      list = LListAppend(list, LMakeArray(basetype, array, count));
    } else {
      LPULL(a)(&a_list, &cell);
      list = LListAppend(list, LTOOBJ(a)(&cell));
    }
  }
  obj = LNew( LLIST, &list );
  val = LEval(obj);
  LFree(obj);
  return val;
}

static int filterfromobj(obj, x)
    LObject *obj;
    LFilter * *x;
{
  if (obj->type != LFILTER) return 0;
  *x = LFILTERVAL(obj);
  return 1;
}

static LObject *filter2obj(x)
    LFilter * *x;
{
  LFilter *copy = OOGLNew(LFilter);
  copy->flag = (*x)->flag;
  copy->value = (*x)->value ? LCopy((*x)->value) : NULL;
  return LNew( LFILTER, &copy );
}

static void filterfree(x)
    LFilter * *x;
{
  if (*x) {
    if ((*x)->value) LFree((*x)->value);
    OOGLFree(*x);
  }
}

static void filterwrite(fp, x)
    FILE *fp;
    LFilter * *x;
{
  switch ((*x)->flag) {
  case VAL:
    fprintf(fp, "filter[VAL,");
    LWrite(fp, (*x)->value);
    fprintf(fp, "]");
    break;
  case ANY:
    fprintf(fp, "filter[ANY]");
    break;
  case NIL:
    fprintf(fp, "filter[NIL]");
    break;
  default:
    fprintf(fp, "filter[???");
    break;
  }
}

LType LFilterp = {
  "filter",
  sizeof(LFilter *),
  filterfromobj,
  filter2obj,
  filterfree,
  filterwrite,
  NULL,
  NULL,
  LSexpr,
  LTypeMagic
  };

LDEFINE(interest, LVOID,
     "(interest (COMMAND [args]))\n\
\n\
	Allows you to express interest in a command.  When geomview\n\
	executes that command in the future it will echo it to the\n\
	communication pool from which the interest command came.\n\
	COMMAND can be any command.  Args specify restrictions on the\n\
	values of the arguments; if args are present in the interest\n\
	command, geomview will only echo calls to the command in which\n\
	the arguments match those given in the interest command.  Two\n\
	special argument values may appear in the argument list.  \"*\"\n\
	matches any value. \"nil\" matches any value but supresses the\n\
	reporting of that value; its value is reported as \"nil\".\n\
\n\
	The purpose of the interest command is to allow external\n\
	modules to find out about things happening inside geomview.\n\
	For example, a module interested in knowing when a geom called\n\
	\"foo\" is deleted could say \"(interest (delete foo))\" and would\n\
	receive the string \"(delete foo)\" when foo is deleted.\n\
\n\
	Picking is a special case of this.  For most modules\n\
	interested in pick events the command \"(interest (pick\n\
	world))\" is sufficient.  This causes geomview to send a string\n\
	of the form \"(pick world ...)\" every time a pick event (right\n\
	mouse double click).  See the \"pick\" command for details.")
{
  Lake *calhoun;
  LList *call;

  LDECLARE(("interest", LBEGIN,
	    LLAKE, &calhoun,
	    LLITERAL, LLIST, &call,
	    LEND));

  return do_interest(calhoun, call, "interest");
}

LDEFINE(uninterest, LVOID,
       "(uninterest (COMMAND [args]))\n\
	Undoes the effect of an \"interest\" command.  (COMMAND [args]) must\n\
	be identical to those used in the \"interest\" command.")
{
  Lake *calhoun;
  LList *call;

  LDECLARE(("uninterest", LBEGIN,
	    LLAKE, &calhoun,
	    LLITERAL, LLIST, &call,
	    LEND));

  return do_interest(calhoun, call, "uninterest");
}

LDEFINE(time_interests, LVOID,
	"(time-interests deltatime initial prefix [suffix])\n\
	Indicates that all interest-related messages, when separated by at\n\
	least \"deltatime\" seconds of real time, should be preceded by\n\
	the string ``prefix'' and followed by ``suffix''; the first message\n\
	is preceded by ``initial''.  All three are printf format strings,\n\
	whose argument is the current clock time (in seconds) on that stream.\n\
	A \"deltatime\" of zero timestamps every message.  Typical usage:\n\
	(time-interests .1 \"(set-clock %g)\" \"(sleep-until %g)\")  or\n\
	(time-interests .1 \"(set-clock %g)\"\n\
		\"(sleep-until %g) (progn (set-clock %g)\" \")\")    or\n\
	(time-interests .1 \"(set-clock %g)\"\n\
			   \"(if (> 0 (sleep-until %g)) (\" \"))\".")
{
  Lake *l;
  float dt;
  char *initial = NULL, *prefix = NULL, *suffix = NULL;
  LDECLARE(("time-interests", LBEGIN,
	LLAKE, &l,
	LOPTIONAL, LFLOAT, &dt,
	LSTRING, &initial,
	LSTRING, &prefix,
	LSTRING, &suffix,
	LEND));
  if(l->timing_interests) {
    l->timing_interests = 0;
    if(l->initial) free(l->initial);
    if(l->prefix) free(l->prefix);
    if(l->suffix) free(l->suffix);
  }
  if(initial) {
    l->timing_interests = 1;
    l->initial = strdup(initial);
    l->prefix = prefix ? strdup(prefix) : NULL;
    l->suffix = suffix ? strdup(suffix) : NULL;
    l->deltatime = dt;
    l->nexttime = -1e10;
  }
  return Lt;
}

static LObject *do_interest(Lake *lake, LList *call, char *action)
{
  int i;
  LList *filter, *cargs;
  char *command;
  LInterest *new;

  if (!call || !call->car) {
    fprintf(stderr,"%s: COMMAND required.\n", action);
    return Lnil;
  }
  if (call->car->type != LSTRING) {
    fprintf(stderr, "%s: COMMAND must be a string (got %s)\n",
	    action, LSummarize(call->car));
    return Lnil;
  }
  command = LSTRINGVAL(call->car);

  /* any remaining args are the command's args */
  cargs = call->cdr;

  if ( (i=funcindex(command)) < 0 ) {
    fprintf(stderr, "%s: no such command \"%s\"\n", action,command);
    return Lnil;
  }

  filter = FilterList(cargs);

  if (strcmp(action, "interest")==0) {
    new = NewInterest();
    new->lake = lake;
    new->filter = filter;
    AppendInterest(&(functable[i].interested),  new);
  } else {
    RemoveInterests(&(functable[i].interested), lake, 1, filter);
    LListFree(filter);
  }
  return Lt;
}

static void RemoveInterests(LInterest **interest, Lake *lake,
			    int usefilter, LList *filter)
{
  LInterest *rest;

  while (*interest) {
    if (InterestMatch(*interest, lake, usefilter, filter)) {
      rest = (*interest)->next;
      DeleteInterest(*interest);
      *interest = rest;
    } else {
      interest = &((*interest)->next);
    }
  }
}

void RemoveLakeInterests(Lake *lake)
{
  int i;

  for (i=0; i<VVCOUNT(funcvvec); ++i) {
    if (functable[i].interested)
      RemoveInterests(&(functable[i].interested), lake, 0, NULL);
  }
}


static int InterestMatch(LInterest *interest, Lake *lake,
			 int usefilter, LList *filter)
{
  LList *ifilter;

  if (interest->lake != lake) return 0;
  if (!usefilter) return 1;
  ifilter = interest->filter;
  while (filter) {
    if (!ifilter) return 0;
    if (!FilterMatch(LFILTERVAL(filter->car),
		     LFILTERVAL(ifilter->car))) return 0;
    filter = filter->cdr;
    ifilter = ifilter->cdr;
  }
  if (ifilter) return 0;
  return 1;
}

static int FilterMatch(LFilter *f1, LFilter *f2)
{
  if (f1 && !f2) return 0;
  if (f2 && !f1) return 0;
  if (!f1 && !f2) return 1;
  if (f1->flag != f2->flag) return 0;
  switch (f1->flag) {
  case ANY:
  case NIL:
    return 1;
  case VAL:
    if (f1->value->type != f2->value->type) return 0;
    return LMATCH(f1->value->type)( &(f1->value->cell), &(f2->value->cell) );
  default:
    OOGLError(0,"invalid filter flag value.  Please report this.");
    return 0;
  }
}

static void DeleteInterest(LInterest *interest)
{
  if (interest) {
    if (interest->filter) LListFree(interest->filter);
    OOGLFree(interest);
  }
}

static LInterest *NewInterest()
{
  LInterest *new = OOGLNewE(LInterest, "interest");
  new->filter = NULL;
  new->next = NULL;
  return new;
}

static void AppendInterest(LInterest **head, LInterest *new)
{
  if (!head) {
    OOGLError(0,"Null head pointer in AppendInterest");
    return;
  }
  while (*head) head = &((*head)->next);
  *head = new;
}

static LList *FilterList(LList *args)
{
  LList *filterlist;
  LFilter *filter;

  if (!args) return NULL;
  filterlist = NULL;
  while (args) {
    if (!args->car) {
      OOGLError(1,"FilterList internal error");
      return NULL;
    }
    if (   (strcmp(LSTRINGVAL(args->car),"*")==0)
	|| (args->car==Lt) ) {
      filterlist = LListAppend(filterlist, LRefIncr(LFAny));
    } else if (   (strcmp(LSTRINGVAL(args->car),"nil")==0)
	       || (args->car==Lnil) ) {
      filterlist = LListAppend(filterlist, LRefIncr(LFNil));
    } else {
      filter = OOGLNew(LFilter);
      filter->flag = VAL;
      filter->value = LRefIncr(args->car);
      filterlist = LListAppend(filterlist, LNew(LFILTER, &filter));
    }
    args = args->cdr;
  }
  return filterlist;
}

static int FilterArgMatch(LList *filter,  LList *args)
{
  int filterflag;
  LObject *filterobj;
  LCell filterval, argval;

  while (args) {

    if (filter) {
      filterflag = LFILTERVAL(filter->car)->flag;
      filterobj = LFILTERVAL(filter->car)->value;
      filter=filter->cdr;
    } else
      filterflag = ANY;

    switch (filterflag) {
    case VAL:
      LFROMOBJ(args->car->type)(args->car, &argval);
      LFROMOBJ(args->car->type)(filterobj, &filterval);
      if (! LMATCH(args->car->type)(&filterval, &argval))
	return 0;
      break;
    case ANY:
    case NIL:
      break;
    }

    args = args->cdr;
  }
  return 1;
}

static void InterestOutput(char *name, LList *args, LInterest *interest)
{
  Lake *lake = interest->lake;
  FILE *outf = lake->streamout;
  LList *filter = interest->filter;
  char *suffix = NULL;
  int filterflag;
  float now = 0.0;

  if (!outf) return;

  if(lake->timing_interests &&
		(now = PoolTimeAt(POOL(lake), NULL)) > lake->nexttime) {
    if(lake->initial) {
	fprintf(outf, lake->initial, now,now,now);
	free(lake->initial);
	lake->initial = NULL;
    }
    if(lake->prefix)
	fprintf(outf, lake->prefix, now,now,now);
    suffix = lake->suffix;
  }

  fprintf(outf, "(%s", name);

  /* first remove any hidden lake arg */
  if (args && args->car && args->car->type == LLAKE)
    args = args->cdr;

  while (args) {

    if (filter) {
      filterflag = LFILTERVAL(filter->car)->flag;
      filter=filter->cdr;
    } else
      filterflag = ANY;

    switch (filterflag) {
    case VAL:
    case ANY:
      fputc(' ', outf);
      LWrite(outf, args->car);
      break;
    case NIL:
      fprintf(outf, " nil");
      break;
    }

    args = args->cdr;
  }
  fprintf(outf, ")\n");
  if(suffix)
    fprintf(outf, suffix, now,now,now);
  fflush(outf);
}

LDEFINE(regtable, LVOID,
       "(regtable) --- shows the registry table")
{
  int i;
  Lake *outlake;
  FILE *outf;
  LInterest *interest;
  LDECLARE(("regtable", LBEGIN,
	    LLAKE, &outlake,
	    LEND));
  outf = outlake->streamout;

  for (i=0; i<VVCOUNT(funcvvec); ++i) {
    if ((interest = functable[i].interested) != NULL) {
      fprintf(outf, "%s:\n", functable[i].name);
      fflush(outf);
      while (interest) {
	fprintf(outf, "\t");
	LListWrite(outf, interest->filter);
	fprintf(outf, "\n");
	fflush(outf);
	interest = interest->next;
      }
      fprintf(outf, "\n");
    }
  }
  return Lt;
}


static void compile(char *str, pattern *p)
{
    int n;
    char *rest, *tail;

    strncpy(p->p0, str, MAXPATLEN-1);
    p->p0[MAXPATLEN-1] = '\0';
    for(rest = p->p0, n = 0; (tail = strchr(rest, '*')) && n < MAXPAT; n++) {
	p->pat[n] = rest;
	p->len[n] = tail-rest;
	*tail = '\0';
	rest = tail+1;
    }
    p->pat[n] = rest;
    p->len[n] = strlen(rest);
    p->n = n;
}

/* Keep the first line unchanged and wrap the remaining lines to 80
 * chars with 8 chars indent on the left.
 */
static void print_help_formatted(FILE *outf, char *message)
{
  char *nl;
  int printed, wordlen, nnl;
  
  /* print the first line unchanged */
  if ((nl = strchr(message, '\n')) && message[0]=='(') {
    fprintf(outf, "%.*s", (int)(nl - message), message);
  }
  if (!nl) {
    return;
  }
  message = nl+1;
  while (*message) {
    fprintf(outf, "\n       ");
    printed = 7;
    while (*message && printed < 72) {
      nnl = 0;
      while (isspace(*message)) {
	/* keep \n\n as paragraph marker */
	if (*message++ == '\n') {
	  ++nnl;
	}
      }
      if (nnl >= 2) {
	fprintf(outf, "\n\n       ");
	printed = 7;
      }
      wordlen = 0;
      while (message[wordlen] && !isspace(message[wordlen])) {
	wordlen++;
      }
      if (printed + wordlen < 72) {
	printed += wordlen+1;
	putc(' ', outf);
	while (wordlen--) {
	  putc((int)*message++, outf);
	}
      } else {
	break;
      }
    }
  }
  putc('\n', outf);
  fflush(outf);
}

static int match(char *str, pattern *p)
{
    int i;
    char *rest;
    if(strncmp(str, p->pat[0], p->len[0])) return 0;	/* Failed */
    rest = str + p->len[0];
    for(i = 1; i <= p->n; i++) {
	if(p->len[i]) {
	    if((rest = strstr(rest, p->pat[i])) == NULL) break;
	    rest += p->len[i];
	}
    }
    return i > p->n && rest && (p->len[p->n] == 0 || *rest == '\0') ? 1 : 0;
}

void LHelpDef(char *key, char *message)
{
  Help **h = &helps;
  Help *new = OOGLNew(Help);

  /* insertion sort... */
  while (*h && (*h)->key && (strcmp(key,(*h)->key)>0))
    h = &((*h)->next);
  new->key = key;
  new->message = message;
  new->next = *h;
  *h = new;
}

void LHelpRedef(char *key, char *message)
{
  Help *h;
  for(h = helps; h != NULL; h = h->next)
    if(strcmp(key, h->key) == 0) {
	h->message = message;
	return;
    }
  LHelpDef(key, message);
}

LDEFINE(help, LVOID,
       "(help		[command])\n\
	Command may include \"*\"s as wildcards; see also \"??\"\n\
	One-line command help; lists names only if multiple commands match.")
{
  char *pat = "*";
  char *nl;
  pattern p;
  int seen = 0;
  Help *h, *last = NULL;
  Lake *brownie;
  FILE *outf;

  LDECLARE(("help", LBEGIN,
	    LLAKE, &brownie,
	    LOPTIONAL,
	    LSTRING, &pat,
	    LEND));
  if((outf = brownie->streamout) == NULL) outf = stdout;
  compile(pat, &p);
  for(h=helps; h!=NULL; h=h->next) {
    if(match(h->key, &p)) {
	if(++seen >= 2) {
	    if(seen == 2) fprintf(outf,"%-15s ", last->key);
	    fprintf(outf, seen%4 ? "%-15s " : "%s\n", h->key);
	}
	last = h;
    }
  }
  switch(seen) {
  default: if(seen%4) fprintf(outf, "\n"); break;
  case 0: fprintf(outf, nomatch, pat); break;
  case 1:
	nl = strchr(last->message, '\n');
	fprintf(outf, "%.*s\n", (int)(nl && last->message[0]=='('
				      ? nl - last->message  : 9999),
		last->message);
	break;
  }
  fflush(outf);
  return Lt;
}

LDEFINE(morehelp, LVOID,
	"(morehelp    command)  \"command\" may include \"*\" wildcards\n\
	Prints more info than \"(help command)\"")
{
  char *pat;
  pattern p;
  int seen = 0;
  Help *h;
  Lake *cedar;
  FILE *outf;

  LDECLARE(("morehelp", LBEGIN,
	    LLAKE, &cedar,
	    LSTRING, &pat,
	    LEND));
  if((outf = cedar->streamout) == NULL) outf = stdout;
  compile(pat, &p);
  for(h=helps; h!=NULL; h=h->next) {
    if(match(h->key, &p)) {
#if 0
	fprintf(outf, "%s\n", h->message);
#else
	print_help_formatted(outf, h->message);
#endif
	seen++;
    }
  }

  if(seen==0) fprintf(outf, nomatch, pat);
  fflush(outf);
  return Lt;
}

LInterest *LInterestList(char *funcname)
{
  int index = funcindex(funcname);
  if (index == REJECT) return NULL;
  return functable[index].interested;
}

char *LakeName(Lake *lake)
{
  return lake ? PoolName(lake->river) : NULL;
}

char *LSummarize(LObject *obj)
{
  int len;
  static FILE *f;
  static char *summary;

  if(f == NULL) {
    f = tmpfile();
    if(f == NULL) {
	return strdup("???");
    }
  }
  rewind(f);
  LWrite(f, obj);
  fflush(f);
  len = ftell(f);
  rewind(f);
  if(len >= 80) len = 79;
  if(summary) free(summary);
  summary = malloc(len+1);
  summary[len] = '\0';
  fread(summary, len, 1, f);
  if(len >= 79)
    strcpy(summary+75, " ...");
  return summary;
}

/************************************************************************/

/*
 * unsigned long object implementation
 */

static int ulongfromobj(obj, x)
    LObject *obj;
    unsigned long *x;
{
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    if (cp[0]=='n' && cp[1]=='i' && cp[2]=='l' && cp[3]=='\0')
      *x = 0;
    else {
      *x = strtol(cp, &cp, 0);
      return cp != LSTRINGVAL(obj) ? 1 : 0;
    }
  } else if (obj->type == LULONG) {
    *x = LULONGVAL(obj);
  } else return 0;
  return 1;
}

static LObject *ulong2obj(x)
    unsigned long *x;
{
  return LNew( LULONG, x );
}

static void ulongfree(x)
    unsigned long *x;
{}

static int ulongmatch(a, b)
    unsigned long *a,*b;
{
  return *a == *b;
}

static void ulongwrite(fp, x)
    FILE *fp;
    unsigned long *x;
{
  fprintf(fp, "%1lu", *x);
}

static void ulongpull(a_list, x)
    va_list *a_list;
    unsigned long *x;
{
  *x = va_arg(*a_list, unsigned long);
}

LObject *ulongparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string, promote it to a ulong, otherwise
     leave it as it is */
  if (obj->type == LSTRING) {
    char *cp = LSTRINGVAL(obj);
    unsigned long val = strtol(cp, &cp, 0);
    if(cp != LSTRINGVAL(obj)) {		/* if valid ulong */
	free(LSTRINGVAL(obj));
	obj->type = LULONG;
	obj->cell.ul = val;
    }
  }
  return obj;
}

LType LULongp = {
  "unsigned long",
  sizeof(unsigned long),
  ulongfromobj,
  ulong2obj,
  ulongfree,
  ulongwrite,
  ulongmatch,
  ulongpull,
  ulongparse,
  LTypeMagic
  };
