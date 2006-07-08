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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/*  A Bison parser, made from ../parse.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	IDENT	258
#define	NUMBER	259
#define	MINUS	260

#line 1 "../parse.y"


#include "fexpr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void fparse_yyerror(char *);
extern int fparse_yyparse();
extern int fparse_yyrestart(void *);
extern void expr_lex_reset_input(char *buf);
extern int fparse_yylex(void);

#define YYDEBUG 1

struct expr_tree {
  struct expr_elem elem;
  struct expr_tree *sub1;
  struct expr_tree *sub2;
};

struct expr_tree *expr_parsed;
struct expression *expr_current;

static struct expr_parse_free {
  void *dat;
  struct expr_parse_free *next;
} *freers = NULL;

static void expr_free_dat(void *dat)
{
  struct expr_parse_free *f;
  f=malloc(sizeof(struct expr_parse_free));
  f->next=freers;
  freers=f;
  f->dat = dat;
}

static void expr_free_freers()
{
  struct expr_parse_free *f, *next=NULL;
  for (f=freers; f; f=next) {
    next = f->next;
    free(f->dat);
    free(f);
  }
  freers = NULL;
}

static void expr_free_freers_no_dat()
{
  struct expr_parse_free *f, *next=NULL;
  for (f=freers; f; f=next) {
    next = f->next;
    free(f);
  }
  freers = NULL;
}

static int count_nodes(struct expr_tree *e)
{
  if (!e) return 0;
  return count_nodes(e->sub1) + count_nodes(e->sub2) + 1;
}

static void store_nodes(struct expr_tree *e, int *pos)
{
  if (!e) return;
  store_nodes(e->sub2, pos);
  store_nodes(e->sub1, pos);
  expr_current->elems[*pos] = e->elem;
  (*pos)++;
}

static char *error_return;

char *expr_parse(struct expression *expr, char *str)
{
  int i;

  error_return = NULL;
  
  expr_current = expr;

  if (!str || !*str) return "Empty expression";
  expr_lex_reset_input(str);

  i = fparse_yyparse();		/* do all the work. */
  fparse_yyrestart(NULL);		/* clean up so we can do this again. */
  
  if (i) {
    expr_free_freers();
    return "Parse error";
  }

  if (error_return) {
    expr_free_freers();
    return error_return;
  }

  expr_free_freers_no_dat();

  /* ok. now we convert the parsed expression to a execution vector. whee. */
  expr->nelem = count_nodes(expr_parsed);
  expr->elems = malloc (expr->nelem * sizeof (struct expr_elem));
  
  /* we let i be a counter variable for where we're writing in 
   * the vectorized expression. */
  i = 0;
  store_nodes(expr_parsed, &i);
  return NULL;
}

static struct expr_monfunc lookup_monfunc(char *s)
{
  struct expr_monfunc *i;
  static struct expr_monfunc fail = { NULL, NULL, NULL };

  for (i=expr_monfuncs; i->name; i++)
    if (!strcmp(i->name,s))
      return *i;

  {
    static char buf[1024];
    sprintf(buf,"Undefined unary function: %s",s);
    error_return = buf;
  }
  return fail;
}

static struct expr_binfunc lookup_binfunc(char *s)
{
  struct expr_binfunc *i;
  static struct expr_binfunc fail = { NULL, NULL, NULL };

  for (i=expr_binfuncs; i->name; i++)
    if (!strcmp(i->name,s))
      return *i;

  {
    static char buf[1024];
    sprintf(buf,"Undefined binary function: %s",s);
    error_return = buf;
  }
  return fail;
}

static struct expr_tree *expr_parse_monop(int opr, struct expr_tree *sub)
{
  struct expr_tree *op;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;
  op->elem.op = MONOP;
  op->elem.u.monop.op = opr;
  op->sub1 = sub;
  expr_free_dat(op);
  return op;
}

static struct expr_tree *expr_parse_binop(int opr, struct expr_tree *sub1, struct expr_tree *sub2)
{
  struct expr_tree *op;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;
  op->elem.op = BINOP;
  op->elem.u.binop.op = opr;
  op->sub1 = sub1;
  op->sub2 = sub2;
  expr_free_dat(op);
  return op;
}

