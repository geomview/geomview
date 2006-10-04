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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include "hpointn.h"
#include "transformn.h"
#include "geomclass.h"
#include "geom.h"
#include "dim.h"
#include "ndmeshP.h"

#include "MakeGoodTransforms.h"
#include "GeomExtentDirection.h"
#include "BasicFns.h"
#include "GeomtoNoff.h"

extern IOBFILE *infile;

/*
#include "ClipOogl.h"
*/

#include "Clip.h"
static Clip theClipper;

#define setClipPlane(X) setClipPlane(&theClipper,X,0)
#define setSide(X) setSide(&theClipper,X)
#define setGeom(X) setGeom(&theClipper,X)
#define clip() do_clip(&theClipper)
#define geom() getGeom(&theClipper)


void initSlicer(void)
{
	clip_init(&theClipper);
}




int GetFocusCam(ClientData clientdata, Tcl_Interp *interp,
		int argc, const char **argv)
{
	static char str[100];
	char *ptr;
	printf("(echo(real-id focus)\\n)\n");
	fflush(stdout);
	iobfgets(str, sizeof(str), infile);
	for(ptr=str+1;*ptr!='"';ptr++);
		*ptr='\0';
	interp->result=str+1;
	return TCL_OK;
}

int GetDim(ClientData clientdata, Tcl_Interp *interp,
	   int argc, const char **argv)
{
	int dim;
        static char str[11];

	iobfrewind(infile);
        printf("(echo(dimension)\\n)\n");
        fflush(stdout);
	if (iobfgetni(infile, 1, &dim, 0) != 1) {
		interp->result = "Error";
		return TCL_ERROR;
	}
	sprintf(str, "%d", dim);
        interp->result=str;
        return TCL_OK;
}

int CreateClipPlane(ClientData clientdata, Tcl_Interp *interp,
		    int argc, const char **argv)
{
	char scratch[16];
	int dim, x, y, i, j, small,big, xy[2];
	TransformN *t, *tworld, *tfinal;
	if(argc!=3) {
		interp->result="Wrong number of arguments";
		return TCL_ERROR;
	}
	dim=atoi(argv[1]);
	iobfrewind(infile);
	printf("(echo(ND-axes %s)\\n)\n",argv[2]);
	fflush(stdout);
	iobftoken(infile, 0);
	if (iobfgetni(infile, 2, xy, 0) != 2) {
		interp->result="Wrong reply to ND-axes";
		iobfrewind(infile);
		return TCL_ERROR;
	}
	/* consume remainder of answer */
	iobfgets(scratch, sizeof(scratch), infile);
	iobfrewind(infile);
	x = xy[0];
	y = xy[1];
	small=(x<y) ? x : y;
	big=(x>y) ? x : y;
	printf("(geometry clip_plane { :cp})\n");
	fflush(stdout);
	printf("(read geometry {define cp \n");
	printf("CnMESH %d\n2 2\n",dim);
	fflush(stdout);
	for(i=0;i<dim;i++)
		printf("0 ");
	printf(".242 .370 1 1");
	printf("\n");
	fflush(stdout);
	for(i=0;i<x;i++)
		printf("0 ");
	printf("1 ");
	for(i++;i<dim;i++)
		printf("0 ");
	printf(".242 .370 1 1");
	printf("\n");
	for(i=0;i<y;i++)
		printf("0 ");
	printf("1 ");
	for(i++;i<dim;i++)
		printf("0 ");
	printf(".242 .370 1 1");
	printf("\n");
	fflush(stdout);
	for(i=0;i<small;i++)
		printf("0 ");
	printf("1 ");
	for(i++;i<big;i++)
		printf("0 ");
	printf("1 ");
	for(i++;i<dim;i++)
		printf("0 ");
	printf(".242 .370 1 1");
	printf("\n");
	fflush(stdout);
        printf("})\n");
        fflush(stdout);
	printf("(echo(ND-xform-get %s universe))\n",argv[2]);
	fflush(stdout);
	t=TmNRead(infile);
	printf("(echo(ND-xform-get universe world))\n");
	fflush(stdout);
	tworld=TmNRead(infile);
	tfinal=TmNConcat(t,tworld,NULL);
	printf("(ND-xform clip_plane ntransform {%d %d\n",t->idim,t->odim);
	for(i=0;i<tfinal->idim;i++)
	{	for(j=0;j<tfinal->odim;j++)
			printf("%f ",*(tfinal->a + (i*tfinal->idim) + j));
		printf("\n");
	}
	printf("})\n");
	return TCL_OK;
}

