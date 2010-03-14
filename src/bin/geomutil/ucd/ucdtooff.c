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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* File:	anytoucd.c:
   Author:	Charlie Gunn originally
                
   read a OOGL object on stdin, and write ucd format on stdout.
*/
#include "vec4.h"
#include "geom.h"
#include "ooglutil.h"
#include "3d.h"
#include "polylistP.h"
#include "plutil.h"
#include <stdio.h>
#include <stdlib.h>
#include "time.h"

/*
  #define DEBUG
*/

typedef struct {
    int id;
    HPoint3 v;
}  ucdvert;

typedef struct {
    int id;
    int m;
    int n;
    int faces;
    int vlist[8];
}  ucdpoly;

#define UCD_NORMAL	1
#define UCD_RGBA	2
#define UCD_RGB		3
#define UCD_Z		4
#define UCD_NUMDATAFIELDS	8

/* Fool some Linux distros which are too eager to improve the quality
 * of source code and turn on FORTIFY_SOURCE by default for the
 * C-compiler gcc.
 */
#if HAVE_VFSCANF
# if defined(__GNUC__) && __GNUC__ >= 3
static int ign_fscanf(FILE *file, const char *format, ...)
    __attribute__((format(scanf, 2, 3)));
# endif

static int ign_fscanf(FILE *file, const char *format, ...)
{
    int result;
    va_list ap;
    
    va_start(ap, format);
    result = vfscanf(file, format, ap);
    va_end(ap);
    return result;
}
#else
# define ign_fscanf fscanf
#endif

int 
gettype(str)
     char *str;
{
    if (strcmp(str, "normal") == 0) return UCD_NORMAL; 
    else if (strcmp(str, "rgba") == 0) return UCD_RGBA; 
    else if (strcmp(str, "rgb") == 0) return UCD_RGB; 
    else return(0);
}

int
nodeidtoindex(id, v, num_nodes)
     int id;
     ucdvert *v;
     int num_nodes;
{
    int i;
    for (i=0; i<num_nodes; ++i)	
	if (v[i].id == id) return(i);
    return(-1);
}

int
cellidtoindex(id, p, num_cells)
     int id;
     ucdpoly *p;
     int num_cells;
{
    int i, index;
    for (i=0, index=0; i<num_cells; index += p[i].faces, ++i)	
	if (p[i].id == id) return(index);
    return(-1);
}

