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

#include "geom.h"
#include "polylistP.h"
#include "discgrpP.h"
#include "point.h"
#include "winged_edge.h"
#include "transform.h"
#include "math.h"

static WEpolyhedron	*wepoly1, **wepoly2; 
HPoint3 DGorigin = {0,0,0,1};
HPoint3 DGrandom = {.1,.2,.3,.4};

extern void do_weeks_code();

static int
is_id(Transform t)
{
      int i,j;

      for (i=0; i<4; ++i)
        for (j=0; j<4; ++j)
            if (fabs(t[i][j] - (i == j)) > .0005) return(0);
      return(1);
}

/* make sure that the center point attached to the discrete group
   isn't fixed by some generator; if it is; redefine the center 
   point to be the center of gravity of the orbit by the generators 
*/
void
DiscGrpCheckCPoint(DiscGrp *dg)
{
    int i, cnt, fixed;
    HPoint3 tmp, out;
    float d;
    
    if (dg->gens == NULL) {
	return;
	}
    
    /* go through the generators, checking for fixed-pointed-ness */
    for  (i = 0, fixed = 0 ; i < dg->gens->num_el; ++i )	
	{
	HPt3Transform(dg->gens->el_list[i].tform, &dg->cpoint, &tmp);
	d = HPt3SpaceDistance(&dg->cpoint, &tmp, dg->attributes & DG_METRIC_BITS);
	if (fabs(d) < .0005)  {
	    fixed = 1;
	    break;
	    }
	}	

    /* no fixed points */
    if (fixed == 0) return;

    /* clean out the special bit */
    for  (i = 0 ; i < dg->gens->num_el; ++i )	
	dg->gens->el_list[i].attributes &= ~DG_TMP;

    out.x = out.y = out.z = out.w = 0.0;
    /* don't average but one of each generator, inverse pair */
    for  (cnt = 0, i = 0 ; i < dg->gens->num_el; ++i )		{
	if (!(dg->gens->el_list[i].attributes & DG_TMP))	{
	    HPt3Transform(dg->gens->el_list[i].tform, &DGrandom, &tmp);
	    HPt3Add(&tmp, &out, &out);
	    dg->gens->el_list[i].inverse->attributes |= DG_TMP;
	    cnt++;
	    }
	}
    HPt3Dehomogenize(&out, &out);
    /* return it or set cpoint?? */
    dg->cpoint = out;
}

void
DiscGrpAddInverses(DiscGrp *discgrp)
{
    int i, j, found = 0;
    Transform product;
    DiscGrpElList *newgens;
    
    /* remove all identity matrices */
    for (j=0, i=0; i<discgrp->gens->num_el; ++i)	{
	if ( !is_id(discgrp->gens->el_list[i].tform) )	{
	    /* ought to have a DiscGrpElCopy() */
	    discgrp->gens->el_list[j] = discgrp->gens->el_list[i];
	    TmCopy(discgrp->gens->el_list[i].tform, 
		discgrp->gens->el_list[j].tform);
	    j++;
	    }
	}
    /* store the new count */
    discgrp->gens->num_el = j;

    for (i=0; i<discgrp->gens->num_el; ++i)	{
      if (discgrp->gens->el_list[i].inverse == NULL)	{
	/* look for inverse among the existing generators */
	for (j=i; j<discgrp->gens->num_el; ++j)	  {
	    TmConcat(discgrp->gens->el_list[i].tform, discgrp->gens->el_list[j].tform, product);
	    if ( is_id(product) ) 	{
		discgrp->gens->el_list[i].inverse = &discgrp->gens->el_list[j];
		discgrp->gens->el_list[j].inverse = &discgrp->gens->el_list[i];
		found++;
		}
	    }
	}
      else found++;
    }

    newgens = OOGLNew(DiscGrpElList);
    newgens->num_el = 2 * discgrp->gens->num_el - found;
    newgens->el_list = OOGLNewN(DiscGrpEl, newgens->num_el );

    memcpy(newgens->el_list, discgrp->gens->el_list,
			sizeof(DiscGrpEl) * discgrp->gens->num_el);

    /* now go through looking for group elements without inverses */
    {
    char c;
    j = discgrp->gens->num_el;
    for (i=0; i<discgrp->gens->num_el; ++i)	{
	if (newgens->el_list[i].inverse == NULL)	{
	    newgens->el_list[j+i] = newgens->el_list[i];
	    /* make the symbol of the inverse be the 'other case' */
	    c = newgens->el_list[i].word[0];
	    if (c < 'a') newgens->el_list[j+i].word[0] = c + 32;
	    else newgens->el_list[j+i].word[0] = c - 32;
	    TmInvert( newgens->el_list[i].tform, newgens->el_list[j+i].tform);
	    newgens->el_list[j+i].inverse = &newgens->el_list[i];
	    newgens->el_list[i].inverse = &newgens->el_list[j+i];
	    }
	else j--;
	}
    }

    DiscGrpElListDelete(discgrp->gens);
    discgrp->gens = newgens;
}