int GetData(ClientData clientdata, Tcl_Interp *interp,
	    int argc, const char **argv)
{
	char scratch[16];
	int a[3],i,j,dim;
	static char str[200];
	TransformN *t;
	Geom *g;
	HPointN *temp,*dir1, *dir2, *norm;
	float Dir1[3], Dir2[3], Norm[3], disp;
	if(argc!=3)
        {       interp->result="Wrong number of arguments";
                return TCL_ERROR;
        }
	dim=atoi(argv[1]);
	printf("(if (real-id clip_plane) (write geometry - clip_plane self) (echo \"nada\"))\n");
	fflush(stdout);
	g=GeomFLoad(infile, "stdin");
	if(g==NULL)
        {       interp->result = "NoObj";
                iobfrewind(infile);
                return TCL_OK;
        }
	printf("(echo(ND-xform-get clip_plane universe)\\n)\n");
	fflush(stdout);
	t=TmNRead(infile);
	for(i=0;i<4;i++)
		(((NDMesh *)g)->p)[i]=HPtNTransform(t,(((NDMesh *)g)->p)[i],(((NDMesh *)g)->p)[i]);
	temp=HPtNCreate(dim+1,NULL);
	for(i=0;i<=dim;i++)
		(temp->v)[i]=((((NDMesh *)g)->p)[0]->v)[i];
	printf("(echo(ND-xform-get %s universe)\\n)\n",argv[2]);
	fflush(stdout);
	t=TmNRead(infile);
	for(i=0;i<4;i++)
                (((NDMesh *)g)->p)[i]=HPtNTransform(t,(((NDMesh *)g)->p)[i],(((NDMesh *)g)->p)[i]);
	dir1=HPtNCreate(dim+1,NULL);
	dir2=HPtNCreate(dim+1,NULL);
	HPtNComb(-1.0,(((NDMesh *)g)->p)[0],1.0,(((NDMesh *)g)->p)[1],dir1);
        HPtNComb(-1.0,(((NDMesh *)g)->p)[0],1.0,(((NDMesh *)g)->p)[2],dir2);
	iobfrewind(infile);
	printf("(echo(ND-axes %s)\\n)\n",argv[2]);
        fflush(stdout);
	iobftoken(infile, 0);
	if (iobfgetni(infile, 3, a, 0) != 3) {
		iobfrewind(infile);
		interp->result="Wrong reply to ND-axes";
		return TCL_ERROR;
	}
	/* consume the remainder of the answer */
	iobfgets(scratch, sizeof(scratch), infile);
	iobfrewind(infile);
	for(i=0;i<3;i++)
	{	Dir1[i]=(dir1->v)[a[i]];
		Dir2[i]=(dir2->v)[a[i]];
	}
	Norm[0]=Dir1[1]*Dir2[2] - Dir1[2]*Dir2[1];
	Norm[1]=Dir1[2]*Dir2[0] - Dir1[0]*Dir2[2];
	Norm[2]=Dir1[0]*Dir2[1] - Dir1[1]*Dir2[0];
	norm=HPtNCreate(dim+1,NULL);
	for(i=0;i<dim+1;i++)
	{	(norm->v)[i]=0;
		for(j=0;j<3;j++)
			(norm->v)[i]+=(*((t->a)+a[j]*(dim+1)+i)*Norm[j]);
	}
	disp=InnerProductN(norm->v,temp->v,dim);
	free(dir1);
	free(dir2);
	sprintf(str,"%f ",disp);
	for(i=0;i<dim;i++)
		sprintf(str+strlen(str),"%f ",(norm->v)[i]);
	sprintf(str+strlen(str),"\n");
	interp->result=str;
	return TCL_OK;
}

