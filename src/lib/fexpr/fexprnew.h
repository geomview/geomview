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
/* $Id: fexprnew.h,v 1.2 2000/09/01 22:38:09 mphillips Exp $ */
/* new fast expression library! wheee! */

#ifndef fexpr_h_included
#define fexpr_h_included

typedef enum {
  FNONE, FREAL, FCOMPLEX, FDERIV, FRDERIV
} ftype;

struct fexpression
{
  struct fcode *code;
  struct ffunctable *funcs;
  ftype type;
};

typedef struct {
  double real,imag;
} fcomplex;

struct fcode;			/* forward reference */
typedef struct fvalue_struct fvalue;

typedef struct {
  int nderivs;
  fvalue *values;
} fjet;

struct fvalue_struct {
  ftype type;
  union fval_union {
    fcomplex complex;
    double real;
    fjet deriv;
  } value;
};


typedef struct {
  struct {
    double (*real)(int nargs, double *args, void *);
    void *data;
  } r;
  struct {
    void (*complex)(int nargs, fcomplex *args, fcomplex *op, void *);
    void *data;
  } c;
  struct {
    void (*deriv)(int nargs, fjet *args, fjet *op, void *data);
    void *data;
  } d;
} fevalfunc;

struct ffuncstruct {
  char *name;
  fevalfunc function;
  int nargs;
};

#if 0

struct ffunctableent {
  char *name;
  fevalfunc function;
  int nargs;			/* negative if we don't
				 * care. otherwise, the only numbers
				 * of arguments this will take. 0
				 * means probably is just a constant
				 * variable. */
};

#endif

#if 0
struct ffunctiontableent {
  char *name;
  ftype type;
  void *function;
  int nargs;
};
#endif

struct fcode {
  int ninsts;
  struct finst {
    enum {
      FIFUNC, FINUMBER
    } type;
    union {
      struct {
	int nargs;
	struct ffuncstruct ent; /* functions are null if they haven't
				   been linked. */
      } func;
      fvalue number;
    } u;
  } *insts;
};

struct ffunctable {
  int nents;
  struct ffuncstruct *ents;
  struct ffunctable *next;
};

#if 0
typedef struct ffunctiontableent ffunctiontable[];
#endif


/* functions */

/* operates on a whole bundle -- for simple users */
extern struct fexpression *fnew_expr(ftype);
extern char *fexpr_parse(struct fexpression *, char *);
extern fvalue *fexpr_getvar(struct fexpression *, char *name);
extern void fexpr_evaluate(struct fexpression *, fvalue *op);
extern void fexpr_free(struct fexpression *);
extern void fexpr_cleanup(void); /* run at end of program */

/* code functions */
extern struct fcode *fnew_code(void);
extern char *fcode_parse(struct fcode *, char *);
extern int fcode_link(struct fcode *, struct ffunctable *, ftype);
extern void fcode_run(struct fcode *, ftype, fvalue *op, ...);
extern void fcode_free(struct fcode *);

extern void fapplyv(struct ffuncstruct *, ftype, fvalue *op, int nargs, ...);
extern void fapply(struct ffuncstruct *, ftype, int nargs, fvalue *args, fvalue *op);

/* func functions */
extern struct ffunctable *fnew_func(void);
extern struct ffunctable *fnew_func_with_defaults(void);
extern fvalue *ffunc_getvar(struct ffunctable *, char *, ftype);
extern struct ffuncstruct *ffunc_findfunc(struct ffunctable *, char *, ftype, int);
extern void ffunc_free(struct ffunctable *);
extern struct ffuncstruct *ffunc_create(struct ffunctable *, char *, fevalfunc, int);
extern void ffunc_free_builtins(void);

/* value functions */
extern void fvalue_free(fvalue *);
extern void fvalue_free_parts(fvalue *);
extern void fvalue_from_double(fvalue *,ftype,double);
extern int fvalue_iszero(fvalue *);
extern int fvalue_isone(fvalue *);
#ifdef EOF
extern void fvalue_print(FILE *,fvalue *);
#endif

/* miscelaneous other things */
extern fvalue *fderiv_variable;

/* default functions and constants */
extern struct ffunctable *fbuiltins;
extern void fbuiltins_build();

/* internal stuff */
extern char *fparse_code_internal(struct fcode *, char *);
extern void fparse_set_string(char *);
extern int fparse_yyparse(void);
extern void fparse_yyerror(char *);
extern int fparse_yylex(void);
#ifdef MNEM
extern void fparse_free_dat_int(int, void *, int);
#else
extern void fparse_free_dat(void *, int);
#endif

/* external stuff. user defines this, if e wants to change it. default
 * is to just print on stderr. */
extern void fexpr_error(char *);

#define FBUF_SIZE 1024

#if defined (FCOMPAT)
/* compatibility stuff. don't use this, as it might be removed soon. */

struct expression {
  ftype type;
  struct fcode *code;
  struct ffunctable *funcs;
};

typedef char *expr_var;	/* just the name */

extern struct expression *expr_new(ftype type);
extern void     expr_free(struct expression *);
extern char    *expr_parse(struct expression *expr, char *e);
extern double   expr_evaluate(struct expression *e);
extern void     expr_evaluate_some(struct expression *e, expr_var v,
				   double min, double max, int npoints,
				   double *buffer);
extern void     expr_evaluate_complex(struct expression *e, fcomplex *op);
extern expr_var expr_create_variable(struct expression *e, char *name,
				     double val);
extern void     expr_set_variable(struct expression *e, expr_var v,
				  double val);
extern void     expr_set_variable_complex(struct expression *e, expr_var v,
					  fcomplex *val);
extern double   expr_get_variable(struct expression *e, expr_var v);
extern void     expr_get_variable_complex(struct expression *e, expr_var v,
					  fcomplex *op);
extern void     expr_copy_variables(struct expression *source, struct expression *dest);

#endif /* fcompat */

#endif /* fexpr_h_included */
