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

#include <math.h>
#include <stdlib.h>
#include "fexpr.h"

#ifndef M_E
#define M_E 2.71828182845904
#endif

void fcomplex_re(fcomplex *s,fcomplex *op)
{
  op->real = s->real;
  op->imag = 0;
}

void fcomplex_im(fcomplex *s,fcomplex *op)
{
  op->real = 0;
  op->imag = s->imag;
}

void fcomplex_abs(fcomplex *s,fcomplex *op)
{
  op->imag = 0;
  op->real = sqrt(s->real*s->real+s->imag*s->imag);
}

void fcomplex_log(fcomplex *s,fcomplex *op)
{
  /* e^(a + bi)= e^a(cos b+i sin b).
     b is argument of s, e^a is abs of s. */
  op->real = log(s->real*s->real+s->imag*s->imag)/2; /* log(sqrt(..)) */
  op->imag = atan2(s->imag,s->real);
}

void fcomplex_log10(fcomplex *s,fcomplex *op)
{
  /* log10(z) = log(z)/log(10) */
  fcomplex_log(s,op);
#ifndef M_LN10
#define M_LN10 log(10);
#endif
  op->real /= M_LN10;
  op->imag /= M_LN10;
}

void fcomplex_sqrt(fcomplex *s,fcomplex *op)
{
  fcomplex tmp;
  tmp.real = 0.5;
  tmp.imag = 0;
  fcomplex_pow(s,&tmp,op);
}

void fcomplex_exp(fcomplex *s,fcomplex *op)
{
  fcomplex tmp;
  tmp.real = M_E;
  tmp.imag = 0;
  fcomplex_pow(&tmp,s,op);
}

void fcomplex_cos(fcomplex *s,fcomplex *op)
{
  /* cos(a+bi) = cos(a)cos(bi) - sin(a)sin(bi)
     = cos(a)cosh(b) - i sin(a)sinh(b) */
  op->real = cos(s->real) * cosh(s->imag);
  op->imag = - sin(s->real) * sinh(s->imag);
}

void fcomplex_arccos(fcomplex *s,fcomplex *op)
{
  /* if w = cos z,
     z = -i*ln(w+sqrt(w*w-1)) */
  fcomplex tmp1,tmp2;
  
  /* tmp1 = w*w-1 */
  tmp1.real = s->real*s->real-s->imag*s->imag - 1;
  tmp1.imag = s->real*s->imag;
  /* tmp2 = sqrt(tmp1); */
  fcomplex_sqrt(&tmp1,&tmp2);
  /* tmp2 += w */
  tmp2.real += s->real;
  tmp2.imag += s->imag;
  /* tmp1 = ln(tmp2); */
  fcomplex_log(&tmp2,&tmp1);
  /* op = -i*tmp1 */
  op->imag = -tmp1.real;
  op->real = tmp1.imag;
}

void fcomplex_sin(fcomplex *s,fcomplex *op)
{
  /* sin(a+bi) = sin(a)cos(bi) + sin(bi)cos(a)
     = sin(a)cosh(b) + isinh(b)cos(a) */
  op->real = sin(s->real)*cosh(s->imag);
  op->imag = cos(s->real)*sinh(s->imag);
}

void fcomplex_arcsin(fcomplex *s,fcomplex *op)
{
  /* if w = sin z,
     z = -i*ln(i*w+sqrt(i*(w*w-1))) */
  fcomplex tmp1, tmp2;

  /* tmp1 = i*(w*w-1) */
  tmp1.real = -s->real*s->imag;
  tmp1.imag = s->real*s->real-s->imag*s->imag-1;
  
  /* tmp2 = sqrt(tmp1); */
  fcomplex_sqrt(&tmp1,&tmp2);
  /* tmp2 += i*w; */
  tmp2.real -= s->imag;
  tmp2.imag += s->real;
  /* tmp1 = ln(tmp2); */
  fcomplex_log(&tmp2,&tmp1);
  /* op = -i*tmp1 */
  op->imag = -tmp1.real;
  op->real = tmp1.imag;
}

