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

#include <stdio.h>
#include <time.h>
#include "polylistP.h"
#include "hpoint3.h"
#include "mg.h"
#include "mgP.h"
#include "mgps.h"
#include "mgpsP.h"
#include "mgpstri.h"

#define TOL 0.05
#define AREA 25000
#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/*static double areaThreshold;*/
static FILE *psout;

static void smoothLine(CPoint3 *p0, CPoint3 *p1);
static void smoothTriangle(CPoint3 *p0, CPoint3 *p1, CPoint3 *p2);
/*static double triArea(double *p0, double *p1, double *p2);*/
static double cdelta(double *p0, double *p1);
static void subdivtri(double *v0, double *v1, double *v2);
static void plerp(double *v0, double *v1, double *l);
static void outtri(double *v0, double *v1, double *v2);

/* PS page initialization stuff follows */

#define PNTS		(72.0)
#define BORDPAGEMARGIN	(0.25)
#define BORDPAGEXSIZE	( 8.5-2.0*BORDPAGEMARGIN)
#define BORDPAGEYSIZE	(11.0-2.0*BORDPAGEMARGIN)

#define FULLPAGEMARGIN	(0.00)
#define FULLPAGEXSIZE	( 8.5-2.0*FULLPAGEMARGIN)
#define FULLPAGEYSIZE	(11.0-2.0*FULLPAGEMARGIN)

static double pagexsize = BORDPAGEXSIZE;
static double pageysize = BORDPAGEYSIZE;
static double pagemargin = BORDPAGEMARGIN;

void
MGPS_startPS(FILE *outf, ColorA *col, double aspect, int width, int height)
{
    double imgxinches, imgyinches;
    double pagexinches, pageyinches, pageaspect;
    double dx, dy;
    double xmargin, ymargin;
    time_t tm;

    psout = outf;
    time(&tm);
    fprintf(psout,"%%!PS-Adobe-2.0 EPSF-1.2\n");
    fprintf(psout,"%%%%Title: Geomview Snapshot\n");
    fprintf(psout,"%%%%Creator: Geomview\n");
    fprintf(psout,"%%%%CreationDate: %s", ctime(&tm));
    fprintf(psout,"%%%%For: %s\n", getenv("USER"));

    /* areaThreshold = (width*height)/(double)AREA;*/
    dx = width;
    dy = height;

    if (aspect>1.0)
    {
	pagexinches = pageysize;
	pageyinches = pagexsize;
    }
    else
    {
	pagexinches = pagexsize;
	pageyinches = pageysize;
    }
    pageaspect = pagexinches/pageyinches;
    if (aspect<=pageaspect)
    {
	imgyinches = pageyinches;
	imgxinches = imgyinches*aspect;
	xmargin = pagemargin+(pagexinches-imgxinches)/2.0;
	ymargin = pagemargin;
    }
    else 
    {
	imgxinches = pagexinches;
	imgyinches = imgxinches/aspect;
	xmargin = pagemargin;
	ymargin = pagemargin+(pageyinches-imgyinches)/2.0;
    }
    if (aspect>1.0)
	fprintf(psout,"%%%%BoundingBox: %d %d %d %d\n",
		(int)(PNTS*ymargin),
		(int)(PNTS*xmargin),
		(int)(PNTS*(ymargin+imgyinches)),
		(int)(PNTS*(xmargin+imgxinches)));
    else
	fprintf(psout,"%%%%BoundingBox: %d %d %d %d\n",
		(int)(PNTS*xmargin),
		(int)(PNTS*ymargin),
		(int)(PNTS*(xmargin+imgxinches)),
		(int)(PNTS*(ymargin+imgyinches)));

    fprintf(psout,"%%%%EndComments\n");
    fprintf(psout,"gsave\n");
    fprintf(psout,"1 setlinecap 1 setlinejoin\n");

    if (aspect>1.0)
    {
	fprintf(psout,"%f %f translate\n",0.0,PNTS*FULLPAGEYSIZE);
	fprintf(psout,"-90.0 rotate\n");
    }
    fprintf(psout,"%f %f translate\n", PNTS*xmargin, PNTS*ymargin);
    fprintf(psout,"%f %f scale\n",PNTS*imgxinches/dx,PNTS*imgyinches/dy);

    /* Now set up commands */
    fprintf(psout,"[  %% stack mark\n");
    fprintf(psout,"/poly {\n");
    fprintf(psout,"  setrgbcolor newpath moveto\n");
    fprintf(psout,"  counttomark 2 idiv { lineto } repeat closepath fill\n");
    fprintf(psout,"} bind def\n");

    fprintf(psout,"/epoly {\n");
    fprintf(psout,"  setrgbcolor newpath moveto\n");
    fprintf(psout,"  counttomark 4 sub 2 idiv { lineto } repeat closepath\n");
    fprintf(psout,"gsave fill grestore setrgbcolor setlinewidth stroke\n");
    fprintf(psout,"} bind def\n");

    fprintf(psout,"/lines {\n");
    fprintf(psout,"  setlinewidth setrgbcolor newpath moveto\n");
    fprintf(psout,"  counttomark 2 idiv { lineto } repeat stroke\n");
    fprintf(psout,"} bind def\n");
    
    fprintf(psout,"/clines {\n");
    fprintf(psout,"  setlinewidth setrgbcolor newpath moveto\n");
    fprintf(psout,"  counttomark 2 idiv { lineto } repeat closepath stroke\n");
    fprintf(psout,"} bind def\n");
    
    fprintf(psout,"/circ {\n");
    fprintf(psout,"  setrgbcolor newpath 0 360 arc fill\n");
    fprintf(psout,"} bind def\n");

    fprintf(psout,"/tri {\n");
    fprintf(psout,"  setrgbcolor newpath moveto lineto lineto\n");
    fprintf(psout,"  closepath fill\n");
    fprintf(psout,"} bind def\n");

    fprintf(psout,"/l {\n");
    fprintf(psout,"  setrgbcolor newpath moveto lineto stroke\n");
    fprintf(psout,"} bind def\n");

    fprintf(psout,"%%\n");
    fprintf(psout,"0 0 moveto %d 0 lineto %d %d lineto 0 %d lineto\n",
		width, width, height, height);
    fprintf(psout,"%g %g %g setrgbcolor closepath fill\n",
		col->r, col->g, col->b);
}