static struct expr_tree *expr_parse_monfunc(struct expr_monfunc f, struct expr_tree *sub1)
{
  struct expr_tree *op;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;
  op->elem.op = MONFUNC;
  op->elem.u.monfunc.func = f.func;
  op->elem.u.monfunc.cfunc = f.cfunc;
  op->sub1 = sub1;
  expr_free_dat(op);
  return op;
}

static struct expr_tree *expr_parse_binfunc(struct expr_binfunc f, struct expr_tree *sub1, struct expr_tree *sub2)
{
  struct expr_tree *op;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;
  op->elem.op = BINFUNC;
  op->elem.u.binfunc.func = f.func;
  op->elem.u.binfunc.cfunc = f.cfunc;
  op->sub1 = sub1;
  op->sub2 = sub2;
  expr_free_dat(op);
  return op;
}

static struct expr_tree *expr_parse_var(char *s)
{
  struct expr_tree *op;
  int i=0;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;

  op->elem.u.pushvar.varnum = -1;

  op->elem.op = PUSHVAR;
  for (i=0; i<expr_current->nvars; i++)
    if (!strcmp(expr_current->varnames[i], s))
      op->elem.u.pushvar.varnum = i;

  if (op->elem.u.pushvar.varnum == -1) {
    static char buf[1024];
    sprintf(buf,"Undefined variable: %s", s);
    error_return = buf;
  }
  expr_free_dat(op);
  return op;
}

static struct expr_tree *expr_parse_num(double d)
{
  struct expr_tree *op;

  op = malloc (sizeof (struct expr_tree));
  op->sub1 = op->sub2 = NULL;
  op->elem.op = PUSHNUM;
  op->elem.u.pushnum.number = d;
  expr_free_dat(op);
  return op;
}

void fparse_yyerror(char *errorstr)
{
  error_return = errorstr;
}
 

#line 246 "../parse.y"
typedef union {
  char *string;
  struct expr_tree *expression;
  double number;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct fparse_yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  fparse_yyltype;

#define YYLTYPE fparse_yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		25
#define	YYFLAG		-32768
#define	YYNTBASE	13

#define YYTRANSLATE(x) ((unsigned)(x) <= 260 ? fparse_yytranslate[x] : 15)

static const char fparse_yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    10,
    11,     9,     5,    12,     6,     2,     8,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     7
};

#if YYDEBUG != 0
static const short fparse_yyprhs[] = {     0,
     0,     2,     6,    10,    14,    18,    22,    25,    30,    37,
    39
};

static const short fparse_yyrhs[] = {    14,
     0,    10,    14,    11,     0,    14,     5,    14,     0,    14,
     6,    14,     0,    14,     9,    14,     0,    14,     8,    14,
     0,     6,    14,     0,     3,    10,    14,    11,     0,     3,
    10,    14,    12,    14,    11,     0,     3,     0,     4,     0
};

#endif

#if YYDEBUG != 0
static const short fparse_yyrline[] = { 0,
   263,   265,   267,   268,   269,   270,   271,   272,   273,   274,
   275
};

static const char * const fparse_yytname[] = {   "$","error","$undefined.","IDENT",
"NUMBER","'+'","'-'","MINUS","'/'","'*'","'('","')'","','","totexpression","expr",
""
};
#endif

static const short fparse_yyr1[] = {     0,
    13,    14,    14,    14,    14,    14,    14,    14,    14,    14,
    14
};

static const short fparse_yyr2[] = {     0,
     1,     3,     3,     3,     3,     3,     2,     4,     6,     1,
     1
};

static const short fparse_yydefact[] = {     0,
    10,    11,     0,     0,     1,     0,     7,     0,     0,     0,
     0,     0,     0,     2,     3,     4,     6,     5,     8,     0,
     0,     9,     0,     0,     0
};

static const short fparse_yydefgoto[] = {    23,
     5
};