void fcomplex_tan(fcomplex *s,fcomplex *op)
{
  /* tan a+bi= Sin[2a]/(Cos[2a]+Cosh[2b]) + i*Sinh[2b]/(Cos[2a]+Cosh[2b]) */
  /* or so says mathematica(probably a registered trademark of wolfram) */
  op->real = sin(s->real*2)/(cos(s->real*2)+cosh(s->imag*2));
  op->imag = sinh(s->imag*2)/(cos(s->real*2)+cosh(s->imag*2));
}

void fcomplex_arctan(fcomplex *s,fcomplex *op)
{
  /* if w = tan z,
     z = i ln(1-iw)/2 */
  fcomplex tmp1,tmp2;

  /* tmp1 = 1-iw */
  tmp1.real = 1+s->imag;
  tmp1.imag = -s->real;

  /* tmp2 = ln(tmp1); */
  fcomplex_log(&tmp2, &tmp1);
  
  /* op = i*tmp2/2 */
  op->real = -tmp2.imag/2;
  op->imag = tmp2.real/2;
}

void fcomplex_cot(fcomplex *s,fcomplex *op)
{
  /* cot = 1 / tan */
  fcomplex tmp1;

  /* tmp1 = tan s */
  fcomplex_tan(s,&tmp1);
  /* op = 1/tmp1 */
  op->real = tmp1.real/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
  op->imag = -tmp1.imag/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
}

void fcomplex_sec(fcomplex *s,fcomplex *op)
{
  fcomplex tmp1;

  fcomplex_cos(s,&tmp1);
  op->real = tmp1.real/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
  op->imag = -tmp1.imag/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
}

void fcomplex_csc(fcomplex *s,fcomplex *op)
{
  fcomplex tmp1;
  fcomplex_sin(s,&tmp1);
  op->real = tmp1.real/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
  op->imag = -tmp1.imag/(tmp1.real*tmp1.real+tmp1.imag*tmp1.imag);
}

void fcomplex_cosh(fcomplex *s,fcomplex *op)
{
  /* cosh[a + b i] = cos[b - ai] */
  fcomplex tmp1;
  tmp1.real = s->imag;
  tmp1.imag = -s->real;
  fcomplex_cos(&tmp1,op);
}

void fcomplex_sinh(fcomplex *s,fcomplex *op)
{
  /* sinh [a + b i] = i * sin[ b - a i] */
  fcomplex tmp1,tmp2;

  tmp1.real = s->imag;
  tmp1.imag = -s->real;
  fcomplex_sin(&tmp1, &tmp2);
  op->real = -tmp2.imag;
  op->imag = tmp2.real;
}

void fcomplex_tanh(fcomplex *s,fcomplex *op)
{
  /* tanh[a+bi] = i*tan[b - a i] */
  fcomplex tmp1,tmp2;

  tmp1.real = s->imag;
  tmp1.imag = -s->real;
  fcomplex_tan(&tmp1,&tmp2);
  op->real = -tmp2.imag;
  op->imag = tmp2.real;
}

void fcomplex_arccosh(fcomplex *s,fcomplex *op)
{
  /* z=cos[iw]=cosh[w]
     arccosh[z] = w = arccos[z]/i
     arccosh[z] = arccos[z]/i */
  fcomplex tmp1;
  fcomplex_arccos(s, &tmp1);
  op->real = tmp1.imag;
  op->imag = -tmp1.real;
}

void fcomplex_arcsinh(fcomplex *s,fcomplex *op)
{
  /* zi=sin[iw]=isinh[w]
     arcsinh[z] = w = arcsin[zi]/i 
     arcsinh[z] = arcsin[zi]/i */
  fcomplex tmp1,tmp2;
  /* tmp1 = i*z */
  tmp1.real = -s->imag;
  tmp1.imag = s->real;
  fcomplex_arcsin(&tmp1,&tmp2);
  /* op = tmp2/i */
  tmp2.real = tmp1.imag;
  tmp2.imag = -tmp1.real;
}

