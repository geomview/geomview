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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdlib.h>
#include <math.h>
#include "fexpr.h"

#define PUSH(val) (op=(val),stack[stackpos++]=op)
#define POP() (stack[--stackpos])

double expr_evaluate(struct expression *e)
{
  double op, *stack;
  int stackpos=0;
  int vectpos;			/* where we are in the vector. */
  double temp1,temp2;

  stack = malloc (sizeof (double) *e->nelem);
  for (vectpos=0; vectpos<e->nelem; vectpos++) {
    struct expr_elem *k;

    k = &(e->elems[vectpos]);
    switch(k->op) {
    case MONOP:
      switch (k->u.monop.op) {
      case '-':
	temp1 = POP();
	PUSH(temp1);
      default:
	abort();		/* help! help! */
	break;
      };
    case BINOP:
      switch (k->u.binop.op) {
      case '+':
	temp1 = POP();
	temp2 = POP();
	PUSH(temp2+temp1);
	break;
      case '-':
	temp1 = POP();
	temp2 = POP();
	PUSH(temp1-temp2);
	break;
      case '*':
	temp1 = POP();
	temp2 = POP();
	PUSH(temp1*temp2);
	break;
      case '/':
	temp1 = POP();
	temp2 = POP();
	PUSH(temp1/temp2);
	break;
      case '^':
	temp1 = POP();
	temp2 = POP();
	PUSH(pow(temp1,temp2));
	break;
      default:
	abort();		/* help! help! */
	break;
      };
      break;
    case MONFUNC:
      temp1 = POP();
      PUSH((*k->u.monfunc.func)(temp1));
      break;
    case BINFUNC:
      temp1 = POP();
      temp2 = POP();
      PUSH((*k->u.binfunc.func)(temp1,temp2));
      break;
    case PUSHVAR:
      PUSH(e->varvals[k->u.pushvar.varnum].real);
      break;
    case PUSHNUM:
      PUSH(k->u.pushnum.number);
      break;
    default: break;
    }
  }
  op = POP();
  free(stack);
  return op;
}

void expr_evaluate_some(struct expression *e, expr_var v, double min, double max, int npoints, double *buffer)
{
  int i;
  for (i=0; i<npoints; i++) {
    expr_set_variable (e, v, min+(((max-min)*i)/(npoints-1)));
    buffer[i] = expr_evaluate(e);
  }
}

#undef PUSH
#undef POP

#define PUSH(val) (temppush=(val),stack[stackpos++]=temppush)
#define POP(loc) ((loc)=stack[--stackpos])

void expr_evaluate_complex(struct expression *e, fcomplex *op)
{
  fcomplex *stack;
  fcomplex temppush;
  int stackpos=0;
  int vectpos;			/* where we are in the vector. */
  fcomplex reg1,reg2,reg3;

  stack = malloc (sizeof (fcomplex) *e->nelem);
  for (vectpos=0; vectpos<e->nelem; vectpos++) {
    struct expr_elem *k;

    k = &(e->elems[vectpos]);
    switch(k->op) {
    case MONOP:
      switch (k->u.monop.op) {
      case '-':
	POP(reg1);
	reg1.real *= -1;
	reg1.imag *= -1;
	PUSH(reg1);
      default:
	abort();		/* help! help! */
	break;
      };
      break;
    case BINOP:
      switch (k->u.binop.op) {
      case '+':
	POP(reg1);
	POP(reg2);
	reg1.real += reg2.real;
	reg1.imag += reg2.imag;
	PUSH(reg1);
	break;
      case '-':
	POP(reg1);
	POP(reg2);
	reg1.real -= reg2.real;
	reg1.imag -= reg2.imag;
	PUSH(reg1);
	break;
      case '*':
	POP(reg1);
	POP(reg2);
	reg3.real = reg2.real*reg1.real-reg2.imag*reg1.imag;
	reg3.imag = reg2.imag*reg1.real+reg1.imag*reg2.real;
	PUSH(reg3);
	break;
      case '/':
	POP(reg1);
	POP(reg2);
	reg3.real = (reg1.real*reg2.real-reg1.imag*reg2.imag  )/ (reg2.real*reg2.real+reg2.imag*reg2.imag);
	reg3.imag = (reg1.imag*reg2.real - reg2.imag*reg1.real ) / (reg2.real*reg2.real + reg2.imag*reg2.imag);
	PUSH(reg3);
	break;
      case '^':
	POP(reg1);
	POP(reg2);
	fcomplex_pow(&reg1, &reg2, &reg3);
	PUSH(reg3);
      default:
	abort();		/* help! help! */
	break;
      };
      break;
    case MONFUNC:
      POP(reg1);
      (*k->u.monfunc.cfunc)(&reg1,&reg2);
      PUSH(reg2);
      break;
    case BINFUNC:
      POP(reg1);
      POP(reg2);
      (*k->u.binfunc.cfunc)(&reg1,&reg2,&reg3);
      PUSH(reg3);
      break;
    case PUSHVAR:
      PUSH(e->varvals[k->u.pushvar.varnum]);
      break;
    case PUSHNUM:
      reg1.real = k->u.pushnum.number;
      reg1.imag = 0;
      PUSH(reg1);
      break;
    default: break;
    }
  }
  POP(*op);
  free(stack);
}
