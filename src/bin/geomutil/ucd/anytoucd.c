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

/* File:	anytoucd.c:
   Author:	Charlie Gunn originally
                
   read a OOGL object on stdin, and write ucd format on stdout.
*/
#include "vec4.h"
#include "geom.h"
#include "3d.h"
#include "polylistP.h"
#include "plutil.h"
#include <stdio.h>
#include "time.h"

#define MAXV	64

/*
#define DEBUG
*/

extern int push(Poly *PP);	/* From stack.c */
extern Poly *getstack();
extern int make_tform(HPoint3 *p1, HPoint3 *p2, HPoint3 *p3, Transform m);
extern int initstack(void);
extern int getsize(void);

#define CAVE	1
#define	VEX	0
static int debug = 0;

#define INSIDE(line, point)	\
	((line.a*(point)->x + line.b*(point)->y + line.c) <= 0)
#define INSIDE2(line, point)	\
	(line.a*(point)->x + line.b*(point)->y + line.c) 

    struct polygon {
	int nv;
	unsigned int new:1;
	Vertex *vlist[MAXV];
	HPoint3 vlistxy[MAXV];
	short inout[MAXV];
	Transform to_xy;	
	} currpoly;

    struct line {
	float a,b,c,angle;
	};

typedef struct polygon polygon;
typedef struct line line;

void
myconvert(CP, cp)
Poly *CP;
polygon *cp;
{
    int i;
    cp->nv = CP->n_vertices;
    for (i=0; i<cp->nv; ++i)
	cp->vlist[i] = CP->v[i];
    cp->new = 1;
}

