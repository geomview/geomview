/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

/*  cplxmodule.c:  external module for geomview
 *                    to view graphs of complex functions
 *  version II: July 1993        by Olaf Holt
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "forms.h"
#include "xforms-compat.h"
#include "cplxpanels2.h"
#define FCOMPAT		/* fexpr old-style compatibility */
#include "fexprnew.h"

#define REAL 0
#define IMAGINARY 1
#define BOTH 2
#define PI 3.14
#define RECTANGULAR 1
#define POLAR 2
#define USERCOORD 3

extern int ok_expr, ok_sexpr, ok_texpr;
extern struct expression *e, *es, *et;

int debug = 0;
int nodefault = 0;
long winid;
int usercoordpanelisopen = 0;

const char *functionstring, *sfunctionstring, *tfunctionstring;
double a = 1.0, b = 0.5;
double RECTx0 = -PI, RECTx1 = PI, RECTy0 = -PI, RECTy1 = PI;
double POLARx0 = 0, POLARx1 = 1, POLARy0 = 0, POLARy1 = 2*PI;
double USERx0 = -1, USERx1 = 1, USERy0 = -1, USERy1 = 1;
int RECTm1 = 20, RECTm2 = 20;
int POLARm1 = 20, POLARm2 = 20;
int USERm1 = 20, USERm2 = 20;
/* int whichgraph = REAL;  */
int	SHOWREALFLAG = 0;
int	SHOWIMAGFLAG = 0;
int SHOW4DFLAG = 1;
int COORDTYPEFLAG = RECTANGULAR;

void setexpression(const char *str);
void setsexpression(const char *str);
void settexpression(const char *str);

main( argc, argv )
int argc; char ** argv;
{
	char s[80];

	if(argc > 1) {
		if ( argv[1][1] == 'd' )  { /* debugging turned on */
			fprintf(stderr, "Debugging on.\n");
			debug = 1;
		} else if (argv[1][1] == 'n' ) { /* do not load a default function */
			nodefault = 1;
		} else if (argv[1][1] == 's' ) { /* start at lowest grid size */
			RECTm1 = 2; RECTm2 = 2;
		} else {
			fprintf(stderr, "Usage:  cplxmodule [-n no defaults]\n");
		}
	}

	FL_INITIALIZE("Cplxview");
	create_the_forms();

	{
	    char fname[512];
	    sprintf(fname, "%.470s/NDview/cplxview/text/cplxhelp.txt",
		getenv("GEOMDATA"));
	    if((fl_load_browser(cplxhelpbrowser,fname)) == 0)
		    fprintf(stderr, "Couldn't load complex help file.\n");
	}
	fl_set_slider_value(abox,a);
	fl_set_slider_value(bbox,b);
	fl_set_counter_value(x0box,RECTx0);
	fl_set_counter_value(x1box,RECTx1);
	fl_set_counter_value(y0box,RECTy0);
	fl_set_counter_value(y1box,RECTy1);
	sprintf(s,"%d",RECTm1);
	fl_set_input(m1box,s);
	sprintf(s,"%d",RECTm2);
	fl_set_input(m2box,s);
	fl_set_button(realrangebutton, 1);
	fl_set_button(imagrangebutton, 1);
	fl_set_button(rectcoordbox, 1);

/* set up the n-d viewing environment */
	printf("(load NDview/cplxview/scripts/4d.cplx)\n");

    winid = fl_show_form(cplxmainpanel,FL_PLACE_SIZE,TRUE,"cplx function viewer");
    winset(winid);
    winpop();


	initcompute();

	printf("(load NDview/cplxview/scripts/4d.colorcplx)\n");

	sfunctionstring = strdup("a*u");
	setsexpression(sfunctionstring);
	fl_set_input(suvbox,sfunctionstring);
	tfunctionstring = strdup("b*v");
	settexpression(tfunctionstring);
	fl_set_input(tuvbox,tfunctionstring);
	if ( nodefault )
		functionstring = strdup("");
	else {
		functionstring = strdup("exp(z)");
		fl_set_input(functionbox,functionstring);
		functioncallback(functionbox, 0);  /* initializes the function */
	}

	while(1) {  fl_do_forms(); }
}

void rangelabelcallback(FL_OBJECT *obj, long i)
{
    winid = fl_show_form(rangelabelpanel,FL_PLACE_POSITION,TRUE,"Range");
    winset(winid);
    winpop();
}

void rangelabelhidecallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(rangelabelpanel);
}

void meshlabelcallback(FL_OBJECT *obj, long i)
{
    winid = fl_show_form(meshlabelpanel,FL_PLACE_POSITION,TRUE,"mesh size");
    winset(winid);
    winpop();
}

void meshlabelhidecallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(meshlabelpanel);
}

void sliderscallback(FL_OBJECT *obj, long i)
{
    winid = fl_show_form(sliderspanel,FL_PLACE_POSITION,TRUE,"function parameters");
    winset(winid);
    winpop();
}

void slidershidecallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(sliderspanel);
}

void coordtypecallback(FL_OBJECT *obj, long i)
{
    winid = fl_show_form(coordtypepanel,FL_PLACE_POSITION,TRUE,"domain definition");
    winset(winid);
    winpop();
}

void coordtypehidecallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(coordtypepanel);
}

void coordcallback(FL_OBJECT *obj, long i)
{
	if ( i == 1)  {  /* rectangular coords are specified */
		COORDTYPEFLAG = RECTANGULAR;
		if (usercoordpanelisopen) usercoordhidecallback(NULL, 0);
		fl_freeze_form(cplxmainpanel);
		fl_set_button(rectcoordbox, 1);
		fl_set_button(polarcoordbox, 0);
		fl_set_button(usercoordbox, 0);
		fl_set_object_label(x0box, "min(Re(z))");
		fl_set_object_label(x1box, "max(Re(z))");
		fl_set_object_label(y0box, "min(Im(z))");
		fl_set_object_label(y1box, "max(Im(z))");
		fl_set_counter_value(x0box,RECTx0);
		fl_set_counter_value(x1box,RECTx1);
		fl_set_counter_value(y0box,RECTy0);
		fl_set_counter_value(y1box,RECTy1);
		fl_unfreeze_form(cplxmainpanel);
		domaincallback(NULL, 0);
	} else if ( i == 2)  {  /* polar coords are specified */
		COORDTYPEFLAG = POLAR;
		if (usercoordpanelisopen) usercoordhidecallback(NULL, 0);
		fl_freeze_form(cplxmainpanel);
		fl_set_button(rectcoordbox, 0);
		fl_set_button(polarcoordbox, 1);
		fl_set_button(usercoordbox, 0);
		fl_set_object_label(x0box, "min(r)");
		fl_set_object_label(x1box, "max(r)");
		fl_set_object_label(y0box, "min(theta)");
		fl_set_object_label(y1box, "max(theta)");
		fl_set_counter_value(x0box,POLARx0);
		fl_set_counter_value(x1box,POLARx1);
		fl_set_counter_value(y0box,POLARy0);
		fl_set_counter_value(y1box,POLARy1);
		fl_unfreeze_form(cplxmainpanel);
		sfunctionstring = strdup("u*cos(v)");
		setsexpression(sfunctionstring);
		tfunctionstring = strdup("u*sin(v)");
		settexpression(tfunctionstring);
		domaincallback(NULL, 0);
	} else if ( i == 3)  {  /* user-def coords are specified */
		COORDTYPEFLAG = USERCOORD;
	    winid = fl_show_form(usercoordpanel,FL_PLACE_POSITION,TRUE,"user defined coordinates");
	    winset(winid);
	    winpop();
		usercoordpanelisopen = 1;
		usercoordcallback(NULL, 0);
		fl_freeze_form(cplxmainpanel);
		fl_set_button(rectcoordbox, 0);
		fl_set_button(polarcoordbox, 0);
		fl_set_button(usercoordbox, 1);
		fl_set_object_label(x0box, "min(u)");
		fl_set_object_label(x1box, "max(u)");
		fl_set_object_label(y0box, "min(v)");
		fl_set_object_label(y1box, "max(v)");
		fl_set_counter_value(x0box,USERx0);
		fl_set_counter_value(x1box,USERx1);
		fl_set_counter_value(y0box,USERy0);
		fl_set_counter_value(y1box,USERy1);
		fl_unfreeze_form(cplxmainpanel);
		domaincallback(NULL, 0);
	} else {
		fprintf(stderr, "Please check source: #28944\n");
	}
}

void usercoordhidecallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(usercoordpanel);
	usercoordpanelisopen = 0;
}

void usercoordcallback(FL_OBJECT *obj, long i)
{
	if ( i == 0) {  /* generic call from inside the code */
		sfunctionstring = fl_get_input(suvbox);
		setsexpression(sfunctionstring);
		tfunctionstring = fl_get_input(tuvbox);
		settexpression(tfunctionstring);
	} else if ( i == 1)  {  /* s(u,v) is specified */
		sfunctionstring = fl_get_input(suvbox);
		setsexpression(sfunctionstring);
		computegraph();
	} else if ( i == 2)  {  /* t(u,v) is specified */
		tfunctionstring = fl_get_input(tuvbox);
		settexpression(tfunctionstring);
		computegraph();
	} else {
		fprintf(stderr, "Please check source: #23948\n");
	}
}

