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
#include <sys/types.h>
#include "ooglutil.h"
#include "color.h"

ColorA  builtin[] = {
{ 0.8, 0.1, 0.1, 0.75},
{ 0.1, 0.65, 0.4, 0.75},
{ 0.1, 0.1, 0.8, 0.75},
{ 0.9, 0.6, 0, 0.75},
{ 0,   0.6, 0.8, 0.75},
{ 0.5, 0, 0.9, 0.75},
{.7, .15, .1, .75},
{.2, .2, .8, .75},
{.9, .6, .02, .75},
{.1, .3, .8, .75},
{.1, .7, .2, .75},
{.8, .8, .4, .75},
{.7, .7, 0, .75},
{.7, 0, .7, .75},
{0, .7, .7, .75},
{.9, 0, .2, .75},
{.2, .9, 0, .75},
{0, .2, .9, .75},
{.75, .75, .75, .75},
{.8, .4, 0, .75},
{.8, .4, 0, .75},
{0, .4, .8, .75},
{0, .4, .8, .75},
{0, .8, .4, .75},
{0, .8, .4, .75},
{.4, 0, .8, .75}
};
	
ColorA *colormap = NULL;
static int cnt;
static int doneread = 0;
static char default_name[] = "sample.cmap";

int
readcmap(cmapfname)
char *cmapfname;
{
	FILE *fp;
	int size = 256;

	if (cmapfname == NULL)	cmapfname = getenv("CMAP_FILE");
	if (cmapfname == NULL)	cmapfname = findfile(NULL, default_name);

  	doneread = 1;
	
	fp = fopen(cmapfname,"r");
	if (fp == NULL)		goto ZXCV;
	
	cnt = 0;
	colormap = (ColorA *) malloc(sizeof(ColorA) * size);
	
	while (fscanf(fp, "%f%f%f%f", 
		&colormap[cnt].r, &colormap[cnt].g, &colormap[cnt].b, &colormap[cnt].a) == 4)
		{
		cnt++;
		if (cnt > size)	{
			size *= 2;
			colormap = (ColorA *) realloc(colormap, sizeof(ColorA) * size);
			if (colormap == NULL) goto ZXCV;
			}
		}
	return(cnt);
/*
	fprintf(stderr,"readcmap: %d entries read\n",cnt);
*/
ZXCV:
	colormap = builtin;
	cnt = sizeof(builtin);
	return(cnt);
}

ColorA
GetCmapEntry(n)
int n;
{
 	if (!doneread)	{
	    char *cmapfile; 
    	    cmapfile = getenv("CMAP_FILE");
	    fprintf(stderr,"Using CMAP_FILE environment variable to read color map\n");
    	    readcmap(cmapfile);
	    }
	if (n < 0  || n > cnt)	return(colormap[0]);
	else return (colormap[n]);
}