void MGPS_finishPS(void)
{
    fprintf(psout,"pop\ngrestore\nshowpage\n\n");
}

void MGPS_polyline(CPoint3 *pts, int num, double width, int *col)
{
    int i;

    if (num == 1)
    {
	fprintf(psout, "%g %g %g %g %g %g circ\n",
		pts[0].x, pts[0].y, (width + 1.0) / 2.0,
		col[0]/255.0, col[1]/255.0, col[2]/255.0);
	return;
    }
    for (i=0; i<num; i++)
	fprintf(psout, "%g %g ", pts[i].x, pts[i].y);
    fprintf(psout, "%g %g %g ", col[0]/255.0, col[1]/255.0, col[2]/255.0);
    fprintf(psout, "%g lines\n", width);
}

void MGPS_spolyline(CPoint3 *pts, int num, double width)
{
    int i;

    if (num == 1)
    {
	fprintf(psout, "%g %g %g %g %g %g circ\n",
		pts[0].x, pts[0].y, (width + 1.0) / 2.0,
		pts[0].vcol.r, pts[0].vcol.g, pts[0].vcol.b);
	return;
    }
    fprintf(psout, "%g setlinewidth\n", width);
    for (i=0; i<num-1; i++)
	smoothLine(&(pts[i]), &(pts[i+1]));
}

void MGPS_poly(CPoint3 *pts, int num, int *col)
{
    int i;

    for (i=0; i<num; i++)
	fprintf(psout, "%g %g ", pts[i].x, pts[i].y);
    fprintf(psout, "%g %g %g ", col[0]/255.0, col[1]/255.0, col[2]/255.0);
    fprintf(psout, "poly\n");
}

