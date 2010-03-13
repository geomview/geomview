/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * lisp.c: minimal (but less minimal than before) lisp interpreter
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
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

static bool match(const char *str, pattern *p);
static void compile(const char *str, pattern *p);
static int LCompare(const char *name, LObject *expr1, LObject *expr2);

typedef struct Help {
  const char *key;
  const char *message;
  struct Help *next;
} Help;

static Help *helps = NULL;

static char nomatch[] = "No commands match \"%s\"; see \"(? *)\" for a list.\n";

static bool FilterArgMatch(LList *filter, LList *args);
static void InterestOutput(char *name, LList *args, LInterest *interest);

static LFilter FAny = {ANY, NULL};
static LFilter FNil = {NIL, NULL};
static LObject LFAny[1] = {{ LFILTER, 1, { &FAny } }};
static LObject LFNil[1] = {{ LFILTER, 1, { &FNil } }};
     

static bool obj2array(LObject *obj,
		      LType *type, char *x, int *n, bool hold);
static bool obj2vararray(LObject *obj,
			 LType *type, char **x, int *n, bool hold);
LObject *LMakeArray(LType *basetype, char *array, int count);

/* Use -1 as the item size of special type markers
 * for quick detection in LParseArgs()/AssignArgs().
 */
LType Larray = { NULL, -1, };
LType Lvararray = { NULL, -1, };
LType Lend = { NULL, -1, };
LType Lrest = { NULL, -1, };
LType Lhold = { NULL, -1, };
LType Lliteral = { NULL, -1, };
LType Loptional = { NULL, -1, };

#define REJECT -1

typedef struct {
  LObjectFunc fptr;
  LObject *lambda; /* != NULL for non-builtins */
  char *name;
  char *help;
  LInterest *interested;
} LFunction;

extern LType LFuncp;
#define LFUNC (&LFuncp)
#define LFUNCVAL(obj) ((int)((obj)->cell.i))

vvec funcvvec;
#define functable VVEC(funcvvec,LFunction)

static Fsa func_fsa;

/* lambda expression parameters */
typedef struct LNameSpace LNameSpace;
struct LNameSpace
{
  vvec       table;
  Fsa        parser;
  LNameSpace *next;
};

static LNameSpace *lambda_namespace;

/* we also support a global variable name-space, accessible via
 *
 * (setq ...)
 *
 * Note that (setq symbol value) does not mean that you can bind,
 * e.g. geometries to a lisp variable: the return value of (geometry
 * BLAH) will just be Lt in the case of success, or Lnil in the case
 * of error. So: (setq foo (geometry blah)) will just bind foo the Lt
 * or Lnil.
 */
static LNameSpace setq_namespace[1];

/*
 * function prototypes
 */

static LParseResult AssignArgs(const char *name, LList *args, va_list a_list);
static int funcindex(const char *name);

static inline LObject *LSexpr0(Lake *lake, int listhow);

#define	LIST_LITERAL	0
#define	LIST_FUNCTION	1
#define	LIST_EVAL	2	/* Parse with intention to evaluate */

LObject *Linterest(Lake *lake, LList *args);
LObject *Luninterest(Lake *lake, LList *args);
LObject *Lregtable(Lake *lake, LList *args);
static LObject *do_interest(Lake *lake, LList *call, char *action);

static void RemoveInterests(LInterest **interest, Lake *lake,
			    int usefilter, LList *filter);
static bool InterestMatch(LInterest *interest, Lake *lake,
			  bool usefilter, LList *filter);
static bool FilterMatch(LFilter *f1, LFilter *f2);
static void DeleteInterest(LInterest *interest);
static LInterest *NewInterest();
static void AppendInterest(LInterest **head, LInterest *new);
static LList *FilterList(LList *args);
static inline bool lambdafromobj(LObject *lambda, LList **args, LList **body);
static inline bool
namespace_put(LNameSpace *ns, char *name, LObject *value, bool overwrite);
static inline LObject *namespace_get(LNameSpace *ns, char *name);
static inline LObject *ParseArg(LType *type, Lake *lake);

/*
 * nil object implementation
 */

static LObject nil = {LLIST, 1, { NULL } };
LObject *Lnil = &nil;

/*
 * t object implementation
 */

static void twrite(FILE *fp, void *value)
{
  (void)value;
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
static LObject t  = {&ttype, 1, { NULL } };
LObject *Lt = &t;

/*
 * int object implementation
 */

static bool intfromobj(LObject *obj, int *x)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    long value;
    double dval;

    if (strcmp(cp, "nil") == 0) {
      *x = 0;
      return true;
    }

    *x = value = strtol(cp, &end, 0);
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      return (long)(int)value == value;
    }
    *x = dval = strtod(cp, &end);
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      return (double)*x == dval;
    }
    return false;
  } else if (obj->type == LINT) {
    *x = LINTVAL(obj);
  } else if (obj->type == LLONG) {
    long val = LLONGVAL(obj);
    *x = (int)val;
    return (long)(int)val == val;
  } else if (obj == Lnil) {
    *x = 0;
  } else if (obj->type == LFLOAT) {
    float val = LFLOATVAL(obj);
    *x = (int)val;
    return (float)(int)val == val;
  } else if (obj->type == LDOUBLE) {
    double val = LDOUBLEVAL(obj);
    *x = (int)val;
    return (double)(int)val == val;
  } else {
    return false;
  }
  return true;
}

static LObject *int2obj(int *x)
{
  return LNew( LINT, x );
}

static void intfree(int *x)
{
  (void)x;
}

static bool intmatch(int *a, int *b)
{
  return *a == *b;
}

static void intwrite(FILE *fp, int *x)
{
  fprintf(fp, "%1d", *x);
}

static void intpull(va_list *a_list, int *x)
{
  *x = va_arg(*a_list, int);
}

static LObject *intparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string, promote it to an int, otherwise
     leave it as it is */
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    int val;

    if (strcmp(cp, "nil") == 0) {
      val = 0;
      end = cp + 3;
    } else {
      val = strtol(cp, &end, 0);
    }
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
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
 * long object implementation
 */

static bool longfromobj(LObject *obj, long *x)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    double dval;

    if (strcmp(cp, "nil") == 0) {
      *x = 0;
      return true;
    }

    *x = strtol(cp, &end, 0);
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      return true;
    }
    *x = dval = strtod(cp, &end);
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      return (double)*x == dval;
    }
    return false;
  } else if (obj->type == LLONG) {
    *x = LLONGVAL(obj);
  } else if (obj->type == LINT) {
    *x = LINTVAL(obj);
  } else if (obj == Lnil) {
    *x = 0;
  } else if (obj->type == LFLOAT) {
    float val = LFLOATVAL(obj);
    *x = (long)val;
    return (float)(long)val == val;
  } else if (obj->type == LDOUBLE) {
    double val = LDOUBLEVAL(obj);
    *x = (long)val;
    return (double)(long)val == val;
  } else {
    return false;
  }
  return true;
}

static LObject *long2obj(long *x)
{
  return LNew( LLONG, x );
}

static void longfree(long *x)
{
  (void)x;
}

static bool longmatch(long *a, long *b)
{
  return *a == *b;
}

static void longwrite(FILE *fp, long *x)
{
  fprintf(fp, "%1lu", *x);
}

static void longpull(va_list *a_list, long *x)
{
  *x = va_arg(*a_list, long);
}

static LObject *longparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string, promote it to a long, otherwise
     leave it as it is */
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    long val;

    if (strcmp(cp, "nil") == 0) {
      val = 0;
      end = cp + 3;
    } else {
      val = strtol(cp, &end, 0);
    }
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      free(LSTRINGVAL(obj));
      obj->type = LLONG;
      obj->cell.l = val;
    }
  }
  return obj;
}

LType LLongp = {
  "long",
  sizeof(long),
  longfromobj,
  long2obj,
  longfree,
  longwrite,
  longmatch,
  longpull,
  longparse,
  LTypeMagic
};

/*
 * float object implementation
 */

static bool floatfromobj(LObject *obj, float *x)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;

    if (strcmp(cp, "nil") == 0) {
      *x = 0;
      return true;
    }

    *x = strtod(cp, &end);
    return ((size_t)end - (size_t)cp) == strlen(cp) ? true : false;
  } else if (obj->type == LFLOAT) {
    *x = LFLOATVAL(obj);
  } else if (obj->type == LLONG) {
    *x = LLONGVAL(obj);
  } else if (obj->type == LINT) {
    *x = LINTVAL(obj);
  } else {
    return false;
  }
  return true;
}

static LObject *float2obj(float *x)
{
  return LNew( LFLOAT, x );
}

static void floatfree(float *x)
{
  (void)x;
}

static bool floatmatch(float *a, float *b)
{
  return *a == *b;
}

static void floatwrite(FILE *fp, float *x)
{
  fprintf(fp, "%1g", *x);
}

static void floatpull(va_list *a_list, float *x)
{
  *x = va_arg(*a_list, double);
}

static LObject *floatparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string or another numerical value, promote it to a
     float, otherwise leave it as it is */
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    float val = strtod(cp, &end);
    /* Allow a conversion only if the entire string is a float;
     * otherwise reject it. There is no point in converting 1.4foobar
     * to a float.
     */
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      free(LSTRINGVAL(obj));
      obj->type = LFLOAT;
      obj->cell.f = val;
    }
  } else if (obj->type == LINT) {
    obj->type = LFLOAT;
    obj->cell.f = LINTVAL(obj);
  } else if (obj->type == LLONG) {
    obj->type = LFLOAT;
    obj->cell.f = LLONGVAL(obj);
  } else if (obj->type == LDOUBLE) {
    obj->type = LFLOAT;
    obj->cell.f = LDOUBLEVAL(obj);
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
 * double object implementation
 */

static bool doublefromobj(LObject *obj, double *x)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;

    if (strcmp(cp, "nil") == 0) {
      *x = 0;
      return true;
    }

    *x = strtod(cp, &end);
    return ((size_t)end - (size_t)cp) == strlen(cp) ? true : false;
  } else if (obj->type == LDOUBLE) {
    *x = LDOUBLEVAL(obj);
  } else if (obj->type == LFLOAT) {
    *x = LFLOATVAL(obj);
  } else if (obj->type == LLONG) {
    *x = LLONGVAL(obj);
  } else if (obj->type == LINT) {
    *x = LINTVAL(obj);
  } else {
    return false;
  }
  return true;
}