int UpdatePicture(ClientData clientdata, Tcl_Interp *interp,
		  int argc, const char **argv)
{
	char scratch[16];
	int i, j, xdim, ydim, zdim, dim, xyz[3];
	float D, *direction;
	char *ptr;
	TransformN *t;
	if (argc != 4)
	{	interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}
	dim=atoi(argv[1]);
	direction=(float *)malloc(dim*(sizeof *direction));
	ptr=strtok((char *)argv[2]," ");
	D=atof(ptr);
	for(i=0;i<dim;i++)
	{	ptr=strtok(NULL," ");
		direction[i]=atof(ptr);
	}
	printf("(echo(ND-xform-get %s universe))\n",argv[3]);
	fflush(stdout);
	t=TmNRead(infile);
	iobfrewind(infile);
	printf("(echo(ND-axes %s)\\n)\n",argv[3]);
        fflush(stdout);
	iobftoken(infile, 0);
	if (iobfgetni(infile, 3, xyz, 0) != 3) {
		iobfrewind(infile);
		interp->result="Wrong reply to ND-axes";
		return TCL_ERROR;
	}
	/* consume the remainder of the answer */
	iobfgets(scratch, sizeof(scratch), infile);
	iobfrewind(infile);
	xdim = xyz[0];
	ydim = xyz[1];
	zdim = xyz[2];
	for(i=0;i<dim;i++)
		*(t->a + (zdim*(dim+1)) + i) = direction[i];
	return_orthonormal_matrix(t,xdim,ydim,zdim,dim);
	for(i=0;i<dim;i++)
	{	direction[i]*=D;
                *(t->a + (dim*(dim+1)) + i) = direction[i];
	}
	printf("(ND-xform %s ntransform {%d %d\n",argv[3],t->idim,t->odim);
        for(i=0;i<t->idim;i++)
        {       for(j=0;j<t->odim;j++)
                        printf("%f ",*(t->a + (i*t->idim) + j));
                printf("\n");
        }
        printf("})\n");
	fflush(stdout);
	printf("(ND-xform clip_plane ntransform {%d %d\n",t->idim,t->odim);
        for(i=0;i<t->idim;i++)
        {       for(j=0;j<t->odim;j++)
                        printf("%f ",*(t->a + (i*t->idim) + j));
                printf("\n");
        }
        printf("})\n");
	fflush(stdout);
	return TCL_OK;
}