void helpcallback(FL_OBJECT *obj, long i)
{
    winid = fl_show_form(cplxhelppanel,FL_PLACE_POSITION,TRUE,"cplxviewerhelp");
    winset(winid);
    winpop();
}

void quitcallback(FL_OBJECT *obj, long i)
{
	/*  Note:  some kind of graceful exit?  */
		exit(0);
}

void quitcplxhelpcallback(FL_OBJECT *obj, long i)
{
	fl_hide_form(cplxhelppanel);
}

void functioncallback(FL_OBJECT *obj, long i)
{
	functionstring = fl_get_input(functionbox);
/*	printf("new function:  %s\n",functionstring); */
/*	fl_set_object_label(messagebox,"note: not implemented :( "); */
	setexpression(functionstring);
	computegraph();
}

void slidercallback(FL_OBJECT *obj, long i)
{
	switch ( i ) {
		case 1: 
			a = (double) fl_get_slider_value(abox);
/*			printf("a = %g\n",a);   */
			break;
		case 2: 
			b = (double) fl_get_slider_value(bbox);
/*			printf("b = %g\n",b);   */
			break;
	}
	computegraph();
}

void domaincallback(FL_OBJECT *obj, long i)
{
	char str[80];
	int errflag = 0;
	double x0, x1, y0, y1;

	switch ( i ) {
		case 0:  /* called just to check values */
			if ( RECTx0 > RECTx1 )
				RECTx0 = RECTx1;
			if ( RECTy0 > RECTy1 )
				RECTy0 = RECTy1;
			if ( POLARx0 < 0 )
				POLARx0 = 0;
			if ( POLARx0 > POLARx1 )
				POLARx0 = POLARx1;
			if ( POLARy0 > POLARy1 )
				POLARy0 = POLARy1;
			if ( USERx0 > USERx1 )
				USERx0 = USERx1;
			if ( USERy0 > USERy1 )
				USERy0 = USERy1;
			break;
		case 1:
			x1 = fl_get_counter_value(x1box);
			x0 = fl_get_counter_value(x0box);
			if ( x1 < x0) {
				fl_set_object_label(messagebox, "min must be less than max!");
				errflag = 1;
				x1 = x0;
			}
			if (errflag) {
				;  /* don't update values */
			} else  {
			if( COORDTYPEFLAG == RECTANGULAR ) {
				RECTx1 = x1;
			} else if( COORDTYPEFLAG == POLAR ) {
				POLARx1 = x1;
			} else if( COORDTYPEFLAG == USERCOORD ) {
				USERx1 = x1;
			}
			fl_set_counter_value(x1box,x1);
			}
/*			printf("x1 = %g\n",x1);             */
			break;
		case 2:
			y1 = fl_get_counter_value(y1box);
			y0 = fl_get_counter_value(y0box);
			if ( y1 < y0) {
				fl_set_object_label(messagebox, "min must be less than max!");
				errflag = 1;
			}
			if (errflag) {
				;  /* don't update values */
			} else  {
			if( COORDTYPEFLAG == RECTANGULAR ) {
				RECTy1 = y1;
			} else if( COORDTYPEFLAG == POLAR ) {
				POLARy1 = y1;
			} else if( COORDTYPEFLAG == USERCOORD ) {
				USERy1 = y1;
			}
			fl_set_counter_value(y1box,y1);
			}
/*			printf("y1 = %g\n",y1);             */
			break;
		case 3:
			x0 = fl_get_counter_value(x0box);
			x1 = fl_get_counter_value(x1box);
			if( COORDTYPEFLAG == POLAR ) {
				if ( x0 < 0 ) {
					fl_set_object_label(messagebox, "r must be positive");
					errflag = 1;
				}
			}
			if ( x1 < x0) {
				fl_set_object_label(messagebox, "min must be less than max!");
				errflag = 1;
			}
			if (errflag) {
				;  /* don't update values */
			} else  {
			if( COORDTYPEFLAG == RECTANGULAR ) {
				RECTx0 = x0;
			} else if( COORDTYPEFLAG == POLAR ) {
				POLARx0 = x0;
			} else if( COORDTYPEFLAG == USERCOORD ) {
				USERx0 = x0;
			}
			fl_set_counter_value(x0box,x0);
			}
/*			printf("x0 = %g\n",x0);             */
			break;
		case 4:
			y0 = fl_get_counter_value(y0box);
			y1 = fl_get_counter_value(y1box);
			if ( y1 < y0) {
				fl_set_object_label(messagebox, "min must be less than max!");
				errflag = 1;
			}
			if (errflag) {
				;  /* don't update values */
			} else  {
			if( COORDTYPEFLAG == RECTANGULAR ) {
				RECTy0 = y0;
			} else if( COORDTYPEFLAG == POLAR ) {
				POLARy0 = y0;
			} else if( COORDTYPEFLAG == USERCOORD ) {
				USERy0 = y0;
			}
			fl_set_counter_value(y0box,y0);
			}
/*			printf("y0 = %g\n",y0);             */
			break;
		default:
			fprintf(stderr, "Please check source: #2394823\n");
	}
	computegraph();
}

