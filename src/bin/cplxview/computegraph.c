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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

#define FCOMPAT		/* Compatibility mode for fexpr library */
#include "fexprnew.h"
#include <stdio.h>

#define REAL 0
#define IMAGINARY 1
#define BOTH 2
#define RECTANGULAR 1
#define POLAR 2
#define USERCOORD 3

extern char *functionstring;
extern char *sfunctionstring;
extern char *tfunctionstring;
extern double a, b;
extern double RECTx0, RECTx1, RECTy0, RECTy1;
extern double POLARx0, POLARx1, POLARy0, POLARy1;
extern double USERx0, USERx1, USERy0, USERy1;
extern int RECTm1, RECTm2;
extern int POLARm1, POLARm2;
extern int USERm1, USERm2;
/* extern int whichgraph;  */
extern int SHOWREALFLAG, SHOWIMAGFLAG, SHOW4DFLAG;
extern int COORDTYPEFLAG;

struct expression *e, *es, *et;
expr_var avar, bvar, xvar, yvar, rvar, thetavar, svar, tvar;
expr_var a1var, b1var, a2var, b2var;
expr_var u1var, u2var, v1var, v2var, ivar, zvar;
expr_var pivar, evar;
int ok_expr = 0, ok_sexpr = 0, ok_texpr = 0;
  
void initcompute()
{
	fcomplex in;

  e = expr_new(FCOMPLEX);
  es = expr_new(FCOMPLEX);
  et = expr_new(FCOMPLEX);

  avar = expr_create_variable(e, "a", 0);
  bvar = expr_create_variable(e, "b", 0);
  a1var = expr_create_variable(es, "a", 0);
  b1var = expr_create_variable(es, "b", 0);
  a2var = expr_create_variable(et, "a", 0);
  b2var = expr_create_variable(et, "b", 0);

  xvar = expr_create_variable(e, "x", 0);
  yvar = expr_create_variable(e, "y", 0);
  rvar = expr_create_variable(e, "r", 0);
  thetavar = expr_create_variable(e, "theta", 0);
  svar = expr_create_variable(e, "s", 0);
  tvar = expr_create_variable(e, "t", 0);

  u1var = expr_create_variable(es, "u", 0);
  u2var = expr_create_variable(et, "u", 0);
  v1var = expr_create_variable(es, "v", 0);
  v2var = expr_create_variable(et, "v", 0);

  zvar = expr_create_variable(e, "z", 0);

  evar = expr_create_variable(e, "e", 0);
  in.real =2.71828183; in.imag = 0;
  expr_set_variable_complex(e,evar,&in);

  pivar = expr_create_variable(e, "pi", 0);
  in.real =3.14159265; in.imag = 0;
  expr_set_variable_complex(e,pivar,&in);

  ivar = expr_create_variable(e, "i", 0);
  in.real =0; in.imag = 1;
  expr_set_variable_complex(e,ivar,&in);
}