static LObject *double2obj(double *x)
{
  return LNew( LDOUBLE, x );
}

static void doublefree(double *x)
{
  (void)x;
}

static bool doublematch(double *a, double *b)
{
  return *a == *b;
}

static void doublewrite(FILE *fp, double *x)
{
  fprintf(fp, "%1g", *x);
}

static void doublepull(va_list *a_list, double *x)
{
  *x = va_arg(*a_list, double);
}

static LObject *doubleparse(Lake *lake)
{
  /* parse the next thing from the lake */
  LObject *obj = LSexpr(lake);

  /* if it's a string or another numerical value, promote it to a
     double, otherwise leave it as it is */
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    char *cp = LSTRINGVAL(obj);
    char *end;
    double val = strtod(cp, &end);
    /* Allow a conversion only if the entire string is a double;
     * otherwise reject it. There is no point in converting 1.4foobar
     * to a double.
     */
    if (((size_t)end - (size_t)cp) == strlen(cp)) {
      free(LSTRINGVAL(obj));
      obj->type = LDOUBLE;
      obj->cell.d = val;
    }
  } else if (obj->type == LINT) {
    obj->type = LDOUBLE;
    obj->cell.f = LINTVAL(obj);
  } else if (obj->type == LLONG) {
    obj->type = LDOUBLE;
    obj->cell.f = LLONGVAL(obj);
  } else if (obj->type == LFLOAT) {
    obj->type = LDOUBLE;
    obj->cell.f = LFLOATVAL(obj);
  }
  return obj;
}

LType LDoublep = {
  "double",
  sizeof(double),
  doublefromobj,
  double2obj,
  doublefree,
  doublewrite,
  doublematch,
  doublepull,
  doubleparse,
  LTypeMagic
};

/*
 * string object implementation
 */


static bool stringfromobj(LObject *obj, char **x)
{
  if (obj->type == LSTRING || obj->type == LSYMBOL) {
    *x = LSTRINGVAL(obj);
  } else {
    return false;
  }
  return true;
}

static LObject *string2obj(char **x)
{
  char *copy = *x ? strdup(*x) : NULL;
  return LNew( LSTRING, &copy );
}

static void stringfree(char **x)
{
  if (*x) free(*x);
}

static bool stringmatch(char **a, char **b)
{
  if (!*a) return *b==NULL;
  if (!*b) return *a==NULL;
  return strcmp(*a,*b)==0 ;
}

static void stringwrite(FILE *fp, char **x)
{
  fprintf(fp, "\"%s\"", *x);
}

static void stringpull(va_list *a_list, char **x)
{
  *x = va_arg(*a_list, char *);
}

static LObject *stringparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);

#if 0
  /* Don't otherwise unquoted strings will not be bound to argument
   * valus; or I would have to edit all LDEFUN() invocations.
   */
  if (obj->type == LSYMBOL) {
    obj->type = LSTRING;
  }
#endif
  return obj;
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
  stringparse,
  LTypeMagic
};

/*
 * Symbol object implementation. We treat symbols and strings
 * interchangeably, with the exception that symbols can bind to
 * values, but strings cannot. This makes a difference during the
 * evaluation of lambda expressions.
 */

static bool symbolfromobj(LObject *obj, char **x)
{
  if (obj->type == LSYMBOL) {
    *x = LSYMBOLVAL(obj);
  } else {
    return false;
  }
  return true;
}

static LObject *symbol2obj(char **x)
{
  char *copy = *x ? strdup(*x) : NULL;
  return LNew(LSYMBOL, &copy);
}

static void symbolwrite(FILE *fp, char **x)
{
  fprintf(fp, "%s", *x);
}

static LObject *symbolparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);

  /* Mmmh. Shouldn't it be the otherway round: somebody requesting an
   * LSYMBOL in the input stream would rather expect to get the symbol
   * value; so maybe we should promote from LSYMBOL to LSTRING
   * here. Maybe not. Leave it as is for the moment.
   */
  if (obj->type == LSTRING) {
    obj->type = LSYMBOL;
  }
  return obj;
}

LType LSymbolp = {
  "symbol",
  sizeof(char *),
  symbolfromobj,
  symbol2obj,
  stringfree,
  symbolwrite,
  stringmatch,
  stringpull,
  symbolparse,
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

  if (!list) {
    return NULL;
  }
  new = LListNew();
  if (list->car) {
    new->car = LCopy(list->car);
  } else {
    new->car = NULL;
  }
  new->cdr = LListCopy(list->cdr);
  return new;
}

