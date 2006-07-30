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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/*
 * GeomExtentDirection.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<time.h>
#include	"geomclass.h"
#include        "geom.h"
#include        "skelP.h"
#include        "ndmeshP.h"
#include        "hpointn.h"
#include        "hpoint3.h"
#include        "pointlist.h"
#include        "quadP.h"
#include        "meshP.h"
#include        "polylistP.h"
#include        "bezierP.h"
#include        "vectP.h"
#include        "listP.h"
#include        "discgrp.h"
#include        "instP.h"
#include        "npolylistP.h"
#include        "dim.h"
#include	"transformn.h"

#include	"BasicFns.h"
#include	"Random.h"
#include	"GeomExtentDirection.h"

#define RANDOMTEST 50 

static int PTLINIT = 1;


/*The following code defines a Geomview function
GeomExtentDir which take a pointer to a Geom, pointer to a TransformN, an integer
giving the dimension of a vector,an array of floats giving the direction and an 
integer flag.  It returns the scaled extent of the Geom, operated on by the
TransformN, in the given direction.  If the flag is true the answer is exact, other
wise an approximation using a sample of randomly selected points is used.
If the direction and object differ in dimension the smaller is padded with 0's.
The function returns a pointer to a block of three floats, the max, min, and
a flag indicating whether the dimensions differed (0) or not (1).
*/


static void *extentDirDefault(int sel, Geom *g, va_list *args)
{       HPoint3 *pts;
        int i, npts, flag, dimdir, dimvert, smdim, test[RANDOMTEST];
        TransformN *t;
        float *point, *direction, *results, temp;
        HPointN *hpt;
        if(PTLINIT)
	{	pointlist_init();
		PTLINIT=0;
	}
        t=va_arg(*args,TransformN *);
        dimdir=va_arg(*args,int);
        direction=va_arg(*args, float *);
        flag=va_arg(*args,int);
        dimvert=GeomDimension(g);
        smdim = (dimvert < dimdir) ? dimvert : dimdir;
	results=(float *)malloc((sizeof *results)*3);
        npts=(int)(long)GeomCall(GeomMethodSel("PointList_length"),g);
        if(npts==0)
                return (void *)NULL;
        pts=OOGLNewNE(HPoint3,npts,"Points");
        pts=GeomCall(GeomMethodSel("PointList_get"),g,TM_IDENTITY,POINTLIST_SELF);
        hpt=HPtNCreate(dimvert+1,NULL);
        hpt=extract(pts,hpt,dimvert);
        hpt=HPtNTransform(t,hpt,hpt);
        point=hpt->v;
        temp=InnerProductN(point,direction,smdim);
        results[0]=results[1]=temp;
        if( (flag) || (npts <=RANDOMTEST) )
                for(i=1;i<npts;i++)
                {       hpt=extract(pts+i,hpt,dimvert);
                        hpt=HPtNTransform(t,hpt,hpt);
                        point=hpt->v;
                        temp=InnerProductN(point,direction,smdim);
                        results[0] = (results[0]>temp) ? results[0] : temp;
                        results[1] = (results[1]<temp) ? results[1] : temp;
                }
        else
        {       srand(time(NULL));
                for(i=0;i<RANDOMTEST;i++)
                test[i]=irand(1,npts-1);
                for(i=0;i<RANDOMTEST;i++)
                {       hpt=extract(pts+test[i],hpt,dimvert);
                        hpt=HPtNTransform(t,hpt,hpt);
                        point=hpt->v;
                        temp=InnerProductN(point,direction,smdim);
                        results[0] = (results[0]>temp) ? results[0] : temp;
                        results[1] = (results[1]<temp) ? results[1] : temp;
                }
        }
        if(dimdir==dimvert)
                results[2]=1.0;
        else
                results[2]=0.0;
        free(hpt);
        return (void *)results;
}