void MGPS_epoly(CPoint3 *pts, int num, int *col,
		double ewidth, int *ecol)
{
    int i;

    fprintf(psout, "%g ", ewidth);
    fprintf(psout, "%g %g %g ", ecol[0]/255.0, ecol[1]/255.0, ecol[2]/255.0);
    for (i=0; i<num; i++)
	fprintf(psout, "%g %g ", pts[i].x, pts[i].y);
    fprintf(psout, "%g %g %g ", col[0]/255.0, col[1]/255.0, col[2]/255.0);
    fprintf(psout, "epoly\n");
}

void MGPS_spoly(CPoint3 *pts, int num)
{
    int i;

    for (i=2; i<num; i++)
	smoothTriangle(pts, pts+i-1, pts+i);
}

void MGPS_sepoly(CPoint3 *pts, int num, double ewidth, int *ecol)
{
    int i;

    for (i=2; i<num; i++)
	smoothTriangle(pts, pts+i-1, pts+i);
    for (i=0; i<num; i++)
	fprintf(psout, "%g %g ", pts[i].x, pts[i].y);
    fprintf(psout, "%g %g %g ", ecol[0]/255.0, ecol[1]/255.0, ecol[2]/255.0);
    fprintf(psout, "%g clines\n", ewidth);
}

static void smoothLine(CPoint3 *p0, CPoint3 *p1)
{
    double x0[5], x1[5], delta;
    int num, i;

    x0[0] = p0->x; x0[1] = p0->y; 
    x0[2] = p0->vcol.r; x0[3] = p0->vcol.g; x0[4] = p0->vcol.b;
    x1[0] = p1->x; x1[1] = p1->y; 
    x1[2] = p1->vcol.r; x1[3] = p1->vcol.g; x1[4] = p1->vcol.b;
    delta = cdelta(x0, x1);
    num = max(1, delta/TOL);
    for (i=0; i<num; i++)
	fprintf(psout, "%g %g %g %g %g %g %g l\n", 
		p0->x + (p1->x - p0->x)*i/(double)num,
		p0->y + (p1->y - p0->y)*i/(double)num,
		p0->x + (p1->x - p0->x)*(i+1)/(double)num,
		p0->y + (p1->y - p0->y)*(i+1)/(double)num,
		p0->vcol.r + (p1->vcol.r - p1->vcol.r)*i/(double)num,
		p0->vcol.g + (p1->vcol.g - p1->vcol.g)*i/(double)num,
		p0->vcol.b + (p1->vcol.b - p1->vcol.b)*i/(double)num);
}

/* 
 *	pssubdivtriangle Draw a color shaded triangle by subdividing it in
 *	C code and generating many small triangles.
 */
static void smoothTriangle(CPoint3 *p0, CPoint3 *p1, CPoint3 *p2)
{
    double x0[5], x1[5], x2[5];

    x0[0] = p0->x; x0[1] = p0->y; 
    x0[2] = p0->vcol.r; x0[3] = p0->vcol.g; x0[4] = p0->vcol.b;
    x1[0] = p1->x; x1[1] = p1->y; 
    x1[2] = p1->vcol.r; x1[3] = p1->vcol.g; x1[4] = p1->vcol.b;
    x2[0] = p2->x; x2[1] = p2->y; 
    x2[2] = p2->vcol.r; x2[3] = p2->vcol.g; x2[4] = p2->vcol.b;
    subdivtri(x0,x1,x2);
}

static double cdelta(double *p0, double *p1)
{
    double dr, dg, db;

    dr = p0[2] - p1[2];
    if(dr<0)
	dr = -dr;
    dg = p0[3] - p1[3];
    if(dg<0)
	dg = -dg;
    db = p0[4] - p1[4];
    if(db<0)
	db = -db;
    return max(max(dr,dg),db);
}