void fcomplex_arctanh(fcomplex *s,fcomplex *op)
{
  /* works same as arcsinh.. */
  fcomplex tmp1,tmp2;
  /* tmp1 = i*z */
  tmp1.real = -s->imag;
  tmp1.imag = s->real;
  fcomplex_arctan(&tmp1,&tmp2);
  /* op = tmp2/i */
  tmp2.real = tmp1.imag;
  tmp2.imag = -tmp1.real;
}

void fcomplex_floor(fcomplex *s,fcomplex *op)
{
  /* just floor the components, i guess... */
  op->real = floor(s->real);
  op->imag = floor(s->imag);
}

void fcomplex_round(fcomplex *s,fcomplex *op)
{
  op->real = rint(s->real);
  op->imag = rint(s->imag);
}

void fcomplex_ceiling(fcomplex *s,fcomplex *op)
{
  op->real = ceil(s->real);
  op->imag = ceil(s->imag);
}

void fcomplex_pow(fcomplex *base,fcomplex *exponent,fcomplex*op)
{
  double Abs, Arg;
  
  double c=exponent->real,d=exponent->imag;
  
  Abs = base->imag*base->imag + base->real*base->real;
  
  Arg = atan2(base->imag,base->real);

  if (Abs==0) { /* if we don't do this, then the log(Abs) below returns
                 * NaN, and throws everything off. */
    op->real = op->imag = 0;
  } else {
    op->real = pow(Abs,c/2.0)*cos(c*Arg + d*log(Abs)/2.0)*exp(-d*Arg);
    op->imag = pow(Abs,c/2.0)*sin(c*Arg + d*log(Abs)/2.0)*exp(-d*Arg);
  }
}
/*
cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)
cos(a+bi) = cos(a)*cosh(b) - sin(a)*i*sinh(b)

sin(a+b) = cos(a)*sin(b) + cos(b)*sin(a)
sin(a+bi) = cos(a)*i*sinh(b) + cosh(b)*sin(a)

(a+bi)^(c+di) = (a+bi)^c*(a+bi)^(di)
 is (from mathematica):

  Power(Abs,c)*
  Cos(c*Arg + 
      d*Log(Abs))/
  Power(E,d*Arg) + 
  Complex(0,1)*Power(Abs,c)*
  Sin(c*Arg + 
      d*Log(Abs))/
  Power(E,d*Arg)
*/

void fcomplex_atan2(fcomplex *y, fcomplex *x, fcomplex *op)
{
  /* mathematica says: -i log((x+yi)/sqrt(x^2+y^2)),
     so i'm going to do:
     i ( log(x^2+y^2)/2 - log(x+y i) ) */
  fcomplex tmp1, tmp2, tmp3;

  /* tmp1 = x^2 + y^2 */
  tmp1.real=x->real*x->real-x->imag*x->imag+y->real*y->real-y->imag*y->imag;
  
  /* tmp2 = log(tmp1) */
  fcomplex_log(&tmp1,&tmp2);

  /* tmp1 = x + y i */
  tmp1.real = x->real - y->imag;
  tmp1.imag = x->imag + y->real;
  
  /* tmp3 = log(tmp1) */
  fcomplex_log(&tmp3,&tmp1);
  
  /* op = i (tmp2/2 - tmp3) */
  op->real = -tmp2.imag/2 + tmp3.imag;
  op->imag = tmp2.real/2 - tmp3.real;
}

/* stubs */

void fcomplex_monstub(fcomplex *arg, fcomplex *op)
{
  op->real = op->imag = atof("NaN");
}

void fcomplex_binstub(fcomplex *arg1, fcomplex *arg2, fcomplex *op)
{
  op->real = op->imag = atof("NaN");
}