static void *extentDirSkel(int sel, Geom *g, va_list *args)
{	float *point, *direction, *results, temp;
	int dimdir, dimvert, i, flag, smdim, test[RANDOMTEST];
	TransformN *t;
	HPointN *hpt;
	t = va_arg(*args,TransformN *);
	dimdir = va_arg(*args,int);
	dimvert = ((Skel *)g)->dim -1;
	direction = va_arg(*args,float *);
	smdim = (dimvert < dimdir) ? dimvert : dimdir;
	if (((Skel *)g)->nvert == 0)
		return (void *)NULL;
	results = (float *)malloc((sizeof *results)*3);
	hpt=HPtNCreate(((Skel *)g)->dim,NULL);
	hpt->v=((Skel *)g)->p;
	hpt=HPtNTransform(t,hpt,hpt);
	point=hpt->v;
	temp=InnerProductN(point,direction,smdim);
	results[0]=results[1]=temp;
	flag=va_arg(*args,int);
	if( (flag) || (((Skel *)g)->nvert <= RANDOMTEST))
	   for(i=1;i<((Skel *)g)->nvert;i++)
	   {    point=(((Skel *)g)->p)+(i*(dimvert+1));
		hpt->v=point;
		hpt=HPtNTransform(t,hpt,hpt);
		point=hpt->v;
	      	temp=InnerProductN(point,direction,smdim);
	        results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
	   }
	else
	{  srand(time(NULL));
	   for(i=0;i<RANDOMTEST;i++)
		test[i]=irand(1,(((Skel *)g)->nvert)-1);
  	   for(i=0;i<RANDOMTEST;i++)
	   {	point=(((Skel *)g)->p)+(test[i]*(dimvert+1));
		hpt->v=point;
                hpt=HPtNTransform(t,hpt,hpt);
                point=hpt->v;
                temp=InnerProductN(point,direction,smdim);
                results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
           }
	}
	if(dimdir==dimvert)
		results[2]=1.0;
	else
		results[2]=0.0;
	free(hpt);
	return (void *)results;
}

static void *extentDirNPolyList(int sel, Geom *g, va_list *args)
{       float *point, *direction, *results, temp;
        int dimdir, dimvert, i, flag, smdim, test[RANDOMTEST];
        TransformN *t;
        HPointN *hpt;
        t = va_arg(*args,TransformN *);
        dimdir = va_arg(*args,int);
        dimvert = ((NPolyList *)g)->pdim -1;
        direction = va_arg(*args,float *);
        smdim = (dimvert < dimdir) ? dimvert : dimdir;
        if (((NPolyList *)g)->n_verts == 0)
                return (void *)NULL;
        results = (float *)malloc((sizeof *results)*3);
        hpt=HPtNCreate(((NPolyList *)g)->pdim,NULL);
        hpt->v=((NPolyList *)g)->v;
        hpt=HPtNTransform(t,hpt,hpt);
        point=hpt->v;
        temp=InnerProductN(point,direction,smdim);
        results[0]=results[1]=temp;
        flag=va_arg(*args,int);
        if( (flag) || (((NPolyList *)g)->n_verts <= RANDOMTEST))
           for(i=1;i<((NPolyList *)g)->n_verts;i++)
           {    point=(((NPolyList *)g)->v)+(i*(dimvert+1));
                hpt->v=point;
                hpt=HPtNTransform(t,hpt,hpt);
                point=hpt->v;
                temp=InnerProductN(point,direction,smdim);
                results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
           }
        else
        {  srand(time(NULL));
           for(i=0;i<RANDOMTEST;i++)
                test[i]=irand(1,(((NPolyList *)g)->n_verts)-1);
           for(i=0;i<RANDOMTEST;i++)
           {    point=(((NPolyList *)g)->v)+(test[i]*(dimvert+1));
                hpt->v=point;
                hpt=HPtNTransform(t,hpt,hpt);
                point=hpt->v;
                temp=InnerProductN(point,direction,smdim);
                results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
           }
        }
        if(dimdir==dimvert)
                results[2]=1.0;
        else
                results[2]=0.0;
        free(hpt);
        return (void *)results;
}