#if 0
static double triArea(double *p0, double *p1, double *p2)
{
    double a, b, c, s;
    a = sqrt((p1[0]-p0[0])*(p1[0]-p0[0]) +
	     (p1[1]-p0[1])*(p1[1]-p0[1]) +
	     (p1[2]-p0[2])*(p1[2]-p0[2]));
    b = sqrt((p2[0]-p0[0])*(p2[0]-p0[0]) +
	     (p2[1]-p0[1])*(p2[1]-p0[1]) +
	     (p2[2]-p0[2])*(p2[2]-p0[2]));
    c = sqrt((p1[0]-p2[0])*(p1[0]-p2[0]) +
	     (p1[1]-p2[1])*(p1[1]-p2[1]) +
	     (p1[2]-p2[2])*(p1[2]-p2[2]));
    s = (a+b+c)/2.0;
    return sqrt(s*(s-a)*(s-b)*(s-c));
}
#endif

#define NONE	0x0
#define D0	0x1
#define D1	0x2
#define D2	0x4

static void subdivtri(double *v0, double *v1, double *v2)
{
    double d0, d1, d2;
    double i0[5];
    double i1[5];
    double i2[5];
    int code;

    d0 = cdelta(v0,v1);
    d1 = cdelta(v1,v2);
    d2 = cdelta(v2,v0);
    code = NONE;
    if (d0 > TOL)
	code |= D0;
    if (d1 > TOL)
	code |= D1;
    if (d2 > TOL)
	code |= D2;
/*
    if (triArea(v0, v1, v2) < areaThreshold)
	code = NONE;
*/
    switch(code) 
    {
	case NONE:
	    outtri(v0,v1,v2);
	    break;
	case D0:
	    plerp(v0,v1,i0);
	    subdivtri(v0,i0,v2);
	    subdivtri(v2,i0,v1);
	    break;
	case D1:
	    plerp(v1,v2,i0);
	    subdivtri(v1,i0,v0);
	    subdivtri(v0,i0,v2);
	    break;
	case D2:
	    plerp(v2,v0,i0);
	    subdivtri(v2,i0,v1);
	    subdivtri(v1,i0,v0);
	    break;
	case D0|D1:
	    plerp(v0,v1,i0);
	    plerp(v1,v2,i1);
	    subdivtri(v0,i0,v2);
	    subdivtri(v2,i0,i1);
	    subdivtri(v1,i1,i0);
	    break;
	case D1|D2:
	    plerp(v1,v2,i0);
	    plerp(v2,v0,i1);
	    subdivtri(v1,i0,v0);
	    subdivtri(v0,i0,i1);
	    subdivtri(v2,i1,i0);
	    break;
	case D2|D0:
	    plerp(v2,v0,i0);
	    plerp(v0,v1,i1);
	    subdivtri(v2,i0,v1);
	    subdivtri(v1,i0,i1);
	    subdivtri(v0,i1,i0);
	    break;
	case D2|D1|D0:
	    plerp(v0,v1,i0);
	    plerp(v1,v2,i1);
	    plerp(v2,v0,i2);
	    subdivtri(v0,i0,i2);
	    subdivtri(v1,i1,i0);
	    subdivtri(v2,i2,i1);
	    subdivtri(i0,i1,i2);
	    break;
    }
}

static void plerp(double *v0, double *v1, double *l)
{
    l[0] = (v0[0]+v1[0])/2.0;
    l[1] = (v0[1]+v1[1])/2.0;
    l[2] = (v0[2]+v1[2])/2.0;
    l[3] = (v0[3]+v1[3])/2.0;
    l[4] = (v0[4]+v1[4])/2.0;
}

static void outtri(double *v0, double *v1, double *v2)
{
    double ar, ag, ab;

    fprintf(psout,"%g %g ",v0[0],v0[1]);
    fprintf(psout,"%g %g ",v1[0],v1[1]);
    fprintf(psout,"%g %g ",v2[0],v2[1]);
    ar = (v0[2]+v1[2]+v2[2])/3.0;
    ag = (v0[3]+v1[3]+v2[3])/3.0;
    ab = (v0[4]+v1[4]+v2[4])/3.0;
    fprintf(psout,"%g %g %g tri\n",ar,ag,ab);
}