void meshcallback(FL_OBJECT *obj, long i)
{
	char str[80];
	int m1, m2;

	switch ( i ) {
		case 1: 
			m1 = atof(fl_get_input(m1box));
			if ( m1 < 2) {
				fl_set_object_label(messagebox, "mesh size must be at least 2");
				m1 = 2;
				sprintf(str,"%d",m1);
				fl_set_input(m1box,str);
			}				
			if ( COORDTYPEFLAG == RECTANGULAR )   RECTm1 = m1;
			else if ( COORDTYPEFLAG == POLAR )   POLARm1 = m1;
			else if ( COORDTYPEFLAG == USERCOORD )   USERm1 = m1;
			break;
		case 2: 
			m2 = atof(fl_get_input(m2box));
			if ( m2 < 2) {
				fl_set_object_label(messagebox, "mesh size must be at least 2");
				m2 = 2;
				sprintf(str,"%d",m2);
				fl_set_input(m2box,str);
			}				
			if ( COORDTYPEFLAG == RECTANGULAR )   RECTm2 = m2;
			else if ( COORDTYPEFLAG == POLAR )   POLARm2 = m2;
			else if ( COORDTYPEFLAG == USERCOORD )   USERm2 = m2;
			break;
	}
	computegraph();
}

void rangecallback(FL_OBJECT *obj, long i)
{
	if ( i == 1 ) {  /* show the real part */
	  if ( fl_get_button(obj) ) { /* the button is pushed */
		SHOWREALFLAG = 1;
		computegraph();
      	fl_set_object_label(messagebox,"real part displayed"); 
	  } else {
		SHOWREALFLAG = 0;
		computegraph();
      	fl_set_object_label(messagebox,"real part not displayed"); 
	  }
	} else if ( i == 2 ) { /* show the imaginary part */
	  if ( fl_get_button(obj) ) { /* the button is pushed */
		SHOWIMAGFLAG = 1;
		computegraph();
      	fl_set_object_label(messagebox,"imaginary part displayed"); 
	  } else {
		SHOWIMAGFLAG = 0;
		computegraph();
      	fl_set_object_label(messagebox,"imaginary part not displayed"); 
	  }
	} else if (i == 3 ) {   /* show in 4D */
	  if ( fl_get_button(obj) ) { 
		SHOW4DFLAG = 1;
		computegraph();
      	fl_set_object_label(messagebox,"4d graph displayed"); 
	  } else {
		SHOW4DFLAG = 0;
		computegraph();
      	fl_set_object_label(messagebox,"4d graph not displayed"); 
	  }
	} else {
		fprintf(stderr, "Please check source: #8275983\n");
	}
}

void setexpression(const char *str)
{
  char *s;
  char errmsg[80];
  if (s=expr_parse(e,(char *)str)) {
/*    printf("Expression parsing error: %s\n",str);  */
      if( sprintf(errmsg,"Expression parsing error: %s",str) == 0)
        fprintf(stderr,"please check source: #234789\n");
      fl_set_object_label(messagebox,errmsg); 
    ok_expr = 0;
  } else
      fl_set_object_label(messagebox,"new function installed"); 
    ok_expr = 1;
}

void setsexpression(const char *str)
{
  char *s;
  char errmsg[80];
  if (s=expr_parse(es,(char *)str)) {
/*    printf("Expression parsing error: %s\n",str);  */
      if( sprintf(errmsg,"Expression s parsing error: %s",str) == 0)
        fprintf(stderr,"please check source: #134783\n");
      fl_set_object_label(messagebox,errmsg); 
    ok_sexpr = 0;
  } else
      fl_set_object_label(messagebox,"new s function installed"); 
    ok_sexpr = 1;
}

void settexpression(const char *str)
{
  char *s;
  char errmsg[80];
  if (s=expr_parse(et,(char *)str)) {
/*    printf("Expression parsing error: %s\n",str);  */
      if( sprintf(errmsg,"Expression t parsing error: %s",str) == 0)
        fprintf(stderr,"please check source: #934389\n");
      fl_set_object_label(messagebox,errmsg); 
    ok_texpr = 0;
  } else
      fl_set_object_label(messagebox,"new t function installed"); 
    ok_texpr = 1;
}