void
DiscGrpSetupDirdom(DiscGrp *discgrp)
{
    WEpolyhedron *dd;

    if ( discgrp->nhbr_list )	{
	OOGLFree(discgrp->nhbr_list->el_list);
	OOGLFree(discgrp->nhbr_list);
	}

    /* worry about fixed points */
    DiscGrpCheckCPoint(discgrp);
    dd = DiscGrpMakeDirdom(discgrp, &discgrp->cpoint, 0);
    discgrp->nhbr_list = DiscGrpExtractNhbrs(dd);
}

/* find the group element whose 'center point' is closest to the point poi */
DiscGrpEl *
DiscGrpClosestGroupEl(DiscGrp *discgrp, HPoint3 *poi)
{
    int count, i, closeri;
    int metric;
    float min = 0, d;
    HPoint3 pt0, pt1;
    DiscGrpEl *closer = NULL, *closest = OOGLNew(DiscGrpEl);
    Transform cinv;

    TmIdentity(closest->tform);
    closest->attributes = 0;

    if (!discgrp->nhbr_list) DiscGrpSetupDirdom(discgrp);

    metric = discgrp->attributes & (DG_METRIC_BITS);

    /* iterate until we're in the fundamental domain */
    count = 0;
    closeri = -1;
    pt0 = *poi;
    while (count < 1000 && closeri != 0)	{
      for (i = 0; i<discgrp->nhbr_list->num_el; ++i)	{
	HPt3Transform(discgrp->nhbr_list->el_list[i].tform, &discgrp->cpoint, &pt1);	
 	d = HPt3SpaceDistance(&pt0, &pt1, metric);
	if (i==0) 	{
	    min = d;
	    closer = &discgrp->nhbr_list->el_list[i];
	    closeri = i;
	    }
	else if (d < min)	{
	    min = d;
	    closer = &discgrp->nhbr_list->el_list[i];
	    closeri = i;
	    }
	} 
      count++;
      if (closeri)	{
          TmConcat(closer->tform, closest->tform, closest->tform);
          /* move the point of interest by the inverse of the closest nhbr 
 	     and iterate */
          TmInvert(closest->tform, cinv);
          HPt3Transform(cinv, poi, &pt0);
	  }
/*
      if (needstuneup(closest->tform))
	tuneup(closest->tform, metric);
*/
    }
    if (is_id(closest->tform)) closest->attributes |= DGEL_IS_IDENTITY;
    return (closest);
}
    static ColorA white = {1,1,1,1};

/* return a list of group elements corresponding to the faces of the
   dirichlet domain */
DiscGrpElList *
DiscGrpExtractNhbrs( WEpolyhedron *wepoly )
{
    int 	i,j,k;
    WEface 		*fptr;	
    DiscGrpElList	*mylist;
    ColorA		GetCmapEntry();
    
    if (!wepoly)	return(NULL);

    /* should use realloc() here to take care of large groups...*/
    mylist = OOGLNew(DiscGrpElList);
    mylist->el_list = OOGLNewN(DiscGrpEl, wepoly->num_faces + 1);
    mylist->num_el = wepoly->num_faces + 1;
    
    /* include the identity matrix */
    TmIdentity( mylist->el_list[0].tform);
    mylist->el_list[0].color = white;
    mylist->el_list[0].attributes = DGEL_IS_IDENTITY;

    /* read the matrices corresponding to the faces of dirichlet domain */
    for  (fptr = wepoly->face_list, k = 1; 
	k<=wepoly->num_faces && fptr != NULL; 
	k++, fptr = fptr->next)
      {
      for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
	  /* the group elements stored in fptr are transposed! */
	  mylist->el_list[k].tform[j][i] = fptr->group_element[i][j];
      mylist->el_list[k].color = GetCmapEntry(fptr->fill_tone);
      }
    if (mylist->num_el != k) 
	OOGLError(1,"Incorrect number of nhbrs.\n");;

    return(mylist);
}

/* attempt to create a scaled copy of fundamental domain for spherical
   groups by taking weighted sum of vertices with the distinguished point */