int SliceNDice(ClientData clientdata, Tcl_Interp *interp,
	       int argc, const char **argv)
{	Geom *g, *converted, **newobjs = NULL, *todo;
	TransformN *t, *identity, *tinv, *tworld;
	float D, d, *direction, *result, smalld, bigd, step;
	char *ptr;
	int i,j,k,dim,gdim,n;
	if(argc<5)
	{	interp->result="Wrong number of arguments";
		return TCL_OK;
	}
	if(atoi(argv[4])==3) {
		if(argc!=9)
		{	interp->result="Wrong number of arguments";
			return TCL_OK;
		}
	} else if(atoi(argv[4])==4) {
		if(argc!=8)
		{	interp->result="Wrong number of arguments";
			return TCL_OK;
		}
	} else {
		if(argc!=5)
		{       interp->result="Wrong number of arguments";
			return TCL_OK;
		}
	}
	dim=atoi(argv[2]);
        direction=(float *)malloc((dim+1)*(sizeof *direction));
        ptr=strtok((char *)argv[3]," ");
        D=atof(ptr);
        for(i=0;i<dim;i++)
        {       ptr=strtok(NULL," ");
                direction[i]=atof(ptr);
        }
        printf("(if (real-id %s) (write geometry - %s self) (echo \"nada\"))\n",argv[1],argv[1]);
        fflush(stdout);
        g=GeomFLoad(infile, "stdin");
        if(g==NULL)
        {       interp->result = "NoObj";
                iobfrewind(infile);
                return TCL_OK;
        }
        printf("(echo(ND-xform-get %s universe)\\n)\n",argv[1]);
        fflush(stdout);
        t=TmNRead(infile);
        printf("(echo(ND-xform-get universe %s)\\n)\n",argv[1]);
        fflush(stdout);
        tinv=TmNRead(infile);
        printf("(echo(ND-xform-get universe world)\\n)\n");
        fflush(stdout);
        tworld=TmNRead(infile);
	tworld=TmNConcat(t,tworld,tworld);
	converted=GeomtoNoff(g,t);
	gdim=GeomDimension(converted);
	if(dim!=gdim)
	{	interp->result="DiffDims";
		return TCL_OK;
	}
	switch (atoi(argv[4]))
	{	case 0 : /*Cut it in half*/
		newobjs=(Geom **)malloc((sizeof *newobjs)*2);
		direction[dim]=D;
		setGeom(converted);
		setClipPlane(direction);
		setSide(0);
		clip();
		newobjs[0]=geom();
/*		newobjs[0]=GeomtoNoff(newobjs[0],tinv); */
		setGeom(converted);
		setClipPlane(direction);
		setSide(1);
		clip();
		newobjs[1]=geom();
/*		newobjs[1]=GeomtoNoff(newobjs[1],tinv); */
		printf("(delete %s)\n",argv[1]);
		fflush(stdout);
		for(i=0;i<2;i++)
		{	printf("(geometry clip_%d\n",i+1);
			GeomFSave(newobjs[i],stdout,"stdout");
			printf(")\n");
			fflush(stdout);
			printf("(ND-xform clip_%d\n",i+1);
			printf("{%d %d\n",tworld->idim,tworld->odim);
			for(j=0;j<tworld->idim;j++)
			{	for(k=0;k<tworld->odim;k++)
					printf("%f ",*(tworld->a + j*(tworld->idim) + k));
				printf("\n");
			}
			printf("})\n");
			fflush(stdout);
			free(newobjs[i]);
		}
		break;
		
		case 1: /*Keep < side only*/
		newobjs=(Geom **)malloc(sizeof *newobjs);
		direction[dim]=D;
		setGeom(converted);
		setClipPlane(direction);
		setSide(0);
		clip();
		*newobjs=geom();
/*		*newobjs=GeomtoNoff(*newobjs,tinv); */
		printf("(delete %s)\n",argv[1]);
		fflush(stdout);
		printf("(geometry clip_1\n");
		GeomFSave(*newobjs,stdout,"stdout");
		printf(")\n");
		fflush(stdout);
		printf("(ND-xform-set clip_1\n");
		printf("{%d %d\n",tworld->idim,tworld->odim);
		for(j=0;j<tworld->idim;j++)
		{	for(k=0;k<tworld->odim;k++)
				printf("%f ",*(tworld->a + j*(tworld->idim) + k));
			printf("\n");
		}
		printf("})\n");
		fflush(stdout);
		free(*newobjs);
		break;

		case 2: /*Keep > side only*/
		newobjs=(Geom **)malloc(sizeof *newobjs);
		direction[dim]=D;
		setGeom(converted);
		setClipPlane(direction);
		setSide(1);
		clip();
		*newobjs=geom();
/*		*newobjs=GeomtoNoff(*newobjs,tinv); */
		printf("(delete %s)\n",argv[1]);
		fflush(stdout);
		printf("(geometry clip_1\n");
		GeomFSave(*newobjs,stdout,"stdout");
		printf(")\n");
		fflush(stdout);
		free(*newobjs);
		printf("(ND-xform-set clip_1\n");
		printf("{%d %d\n",tworld->idim,tworld->odim);
		for(j=0;j<tworld->idim;j++)
		{	for(k=0;k<t->odim;k++)
				printf("%f ",*(tworld->a + j*(tworld->idim) + k));
			printf("\n");
		}
		printf("})\n");
		fflush(stdout);
		break;

		case 3: /*Dice*/
		n=atoi(argv[5]);
		d=atof(argv[6]);
		smalld = (d<D) ? d : D;
		bigd = (d > D) ? d : D;
		step=(bigd-smalld)/(float)n;
		newobjs=(Geom **)malloc((sizeof *newobjs)*n);
		direction[dim]=smalld;
		setGeom(converted);
		setClipPlane(direction);
		setSide(1);
		clip();
		todo=geom();
		for(i=1;i<=n;i++)
		{	direction[dim]=smalld+(i*step);
			setGeom(todo);
			setClipPlane(direction);
			setSide(0);
			clip();
			newobjs[i-1]=geom();
/*			newobjs[i-1]=GeomtoNoff(newobjs[i-1],tinv); */
			direction[dim]=smalld+(i*step);
			setGeom(todo);
			setClipPlane(direction);
			setSide(1);
			clip();
			todo=geom();
		}
		printf("(delete %s)\n",argv[1]);
		fflush(stdout);
		for(i=0;i<n;i++)
		{       printf("(geometry clip_%d\n",i+1);
			GeomFSave(newobjs[i],stdout,"stdout");
			printf(")\n");
			fflush(stdout);
			printf("(ND-xform-set clip_%d\n",i+1);
			printf("{%d %d\n",tworld->idim,tworld->odim);
			for(j=0;j<tworld->idim;j++)
			{	for(k=0;k<tworld->odim;k++)
					printf("%f ",*(tworld->a + j*(tworld->idim) + k));
				printf("\n");
			}
			printf("})\n");
			fflush(stdout);
			free(newobjs[i]);
		}
		if(atoi(argv[7]))
		{	for(i=2;i<=n;i+=2)
				printf("(merge-ap clip_%d appearance {*- edge})\n",i);
			fflush(stdout);
		}
		if(atoi(argv[8]))
		{	for(i=2;i<=n;i+=2)
				printf("(merge-ap clip_%d appearance {*- face})\n",i);
			fflush(stdout);
		}
		break;

		case 4: /*Dice with some thought*/
		identity=TmNCreate(t->idim,t->odim,NULL);
		for(i=0;i<identity->idim;i++)
			for(j=0;j<identity->odim;j++)
				if(i==j)
					*(identity->a + i*(t->idim)+j)=1.0;
		result=GeomExtentDir(converted,identity,dim,direction,1);
		bigd=result[0];
		smalld=result[1];
		free(result);
		n=atof(argv[5]);				
                step=(bigd-smalld)/(float)n;
                newobjs=(Geom **)malloc((sizeof *newobjs)*n);
                direction[dim]=smalld;
                setGeom(converted);
                setClipPlane(direction);
                setSide(1);
                clip();
                todo=geom();
                for(i=1;i<=n;i++)
                {       direction[dim]=smalld+(i*step);
                        setGeom(todo);
                        setClipPlane(direction);
                        setSide(0);
                        clip();
                        newobjs[i-1]=geom();
/*			newobjs[i-1]=GeomtoNoff(newobjs[i-1],tinv); */
                        direction[dim]=smalld+(i*step);
                        setGeom(todo);
                        setClipPlane(direction);
                        setSide(1);
                        clip();
                        todo=geom();
                }
		printf("(delete %s)\n",argv[1]);
		fflush(stdout);
                for(i=0;i<n;i++)
                {       printf("(geometry clip_%d\n",i+1);
                        GeomFSave(newobjs[i],stdout,"stdout");
                        printf(")\n");
                        fflush(stdout);
			printf("(ND-xform-set clip_%d\n",i+1);
			printf("{%d %d\n",tworld->idim,tworld->odim);
			for(j=0;j<tworld->idim;j++)
			{	for(k=0;k<tworld->odim;k++)
					printf("%f ",*(tworld->a + j*(tworld->idim) + k));
				printf("\n");
			}
			printf("})\n");
			fflush(stdout);
                        free(newobjs[i]);
                }
		if(atoi(argv[6]))
		{	for(i=2;i<=n;i+=2)
				printf("(merge-ap clip_%d appearance {*- edge})\n",i);
			fflush(stdout);
		}
		if(atoi(argv[7]))
		{	for(i=2;i<=n;i+=2)
				printf("(merge-ap clip_%d appearance {*- face})\n",i);
			fflush(stdout);
		}
		free(identity);
		break;
	}
	free(g);
	free(t);
	free(newobjs);
	free(converted);
	free(direction);
	interp->result="yes";
	return TCL_OK;
}