polygon *
chopup (pp, PP)
polygon *pp;
Poly *PP;
{
    int n, i ;
    int convex, orient;
    line edges[MAXV];
    double dangle, sum;
    polygon *newpoly; 

    if (debug){
	fprintf(stderr,"Entering chopup with %d vertices \n",pp->nv);
	for (i=0; i<pp->nv; ++i)
		fprintf(stderr,"%f %f %f \n",pp->vlist[i]->pt.x, pp->vlist[i]->pt.y, pp->vlist[i]->pt.z);
	}

    if (pp->nv <= 4 && pp->nv > 1)	
	{
	PP->n_vertices = pp->nv;
	for (i=0; i<pp->nv; ++i)	
	    PP->v[i] = pp->vlist[i];
	if (push(PP) == 0)
	    OOGLError(1,"anytoucd: unable to push polygon\n");
	return(NULL);
	}
    else if (pp->nv == 1) 
	{
	return(NULL);
	}

    else {		/* something to chop */

    newpoly = OOGLNew(polygon);
    n = pp->nv;
  if (pp->new)
    {
    /* need to work in the x,y plane, so compute approp. tforms */
    for (i=0; i<n; ++i)	/* may need to hunt for independent vectors */
	    if (make_tform(&pp->vlist[i]->pt, &pp->vlist[(i+1)%n]->pt, &pp->vlist[(i+2)%n]->pt, pp->to_xy) >= 0) break;;
    TmInvert(pp->to_xy, pp->to_xy);

    for (i=0; i<pp->nv; ++i)
	HPt3Transform(pp->to_xy, &pp->vlist[i]->pt, &pp->vlistxy[i]);
    pp->new = 0;
    }


    {
    double dx, dy;
    for (i=0; i<pp->nv; ++i)
	{
	dx = pp->vlistxy[(i+1)%n].x - pp->vlistxy[i].x;	
	dy = pp->vlistxy[(i+1)%n].y - pp->vlistxy[i].y;	
	edges[i].a = dy;
	edges[i].b = -dx;
	edges[i].c = -(edges[i].a*pp->vlistxy[i].x+edges[i].b*pp->vlistxy[i].y);
	edges[i].angle = atan2(dy,dx);
    if (debug)
	fprintf(stderr,"edge %d: angle %f\n",i,edges[i].angle);
	}
    }

    /* armed with this info, identify concave and convex bndy pts*/
    for (sum = 0.0, i=0; i<pp->nv; ++i)
	{
	dangle = edges[i].angle - edges[(i-1+n)%n].angle ;
	/* clip to (-PI, +PI) */
	while (dangle < -PI) 	dangle += 2*PI;
	while (dangle > PI) 	dangle -= 2*PI;
	sum += dangle;
	if (dangle > 0)		pp->inout[i] = VEX;
	else 			pp->inout[i] = CAVE;
	}
	
    if (debug)
        fprintf(stderr,"Total angle change is %f\n",sum);
    if (sum > 0 ) orient = 1;
    else 	  orient = -1;
    for (convex = 1, i=0; i<pp->nv; ++i)
	{
	if (orient == -1)	
	    {
    	    pp->inout[i] = 1 - pp->inout[i];
	    edges[i].a *= -1;
	    edges[i].b *= -1;
	    edges[i].c *= -1;
	    }
	if (pp->inout[i] == CAVE)	convex = 0;
	}

    if (debug)
	if (convex) fprintf(stderr,"Polygon is convex\n");

    /* next look for triangles to chop off */
    {
    line cutcorner;
    double dx, dy;	
    int cutoff, j, jj, oc, nc;

    for (nc = 0, oc=0; oc<pp->nv-1; ++oc, ++nc)
	{
	/* copy out current vertex */
	newpoly->vlist[nc] = pp->vlist[oc];
	HPt3Copy(&pp->vlistxy[oc], &newpoly->vlistxy[nc]);
	if (pp->inout[(oc+1)%n] == VEX)
	  {
	  cutoff = 1;		/* default is to cut if off */
	  if (!convex)
	    {
	    dx = pp->vlistxy[(oc+2)%n].x - pp->vlistxy[oc].x;
	    dy = pp->vlistxy[(oc+2)%n].y - pp->vlistxy[oc].y;
	    /* orient this line to run from v2 to v0, to agree with edges */
	    cutcorner.a = -dy*orient;
	    cutcorner.b = dx*orient;
	    cutcorner.c = -(cutcorner.a*pp->vlistxy[(oc+2)%n].x+cutcorner.b*pp->vlistxy[(oc+2)%n].y);
	    cutcorner.angle = -atan2(dy,dx);
	    /* check for concave pts inside this triangle */
	    /* don't check the vertices of this triangle */
    	      for ( j=0, jj=oc+3; j<pp->nv-3 && cutoff; ++j,++jj)
		{
		if (jj == pp->nv) jj = 0;
		if ( pp->inout[jj] == CAVE )
		    {
		    if (INSIDE(cutcorner,&pp->vlistxy[jj]) &&
			INSIDE(edges[oc], &pp->vlistxy[jj]) &&
			INSIDE(edges[(oc+1)%n], &pp->vlistxy[jj]))
			{
    if (debug)
	fprintf(stderr,"vertex %d is inside cutoff corner %d %d %d\n",jj, oc, oc+1, oc+2);
			cutoff = 0; 
			}
		    }
		}
	      }
	    if (cutoff)		/* can cut this corner off */
	      {
	      int iv0, iv1, iv2, iv3;
	      if (convex & (oc < pp->nv-2 ) )	
	      /* can cut off 4 vertices */
	        {
		iv0 = oc;
		iv1 = (oc+1)%n;
		iv2 = (oc+2)%n;
		iv3 = (oc+3)%n;
		PP->n_vertices = 4;
		PP->v[0] = pp->vlist[iv0];
		PP->v[1] = pp->vlist[iv1];
		PP->v[2] = pp->vlist[iv2];
		PP->v[3] = pp->vlist[iv3];
		if (push(PP) == 0)
	    	    OOGLError(1,"anytoucd: unable to push polygon\n");
	        oc++;
	        oc++;
		}
	      else 
	        {
		iv0 = oc;
		iv1 = (oc+1)%n;
		iv2 = (oc+2)%n;
		PP->n_vertices = 3;
		PP->v[0] = pp->vlist[iv0];
		PP->v[1] = pp->vlist[iv1];
		PP->v[2] = pp->vlist[iv2];
		if (push(PP) == 0)
	    	    OOGLError(1,"anytoucd: unable to push polygon\n");
	        oc++;
		}
	
	      }
	  }
	}	
    while (oc < pp->nv)
	{
	newpoly->vlist[nc] = pp->vlist[oc];
	HPt3Copy(&pp->vlistxy[oc], &newpoly->vlistxy[nc]);
	oc++;
	nc++;
	}
    newpoly->new = 0;
    newpoly->nv = nc;

    /* if nothing got chopped, or if what's left
     * has only 1 or 2 vertices */
    if (newpoly->nv == pp->nv || newpoly->nv <= 2)
	{
/*
	fprintf(stderr,"chopup:Unable to cut a corner\n");
*/
	OOGLFree(newpoly);
	return(NULL);
	}

    else	return(newpoly);
    }
  }
}