static void
DiscGrpScalePolyList(DiscGrp *dg, PolyList *dirdom,  HPoint3 *pt0, float scale)
{
    int i, metric;
    HPoint3 tmp1, tmp2, tpt0, tpt1;
    HPt3Copy(pt0, &tpt0);
    metric = dg->attributes & DG_METRIC_BITS;
    if (metric != DG_EUCLIDEAN) 	{
	HPt3SpaceNormalize(&tpt0, metric);
    	HPt3Scale( 1.0 - scale, &tpt0, &tmp2);
    	for (i=0; i<dirdom->n_verts; ++i)	{
	    HPt3Copy(&dirdom->vl[i].pt, &tpt1);
    	    HPt3SpaceNormalize(&tpt1, metric);
            HPt3SpaceNormalize(&tpt1, metric);
	    HPt3Scale( scale, &tpt1, &tmp1);
	    HPt3Add(&tmp1, &tmp2, &dirdom->vl[i].pt);
	    }
	}
    else	{
	Transform T, TT, ITT, tmp;
	static HPoint3 average = {0,0,0,0};
	/* compute average */
        for (i=0; i<dirdom->n_verts; ++i)	
	    HPt3Add(&average, &dirdom->vl[i].pt, &average);
	HPt3Dehomogenize(&average, &average);

	TmTranslate(TT, average.x, average.y, average.z );
	TmInvert(TT, ITT);
	TmScale(T, scale, scale, scale);
	TmConcat(ITT, T, tmp);
	TmConcat(tmp, TT, tmp);
    	for (i=0; i<dirdom->n_verts; ++i)	
	    HPt3Transform(tmp, &dirdom->vl[i].pt, &dirdom->vl[i].pt);	
	}
}

Geom *small_dd, *large_dd;
Geom *
DiscGrpDirDom(DiscGrp *dg)
{
    Geom *oogldirdom;
    WEpolyhedron *dd;
    extern Geom             *WEPolyhedronToPolyList();
    Geom *mylist, *smlist;

      if (dg->flag & DG_DDBEAM)	{
        WEpolyhedron *poly = DiscGrpMakeDirdom( dg, &dg->cpoint, 0); 
        Geom *beams;
        beams = WEPolyhedronToBeams(poly, dg->scale);
        return(beams);
	}
      else	{
	float scale;
	/* first a full-size, wireframe version of dd */
        dd = DiscGrpMakeDirdom(dg, &dg->cpoint, 0);
        if (dd) {
	    oogldirdom = WEPolyhedronToPolyList(dd);
 	    scale = 1.0;
	    DiscGrpScalePolyList(dg, (PolyList *)oogldirdom, &dg->cpoint, scale);
	    large_dd = oogldirdom;
	    large_dd->ap = ApCreate(AP_DO, APF_EDGEDRAW, AP_DONT, APF_FACEDRAW, AP_END);
	    }
        else return((Geom *) NULL);
	/* next a scaled version with cusps cut off */
        dd = DiscGrpMakeDirdom(dg, &dg->cpoint, 1);
        if (dd) {
	    oogldirdom = WEPolyhedronToPolyList(dd);
	    DiscGrpScalePolyList(dg, (PolyList *)oogldirdom, &dg->cpoint, dg->scale);
	    small_dd = oogldirdom;
	    small_dd->ap = ApCreate(AP_DONT, APF_EDGEDRAW, AP_DO, APF_FACEDRAW, AP_END);
	    }
        else return((Geom *) NULL);
	smlist = GeomCreate("list", CR_GEOM, small_dd, CR_END);
	mylist = GeomCreate("list", CR_GEOM, large_dd, CR_CDR, smlist, CR_END);
	return(mylist);
	}
}

WEpolyhedron *
DiscGrpMakeDirdom(DiscGrp *gamma, HPoint3 *poi, int slice)
{
	int i, j, k;
	proj_matrix *gen_list;
    	point origin;
	int metric, transp;

	transp = gamma->attributes & DG_TRANSPOSED;
	/* transform from floating point to double, essentially */
        gen_list = OOGLNewNE(proj_matrix, gamma->gens->num_el, "DiscGrp gens");
	/* jeff week's code basically uses transposed matrices, so
	if the transposed flag is set, we do nothing, otherwise we
	have to transpose! */
        for (i=0; i<gamma->gens->num_el; ++i) 
	    for (j=0; j<4; ++j)
	        for (k=0; k<4; ++k)
		    {
		    if (transp) gen_list[i][j][k] = gamma->gens->el_list[i].tform[j][k];
		    else  gen_list[i][k][j] = gamma->gens->el_list[i].tform[j][k];
		    }
        origin[0] = poi->x;
        origin[1] = poi->y;
        origin[2] = poi->z;
        origin[3] = poi->w;

	wepoly2 = &wepoly1;
	metric = (gamma->attributes & DG_METRIC_BITS); 
	do_weeks_code(wepoly2, origin, gen_list, gamma->gens->num_el, metric,slice);

	OOGLFree(gen_list);

	/* turn off the 'new dirdom' bit */
        gamma->flag &= ~DG_NEWDIRDOM;
	return(*wepoly2);
}