static const short fparse_yypact[] = {    17,
    -8,-32768,    17,    17,    34,    17,    -4,    20,    17,    17,
    17,    17,     7,-32768,    -4,    -4,-32768,-32768,-32768,    17,
    27,-32768,    10,    11,-32768
};

static const short fparse_yypgoto[] = {-32768,
    -3
};


#define	YYLAST		43


static const short fparse_yytable[] = {     7,
     8,     6,    13,    11,    12,    15,    16,    17,    18,    24,
    25,     9,    10,     0,    11,    12,    21,    19,    20,     1,
     2,     0,     3,     0,     9,    10,     4,    11,    12,     0,
    14,     9,    10,     0,    11,    12,     0,    22,     9,    10,
     0,    11,    12
};

static const short fparse_yycheck[] = {     3,
     4,    10,     6,     8,     9,     9,    10,    11,    12,     0,
     0,     5,     6,    -1,     8,     9,    20,    11,    12,     3,
     4,    -1,     6,    -1,     5,     6,    10,     8,     9,    -1,
    11,     5,     6,    -1,     8,     9,    -1,    11,     5,     6,
    -1,     8,     9
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define fparse_yyerrok		(fparse_yyerrstatus = 0)
#define fparse_yyclearin	(fparse_yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto fparse_yyerrlab1
/* Like YYERROR except do call fparse_yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto fparse_yyerrlab
#define YYRECOVERING()  (!!fparse_yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (fparse_yychar == YYEMPTY && fparse_yylen == 1)				\
    { fparse_yychar = (token), fparse_yylval = (value);			\
      fparse_yychar1 = YYTRANSLATE (fparse_yychar);				\
      YYPOPSTACK;						\
      goto fparse_yybackup;						\
    }								\
  else								\
    { fparse_yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		fparse_yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		fparse_yylex(&fparse_yylval, &fparse_yylloc, YYLEX_PARAM)
#else
#define YYLEX		fparse_yylex(&fparse_yylval, &fparse_yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		fparse_yylex(&fparse_yylval, YYLEX_PARAM)
#else
#define YYLEX		fparse_yylex(&fparse_yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	fparse_yychar;			/*  the lookahead symbol		*/
YYSTYPE	fparse_yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE fparse_yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int fparse_yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int fparse_yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int fparse_yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __fparse_yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__fparse_yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__fparse_yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "/usr/local/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into fparse_yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
fparse_yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int fparse_yystate;
  register int fparse_yyn;
  register short *fparse_yyssp;
  register YYSTYPE *fparse_yyvsp;
  int fparse_yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int fparse_yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	fparse_yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE fparse_yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *fparse_yyss = fparse_yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *fparse_yyvs = fparse_yyvsa;	/*  to allow fparse_yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE fparse_yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *fparse_yyls = fparse_yylsa;
  YYLTYPE *fparse_yylsp;

#define YYPOPSTACK   (fparse_yyvsp--, fparse_yyssp--, fparse_yylsp--)
#else
#define YYPOPSTACK   (fparse_yyvsp--, fparse_yyssp--)
#endif

  int fparse_yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int fparse_yychar;
  YYSTYPE fparse_yylval;
  int fparse_yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE fparse_yylloc;
#endif
#endif

  YYSTYPE fparse_yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int fparse_yylen;

#if YYDEBUG != 0
  if (fparse_yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  fparse_yystate = 0;
  fparse_yyerrstatus = 0;
  fparse_yynerrs = 0;
  fparse_yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  fparse_yyssp = fparse_yyss - 1;
  fparse_yyvsp = fparse_yyvs;
#ifdef YYLSP_NEEDED
  fparse_yylsp = fparse_yyls;
#endif

/* Push a new state, which is found in  fparse_yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
fparse_yynewstate:

  *++fparse_yyssp = fparse_yystate;

  if (fparse_yyssp >= fparse_yyss + fparse_yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *fparse_yyvs1 = fparse_yyvs;
      short *fparse_yyss1 = fparse_yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *fparse_yyls1 = fparse_yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = fparse_yyssp - fparse_yyss + 1;

#ifdef fparse_yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if fparse_yyoverflow is a macro.  */
      fparse_yyoverflow("parser stack overflow",
		 &fparse_yyss1, size * sizeof (*fparse_yyssp),
		 &fparse_yyvs1, size * sizeof (*fparse_yyvsp),
		 &fparse_yyls1, size * sizeof (*fparse_yylsp),
		 &fparse_yystacksize);
#else
      fparse_yyoverflow("parser stack overflow",
		 &fparse_yyss1, size * sizeof (*fparse_yyssp),
		 &fparse_yyvs1, size * sizeof (*fparse_yyvsp),
		 &fparse_yystacksize);
#endif

      fparse_yyss = fparse_yyss1; fparse_yyvs = fparse_yyvs1;
#ifdef YYLSP_NEEDED
      fparse_yyls = fparse_yyls1;
#endif
#else /* no fparse_yyoverflow */
      /* Extend the stack our own way.  */
      if (fparse_yystacksize >= YYMAXDEPTH)
	{
	  fparse_yyerror("parser stack overflow");
	  return 2;
	}
      fparse_yystacksize *= 2;
      if (fparse_yystacksize > YYMAXDEPTH)
	fparse_yystacksize = YYMAXDEPTH;
      fparse_yyss = (short *) alloca (fparse_yystacksize * sizeof (*fparse_yyssp));
      __fparse_yy_memcpy ((char *)fparse_yyss1, (char *)fparse_yyss, size * sizeof (*fparse_yyssp));
      fparse_yyvs = (YYSTYPE *) alloca (fparse_yystacksize * sizeof (*fparse_yyvsp));
      __fparse_yy_memcpy ((char *)fparse_yyvs1, (char *)fparse_yyvs, size * sizeof (*fparse_yyvsp));
#ifdef YYLSP_NEEDED
      fparse_yyls = (YYLTYPE *) alloca (fparse_yystacksize * sizeof (*fparse_yylsp));
      __fparse_yy_memcpy ((char *)fparse_yyls1, (char *)fparse_yyls, size * sizeof (*fparse_yylsp));
#endif
#endif /* no fparse_yyoverflow */

      fparse_yyssp = fparse_yyss + size - 1;
      fparse_yyvsp = fparse_yyvs + size - 1;
#ifdef YYLSP_NEEDED
      fparse_yylsp = fparse_yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (fparse_yydebug)
	fprintf(stderr, "Stack size increased to %d\n", fparse_yystacksize);
#endif

      if (fparse_yyssp >= fparse_yyss + fparse_yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (fparse_yydebug)
    fprintf(stderr, "Entering state %d\n", fparse_yystate);
#endif

  goto fparse_yybackup;
 fparse_yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* fparse_yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  fparse_yyn = fparse_yypact[fparse_yystate];
  if (fparse_yyn == YYFLAG)
    goto fparse_yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* fparse_yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (fparse_yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (fparse_yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      fparse_yychar = YYLEX;
    }

  /* Convert token to internal form (in fparse_yychar1) for indexing tables with */

  if (fparse_yychar <= 0)		/* This means end of input. */
    {
      fparse_yychar1 = 0;
      fparse_yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (fparse_yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      fparse_yychar1 = YYTRANSLATE(fparse_yychar);

#if YYDEBUG != 0
      if (fparse_yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", fparse_yychar, fparse_yytname[fparse_yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, fparse_yychar, fparse_yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  fparse_yyn += fparse_yychar1;
  if (fparse_yyn < 0 || fparse_yyn > YYLAST || fparse_yycheck[fparse_yyn] != fparse_yychar1)
    goto fparse_yydefault;

  fparse_yyn = fparse_yytable[fparse_yyn];

  /* fparse_yyn is what to do for this token type in this state.
     Negative => reduce, -fparse_yyn is rule number.
     Positive => shift, fparse_yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (fparse_yyn < 0)
    {
      if (fparse_yyn == YYFLAG)
	goto fparse_yyerrlab;
      fparse_yyn = -fparse_yyn;
      goto fparse_yyreduce;
    }
  else if (fparse_yyn == 0)
    goto fparse_yyerrlab;

  if (fparse_yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (fparse_yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", fparse_yychar, fparse_yytname[fparse_yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (fparse_yychar != YYEOF)
    fparse_yychar = YYEMPTY;

  *++fparse_yyvsp = fparse_yylval;
#ifdef YYLSP_NEEDED
  *++fparse_yylsp = fparse_yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (fparse_yyerrstatus) fparse_yyerrstatus--;

  fparse_yystate = fparse_yyn;
  goto fparse_yynewstate;

/* Do the default action for the current state.  */
fparse_yydefault:

  fparse_yyn = fparse_yydefact[fparse_yystate];
  if (fparse_yyn == 0)
    goto fparse_yyerrlab;

/* Do a reduction.  fparse_yyn is the number of a rule to reduce with.  */
fparse_yyreduce:
  fparse_yylen = fparse_yyr2[fparse_yyn];
  if (fparse_yylen > 0)
    fparse_yyval = fparse_yyvsp[1-fparse_yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (fparse_yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       fparse_yyn, fparse_yyrline[fparse_yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = fparse_yyprhs[fparse_yyn]; fparse_yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", fparse_yytname[fparse_yyrhs[i]]);
      fprintf (stderr, " -> %s\n", fparse_yytname[fparse_yyr1[fparse_yyn]]);
    }
#endif


  switch (fparse_yyn) {

case 1:
#line 263 "../parse.y"
{ expr_parsed = fparse_yyvsp[0].expression; ;
    break;}
case 2:
#line 266 "../parse.y"
{ fparse_yyval.expression = fparse_yyvsp[-1].expression; ;
    break;}
case 3:
#line 267 "../parse.y"
{ fparse_yyval.expression = expr_parse_binop('+', fparse_yyvsp[-2].expression, fparse_yyvsp[0].expression); ;
    break;}
case 4:
#line 268 "../parse.y"
{ fparse_yyval.expression = expr_parse_binop('-', fparse_yyvsp[-2].expression, fparse_yyvsp[0].expression); ;
    break;}
case 5:
#line 269 "../parse.y"
{ fparse_yyval.expression = expr_parse_binop('*', fparse_yyvsp[-2].expression, fparse_yyvsp[0].expression); ;
    break;}
case 6:
#line 270 "../parse.y"
{ fparse_yyval.expression = expr_parse_binop('/', fparse_yyvsp[-2].expression, fparse_yyvsp[0].expression); ;
    break;}
case 7:
#line 271 "../parse.y"
{ fparse_yyval.expression = expr_parse_monop('-', fparse_yyvsp[0].expression); ;
    break;}
case 8:
#line 272 "../parse.y"
{ fparse_yyval.expression = expr_parse_monfunc(lookup_monfunc(fparse_yyvsp[-3].string), fparse_yyvsp[-1].expression); ;
    break;}
case 9:
#line 273 "../parse.y"
{ fparse_yyval.expression = expr_parse_binfunc(lookup_binfunc(fparse_yyvsp[-5].string), fparse_yyvsp[-3].expression, fparse_yyvsp[-1].expression); ;
    break;}
case 10:
#line 274 "../parse.y"
{ fparse_yyval.expression = expr_parse_var(fparse_yyvsp[0].string); ;
    break;}
case 11:
#line 275 "../parse.y"
{ fparse_yyval.expression = expr_parse_num(fparse_yyvsp[0].number); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "/usr/local/lib/bison.simple"

  fparse_yyvsp -= fparse_yylen;
  fparse_yyssp -= fparse_yylen;
#ifdef YYLSP_NEEDED
  fparse_yylsp -= fparse_yylen;
#endif

#if YYDEBUG != 0
  if (fparse_yydebug)
    {
      short *ssp1 = fparse_yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != fparse_yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++fparse_yyvsp = fparse_yyval;

#ifdef YYLSP_NEEDED
  fparse_yylsp++;
  if (fparse_yylen == 0)
    {
      fparse_yylsp->first_line = fparse_yylloc.first_line;
      fparse_yylsp->first_column = fparse_yylloc.first_column;
      fparse_yylsp->last_line = (fparse_yylsp-1)->last_line;
      fparse_yylsp->last_column = (fparse_yylsp-1)->last_column;
      fparse_yylsp->text = 0;
    }
  else
    {
      fparse_yylsp->last_line = (fparse_yylsp+fparse_yylen-1)->last_line;
      fparse_yylsp->last_column = (fparse_yylsp+fparse_yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  fparse_yyn = fparse_yyr1[fparse_yyn];

  fparse_yystate = fparse_yypgoto[fparse_yyn - YYNTBASE] + *fparse_yyssp;
  if (fparse_yystate >= 0 && fparse_yystate <= YYLAST && fparse_yycheck[fparse_yystate] == *fparse_yyssp)
    fparse_yystate = fparse_yytable[fparse_yystate];
  else
    fparse_yystate = fparse_yydefgoto[fparse_yyn - YYNTBASE];

  goto fparse_yynewstate;

fparse_yyerrlab:   /* here on detecting error */

  if (! fparse_yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++fparse_yynerrs;

#ifdef YYERROR_VERBOSE
      fparse_yyn = fparse_yypact[fparse_yystate];

      if (fparse_yyn > YYFLAG && fparse_yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -fparse_yyn if nec to avoid negative indexes in fparse_yycheck.  */
	  for (x = (fparse_yyn < 0 ? -fparse_yyn : 0);
	       x < (sizeof(fparse_yytname) / sizeof(char *)); x++)
	    if (fparse_yycheck[x + fparse_yyn] == x)
	      size += strlen(fparse_yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (fparse_yyn < 0 ? -fparse_yyn : 0);
		       x < (sizeof(fparse_yytname) / sizeof(char *)); x++)
		    if (fparse_yycheck[x + fparse_yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, fparse_yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      fparse_yyerror(msg);
	      free(msg);
	    }
	  else
	    fparse_yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	fparse_yyerror("parse error");
    }

  goto fparse_yyerrlab1;
fparse_yyerrlab1:   /* here on error raised explicitly by an action */

  if (fparse_yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (fparse_yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (fparse_yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", fparse_yychar, fparse_yytname[fparse_yychar1]);
#endif

      fparse_yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  fparse_yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto fparse_yyerrhandle;

fparse_yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  fparse_yyn = fparse_yydefact[fparse_yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (fparse_yyn) goto fparse_yydefault;
#endif

fparse_yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (fparse_yyssp == fparse_yyss) YYABORT;
  fparse_yyvsp--;
  fparse_yystate = *--fparse_yyssp;
#ifdef YYLSP_NEEDED
  fparse_yylsp--;
#endif

#if YYDEBUG != 0
  if (fparse_yydebug)
    {
      short *ssp1 = fparse_yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != fparse_yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

fparse_yyerrhandle:

  fparse_yyn = fparse_yypact[fparse_yystate];
  if (fparse_yyn == YYFLAG)
    goto fparse_yyerrdefault;

  fparse_yyn += YYTERROR;
  if (fparse_yyn < 0 || fparse_yyn > YYLAST || fparse_yycheck[fparse_yyn] != YYTERROR)
    goto fparse_yyerrdefault;

  fparse_yyn = fparse_yytable[fparse_yyn];
  if (fparse_yyn < 0)
    {
      if (fparse_yyn == YYFLAG)
	goto fparse_yyerrpop;
      fparse_yyn = -fparse_yyn;
      goto fparse_yyreduce;
    }
  else if (fparse_yyn == 0)
    goto fparse_yyerrpop;

  if (fparse_yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (fparse_yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++fparse_yyvsp = fparse_yylval;
#ifdef YYLSP_NEEDED
  *++fparse_yylsp = fparse_yylloc;
#endif

  fparse_yystate = fparse_yyn;
  goto fparse_yynewstate;
}
#line 279 "../parse.y"

/* more c */
