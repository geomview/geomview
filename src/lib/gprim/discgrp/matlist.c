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
#include <stdlib.h>
#include <math.h>
#include "vec4.h"
#include "enum.h"
#include "transform.h"
#include "dgflag.h"


	extern int debug, stringent, metric;
	static float epsilon = .005;

void traverse_list();
int is_same();


struct node {
	Transform t;
	struct node *l,*r, *p;	/* left, right, parent */
	struct node *same;	/* if it's "close" to this one */
	int num;
	int type;		/* process but don't print? */
	float norm;
	char *word;
	} ;

static struct node *head;

double
getnorm(int metric, Transform t)
{
    switch (metric)	{
        case  DG_EUCLIDEAN:
	    return(sqrt(t[3][0]*t[3][0] + t[3][1]*t[3][1] + t[3][2]*t[3][2]));
	    break;
        case  DG_HYPERBOLIC:
	    if (ABS(t[3][3]) < 1.0) return(0.0);
	    return ( acosh( ABS( t[3][3])) );
	    break;
        case  DG_SPHERICAL:
	    {
	    float sum = 0;
	    int i,j;
	    for (i=0;  i<4; ++i)	
	      for (j=0; j<4; ++j)  
		sum += fabs(t[i][j] - (i==j));	
	    return(sum);
	    }
	    break;
	}
    return (double)0;
}

static
struct node *
alloc_node()
{
	struct node *n;
	n = (struct node *) malloc(sizeof(struct node) );
	if (n == NULL)	exit(printf("Unable to allocate: alloc_node\n"));
	n->p = n->l = n->r = n->same = NULL;
	n->num = 1;
	return(n);
}

int
insert_or_match_mat(mat, mode)
Transform mat;
int mode;	/* insert or match? */
{
	struct node *n = NULL, *p; 
	float d;
	struct node tnode;
	double getnorm();

	if (debug == 4)	traverse_list(head);

	if (mode & INSERT ) n = alloc_node();
	else if (mode & MATCH)  n = &tnode;
	
	else n->type = 0;

	n->norm = getnorm(metric, mat);

	TmCopy(mat, n->t);

	if (head == NULL) 	
	    {
	    if (mode & MATCH)	return(0);	/* no match */
	    else if (mode & INSERT)	{
	        head = n;
	        return(1);		/* successful insert */
	    	}
	    }
	/* ...else... */
	for (p = head; p != NULL; )	{
	    d = fabs(p->norm - n->norm);
	    if (ABS(d) < epsilon)	{		/* insert! */
		if (mode & INSERT)	{
		    /* and stick it at the end of the cluster */
		    p->num++;		/* count this one */
		    for (  ; p->same != NULL; p = p->same) ;
		    /* back up one */
		    p->same = n;		
		    n->p = p;		/* mark the parent */
		    return(1);
		    }
		else {		/* look here for a match */
		    for (  ; p != NULL; p = p->same)	
			if (is_same(p->t, n->t)) 
				return( 1 );	/* matched! */
		    return(0);
		    }
		}  
	    else if (d > 0)	{		/* go right */
		if (p->r == NULL)	{
		    if (mode & INSERT)	{
			p->r = n;
			n->p = p;
			return(1);
			}
		    else 	{	/* no match */
			return(0);		
		        } 
		    }
		else	p = p->r;		/* continue search */
		}
	    else if (d < 0)	{		/* go left */
		if (p->l == NULL)	{
		    if (mode & INSERT)    {
		        p->l = n;
		        n->p = p;
		        return(1);
		        }
		    else 	{	/* no match */
			return(0);		
			}
		    } 
		else	p = p->l;		/* continue search */
		}
	    }
	return 0;
}

int
is_same(t0, t1)
Transform t0, t1;
{
    int i, j, same = 1;

    if (stringent)	{
	float factor, fepsilon;
	Transform tt1, tt2;
	Tm3Invert(t0, tt1);
	Tm3Concat(t1,tt1,tt2);
	/* is tt2 the identity, or a multiple of the identity? */
	factor = tt2[0][0];
	fepsilon = ABS(factor * epsilon);
    	for (i=0; i<4; ++i) 
    	    for (j=0; j<4; ++j) 
		{
		/* check against identity matrix */
		if ( ABS( tt2[i][j] - factor * ((i == j) ? 1 : 0) ) > fepsilon )
		     {
		     same = 0;
		     goto OUT;
		     }
		}
	}
    else 	{
      for (i=0; i<4; ++i) 
	{
    	for (j=0; j<4; ++j) 
	    if (ABS(t0[i][j] - t1[i][j]) > epsilon) 
		{
		same = 0;
		goto OUT;
		}
	}
 	}
OUT:
    return(same); 
}

int
is_new(t)
Transform t;
{
    if ( insert_or_match_mat(t, MATCH) )
	    {
	    return(0);
	    }
    return(DG_CONSTRAINT_NEW);
}


void
traverse_list(n)
struct node *n;
{
	if (n == NULL)	return;
	traverse_list(n->l);
	fprintf(stderr,"%10f\t%d\n",n->t[3][3], n->num);
	traverse_list(n->r);
}

static void
_delete_list(n)
struct node *n;
{
 	struct node *nt, *ot;
	if (n == NULL) return;
	_delete_list(n->l);
	_delete_list(n->r);
	/* erase the cluster of equal values at this node */
 	/* first go to the end of the cluster ... */
	for (ot = n  ; ot->same != NULL; ot = ot->same) ;
	/* ... then climb back up, erasing as you go */
	if (ot != n) for ( ; ot->p != n; ) {
		nt = ot->p;
		free(ot);
		ot = nt;
		}
	/* erase the evidence of this node in its parent */
	if (n->p) {
	    if (n->p->l == n) n->p->l = NULL;
	    if (n->p->r == n) n->p->r = NULL;
	    }
	free(n);
}
	
void
delete_list()
{
    _delete_list(head);
    head = NULL;
}

