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
#include <math.h>
#include "wa.h"
#include "vec4.h"
#include "dgflag.h"
#include "enum.h"
#include "color.h"
#include "3d.h"
#include "ooglutil.h"
#include "discgrpP.h"


static DiscGrp *mydg;
static ColorA white = {1,1,1,.75};
/*static DiscGrpEl grpel;*/
static Transform mlist[128];
static int (*constraintfn)();
int check_big = 1, 	/* this is currently never non-zero */
	check_new = 1, 	/* ditto */
	have_matrices = 1,
	metric = DG_HYPERBOLIC,
	stringent = 0;
int 
long_cnt = 0,
	same_cnt = 0,
	far_cnt = 0, 
	print_cnt = 0, 
	store_cnt = 0;
/*static int numchunks = 1;*/
static int ngens = 0;
int debug = 0;
static char symbollist[64];
static void get_matrices();

int
getindex(c)
char c;
{
    int i;
    for (i=0; i<ngens; ++i)
	if (symbollist[i] == c) return(i);
    return(-1);
}

static void
word_to_mat(word, mat1)
char *word;
Transform mat1;
{
    int i, index;
    /*printf("# %s\n",word); */
    TmIdentity(mat1);

    for (i=0; word[i] != 0; ++i)
	{
	index = getindex(word[i]);
	if (index < 0)	{
		printf("Bad symbol\n");
		return;
		}
	TmConcat(mat1, mlist[index], mat1);
	}
}

static int 
is_big_and_new(DiscGrpEl *dgel)
{
    int is_big=0, is_n = DG_CONSTRAINT_NEW;
    if (check_new)	is_n = is_new(dgel->tform);
    if (is_n) {
	is_big = constraintfn(dgel);
	if (is_big & DG_CONSTRAINT_LONG) long_cnt++;
	if (is_big & DG_CONSTRAINT_PRINT) print_cnt++;
	if (is_big & DG_CONSTRAINT_STORE) store_cnt++;
	if (is_big & DG_CONSTRAINT_TOOFAR) far_cnt++;
	}
    else same_cnt++;
    return(is_big | is_n );
}	


static int
process(DiscGrpEl *dgel, int stacking)
{
	int is_ok = 0;

    	if (have_matrices)    	{
            is_ok = is_big_and_new(dgel);
	    /* remember that only NEW and PRINT are required to be here */
            if (is_ok & DG_CONSTRAINT_NEW )	{
	     if (!(is_ok & DG_CONSTRAINT_LONG))	{
	      if (is_ok & (DG_CONSTRAINT_STORE | DG_CONSTRAINT_PRINT)) 	{
          	if (check_new)	{
        	    insert_mat(dgel->tform);
          	    /* and put it on the stack */
          	    if (stacking) push_new_stack(dgel->word);
        	    }
              	if (is_ok & DG_CONSTRAINT_PRINT)   enumpush(dgel);
          	}
	      }
	     }
            }
     	return(is_ok);
}

static int
enumerate(int state, int depth, DiscGrpEl *dgel)
{
	int i, newstate, pval;

	if ( ! ((pval = process(dgel, 0)) & DG_CONSTRAINT_STORE)) return 0;
	if (pval & DG_CONSTRAINT_MAXLEN)	return 0;
	if (depth > MAXDEPTH) return 0;

	for (i=1; i<mydg->fsa->ngens; ++i)
	    {
	    newstate = mydg->fsa->action[state][i];
	    if ( newstate != mydg->fsa->fail)
		{
	        dgel->word[depth] = mydg->fsa->genlist[i-1][0];
	    	dgel->word[depth+1] = 0;	/* null-terminate */
		word_to_mat(dgel->word, dgel->tform);
	        enumerate(newstate, depth+1, dgel);
	   	}
	    }
	return 1;
}

static int
dumb_enumerate(int depth, DiscGrpEl *dgel)
{
	int i, j;
	char *word;
	extern char  *pop_old_stack();

	(void)depth;

	init_stack();
	process(dgel, 1);
	for (j = 0; j < MAXDEPTH ; ++j)
	    {
	    make_new_old();
	    /* are we interested in his descendents ? */
	    while ( (word = pop_old_stack()) != NULL)	{
		/* these words have length j */
		strcpy(dgel->word, word);
	    	for (i=0; i<ngens; ++i)
	    	    {
	    	    dgel->word[j] = symbollist[i];
	    	    dgel->word[j+1] = 0;	/* null-terminate */
		    word_to_mat(dgel->word, dgel->tform);
		    process(dgel, 1);
	    	    }
		}
	    }
	return 1;
}	
 
/*static char emptyword[64] = "";*/
/* 
 * hack together an enumerate routine
 */

DiscGrpElList *
DiscGrpEnum(DiscGrp *discgrp, int (* constraint)(void) )
{
    DiscGrpElList *enum_list = OOGLNewE(DiscGrpElList, "DiscGrpEnum");
    extern DiscGrpEl *enumgetstack();
    DiscGrpEl dgel;

    /* initialize the local variables */
    constraintfn = constraint;
    have_matrices = 1;
    same_cnt = 0;
    far_cnt = 0;
    print_cnt = 0;
    store_cnt = 0;
    long_cnt = 0;
    ngens = discgrp->gens->num_el;
    metric = discgrp->attributes & DG_METRIC_BITS;
    memset(dgel.word, 0, sizeof(dgel.word));
    dgel.attributes = discgrp->attributes;
    TmIdentity(dgel.tform);
    dgel.color = white;
    mydg = discgrp;

    init_out_stack();
    get_matrices();
    if (mydg->fsa) enumerate(mydg->fsa->start, 0, &dgel);
    else dumb_enumerate(0,&dgel);

    /* clean up the mess */
    delete_list();

    enum_list->num_el = enumgetsize();
    enum_list->el_list = enumgetstack();

    if (mydg->flag & DG_DEBUG)	{
	fprintf(stderr,"%d elements printed \n",print_cnt);
	fprintf(stderr,"%d elements stored \n",store_cnt);
	fprintf(stderr,"%d elements move too far \n",far_cnt);
	fprintf(stderr,"%d elements too long \n",long_cnt);
	fprintf(stderr,"%d elements duplicates \n",same_cnt);
	}
    return(enum_list);
}


static void
get_matrices()
{
	int i;
	
	for (i=0; i<mydg->gens->num_el; ++i)
		{
		symbollist[i] = mydg->gens->el_list[i].word[0];
		TmCopy(mydg->gens->el_list[i].tform, mlist[i]);
		}
	fprintf(stderr,"%d generators read\n",i);
}