int main(int argc, char **argv)
{
    FILE *fp = stdin;
    int num_nodes, 
	num_node_data_comp,
	*node_data_comp = NULL,
	num_cells, 
	num_cell_data_comp,
	*cell_data_comp = NULL,
	offset,
	i,j,k,id,index,n, plflags = 0,
	num_faces = 0, num_vertinds = 0;
    float *ncdptrs[256], tt;
    HPoint3 *verts;
    int *nverts, *vertlist;
    Point3 *normal = NULL;
    ColorA *color = NULL;
    ucdvert *ucdv;
    ucdpoly *ucdp;
    char str[UCD_NUMDATAFIELDS][32], label[UCD_NUMDATAFIELDS][32];

    {
	char *timestring;
	time_t mytime;
	time(&mytime);
	timestring = ctime(&mytime);
	printf("#  Created by ucdtooff on %s \n",timestring);
    }

    {
	int buff[5];
	/* get the header; fgetni skips ucd comments  */
	fgetni(fp,5,buff,0);
	num_nodes = buff[0];
	num_cells = buff[1];
	num_node_data_comp = buff[2];
	num_cell_data_comp = buff[3];
	/* num_model_data = buff[4];*/
    }

    ucdv  = OOGLNewN(ucdvert, num_nodes); 
    verts = OOGLNewN(HPoint3, num_nodes); 
    ucdp = OOGLNewN(ucdpoly, num_cells); 
    if (num_node_data_comp)
        node_data_comp = OOGLNewN(int, num_node_data_comp);
    if (num_cell_data_comp)
        cell_data_comp = OOGLNewN(int, num_cell_data_comp);

    /* read the vertices */
    for (i=0; i<num_nodes; ++i)
 	{
	    ign_fscanf(fp,"%d%g%g%g",&ucdv[i].id,&ucdv[i].v.x,&ucdv[i].v.y,&ucdv[i].v.z);
	    ucdv[i].v.w = 1.0;
	}
    /* translate the id's into indices */
    for (i=0; i<num_nodes; ++i)	{
	index = nodeidtoindex(id = ucdv[i].id, ucdv, num_nodes); 
	if (index < 0) OOGLError(1,"Bad node id %d in ucdtooff\n",id);
	verts[index] = ucdv[i].v;
    }

    /* read the faces */
    num_faces = 0;
    num_vertinds = 0;
    for (i=0; i<num_cells; ++i)	{
	int n;
	char str[64];
	ign_fscanf(fp,"%d",&ucdp[i].id);
	ign_fscanf(fp,"%d",&ucdp[i].m);	/* unused ? */
	ign_fscanf(fp,"%s",str);
	ucdp[i].faces = 1;
	if (strcmp(str,"line") == 0)
	    n = 2, num_vertinds += 2;
	else if (strcmp(str,"tri") == 0)
	    n = 3, num_vertinds += 3;
	else if (strcmp(str,"quad") == 0)
	    n = 4, num_vertinds += 4;
	else if (strcmp(str,"hex") == 0) 
	    n = 8, num_vertinds += 24, ucdp[i].faces = 6;
	else if (strncmp(str,"tet",3) == 0)
	    n = 4, num_vertinds += 12, ucdp[i].faces = 4;
	else
	    exit(fprintf(stderr,"Illegal cell type %s in ucdtooff\n",str));
	num_faces += ucdp[i].faces;
	ucdp[i].n = n;
        if(fgetni(fp, n, &ucdp[i].vlist[0], 0) != n)
	    fprintf(stderr, "Couldn't read %d vert indices for UCD cell %d\n",
		    n, ucdp[i].id);
    }

    nverts = OOGLNewNE(int, num_faces, "vert counts"); 
    vertlist = OOGLNewNE(int, num_vertinds, "vert indices");
    /* translate the id's into indices */
    {
	int nvi, vi;
	static int plain[] = {0,1,2,3};
	static int hex[] = {7,6,5,4, 0,1,2,3, 5,1,0,4, 2,6,7,3, 0,3,7,4, 2,1,5,6};
	static int tet[] = {0,1,2, 1,0,3, 2,1,3, 3,0,2};

	for (nvi=0, vi=0, i=0; i<num_cells; ++i)	{
	    int n = ucdp[i].n;
	    int k;
	    int *inds = NULL, indsperface = 0;
	    switch(ucdp[i].faces) {
	    case 1: inds = plain;  indsperface = n; break;
	    case 4: inds = tet;    indsperface = 3; break;
	    case 6: inds = hex;    indsperface = 4; break;
	    }
	    for(k = 0; k < ucdp[i].faces; k++) {
		nverts[vi++] = indsperface;
		for(j = 0; j < indsperface; j++) {
		    id = ucdp[i].vlist[*inds++];
		    index = nodeidtoindex(id, ucdv, num_nodes); 
		    if (index < 0) OOGLError(1,"Bad node id %d in ucdtooff\n",id);
		    vertlist[nvi++] = index;
		}
	    }
	}
    }

    /* theoretically, we could get a reasonable polylist at this point */
	
    /* get the node data descriptor */
    if (num_node_data_comp != 0)  {
	ign_fscanf(fp,"%d", &num_node_data_comp);
	for (i=0; i<num_node_data_comp; ++i)
	    {
		ign_fscanf(fp,"%d", &node_data_comp[i]);
	    }

	/* get the node labels */
	for (i=0; i<num_node_data_comp; ++i)	{
	    ign_fscanf(fp,"%31s",str[i]);
	    ign_fscanf(fp,"%31s",label[i]);	/* this is ignored */
	}

	/* get the node data */
	for (i=0; i<num_node_data_comp; ++i)	{
	    n = strlen(str[i]);
	    /* get rid of commas */
	    if (str[i][n-1] == ',')  str[i][n-1] = '\0';
	    switch (gettype(str[i]))	{
	    case UCD_NORMAL:
		if (node_data_comp[i] != 3)
		    OOGLError(1,"ucdtooff: bad normal descriptor\n");
		plflags |= PL_HASVN;
		normal = OOGLNewN(Point3, num_nodes);	
		ncdptrs[i] = (float *) normal;
		break;

	    case UCD_RGB:
		if (node_data_comp[i] != 3)
		    OOGLError(1,"ucdtooff: bad color descriptor\n");
		plflags |= PL_HASVCOL;
		color = OOGLNewN(ColorA, num_nodes);	
		ncdptrs[i] = (float *) color;
		break;

	    case UCD_RGBA:
		if (node_data_comp[i] != 4)
		    OOGLError(1,"ucdtooff: bad colora descriptor\n");
		plflags |= PL_HASVCOL;
		color = OOGLNewN(ColorA, num_nodes);	
		ncdptrs[i] = (float *) color;
		break;

	    default:
		fprintf(stderr,"ucdtooff: ignoring unknown node-data type \"%s\"\n",str[i]);
		ncdptrs[i] = (float *) 0;
		break;
	    }	
	}
	for (j=0;j<num_nodes; ++j)	{
	    ign_fscanf(fp,"%d",&id);
	    index = nodeidtoindex(id, ucdv, num_nodes);
	    if (index < 0) OOGLError(1,"Bad node id %d in ucdtooff\n",id);
    	    for (i=0; i<num_node_data_comp; ++i)	{
	        if (gettype(str[i]) == UCD_RGB)
		    offset = index * 4;
		else offset = index * node_data_comp[i];
		if (ncdptrs[i])	{
		    for (k = 0; k < node_data_comp[i]; ++k)
	    		ign_fscanf(fp,"%g", ncdptrs[i] + offset + k);
		    if (gettype(str[i]) == UCD_RGB) ncdptrs[i][offset + 3] = 1.0;
		}
		else	{
		    for (k = 0; k < node_data_comp[i]; ++k)
	    		ign_fscanf(fp,"%g", &tt);
		}
	    }
	}
    }

    /* get the cell data descriptor */
    if (num_cell_data_comp != 0)  {
	ign_fscanf(fp,"%d", &num_cell_data_comp);
	for (i=0; i<num_cell_data_comp; ++i)
	    ign_fscanf(fp,"%d", &cell_data_comp[i]);

	/* get the cell labels */
	for (i=0; i<num_cell_data_comp; ++i)	{
	    ign_fscanf(fp,"%31s",str[i]);
	    ign_fscanf(fp,"%31s",label[i]);	/* this is ignored */
	}

	/* get the cell data */
	for (i=0; i<num_cell_data_comp; ++i)	{
	    n = strlen(str[i]);
	    /* get rid of commas */
	    if (str[i][n-1] == ',')  str[i][n-1] = '\0';
	    switch (gettype(str[i]))	{
	    case UCD_NORMAL:
		if (cell_data_comp[i] != 3)
		    OOGLError(1,"ucdtooff: bad normal descriptor\n");
		plflags |= PL_HASPN;
		normal = OOGLNewN(Point3, num_faces);	
		ncdptrs[i] = (float *) normal;
		break;

	    case UCD_RGBA:
		if (cell_data_comp[i] != 4)
		    OOGLError(1,"ucdtooff: bad colora descriptor\n");
		plflags |= PL_HASPCOL;
		color = OOGLNewN(ColorA, num_faces);	
		ncdptrs[i] = (float *) color;
		break;

	    case UCD_RGB:
		if (cell_data_comp[i] != 3)
		    OOGLError(1,"ucdtooff: bad color descriptor\n");
		plflags |= PL_HASPCOL;
		color = OOGLNewN(ColorA, num_faces);	
		ncdptrs[i] = (float *) color;
		break;
	    default:
		fprintf(stderr,"ucdtooff: ignoring unknown cell-data type \"%s\"\n",str[i]);
		/* skip over the data */
		ncdptrs[i] = (float *) 0;
		break;
	    }	
	}
	for (j=0;j<num_cells; ++j)	{
	    ign_fscanf(fp,"%d",&id);
	    index = cellidtoindex(id, ucdp, num_cells);
	    if (index < 0) OOGLError(1,"Bad cell id %d in ucdtooff\n",id);
    	    for (i=0; i<num_cell_data_comp; ++i)	{
		int incr = (gettype(str[i])==UCD_RGB) ? 4 : cell_data_comp[i];
		offset = index * incr;
		if (ncdptrs[i])	{
		    for (k = 0; k < cell_data_comp[i]; ++k)
	    		ign_fscanf(fp,"%g", ncdptrs[i] + offset + k);
		    if (gettype(str[i]) == UCD_RGB)
			ncdptrs[i][offset + 3] = 1.0;
		    /* Clone property over all faces of multifaced solid */
		    for (k = 1; k < ucdp[j].faces; k++)
			memcpy( ncdptrs[i] + offset + incr*k,
			        ncdptrs[i] + offset,
				incr * sizeof(float)); 
		}
		else	{
		    for (k = 0; k < cell_data_comp[i]; ++k)
	    		ign_fscanf(fp,"%g", &tt);
		}
	    }
	}
    }

    printf("%s%sOFF\n%d %d 0\n\n",
	   plflags&PL_HASVCOL ? "C":"",
	   plflags&PL_HASVN ? "N":"",
	   num_nodes, num_faces);
    for(i = 0; i < num_nodes; i++) {
	printf("%g %g %g", verts[i].x, verts[i].y, verts[i].z);
	if(plflags & PL_HASVN)
	    printf("\t%g %g %g", normal[i].x,normal[i].y,normal[i].z);
	if(plflags & PL_HASVCOL) 
	    printf("\t%.3g %.3g %.3g %.3g", color[i].r,color[i].g,color[i].b,color[i].a);
	printf("\n");
    }
    printf("\n");
    for(i = 0, j = 0; i < num_faces; i++) {
	k = nverts[i];
	printf("%d\t", k);
	while(--k >= 0)
	    printf("%d ", vertlist[j++]);
	if(plflags & PL_HASPCOL)
	    printf("\t%.3g %.3g %.3g %.3g\n", color[i].r,color[i].g,color[i].b,color[i].a);
	else
	    printf("\n");
    }
    exit(0);
}