Poly *CP, thisPoly;

int main(int argc, char **argv)
{
    polygon *ncp, *cp;
    int pcountold;
    Geom *thisgeom;
    PolyList *thispl;
    Vertex *vlist[4];
    Transform id;

    TmIdentity(id);
    thisgeom = GeomFLoad(iobfileopen(stdin), NULL);
    thispl = (PolyList *)AnyToPL(thisgeom, id);
    if (initstack() == 0) 
	OOGLError(1,"anytoucd: unable to init stack\n");

    cp = OOGLNew(polygon);
    CP = &thisPoly;
    /* at most 4 vertices in each face */
    for (pcountold = 0; pcountold < thispl->n_polys; ++pcountold)	
	{
	myconvert(&thispl->p[pcountold], cp);
	*CP = thispl->p[pcountold];
        CP->v = vlist;
	(void)OOGLNew(polygon);
   	while ( (ncp = chopup(cp, CP)) != NULL ) 
	    {
        if (debug)
	    fprintf(stderr,"Chop up: calling again\n");

	    OOGLFree(cp);
	    cp = ncp; 
	    }
	}
    /* now get the list of polygons from the stack */
    thispl->n_polys = getsize();
    thispl->p = getstack();

    /* now print out the stuff in ucd format */
    /* GeomFSave(thispl, stdout, NULL); */
    {
    FILE *fp = stdout;
    int num_nodes, 
	num_node_data_comp = 0, 
	node_data_comp[2],
	num_cells, 
	num_cell_data_comp = 0, 
	total_node_comp = 0,
	total_cell_comp = 0,
	cell_data_comp[2],
	i,n;
    Poly *p;
    Vertex *v, **vp;

    num_nodes = thispl->n_verts;
    num_cells = thispl->n_polys;
    if (thispl->geomflags & PL_HASVCOL) {
	node_data_comp[0] = 4;	/* ColorA's : 4 component */
	num_node_data_comp += 1;
	total_node_comp += 4;
	}
    if (thispl->geomflags & PL_HASVN) {
	node_data_comp[num_node_data_comp] = 3;
	num_node_data_comp += 1;
	total_node_comp += 3;
	}
#ifdef FOURD
    if (thispl->geomflags & VERT_4D)	{
	node_data_comp[num_node_data_comp] = 1;	/* ColorA's : 4 component */
	num_node_data_comp += 1;
	total_node_comp += 1;
	}
#endif
    if (thispl->geomflags & PL_HASPCOL) {
	cell_data_comp[0] = 4;	/* ColorA's : 4 component */
	num_cell_data_comp += 1;
	total_cell_comp += 4;
	}
    if (thispl->geomflags & PL_HASPN) {
	cell_data_comp[num_cell_data_comp] = 3;
	num_cell_data_comp += 1;
	total_cell_comp += 3;
	}
    {
    char *timestring;
    long mytime;
    time_t myt;
    myt = time(&mytime);
    timestring = ctime(&myt);
    fprintf(fp,"#  Created by anytoucd on %s \n",timestring);
    }

    fprintf(fp,"%d %d %d %d 0\n",num_nodes, num_cells, total_node_comp, total_cell_comp);
    /* write out vertices */
    /* what to do about 4D vertices? */
    for (v = thispl->vl, i=0; i<thispl->n_verts; ++i, ++v)
#ifdef FOURD
	fprintf(fp,"%d %g %g %g\n",i+1,v->pt.x, v->pt.y, v->pt.z);
#else
	{
	float w = v->pt.w;
	if (w) w = 1.0/w;
	fprintf(fp,"%d %g %g %g\n",i+1,w*v->pt.x, w*v->pt.y, w*v->pt.z);
	}
#endif
	

    /* write out faces */
    for (i=0, p = thispl->p; i<thispl->n_polys; ++i, ++p)	{
	fprintf(fp,"%d 1 ",i+1);
	n = p->n_vertices;
	if (n==2)  fprintf(fp,"line ");	/* is this right? */
	else if (n==3)  fprintf(fp,"tri ");
	else if (n==4) fprintf(fp, "quad ");
	/* else signal error */
	else return 1;
        for(vp = p->v; --n >= 0; vp++)
                fprintf(fp, "%d ", (int)((*vp) - thispl->vl + 1));
	fprintf(fp,"\n");
	}
	
    if (num_node_data_comp) {
      fprintf(fp,"%d ", num_node_data_comp);
      for (i=0; i<num_node_data_comp; ++i)
            fprintf(fp,"%d ", node_data_comp[i]);
      fprintf(fp,"\n");

      if (thispl->geomflags & PL_HASVCOL)	fprintf(fp,"rgba, NULL\n");
      if (thispl->geomflags & PL_HASVN)	fprintf(fp,"normal, NULL\n"); 
      if (thispl->geomflags & VERT_4D)	fprintf(fp,"w, NULL\n");

      for (v = thispl->vl, i=0; i<thispl->n_verts; ++i, ++v)	{
        /* print vertex colors */
	fprintf(fp,"%d ", i+1);
        if (thispl->geomflags & PL_HASVCOL)		{
	    fprintf(fp,"%g %g %g %g", v->vcol.r, v->vcol.g, v->vcol.b, v->vcol.a);
	    }
        /* print vertex normals */
        if (thispl->geomflags & PL_HASVN)		{
	    fprintf(fp," %g %g %g", v->vn.x, v->vn.y, v->vn.z);
	    }
#ifdef FOURD
        /* print fourth coordinate if present */
        if (thispl->geomflags & VERT_4D)	{
	    fprintf(fp," %g", v->pt.w);
	    }
#endif
	fprintf(fp,"\n");
	}
     }

    /* now process cell data */
    if (num_cell_data_comp) {
      fprintf(fp,"%d ", num_cell_data_comp);
      for (i=0; i<num_cell_data_comp; ++i)
            fprintf(fp,"%d ", cell_data_comp[i]);
      fprintf(fp,"\n");
	
      /* print labels */
      if (thispl->geomflags & PL_HASPCOL) fprintf(fp,"rgba, NULL\n");
      if (thispl->geomflags & PL_HASPN)   fprintf(fp,"normal, NULL\n");

      for (p = thispl->p, i=0; i<thispl->n_polys; ++i, ++p)	{
	fprintf(fp,"%d ",i+1);
        /* print face colors */
        if (thispl->geomflags & PL_HASPCOL)		{
	    fprintf(fp," %g %g %g %g", p->pcol.r, p->pcol.g, p->pcol.b, p->pcol.a);
	}
        /* print face normals */
        if (thispl->geomflags & PL_HASPN)		{
	    fprintf(fp," %g %g %g", p->pn.x, p->pn.y, p->pn.z);
	    }
	fprintf(fp,"\n");
        }
      }
    }

    return 0;
}