LList *LListShallowCopy(LList *list)
{
  LList *new;

  if (!list) {
    return NULL;
  }
  new = LListNew();
  if (list->car) {
    new->car = LRefIncr(list->car);
  } else {
    new->car = NULL;
  }
  new->cdr = LListShallowCopy(list->cdr);
  return new;
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
  if (list == NULL) {
    fprintf(fp, "nil");
  } else {
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
}

/**********************************************************************/

bool listfromobj(LObject *obj, LList **x)
{
  if (obj->type != LLIST) {
    return false;
  }
  *x = LLISTVAL(obj);
  return true;
}

LObject *list2obj(LList **x)
{
  LList *list = *x ? LListCopy(*x) : NULL;
  return LNew( LLIST, &list );
}

void listfree(LList **x)
{
  if (*x) {
    LListFree(*x);
  }
}

bool listmatch(LList **a, LList **b)
{
  return *a == *b;
}

void listwrite(FILE *fp, LList **x)
{
  LListWrite(fp, *x);
}

void listpull(va_list *a_list, LList **x)
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

bool objfromobj(LObject *obj, LObject **x)
{
  *x = obj;
  return true;
}

LObject *obj2obj(LObject **x)
{
  if (*x) LRefIncr(*x);
  return *x;
}

void objpull(va_list *a_list, LObject **x)
{
  *x = va_arg(*a_list, LObject *);
}

bool objmatch(LObject **a, LObject **b)
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
  lake->streamout = streamout ? streamout : stdout;
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


static bool lakefromobj(LObject *obj, Lake **x)
{
  if (obj->type == LLAKE) {
    *x = LLAKEVAL(obj);
    return true;
  }
  return false;
}

static LObject *lake2obj(Lake **x)
{
  return LNew( LLAKE, x );
}

static void lakefree(Lake **x)
{
  (void)x;
}

static void lakewrite(FILE *fp, Lake **x)
{
  (void)x;
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

bool funcfromobj(LObject *obj, int *x)
{
  if (obj->type == LSYMBOL) {
    *x = funcindex(LSYMBOLVAL(obj));
    if (*x == REJECT) return false;
  } else if (obj->type == LFUNC) {
    *x = LFUNCVAL(obj);
  } else if (obj->type == LLIST) {
    if (lambdafromobj(obj, NULL, NULL)) {
      *x = funcindex("\a\bEvalLambda");
    }
  } else {
    return false;
  }
  return true;
}

LObject *func2obj(int *x)
{
  return LNew( LFUNC, x );
}

void funcfree(int *x)
{
  (void)x;
}

bool funcmatch(int *a, int *b)
{
  return *a == *b;
}

void funcwrite(FILE *fp, int *x)
{
  fprintf(fp, "%s", functable[*x].name);
}

void funcpull(va_list *a_list, int *x)
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

static inline LObject *FUNCTOOBJ(const char *name)
{
  int idx = funcindex(name);

  if (idx < 0) {
    return Lnil;
  }
  return func2obj(&idx);
}

/**********************************************************************/

void LInit()
{
  /* Function name-space */
  VVINIT(funcvvec, LFunction, 256);
  func_fsa = fsa_initialize( NULL, (void*)REJECT );

  /* estabilish the global setq name-space */
  VVINIT(setq_namespace->table, LObject *, 256);
  setq_namespace->parser = fsa_initialize(NULL, (void *)REJECT);
  lambda_namespace = setq_namespace;

  {
    extern LObject *Lhelp(Lake *lake, LList *args);
    extern LObject *Lmorehelp(Lake *lake, LList *args);
    LDefun("?", Lhelp,
	   "(?  [command])"
	   "Command may include \"*\"s as wildcards; see also \"??\". "
	   "One-line command help; lists names only if multiple commands match."
	   "? is a synonym for \"help\".");
    LDefun("??", Lmorehelp,
	   "(?? command)\n"
	   "\"command\" may include \"*\" wildcards"
	   "Prints more info than \"(? command)\".  ?? is a synonym "
	   "for \"morehelp\".");
  }

  clisp_init();

  LHelpDef("STATEMENT",
	   "\nSTATEMENT represents a function call.  Function calls have "
	   "the form \"( func arg1 arg2 ... )\", where func is the name "
	   "of the function and arg1, arg2, ... are the arguments.");

}
  
LDEFINE(quote, LLOBJECT,
	"(quote EXPR)\n"
	"returns the symbolic lisp expression EXPR without evaluating it.")
{
  LObject *arg;

  LDECLARE(("quote", LBEGIN,
	    LHOLD, LLOBJECT, &arg,
	    LEND));
  LRefIncr(arg);
  return arg;
}

/* The purpose of the function below is to allow the evaluation of
 * quoted lists, or lists constructed via cons, car, cdr, or the &rest
 * argument of defun: if the car of a list is a symbol and not a
 * function call, then it is first replaced by a matching function
 * call if possible, and a lake argument is added as second arg.
 */
static void LListSexpr(LList *expr, Lake *lake)
{
  LList *subexpr, *cdr = expr->cdr;
  int fidx;
  
  if (expr->car && expr->car->type != LFUNC && funcfromobj(expr->car, &fidx)) {
    /* try to convert into a function name and add a lake argument. */
    LList *lakenode = LListNew();
    
    lakenode->cdr = cdr;
    lakenode->car = lake2obj(&lake);

    if (expr->car->type == LSYMBOL) {
      /* Builtin function or defun */
      LFree(expr->car);
      expr->car = LNew(LFUNC, &fidx);
      expr->cdr = lakenode;
    } else {
      /* anonymous lambda expression */
      expr->cdr = LListNew();
      expr->cdr->car = expr->car;
      expr->car = LNew(LFUNC, &fidx);
      expr->cdr->cdr = lakenode;
    }
  }
  while (cdr) {
    if (expr->car) {
      if (expr->car->type == LLAKE) {
	expr->car->cell.p = lake;
      } else if (listfromobj(expr->car, &subexpr)) {
	LListSexpr(subexpr, lake);
      }
    }
    cdr = cdr->cdr;
  }
}

LDEFINE(eval, LLOBJECT,
	"(eval EXPR)\n"
	"Evaluate a lisp expression. If EXPR is an unevaluated S-expression "
	"as returned by the \"(quote ...)\" command then the effect will be "
	"as if calling the un-quoted expression directly.")
{
  LObject *arg;
  LList *sexpr;
  Lake *caller;

  LDECLARE(("eval", LBEGIN,
	    LLAKE, &caller,
	    LLOBJECT, &arg,
	    LEND));

  if (listfromobj(arg, &sexpr)) {
    LListSexpr(sexpr, caller);
  }

  return LEval(arg);
}

LDEFINE(lambda, LLOBJECT,
	"(lambda (arg1 arg2 ...) EXPR1 ... EXPRN)\n"
	"A lambda expression is like a function. "
	"To \"call\" a lambda expression, it has to be evoked like a "
	"function:\n\n"
	"((lambda (arg) (+ 1 arg)) 2)\n\n"
	"In this example, the value of the entire expression would be 3. "
	"In general, the value of the call will be the value of exprN. "
	"The first list serves to define formal parameters. The values "
	"of the formal parameters can be changed using `(setq ...)'. The "
	"binding will only be in effect during the evaluation of the "
	"lambda-expression. The lambda "
	"expression itself is just a list, starting with the key-word lambda, "
	"followed by several quoted lists; it evaluates to itself if not "
	"called as a function.")
{
  LObject *lambda;
  LList *arglist;
  LList *body;
  
  LDECLARE(("lambda", LBEGIN,
	    LLITERAL, LLIST, &arglist,
	    LHOLD, LREST, &body,
	    LEND));

  /* We avaluate to ourselves */
  lambda = list2obj(&args);
  return lambda;
}

LDEFINE(let, LLOBJECT,
	"(let ARGUMENTS EXPR1 ... EXPRN)\n"
	"Generate a lambda expression from EXRP1 ... EXPRN, with the "
	"argument bindings described by ARGUMENTS. ARGUMENTS is a list "
	"of symbols (bound to nil by default) or lists of the form "
	"`(ARG VALUE)' where ARG is a symbol and not evaluated and VALUE "
	"is a S-expr which is first evaluated, then its value is bound to "
	"ARG. The entire expression evaluates to the value of EXPRN, the "
	"last expression in the body of the statement. The argument list "
	"must be present, but can be empty; in the latter case the "
	"`(let () ...)' statement is equivalent to a `(progn ...)'")
{
  if (!LPARSEMODE) {
    return LEvalLambda(NULL, args);
  } else {
    /* We cannot use LParseArgs() to parse the lake into ARGLIST,
     * otherwise the parameter names would be turned into function
     * calls, and in case of a matching function name the following
     * data would be parsed according to the rules of the function
     * which is not appropriate here. The strategy is to convert the
     * let-expression into an equivalent lambda-expression during
     * parsing.
     */
    LList *lambda;
    LList **arglistp;
    int quote;
    bool par;

    if (!LakeMore(lake) || !LakeNewSexpr(lake)) {
      OOGLSyntax(lake->streamin,
		 "Llet(): Reading \"%s\": missing parameter list",
		 LakeName(lake));
    }

    /* lambda-expression */
    lambda = LListNew();
    LListAppend(args, LNew(LLIST, &lambda));
    args = args->cdr;

    /* head of lambda expression */
    lambda->car = FUNCTOOBJ("lambda");
    /* lambda argument list */
    lambda->cdr = LListNew();
    lambda->cdr->car = LNew(LLIST, NULL);
    arglistp = (LList **)&lambda->cdr->car->cell.p;

    /* stash the lake as first argument value */
    LListAppend(args, lake2obj(&lake));
    args = args->cdr;

    /* argument values then go to args->cdr, if present */

    LakeNextToken(lake, &quote); /* consume '(' */
    while (LakeMore(lake)) {
      LObject *larg, *lval;
      char *arg;
      
      if ((par = LakeNewSexpr(lake))) {
	/* (ARG VALUE) */
	LakeNextToken(lake, &quote); /* consume '(' */
      }
      /* fetch the argument name */
      if ((larg = LLiteral(lake)) == Lnil || !symbolfromobj(larg, &arg)) {
	OOGLSyntax(lake->streamin,
		   "Llet(): Reading \"%s\": "
		   "argument name missing or not a symbol (`%s'?!)",
		   LakeName(lake), LSummarize(larg));
	goto parsebad;
      }

      if (par) {
	/* fetch the argument value */
	lval = LSexpr(lake);
	if (LakeMore(lake)) {
	  OOGLSyntax(lake->streamin,
		     "Llet(): Reading \"%s\": "
		     "excess data in argument definition",
		     LakeName(lake));
	  LFree(lval);
	  goto parsebad;
	}
	LakeNextToken(lake, &quote); /* consume ')' */
      } else {
	lval = Lnil;
      }
      
      /* put the argument name into the argument list of the lambda
       * expression
       */
      *arglistp = LListNew();
      (*arglistp)->car = larg;
      arglistp = &(*arglistp)->cdr;

      /* put the argument value into the value list of the lambda
       * function call, i.e. just append to args.
       */
      LListAppend(args, lval);
      args = args->cdr;
    }
    LakeNextToken(lake, &quote); /* consume ')' */

    /* suck the body of the let statement in */
    args = lambda->cdr;
    LDECLARE(("let", LBEGIN,
	      LHOLD, LREST, NULL,
	      LEND));

    return Lt; /* not reached */
  parsebad:
    /* Consume the remainder of the expression */
    while (LakeMore(lake)) {
      LFree(LSexpr(lake));
    }
    LakeNextToken(lake, &quote); /* consume ')' */
    if (par) {
      par = false;
      goto parsebad;
    }
    /* this was for the argument list, now go for the body */
    while (LakeMore(lake)) {
      LFree(LSexpr(lake));
    }
    return Lnil;
  }
}

LDEFINE(defun, LLOBJECT,
	"(defun NAME (ARG1 ...) [DOCSTRING] EXPR1 ...)\n"
	"Define a named lambda-expression, that is: define NAME to evaluate "
	"to the lambda-expression \"(lambda (ARG1...) (EXPR1...))\" when "
	"called as a function. Also, install DOCSTRING as response to the "
	"commands \"(help NAME)\" and \"(morehelp NAME)\". Not that DOCSTRING "
	"need not contain the command-synopsis, it is generated automatically. "
	"EXPR1 cannot be a string if DOCSTRING is omitted; it "
	"would be interpreted as the doc-string. The return value of "
	"(defun ...) is the function name.")
{
  char *name;
  char *helpstring = NULL;
  LList *arglist, *body, *arg;
  LObject *lambda;
  int fidx, nargs, helpsize;
  char *help, *argname;

  /* Not pretty standard: in order that our Sexpr0() uses the correct
   * function for parsing the arguments of the defun we have to add a
   * dummy function entry right at the start. That entry points to
   * EvalDefun(), which makes sure we have a LAKE and the remaining
   * arguments as LREST list.
   */

  if (!LPARSEMODE) {
    LDECLARE(("defun", LBEGIN,
	      LSYMBOL, &name,
	      LLITERAL, LLIST, &arglist,
	      LHOLD, LREST, &body,
	      LEND));
  } else {
    LObject *lname;
    char *name;
    
    /* Dequeue the function name */
    if (!LakeMore(lake) ||
	(lname = LLiteral(lake)) == Lnil || !symbolfromobj(lname, &name)) {
      OOGLSyntax(lake->streamin,
		 "Ldefun(): Reading \"%s\": missing function name",
		 LakeName(lake));
      return Lnil;
    }

    /* Append the function name to the argument list */
    LListAppend(args, lname);

    /* Bind it. Error recovery is flakey here, because we bind the new
     * name even though we do not know yet whether the following
     * S-expr is syntactically correct.
     */
    LDefun(name, LEvalDefun, NULL);

    /* Then proceed with normal argument parsing; this will already
     * use the new function binding and the parsing rules of
     * LEvalDefun().
     */
    LDECLARE(("defun", LBEGIN,
	      LLITERAL, LLIST, &arglist,
	      LHOLD, LREST, &body,
	      LEND));
  }

  /* evaluation mode */

  /* Fetch the index into the jump table */
  if ((fidx = funcindex(name)) < 0) {
    OOGLError(0, "Ldefun(%s): Error: LDefun(%s) failed.", name, name);
    return Lnil;
  }

  /* Extract doc-string and body */
  if (body && LFROMOBJ(LSTRING)(body->car, &helpstring)) {
    body = body->cdr;
  }

  /* Construct a suitable help-string. The heading is auto-generated. */
  if (helpstring == NULL) {
    helpstring = "Undocumented lisp-function.";
  }
  helpsize = strlen(helpstring) + strlen(name) + strlen("()\n");
  for (nargs = 0, arg = arglist; arg && arg->car; arg = arg->cdr, ++nargs) {
    if (!LFROMOBJ(LSYMBOL)(arg->car, &argname)) {
      OOGLError(0, "Ldefun(%s): Error: "
		"argument name -- %s -- is not a symbol (an unquoted token).",
		name, LSummarize(arg->car));
      return Lnil;
    }
    helpsize += strlen(argname);
  }
  helpsize += nargs; /* spaces */
  
  help = malloc(helpsize+1+ /* safeguard */ 10);
  
  switch (nargs) {
  case 0:
    sprintf(help, "(%s)\n%s", name, helpstring);
    break;
  case 1:
    LFROMOBJ(LSYMBOL)(arglist->car, &argname);
    sprintf(help, "(%s %s)\n%s", name, argname, helpstring);
    break;
  default:
    helpsize = sprintf(help, "(%s", name);
    for (arg = arglist; arg; arg = arg->cdr) {
      LFROMOBJ(LSYMBOL)(arg->car, &argname);
      helpsize += sprintf(help + helpsize, " %s", argname);
    }
    strcpy(help + helpsize, ")\n"); helpsize += 2;
    strcpy(help + helpsize, helpstring);
    break;
  }

  /* Defining a function should not really be timing critical, so we
   * use the slow C-interface here:
   */
  lambda = LEvalFunc("lambda", LLIST, arglist, LREST, body, LEND);
  if (lambda == Lnil) {
    free(help);
    OOGLError(0, "Ldefun(%s): Error: could not generate lambda-expression.",
	      name);
    return Lnil;
  }
  functable[fidx].lambda = lambda;
  functable[fidx].help = help;

  LHelpDef(functable[fidx].name, functable[fidx].help);

  return LTOOBJ(LSYMBOL)(&name);
}

LDEFINE(setq, LLOBJECT,
	"(setq SYM SEXPR)\n"
	"Bind the symbold SYM to the value of SEXPR. SYM must be an "
	"unqualified symbol, i.e. not quoted, and literal:\n\n"
	"(setq \"foo\" bar)\n\n"
	"will not work. Likewise\n\n"
	"(setq (bar STUFF) foo)\n\n"
	"will also not work, even if (bar ...) would evaluate to an "
	"unqualified symbol: varible names must be literals. "
	"Note that calling `(setq SYM ...)' will alter the value of "
	"SYM within the current name-space: if SYM, e.g., is bound as "
	"local variable by a lambda, let or defun expression, then "
	"`(setq SYM ...)' will change the value of the local variable, "
	"the global binding will remain unchanged. "
	"It is NOT possible to un-bind a symbol. However, subsequent "
	"`(setq SYM ...)' invocations will re-bind SYM to another value "
	"and free the lisp-object previously bound to SYM.")
{
  Lake *caller;
  LObject *sym, *val;

  LDECLARE(("setq", LBEGIN,
	    LLAKE, &caller,
	    LLITERAL, LLOBJECT, &sym,
	    LLOBJECT, &val,
	    LEND));
  
  if (sym->type != LSYMBOL) {
    OOGLSyntax(caller->streamin,
	       "Lsetq(): Reading \"%s\": "
	       "trying to bind symbol(?) `%s': "
	       "variable names need to be literals (unquoted atoms)",
	       LakeName(caller), LSummarize(sym));
    return Lnil;
  }

  namespace_put(lambda_namespace, LSYMBOLVAL(sym), val, true);
  
  return LRefIncr(val);
}

/* A (while ...) statement is very imported: although in principle a
 * loop can be emulated by a recursion, such thing as _wanted_
 * infinite loops can only be constructed by a real loop-statemnt like
 * this. Actually, one would want to be able to enter Geomview's
 * main-loop from inside a lisp script. This, however, is not our
 * concern. This module does not need to pay attention to that stuff.
 */
LDEFINE(while, LVOID,
	"(while TEST BODY)\n"
	"Iterate: \"evaluate TEST, if non nil, evaluate BODY\".")
{
  LObject *test, *body, *val, *cp;
  
  LDECLARE(("while", LBEGIN,
	    LHOLD, LLOBJECT, &test,
	    LHOLD, LLOBJECT, &body,
	    LEND));
  while ((val = LEval(cp = LCopy(test))) != Lnil) {
    LFree(val);
    LFree(cp);
    val = LEval(cp = LCopy(body));
    LFree(val);
    LFree(cp);
  }
  LFree(val);
  LFree(cp);
  return Lt;
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

LDEFINE(and, LLOBJECT,
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

LDEFINE(greater, LLOBJECT,
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

/* Note: comparison is promoted to the weakest numerical type, with
 * the ordering LINT < LLONG < LFLOAT < LSTRING.
 *
 * The return value is ((expr1 > expr2) - (expr1 < expr2))
 */
static int LCompare(const char *name, LObject *expr1, LObject *expr2)
{
  char   *s1, *s2;
  double d1, d2;
  long   l1, l2;
  int    i1, i2;
  
  if (LFROMOBJ(LINT)(expr1, &i1) && LFROMOBJ(LINT)(expr2, &i2)) {
    return (i1 > i2) - (i1 < i2);
  }
  if (LFROMOBJ(LLONG)(expr1, &l1) && LFROMOBJ(LLONG)(expr2, &l2)) {
    return (l1 > l2) - (l1 < l2);
  }
  if (LFROMOBJ(LDOUBLE)(expr1, &d1) && LFROMOBJ(LDOUBLE)(expr2, &d2)) {
    return (d1 > d2) - (d1 < d2);
  }
  if (LFROMOBJ(LSTRING)(expr1, &s1) && LFROMOBJ(LSTRING)(expr2, &s2)) {
    return strcmp(s1, s2);
  }
  OOGLError(0, "%s: arg1 and arg2 must at least be strings to be compared.",
	    name);
  return 0;
}

LDEFINE(add, LLOBJECT,
	"(+ EXPR1 EXPR2)\n"
	"Adds EXPR1 to EXPR2 if both convert to a numerical value.")
{
  LObject *expr1, *expr2;
  double d1, d2;
  long   l1, l2;
  int    i1, i2;

  LDECLARE(("+", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  
  if (LFROMOBJ(LINT)(expr1, &i1) && LFROMOBJ(LINT)(expr2, &i2)) {
    return LINTTOOBJ(i1 + i2);
  }
  if (LFROMOBJ(LLONG)(expr1, &l1) && LFROMOBJ(LLONG)(expr2, &l2)) {
    return LLONGTOOBJ(l1 + l2);
  }
  if (LFROMOBJ(LDOUBLE)(expr1, &d1) && LFROMOBJ(LDOUBLE)(expr2, &d2)) {
    return LDOUBLETOOBJ(d1 + d2);
  }
  OOGLError(0, "\"+\": ARG1 and ARG2 must be numerical values.");
  OOGLError(0, "\"+\": ARG1: %s", LSummarize(expr1));
  OOGLError(0, "\"+\": ARG2: %s", LSummarize(expr2));
  return Lnil;
}

LDEFINE(substract, LLOBJECT,
	"(- EXPR1 [EXPR2])\n"
	"Substracts EXPR1 from EXPR2 if both convert to a numerical value. "
	"If EXPR2 is omitted negate EXPR1 if it converts to a numerical value.")
{
  LObject *expr1, *expr2 = NULL;
  double d1, d2;
  long   l1, l2;
  int    i1, i2;

  LDECLARE(("-", LBEGIN,
	    LLOBJECT, &expr1,
	    LOPTIONAL, LLOBJECT, &expr2,
	    LEND));

  if (expr2) {
    if (LFROMOBJ(LINT)(expr1, &i1) && LFROMOBJ(LINT)(expr2, &i2)) {
      return LINTTOOBJ(i1 - i2);
    }
    if (LFROMOBJ(LLONG)(expr1, &l1) && LFROMOBJ(LLONG)(expr2, &l2)) {
      return LLONGTOOBJ(l1 - l2);
    }
    if (LFROMOBJ(LDOUBLE)(expr1, &d1) && LFROMOBJ(LDOUBLE)(expr2, &d2)) {
      return LDOUBLETOOBJ(d1 - d2);
    }
    OOGLError(0, "\"-\": ARG1 and ARG2 must be numerical values.");
    OOGLError(0, "\"-\": ARG1: %s", LSummarize(expr1));
    OOGLError(0, "\"-\": ARG2: %s", LSummarize(expr2));
  } else {
    if (LFROMOBJ(LINT)(expr1, &i1)) {
      if (i1 == 0 || -i1 != i1) {
	return LINTTOOBJ(-i1);
      }
    }
    if (LFROMOBJ(LLONG)(expr1, &l1)) {
      if (l1 == 0 || -l1 != l1) {
	return LLONGTOOBJ(-l1);
      }
    }
    if (LFROMOBJ(LDOUBLE)(expr1, &d1)) {
      return LDOUBLETOOBJ(-d1);
    }
    OOGLError(0, "\"-\": ARG %s  must be a numerical value.",
	      LSummarize(expr1));
  }
  return Lnil;
}

LDEFINE(multiply, LLOBJECT,
	"(* EXPR1 EXPR2)\n"
	"Multiplies EXPR1 with EXPR2 if both convert to a numerical value.")
{
  LObject *expr1, *expr2;
  double d1, d2;
  long   l1, l2;
  int    i1, i2;

  LDECLARE(("*", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  
  if (LFROMOBJ(LINT)(expr1, &i1) && LFROMOBJ(LINT)(expr2, &i2)) {
    return LLONGTOOBJ((long)i1 * (long)i2);
  }
  if (LFROMOBJ(LLONG)(expr1, &l1) && LFROMOBJ(LLONG)(expr2, &l2)) {
    if ((l2 == 0 || l1 * l2 / l2 == l1) ||
	(l1 == 0 || l2 * l1 / l1 == l2)) {
      return LLONGTOOBJ(l1 * l2);
    }
  }
  if (LFROMOBJ(LDOUBLE)(expr1, &d1) && LFROMOBJ(LDOUBLE)(expr2, &d2)) {
    return LDOUBLETOOBJ(d1 * d2);
  }
  OOGLError(0, "\"*\": ARG1 and ARG2 must be numerical values.");
  OOGLError(0, "\"*\": ARG1: %s", LSummarize(expr1));
  OOGLError(0, "\"*\": ARG2: %s", LSummarize(expr2));
  return Lnil;
}

LDEFINE(divide, LLOBJECT,
	"(/ EXPR1 EXPR2)\n"
	"Divides EXPR1 by EXPR2 if both convert to a numerical value.")
{
  LObject *expr1, *expr2;
  double d1, d2;
  LDECLARE(("/", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  
  if (LFROMOBJ(LDOUBLE)(expr1, &d1) && LFROMOBJ(LDOUBLE)(expr2, &d2)) {
    return LDOUBLETOOBJ(d1 / d2);
  }
  OOGLError(0, "\"/\": ARG1 and ARG2 must be numerical values.");
  OOGLError(0, "\"/\": ARG1: %s", LSummarize(expr1));
  OOGLError(0, "\"/\": ARG2: %s", LSummarize(expr2));
  return Lnil;
}

LDEFINE(remainder, LLOBJECT,
	"(mod EXPR1 EXPR2)\n"
	"Divides EXPR1 by EXPR2 which must be integers and "
	"returns the remainder.")
{
  LObject *expr1, *expr2;
  long   l1, l2;
  int    i1, i2;

  LDECLARE(("mod", LBEGIN,
	    LLOBJECT, &expr1,
	    LLOBJECT, &expr2,
	    LEND));
  
  if (LFROMOBJ(LINT)(expr1, &i1) && LFROMOBJ(LINT)(expr2, &i2)) {
    return LINTTOOBJ(i1 % i2);
  }
  if (LFROMOBJ(LLONG)(expr1, &l1) && LFROMOBJ(LLONG)(expr2, &l2)) {
    return LLONGTOOBJ(l1 % l2);
  }
  OOGLError(0, "\"mod\": ARG1 and ARG2 must be integer values.");
  OOGLError(0, "\"mod\": ARG1: %s", LSummarize(expr1));
  OOGLError(0, "\"mod\": ARG2: %s", LSummarize(expr2));
  return Lnil;
}

LDEFINE(truncate, LLOBJECT,
	"(truncate EXPR\n"
	"Truncates EXPR which must convert to a numerical value, that is, "
	"round EXPR towards zero to an integral value.")
{
  LObject *expr1;
  double d1;
  long   l1;
  int    i1;

  LDECLARE(("truncate", LBEGIN,
	    LLOBJECT, &expr1,
	    LEND));

  if (LFROMOBJ(LINT)(expr1, &i1)) {
    return LINTTOOBJ(i1);
  }
  if (LFROMOBJ(LLONG)(expr1, &l1)) {
    return LLONGTOOBJ(l1);
  }
  if (LFROMOBJ(LDOUBLE)(expr1, &d1)) {
    return LLONGTOOBJ((long)d1);
  }
  OOGLError(0, "\"-\": ARG %s  must be a numerical value.",
	    LSummarize(expr1));
  return Lnil;
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
  LListWrite(stderr, list);
}

const char *LListSummarize(LList *list) 
{
  const char *msg;
  LObject *lobj;
  
  lobj = LNew(LLIST, &list);
  msg = LSummarize(lobj);
  lobj->cell.p = NULL;
  LFree(lobj);
  
  return msg;
}

/*
 * Lisp object implementation
 */
static DEF_FREELIST(LObject);

LObject *LNew(LType *type, void *vcell)
{
  LCell *cell = (LCell *)vcell;
  LObject *obj;

  FREELIST_NEW(LObject, obj);

  obj->type = type;
  obj->ref = 1;
  if (!cell) {
    memset(&obj->cell, 0, sizeof(obj->cell));
  } else {
    memcpy(&obj->cell, cell, LSIZE(type));
  }
  return obj;
}

void LWrite(FILE *fp, LObject *obj)
{
  (*obj->type->write)(fp, &(obj->cell));
}

void LWriteFile(const char *fname, LObject *obj)
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

void _LFree(LObject *obj)
{
  (*obj->type->free)(&(obj->cell));
  FREELIST_FREE(LObject, obj);
}

#if 0 /* now as inline functions in lisp.h */
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
#endif

/* lambda-expression argument name-space handling */

/* push a new namespace */
static inline void namespace_push(LNameSpace **ns, LNameSpace *new_ns)
{
  if (new_ns) {
    new_ns->parser = fsa_initialize(NULL, (void *)REJECT);
    VVINIT(new_ns->table, LObject *, 8);
    new_ns->next = *ns;
    *ns = new_ns;
  }
}

/* pop the current name-space and destroy it */
static inline void namespace_pop(LNameSpace **ns)
{
  int i;
  
  if (*ns) {
    for (i = 0; i < VVCOUNT((*ns)->table); i++) {
      LFree(VVEC((*ns)->table, LObject *)[i]);
    }
    vvfree(&(*ns)->table);
    fsa_delete((*ns)->parser);
  }
  
  *ns = (*ns)->next;
}

static inline LObject **_namespace_get(LNameSpace *ns, char *name) 
{
  int idx = REJECT;
  
  idx = (int)(long)fsa_parse(ns->parser, name);
  if (idx == REJECT) {
    return NULL;
  }
  return VVEC(ns->table, LObject *)+idx;
}

static inline LObject *namespace_get(LNameSpace *ns, char *name) 
{
  LObject **obj;
  
  if (ns == NULL) {
    return NULL;
  }

  do {
    obj = _namespace_get(ns, name);
  } while (obj == NULL && (ns = ns->next) != NULL);

  return obj ? LRefIncr(*obj) : NULL;
}

static inline bool
namespace_put(LNameSpace *ns, char *name, LObject *value, bool overwrite)
{
  int idx;
  LObject **var;

  if ((var = _namespace_get(ns, name)) != NULL) {
    if (!overwrite) {
      return false;
    }
    LFree(*var);
  } else { /* create a new entry */
    idx = VVCOUNT(ns->table);
    var = VVAPPEND(ns->table, LObject *);
    fsa_install(ns->parser, name, (void *)(long)idx);
  }
  *var = LRefIncr(value);
  return true;
}

/********************** end of argument name-space ****************************/

/* Extract body, parameter names and parameter values from a lambda
 * expression. Return false if LAMBDA is not a lambda expression.
 *
 * args and body maybe NULL in which case only the checking is done.
 */
static inline bool lambdafromobj(LObject *lambda, LList **args, LList **body)
{
  int idx;
  LList *llist;

  if (lambda == NULL || !LFROMOBJ(LLIST)(lambda, &llist)) {
    return false; /* lambda expressions are lists */
  }
  if (!funcfromobj(llist->car, &idx) || functable[idx].fptr != Llambda) {
    return false; /* not a lambda expression */
  }
  if (llist->cdr == NULL ||
      llist->cdr->car == NULL ||
      llist->cdr->car->type != LLIST) {
    return false; /* parameters must be stored in a list (and there
		   * must be a parameter list, at least an empty
		   * one) */
  }

  if (args) {
    LFROMOBJ(LLIST)(llist->cdr->car, args);
  }

  /* Remaining stuff is generic, further checking is done when
   * assigning the parameter values.
   */
  if (body) {
    *body = llist->cdr->cdr; /* body is the tail of the entire list */
  }

  return true;
}

/* Bind the values given in ARGVALS to the names given in ARGS within
 * the name-space defined by ARGNS. LAKE and CALL are only used to
 * report syntax errors.
 */
static inline bool BindLambdaParameters(Lake *lake, LList *call,
					LNameSpace *argns,
					LList *args, LList *argvals)
{
  LObject *lval;
  bool rest = false, optional = false;
  int ngot = 0;
  int nreq = 0;
  int nargs = 0;
  
  for (; args; args = args->cdr) {
    char *argname;
    
    lval = NULL;
    if (!LFROMOBJ(LSYMBOL)(args->car, &argname)) {
      char *sumcall = strdup(LListSummarize(call));
      OOGLSyntax(lake->streamin,
		 "LParseLambda: Reading \"%s\": parsing \"%s\": "
		 "parameter name `%s' is not a symbol (unquoted atom)",
		 LakeName(lake), sumcall, LSummarize(args->car));
      free(sumcall);
      goto errorout;
    }
    if (strcmp("&rest", argname) == 0) {
      optional = rest = true; /* rest implies optional */
      continue;
    } else if (strcmp("&optional", argname) == 0) {
      optional = true;
      continue;
    }
    ++nargs;
    nreq += !optional;
    if (rest) {
      lval = argvals ? LLISTTOOBJ(argvals) : Lnil;
    } else {
      lval = argvals ? LRefIncr(argvals->car) : Lnil;
    }
    if (!namespace_put(argns, argname, lval, false)) {
      OOGLSyntax(lake->streamin,
		 "BindLambdaParameters: Reading \"%s\": parsing \"%s\": "
		 "duplicate parameter name `%s'",
		 LakeName(lake), LListSummarize(call), argname);
      goto errorout;
    }
    ngot += argvals != NULL;
    LFree(lval);
    if (rest) {
      args = args->cdr;
      break;
    }
    if (argvals) {
      argvals = argvals->cdr;
    }
  }

  /* Error checking */
  if (rest && args) {
    char *sumcall = strdup(LListSummarize(call));
    OOGLSyntax(lake->streamin,
	       "BindLambdaParameters: Reading \"%s\": parsing \"%s\": "
	       "excess argument names after `&rest' parameter: `%s'",
	       LakeName(lake), sumcall, LListSummarize(args));
    free(sumcall);
    goto errorout;
  } else if (ngot < nreq) {
    OOGLSyntax(lake->streamin,
	       "BindLambdaParameters: Reading \"%s\": parsing \"%s\": "
	       "missing parameter values",
	       LakeName(lake), LListSummarize(call));
    goto errorout;
  } else if (ngot > nargs) {
    char *sumcall = strdup(LListSummarize(call));
    OOGLSyntax(lake->streamin,
	       "BindLambdaParameters: Reading \"%s\": parsing \"%s\": "
	       "excess parameter values: `%s'",
	       LakeName(lake), sumcall, LListSummarize(argvals));
    free(sumcall);
    goto errorout;
  }
  return true;

 errorout:
  LFree(lval);
  return false;
}

/* Copy the body of a lambda expression and substitute LAKE for each
 * lake argument. This is necessary because (setq ...) can operate on a
 * lambda-expression, and the lambda-expression could also stem from a
 * (defun ...). In both case the lake arguments stored in the original
 * body are out of date.
 */
static LList *LBody(LList *lbody, Lake *lake)
{
  LList *body;

  if (!lbody) {
    return NULL;
  }
  body = LListNew();
  if (lbody->car) {
    body->car = LCopy(lbody->car);
    if (body->car->type == LLAKE) {
      body->car->cell.p = lake;
    }
  }
  body->cdr = LBody(lbody->cdr, lake);
  return body;
}

/* Evaluate a lambda-expression or a defun; convert an anonymous
 * lambda-expression into a progn, convert a defun into the named
 * function. This function has a special calling convention during
 * parse-mode: it must be called like
 *
 * (\a\bEvalLambda (lambda ...) ...)
 *
 * That is, the argument list already contains the lambda expression
 * as first argument.
 */
LDEFINE(EvalLambda, LLOBJECT,
	"\a\b(EvalLambda (lambda ...) (args))\n"
	"Evaluate the given lambda-expression with the given arguments. "
	"Internal use only. DO NOT USE THIS FUNCTION.")
{
  LNameSpace lambda_ns;
  Lake *caller;
  LList *argvals, *largs, *lbody;
  LObject *val, *lexpr, *body;

  if (!LPARSEMODE) {
    /* The first LDECLARE after LDEFINE wins, so make sure this is the
       definition suitable for l_EvalLambda(). */
    LDECLARE(("\a\bEvalLambda", LBEGIN,
	      LHOLD, LLOBJECT, &lexpr,
	      LLAKE, &caller,
	      LREST, &argvals,
	      LEND));
  } else {
    /* In parse mode there is no lambda expression, it is added by
     * LSexpr0() automatically.
     */
    LDECLARE(("\a\bEvalLambda", LBEGIN,
	      LLAKE, &caller,
	      LREST, &argvals,
	      LEND));
  }

  /* When we reach here we are in execution mode. */
  if (!lambdafromobj(lexpr, &largs, &lbody)) {
    return Lnil;
  }
  
  /* push a new name-space */
  namespace_push(&lambda_namespace, &lambda_ns);
  if (!BindLambdaParameters(caller, args->cdr, &lambda_ns, largs, argvals)) {
    namespace_pop(&lambda_namespace);
    return Lnil;
  }

  /* We have to copy the lambda-expression because assign args will
   * substitute the evaluated function calls into the argument lists.
   */
  body = LLISTTOOBJ(NULL);
  body->cell.p = LListNew();
  LLISTVAL(body)->car = FUNCTOOBJ("progn");
  LLISTVAL(body)->cdr = LBody(lbody, caller); /* copy with lake substitution */

  /* We can now simply return LEval() which will evaluate body in the
   * context of the given name-space; we use Lprogn() for this
   * purpose.
   */
  val = LEval(body);

  LFree(body);
  
  /* pop the saved name-space */
  namespace_pop(&lambda_namespace);

  return val;
}

/* Evaluate a (defun ...). This function is what is entered into the
 * function-table to evaluate a named lambda expression. The hard work
 * is done in LEvalLambda(), we simply call that function with the
 * lambda expression saved in the function table.
 *
 * To allow recursion we must be careful; the actual parsing of the
 * substituted lambda-expression must go to the evaluation pass. For
 * this purpose we must remember the lake as hidden argument.
 */
LDEFINE(EvalDefun, LLOBJECT,
	"(\a\bEvalDefun EXPR)\n"
	"Internal function which evaluates EXPR as a defun, i.e. a named "
	"lambda-expression. DO NOT USE THIS FUNCTION.")
{
  Lake *caller;
  LList *lambda, *argvals;
  LObject *val;
  int fidx;

  LDECLARE(("\a\bEvalDefun", LBEGIN,
	    LLAKE, &caller,
	    LREST, &argvals,
	    LEND));

  /* Forward everything to the eval-step of LEvalLambda(), then
   * evaluate the object returned by LEvalLambda() and return that
   * value to the caller.
   */
  if (!LFROMOBJ(LFUNC)(args->car, &fidx) || functable[fidx].lambda == NULL) {
    /* should not happen, but ... */
    return Lnil;
  }

  lambda = LListNew();
  lambda->car = LRefIncr(functable[fidx].lambda);
  lambda->cdr = args->cdr;
  args->cdr = lambda;

  /* Invoke LEvalLambda() in evaluation mode */
  val = LEvalLambda(NULL, args);

  args->cdr = lambda->cdr;
  lambda->cdr = NULL; /* avoid freeing the argument list */
  LListFree(lambda);
  
  return val;
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

static inline LObject *ParseArg(LType *type, Lake *lake)
{
  if (LakeNewSexpr(lake)) {
    /* ok, its a S-expr, do not invoke the type specific parser, but
     * parse it as a sexpr.
     */
    return LSexpr(lake);
  } else {
    /* otherwise invoke the type-specific parser */
    return LPARSE(type)(lake);
  }
}

/* LSexpr0() does the work of both LSexpr() and LLiteral();
   special says whether to interpret lists specially */
static inline LObject *LSexpr0(Lake *lake, int listhow)
{
  LObject *obj, *head;
  int i, quote;
  const char *tok;
  
  if ((tok = LakeNextToken(lake, &quote)) == NULL) {
    return Lnil;
  }
  if (quote == '\0' && *tok == '(') {
    obj = LNew(LLIST, NULL);
    if(listhow == LIST_LITERAL) {
      while (LakeMore(lake)) {
	obj->cell.p = (void*) LListAppend((LList*)(obj->cell.p),
					  LSexpr0(lake, LIST_LITERAL));
      }
    } else if (LakeMore(lake)) {
      /* if we have a non-empty list ... */
      /* ... get the first element and see if it's a function name */
      head = LEvalSexpr(lake);
      obj->cell.p = (void*) LListAppend(LLISTVAL(obj), head);
      if (funcfromobj(head, &i)) {
	/* It's a function name.  Enter the function as the first element
	   of our list, and then call the function in parse mode to
	   construct the rest of the list (arguments to the function) */
	if (head->type == LSYMBOL) {
	  /* Builtin function or defun */
	  LFree(head);
	  LLISTVAL(obj)->car = head = LNew(LFUNC, &i);
	} else {
	  /* anonymous lambda expression */
	  LLISTVAL(obj)->cdr = LListNew();
	  LLISTVAL(obj)->cdr->car = LLISTVAL(obj)->car;
	  LLISTVAL(obj)->car = head = LNew(LFUNC, &i);
	}

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
	  OOGLSyntax(lake->streamin,
		     "Reading \"%s\": call to unknown function \"%s\"",
		     LakeName(lake), LSummarize(head));
	while (LakeMore(lake)) {
	  obj->cell.p = (void*) LListAppend(LLISTVAL(obj),
					    LSexpr0(lake, listhow));
	}
      }
    }
    tok = LakeNextToken(lake, &quote);
  } else {
    obj = LNew(quote == '\0' ? LSYMBOL : LSTRING, NULL);
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

  /* Lists are function calls, symbols may have values bound to them,
   * everything else evaluates to itself.
   */
  if (obj->type == LSYMBOL) {
    LObject *val;

    val = namespace_get(lambda_namespace, LSYMBOLVAL(obj));
    if (val != NULL) {
      return val;
    }

    val = namespace_get(setq_namespace, LSYMBOLVAL(obj));
    if (val != NULL) {
      return val;
    }
  }

  if (obj->type != LLIST) {
    LRefIncr(obj);
    return obj;
  }

  list = LLISTVAL(obj);

  /* the empty list evaluates to itself */
  if (list == NULL || list->car == NULL) {
    return Lnil;
  }

  /* a nonempty list corresponds to a function call;
     the list's value is the value returned by the function */
  if (list->car->type == LFUNC) {
    fentry = &functable[LFUNCVAL(list->car)];
    
#if 0
    /* deal with any interests in the function first */
    if ((interest=fentry->interested) != NULL) {
      args = list->cdr;
      while (interest) {
	if (FilterArgMatch(interest->filter, args)) {
	  InterestOutput(fentry->name, args, interest);
	}
	interest = interest->next;
      }
    }
#endif

    /* then call the function */
    ans = fentry->fptr(NULL, list);

    /* deal with any interests in the function after calling the
     * function; otherwise the arguments are in an unevaluated state.
     */
    if ((interest=fentry->interested) != NULL) {
      args = list->cdr;
      while (interest) {
	if (FilterArgMatch(interest->filter, args)) {
	  InterestOutput(fentry->name, args, interest);
	}
	interest = interest->next;
      }
    }

    return ans;
  } else {
    OOGLError(0, "lisp error: call to unknown function %s",
	      LSummarize(list->car));
    return Lnil;
  }
}

LList *LListAppend(LList *list, LObject *obj)
{
  LList *l, *new = LListNew();

  new->car = obj;
  if ((l = list) != NULL) {
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
	"(car LIST)\n"
	"returns the first element of LIST.")
{
  LList *list;
  LDECLARE(("car", LBEGIN,
	    LLIST, &list,
	    LEND));
  if (list && list->car) {
    return LRefIncr(list->car);
  }
  return Lnil;
}

LDEFINE(cdr, LLOBJECT,
	"(cdr LIST)\n"
	"returns the list obtained by removing the first element of LIST.")
{
  LList *list;

  LDECLARE(("cdr", LBEGIN,
	    LLIST, &list,
	    LEND));

  if (list && list->cdr) {
    LList *copy = LListShallowCopy(list->cdr);
    return LNew(LLIST, &copy);
  }
  return Lnil;
}

LDEFINE(cons, LLOBJECT,
	"(cons EXPR LIST)\n"
	"returns the list obtained by adding EXPR as first element of LIST.")
{
  LObject *llist;
  LObject *car;
  LList *cdr;

  LDECLARE(("cons", LBEGIN,
	    LLOBJECT, &car,
	    LLIST, &cdr,
	    LEND));

  llist = LNew(LLIST, NULL);
  llist->cell.p = LListNew();
  LLISTVAL(llist)->car = LRefIncr(car);
  LLISTVAL(llist)->cdr = LListShallowCopy(cdr);

  return llist;
}

/*
 * function definition implementation
 */
bool LDefun(const char *name, LObjectFunc func, const char *help)
{
  int index = funcindex(name);
  LFunction *lfunction;

  if (index >= 0) {
    lfunction = VVINDEX(funcvvec, LFunction, index);
    if (lfunction->lambda == NULL) {
      VARARRAY(builtin, char, strlen(name)+sizeof("-builtin--"));
      OOGLWarn("Warning: replacing existing definition of builtin function\n"
	       "                       \"%s\"\n"
	       "The old definition is still available under the new name\n"
	       "               \"-builtin-%s-\"",
	       name, name);
      sprintf(builtin, "-builtin-%s-", name);
      LDefun(builtin, lfunction->fptr, lfunction->help);
    }
    lfunction = VVINDEX(funcvvec, LFunction, index);
    if (lfunction->lambda) {
      LFree(lfunction->lambda);
    }
    if (lfunction->help) {
      free(lfunction->help);
    }
  } else {
    index = VVCOUNT(funcvvec)++;
    lfunction = VVINDEX(funcvvec, LFunction, index);
    lfunction->name = strdup(name);
  }
  lfunction->fptr = func;
  lfunction->help = NULL;
  lfunction->lambda = NULL;
  lfunction->interested = NULL;
  fsa_install(func_fsa, lfunction->name, (void *)(long)index);
  if (help) {
    lfunction->help = strdup(help);
    LHelpDef(lfunction->name, lfunction->help);
  }
  return true;
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

static int funcindex(const char *name)
{
  return (int)(long)fsa_parse( func_fsa, name );
}

/*
 * The LDECLARE() macro calls this function.
 */
LParseResult LParseArgs(const char *name, Lake *lake, LList *args, ...)
{
  bool moreargspecs = true, literal = false;
  int argsgot = 0, argsrequired= -1, argspecs = 0;
  LType *argclass;
  va_list a_list;

  va_start(a_list, args);
  
  if (lake == NULL) {
    LParseResult val = AssignArgs(name, args->cdr, a_list);
    va_end(a_list);
    return val;
  }
  
  while (moreargspecs) {
    argclass=va_arg(a_list, LType *);
    if (argclass->size < 0) {
      if (argclass == LEND) {
	moreargspecs = false;
      } else if (argclass == LOPTIONAL) {
	argsrequired = argspecs;
      } else if (argclass == LHOLD) {
	/* "LHOLD" has no meaning during the parsing stage */
      } else if (argclass == LLITERAL) {
	/* literal affects the way an argument is parsed (as well as
	   implying "hold" in the assignment stage). It should only be
	   used on LLOBJECT or LLIST.  It means: "parse the argument
	   literally". In non-literal parsing, lists are treated as
	   function calls and the function is called to parse the
	   arguments. In literal parsing, we don't treat lists as
	   function calls. Just parse them as lists. */
	literal = true;
      } else if (argclass == LARRAY) {
	/* special case for this because it takes 3 args: the base type,
	   the array itself, and a count */
	(void)va_arg(a_list, LType *);
	(void)va_arg(a_list, void *);
	(void)va_arg(a_list, int *);

	++argspecs;
	if (LakeMore(lake)) {
	  LListAppend(args, LSexpr(lake));
	  ++argsgot;
	}
      } else if (argclass == LVARARRAY) {
	/* special case for this because it takes 3 args: the base type,
	   the array-pointer itself, and a count */
	(void)va_arg(a_list, LType *);
	(void)va_arg(a_list, void **);
	(void)va_arg(a_list, int *);

	++argspecs;
	if (LakeMore(lake)) {
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

	while(LakeMore(lake)) {
	  /* Stash args for AssignArgs to grab */
	  LListAppend(args, literal ? LLiteral(lake) : LSexpr(lake));
	}
	moreargspecs = false;
      }
    } else if(argclass == LLAKE) {
      (void)va_arg(a_list, Lake **);
      LListAppend(args, LTOOBJ(LLAKE)(&lake));
    } else {
      ++argspecs;
      (void)va_arg(a_list, void *);
      if (LakeMore(lake)) {
	LObject *arg;

	if (literal) {
	  /* literal should only be used on LLOBJECT or LLIST
	     types, both of which use the LSexpr() parse method; in
	     the literal case, we use LLiteral() instead. */
	  arg = LLiteral(lake);
	  literal = false;
	} else {
	  /* ParseArg() invokes SExpr() on S-expr and the
	   * type-specific parser otherwise.
	   */
	  arg = ParseArg(argclass, lake);
	}
	LListAppend(args, arg);
	++argsgot;
      }
    }
  }
  if (argsrequired < 0) {
    argsrequired = argspecs;
  }
  va_end(a_list);
  if (argsgot < argsrequired) {
    OOGLSyntax(lake->streamin,
	       "Reading from \"%s\": %s requires %d args, got %d",
	       PoolName(POOL(lake)),name,argsrequired,argsgot);
    return LPARSE_BAD;
  }
  if (LakeMore(lake)) {
    OOGLSyntax(lake->streamin,
	       "In \"%s\": %s: ignoring additional arguments (expected %1d)\n",
	       PoolName((Pool *)(lake->river)), name, argsgot);
    while (LakeMore(lake)) {
      LFree(LSexpr(lake));
    }
  }
  return LPARSE_GOOD;
}

static bool obj2array(LObject *obj, LType *type, char *x, int *n, bool hold)
{
  int max= abs(*n);
  LList *list;
  char *tmp;

  *n = 0;

  /* interprete the nil object as an empty list */
  if ((obj == Lnil) || (stringfromobj(obj, &tmp) && strcmp(tmp, "nil") == 0)) {
    return true;
  }
  
  list = LLISTVAL(obj);
  if (obj->type != LLIST) {
    return false;
  }
  if (list == NULL || list->car == NULL) {
    return true;
  }
  if (list->car->type == LLAKE) {
    list = list->cdr;
  }
  while (list && list->car && *n < max) {
    LObject *obj = hold ? LRefIncr(list->car) : LEval(list->car);
    if (!LFROMOBJ(type)(obj, (void*)(x + (*n)*LSIZE(type)))) {
      LFree(obj);
      return false;
    }
    LFree(obj);
    (*n)++;
    list = list->cdr;
  }
  if (*n == max && list) {
    return false;
  }
  return true;
}

/* variable length array */
static bool obj2vararray(LObject *obj, LType *type, char **x, int *n, bool hold)
{
  LList *list;
  char *tmp;

  /* interprete the nil object as an empty list */
  if ((obj == Lnil) || (stringfromobj(obj, &tmp) && strcmp(tmp, "nil") == 0)) {
    if (*x) {
      OOGLFree(*x);
    }
    *x = NULL;
    *n = 0;
    return true;
  }
  
  list = LLISTVAL(obj);
  if (obj->type != LLIST) {
    if (*x) {
      OOGLFree(*x);
    }
    *x = NULL;
    *n = 0;
    return false;
  }
  if (list == NULL || list->car == NULL) {
    if (*x) {
      OOGLFree(*x);
    }
    *x = NULL;
    return true;
  }
  if (list->car->type == LLAKE) {
    list = list->cdr;
  }
  *n = LListLength(list);
  *x = OOGLRenewNE(char, *x, (*n)*LSIZE(type), "C-lisp vararray");
  *n = 0;
  while (list && list->car) {
    LObject *obj = hold ? LRefIncr(list->car) : LEval(list->car);
    if (!LFROMOBJ(type)(list->car, (void * )((*x) + (*n)*LSIZE(type)))) {
      LFree(obj);
      return false;
    }
    LFree(obj);
    (*n)++;
    list = list->cdr;
  }
  return true;
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

/* LParseArgs() MUST NOT evaluate the arguments, this is left to
   AssignArgs().
 */
static LParseResult AssignArgs(const char *name, LList *args, va_list a_list)
{
  bool moreargspecs = true, hold = false, convok;
  int argsgot = 0, argsrequired= -1, argspecs = 0;
  Lake *lake = NULL;
  LObject *arg;
  LType *argtype;
  
  while (moreargspecs) {
    if (args && args->car && lakefromobj(args->car, &lake)) {
      args = args->cdr;
    }
    argtype=va_arg(a_list, LType *);
    if (argtype->size < 0) {
      if (argtype == LEND) {
	moreargspecs = false;
      } else if (argtype == LOPTIONAL) {
	argsrequired = argspecs;
      } else if (argtype == LHOLD) {
	hold = true; /* do not evaluate the arguments */
      } else if (argtype == LLITERAL) {
	/* in the assignment stage, literal means the same as hold */
	hold = true;
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
	  convok = obj2array(arg, argtype, array, count, hold);
	  if (!convok) {
	    OOGLError(0, "%s: array of at most %1d %ss expected in\n"
		      "arg position %1d (got %s)\n",
		      name, origcount, argtype->name, argsgot,
		      LSummarize(arg));
	  }
	  args = args->cdr;
	} else {
	  (void)va_arg(a_list, void *);
	  (void)va_arg(a_list, void *);
	}
	hold = false;
      } else if (argtype == LVARARRAY) {
	/* get the base type of the array */
	argtype=va_arg(a_list, LType *);
	++argspecs;
	if (args) {
	  void *arrayp = va_arg(a_list, void*);
	  int *countp = va_arg(a_list, int*);

	  if (hold) {
	    arg = LRefIncr(args->car);
	  } else {
	    arg = LEval(args->car);
	  }
	  ++argsgot;
	  convok = obj2vararray(arg, argtype, arrayp, countp, hold);
	  if (!convok) {
	    OOGLError(0,
		      "%s: variable length array conversion failed "
		      "after converting %1d %ss in\n"
		      "arg position %1d (got %s)\n",
		      name, *countp, argtype->name,
		      argsgot, LSummarize(arg));
	  }
	  args = args->cdr;
	} else {
	  (void)va_arg(a_list, void *);

	  (void)va_arg(a_list, void *);
	}
	hold = false;
      } else if(argtype == LREST) {
	LList **restp = va_arg(a_list, LList **);
	if(restp) {
	  *restp = args;
	}
	if (!hold) {
	  /* Evaluate the arguments if !hold */
	  while (args) {
	    LObject *car = args->car;
	    args->car = LEval(car);
	    LFree(car);
	    args = args->cdr;
	  }
	}
	moreargspecs = false;
	args = NULL; /* Don't complain of excess args */
      }
    } else if (argtype == LLAKE) {
      if (lake) {
	*va_arg(a_list, Lake **) = lake;
      } else {
	OOGLError(0, "%s: internal lake assignment out of whack.", name);
	return LASSIGN_BAD;
      }
    } else {
      ++argspecs;
      if (args) {
	if (!hold) {
	  /* Evaluate the object and replace it in the argument list
	   * such that the caller can free the result. The original
	   * S-expr is free-ed here.
	   */
	  arg = LEval(args->car);
	  LFree(args->car);
	  args->car = arg;
	}
	++argsgot;
	convok = LFROMOBJ(argtype)(args->car, va_arg(a_list, void *));
	if (!convok) {
	  OOGLError(0,"%s: %s expected in arg position %1d (got %s)\n",
		    name, LNAME(argtype), argsgot, LSummarize(args->car));
	  return LASSIGN_BAD;
	}
	args = args->cdr;
      } else {
	(void)va_arg(a_list, void *);
      }
      hold = false;
    }
  }
  if (argsrequired < 0) {
    argsrequired = argspecs;
  }
  if (argsgot < argsrequired) {
    OOGLError(0, "%s: internal argument list deficit; require %1d, got %1d",
	      name, argsrequired, argsgot);
    return LASSIGN_BAD;
  }
  if (args) {
    OOGLError(1, "%s: internal argument list excess", name);
    return LASSIGN_BAD;
  }
  return LASSIGN_GOOD;
}

bool LArgClassValid(LType *type)
{
  return (type->magic == LTypeMagic);
}

LObject *LEvalFunc(const char *name, ...)
{
  va_list a_list;
  LList *list, *tail, *rest = NULL;
  LObject *obj, *val;
  int i;
  LType *a;
  LCell cell;

  if ((i = funcindex(name)) != REJECT) {
    list = LListAppend(NULL, LNew(LFUNC, &i));
  } else {
    list = LListAppend(NULL, LNew(LSYMBOL, &name));
  }
  tail = list;

  va_start(a_list, name);
  while ((a = va_arg(a_list, LType *)) != LEND) {
    if (a == LHOLD || a == LLITERAL || a == LOPTIONAL) {
      /* do nothing */
    } else if (a == LARRAY || a == LVARARRAY) {
      LType *basetype=va_arg(a_list, LType *);
      void *array = va_arg(a_list, void *);
      int count = abs(va_arg(a_list, int));

      tail->cdr = LListAppend(NULL, LMakeArray(basetype, array, count));
      tail = tail->cdr;
    } else if (a == LREST) {
      /* This is a special case: the argument list is terminated, and
       * "rest" is treated as the tail of the argument list.
       */
      LPULL(LLIST)(&a_list, &rest);
      
      tail->cdr = rest;
      if (va_arg(a_list, LType *) != LEND) {
	OOGLError(0, "LEvalFunc%(s): Error: excess arguments after LREST.",
		  name);
	LListFree(list);
	return Lnil;
      }
      break;
    } else {
      LPULL(a)(&a_list, &cell);
      tail->cdr = LListAppend(NULL, LTOOBJ(a)(&cell));
      tail = tail->cdr;
    }
  }
  /* This makes a copy of "list", slow but safe. */
  obj = LNew(LLIST, &list);
  val = LEval(obj);
  tail->cdr = NULL; /* Do not delete rest! */
  LFree(obj);
  return val;
}

static bool filterfromobj(LObject *obj, LFilter **x)
{
  if (obj->type != LFILTER) return false;
  *x = LFILTERVAL(obj);
  return true;
}

static LObject *filter2obj(LFilter **x)
{
  LFilter *copy = OOGLNew(LFilter);
  copy->flag = (*x)->flag;
  copy->value = (*x)->value ? LCopy((*x)->value) : NULL;
  return LNew( LFILTER, &copy );
}

static void filterfree(LFilter **x)
{
  if (*x) {
    if ((*x)->value) LFree((*x)->value);
    OOGLFree(*x);
  }
}

static void filterwrite(FILE *fp, LFilter **x)
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
    if(l->initial) free((char *)l->initial);
    if(l->prefix) free((char *)l->prefix);
    if(l->suffix) free((char *)l->suffix);
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
  if (!symbolfromobj(call->car, &command)) {
    fprintf(stderr, "%s: COMMAND must be a symbol (got `%s')\n",
	    action, LSummarize(call->car));
    return Lnil;
  }

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


static bool InterestMatch(LInterest *interest, Lake *lake,
			  bool usefilter, LList *filter)
{
  LList *ifilter;

  if (interest->lake != lake) return false;
  if (!usefilter) return true;
  ifilter = interest->filter;
  while (filter) {
    if (!ifilter) return false;
    if (!FilterMatch(LFILTERVAL(filter->car),
		     LFILTERVAL(ifilter->car))) return false;
    filter = filter->cdr;
    ifilter = ifilter->cdr;
  }
  if (ifilter) return false;
  return true;
}

static bool FilterMatch(LFilter *f1, LFilter *f2)
{
  if (f1 && !f2) return false;
  if (f2 && !f1) return false;
  if (!f1 && !f2) return true;
  if (f1->flag != f2->flag) return false;
  switch (f1->flag) {
  case ANY:
  case NIL:
    return true;
  case VAL:
    if (f1->value->type != f2->value->type) return false;
    return LMATCH(f1->value->type)( &(f1->value->cell), &(f2->value->cell) );
  default:
    OOGLError(0,"invalid filter flag value.  Please report this.");
    return false;
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
    if ((strcmp(LSTRINGVAL(args->car),"*")==0) || (args->car==Lt)) {
      filterlist = LListAppend(filterlist, LRefIncr(LFAny));
    } else if ((strcmp(LSTRINGVAL(args->car),"nil")==0) || (args->car==Lnil) ) {
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

static bool FilterArgMatch(LList *filter,  LList *args)
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
	return false;
      break;
    case ANY:
    case NIL:
      break;
    }

    args = args->cdr;
  }
  return true;
}

static void InterestOutput(char *name, LList *args, LInterest *interest)
{
  Lake *lake = interest->lake;
  FILE *outf = lake->streamout;
  LList *filter = interest->filter;
  const char *suffix = NULL;
  int filterflag;
  float now = 0.0;

  if (!outf) return;

  if(lake->timing_interests &&
     (now = PoolTimeAt(POOL(lake), NULL)) > lake->nexttime) {
    if(lake->initial) {
      fprintf(outf, lake->initial, now,now,now);
      free((char *)lake->initial);
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


static void compile(const char *str, pattern *p)
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
static void print_help_formatted(FILE *outf, const char *message)
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
      /* keep \n\n as hard line break marker */
      while (isspace(*message)) {
	if (*message++ == '\n') {
	  ++nnl;
	}
	if (nnl == 2) {
	  fprintf(outf, "\n       ");
	  printed = 7;
	  /* use \n\n\t\t\t as indentation hint */
	  while (*message == '\t') {
	    fprintf(outf, "        ");
	    printed += 8;
	    message++;
	  }
	  nnl = 0;
	}
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

static bool match(const char *str, pattern *p)
{
  int i;
  const char *rest;
  if(strncmp(str, p->pat[0], p->len[0])) return false;	/* Failed */
  rest = str + p->len[0];
  for(i = 1; i <= p->n; i++) {
    if(p->len[i]) {
      if((rest = strstr(rest, p->pat[i])) == NULL) break;
      rest += p->len[i];
    }
  }
  return i > p->n && rest && (p->len[p->n] == 0 || *rest == '\0') ? 1 : 0;
}

void LHelpDef(const char *key, const char *message)
{
  Help **h = &helps;
  Help *new;
  int cmp = -1;

  /* insertion sort... */
  while (*h && (*h)->key && (cmp = strcmp(key,(*h)->key)) > 0) {
    h = &((*h)->next);
  }
  if (cmp == 0) {
    /* replace an existing message */
    new = *h;
  } else {
    new = OOGLNew(Help);
    new->key = key;
    new->next = *h;
    *h = new;
  }
  new->message = message;
}

LDEFINE(help, LVOID,
	"(help [COMMAND])\n"
	"\"COMMAND\" may include \"*\"s as wildcards; see also \"??\". "
	"One-line command help; lists names only if multiple commands match.")
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
	"(morehelp COMMAND)\n"
	"\"COMMAND\" may include \"*\" wildcards\n"
	"Prints more info than \"(help COMMAND)\".")
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

LInterest *LInterestList(const char *funcname)
{
  int index = funcindex(funcname);
  if (index == REJECT) return NULL;
  return functable[index].interested;
}

const char *LakeName(Lake *lake)
{
  return lake ? PoolName(lake->river) : NULL;
}

const char *LSummarize(LObject *obj)
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
  if(summary) {
    free(summary);
  }
  summary = malloc(len+1);
  summary[len] = '\0';
  if (fread(summary, len, 1, f) != 1) {
    free(summary);
    return strdup("???");
  }
  if(len >= 79) {
    strcpy(summary+75, " ...");
  }
  return summary;
}

/************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