static void *extentDirNDMesh(int sel, Geom *g, va_list *args)
{	float *point, *direction, *results, temp;
	int dimdir, dimvert, i, flag, test[RANDOMTEST], smdim,  numpts=1;
	TransformN *t;
	HPointN *hpt;
	hpt = (HPointN *)malloc(sizeof *hpt);
	t = va_arg(*args,TransformN *);
	dimdir = va_arg(*args,int);
	dimvert = ((NDMesh *)g)->pdim;
	direction = va_arg(*args, float *);
	smdim = (dimvert < dimdir) ? dimvert : dimdir;
	for(i=0;i<((NDMesh *)g)->meshd;i++)
		numpts*=(((NDMesh *)g)->mdim)[i];
	if (numpts == 1)
		return (void *)NULL;
	results = (float *)malloc((sizeof *results)*3);
	hpt=HPtNTransform(t,(((NDMesh *)g)->p)[0],NULL);
	point=hpt->v;
	temp=InnerProductN(point,direction,smdim);
	results[0]=results[1]=temp;
	flag=va_arg(*args,int);
	if( (flag) || (numpts<=RANDOMTEST) )
	   for(i=1;i<numpts;i++)
   	   {	hpt=HPtNTransform(t,(((NDMesh *)g)->p)[i],hpt); 
		point=hpt->v;
                temp=InnerProductN(point,direction,smdim);
                results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
	   }
	else
        {  srand(time(NULL));
	   for(i=0;i<RANDOMTEST;i++)
		test[i]=irand(1,numpts-1);
	   for(i=0;i<RANDOMTEST;i++)
	   {    hpt=HPtNTransform(t,(((NDMesh *)g)->p)[test[i]],hpt);
		point=hpt->v;
                temp=InnerProductN(point,direction,smdim);
                results[0] = (results[0]>temp) ? results[0] : temp;
                results[1] = (results[1]<temp) ? results[1] : temp;
           }
	} 
	if(dimdir==dimvert)
                results[2]=1.0;
        else
                results[2]=0.0;
        free(hpt);
	return (void *)results;
}

static void *extentDirList(int sel, Geom *g, va_list *args)
{	TransformN *t;
	int dimdir, flag;
	float *direction, *results, *temp;
	List *l;
	t=va_arg(*args,TransformN *);
	dimdir=va_arg(*args, int);
	direction=va_arg(*args,float *);
	flag=va_arg(*args,int);
	l=(List *)g;
	if (l==NULL)
		return (void *)NULL;
	results=GeomExtentDir(l->car,t,dimdir,direction,flag);
	for(l = l->cdr; l!=NULL; l = l->cdr)
	{	temp=GeomExtentDir(l->car,t,dimdir,direction,flag);
		if (temp==NULL)
			return (void *)NULL;
		results[0] = (results[0]>temp[0]) ? results[0] : temp[0];
                results[1] = (results[1]<temp[1]) ? results[1] : temp[1];
		results[2] = (temp[2]==0.0) ? 0.0 : results[2];
		free(temp);
	}
	return (void *)results;
}

static int NumSel = 0;

float *GeomExtentDir(Geom *g, TransformN *t, int dimdir, float *direction, int flag)
{	if(NumSel == 0)
	{	NumSel = GeomNewMethod("extentDir",extentDirDefault);
		GeomSpecifyMethod(NumSel, SkelMethods(), extentDirSkel);
		GeomSpecifyMethod(NumSel, NDMeshMethods(), extentDirNDMesh);
		GeomSpecifyMethod(NumSel, ListMethods(), extentDirList); 
		GeomSpecifyMethod(NumSel, NPolyListMethods(), extentDirNPolyList); 
	}
	return (float *)GeomCall(NumSel, g, t, dimdir, direction, flag);
}