void computegraph(int ac, char **av)
{
  int i, j;
  register double xval, yval, newxval, newyval;
  fcomplex zval;
  double x0,x1, y0, y1;
  int m1, m2;

  if (!ok_expr) return;

	if ( COORDTYPEFLAG == RECTANGULAR ) {
		x0 = RECTx0; x1 = RECTx1;
		y0 = RECTy0; y1 = RECTy1;
		m1 = RECTm1; m2 = RECTm2;
	} else if ( COORDTYPEFLAG == POLAR ) {
  		if (!ok_sexpr || !ok_texpr) return;
		x0 = POLARx0; x1 = POLARx1;
		y0 = POLARy0; y1 = POLARy1;
		m1 = POLARm1; m2 = POLARm2;
	} else if ( COORDTYPEFLAG == USERCOORD ) {
  		if (!ok_sexpr || !ok_texpr) return;
		x0 = USERx0; x1 = USERx1;
		y0 = USERy0; y1 = USERy1;
		m1 = USERm1; m2 = USERm2;
  		expr_set_variable(es, a1var, a);
	  	expr_set_variable(es, b1var, b);
	  	expr_set_variable(et, a2var, a);
	  	expr_set_variable(et, b2var, b);
	} else {
		fprintf(stderr, "Please check source: #2394877\n");
	}
  expr_set_variable(e, avar, a);
  expr_set_variable(e, bvar, b);

  printf("( geometry cplxobj { LIST \n");

 if( SHOWREALFLAG ) {
  printf("{ appearance {material {ambient 0 0 1 diffuse 0 .5 .5} } "); 
  printf("MESH\n");
  printf("%d %d\n",m2,m1);
  for ( i = 0; i< m1; i++) {
    	xval = x0 + (double) i * ( x1 - x0 ) / ((double)m1 - 1);
    for ( j = 0; j< m2; j++) {
      yval = y0 + (double) j * ( y1 - y0 ) / ((double)m2 - 1);
	 if ( COORDTYPEFLAG == POLAR )  {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, rvar, newxval);
        expr_set_variable(e, thetavar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",newxval,newyval,zval.real);
	 } else if ( COORDTYPEFLAG == USERCOORD ) {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, svar, newxval);
        expr_set_variable(e, tvar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",newxval,newyval,zval.real);
	 } else {  /* COORDTYPEFLAG == RECTANGULAR */
        expr_set_variable(e, xvar, xval);
        expr_set_variable(e, yvar, yval);
		zval.real = xval; zval.imag = yval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",xval,yval,zval.real);
	 }
    }
  }
  printf(" }\n");
 }

 if( SHOWIMAGFLAG ) {
  printf("{ appearance {material {ambient 0 1 1 diffuse 0 .5 0} } "); 
  printf("MESH\n");
  printf("%d %d\n",m2,m1);
  for ( i = 0; i< m1; i++) {
    	xval = x0 + (double) i * ( x1 - x0 ) / ((double)m1 - 1);
    for ( j = 0; j< m2; j++) {
      yval = y0 + (double) j * ( y1 - y0 ) / ((double)m2 - 1);
	 if ( COORDTYPEFLAG == POLAR )  {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, rvar, newxval);
        expr_set_variable(e, thetavar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",newxval,newyval,zval.imag);
	 } else if ( COORDTYPEFLAG == USERCOORD ) {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, svar, newxval);
        expr_set_variable(e, tvar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",newxval,newyval,zval.imag);
	 } else {  /* COORDTYPEFLAG == RECTANGULAR */
        expr_set_variable(e, xvar, xval);
        expr_set_variable(e, yvar, yval);
		zval.real = xval; zval.imag = yval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
      	printf("%g %g %g\n",xval,yval,zval.imag);
	 }
    }
  }
  printf(" }\n");
 }

 if( SHOW4DFLAG ) {
  printf("{ appearance {material {ambient 1 1 0 diffuse .5 0 .5} } "); 
  printf("4MESH\n");
  printf("%d %d\n",m2,m1);
  for ( i = 0; i< m1; i++) {
    	xval = x0 + (double) i * ( x1 - x0 ) / ((double)m1 - 1);
    for ( j = 0; j< m2; j++) {
      yval = y0 + (double) j * ( y1 - y0 ) / ((double)m2 - 1);
	 if ( COORDTYPEFLAG == POLAR )  {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, rvar, newxval);
        expr_set_variable(e, thetavar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
        printf("%g %g %g %g\n",newxval,newyval,zval.real,zval.imag);
	 } else if ( COORDTYPEFLAG == USERCOORD ) {
		expr_set_variable(es,u1var,xval);
		expr_set_variable(es,v1var,yval);
		newxval = expr_evaluate(es);
		expr_set_variable(et,u2var,xval);
		expr_set_variable(et,v2var,yval);
		newyval = expr_evaluate(et);
        expr_set_variable(e, svar, newxval);
        expr_set_variable(e, tvar, newyval);
		zval.real = newxval; zval.imag = newyval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
        printf("%g %g %g %g\n",newxval,newyval,zval.real,zval.imag);
	 } else {  /* COORDTYPEFLAG == RECTANGULAR */
        expr_set_variable(e, xvar, xval);
        expr_set_variable(e, yvar, yval);
		zval.real = xval; zval.imag = yval;
        expr_set_variable_complex(e, zvar, &zval);
      	expr_evaluate_complex(e, &zval);
        printf("%g %g %g %g\n",xval,yval,zval.real,zval.imag);
	 }
    }
  }
  printf(" }\n");
 }

	printf(" } )\n");

  fflush(stdout);
}

