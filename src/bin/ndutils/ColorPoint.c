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
 * ColormapPoint.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#include	<stdio.h>
#include	"ColorMapDefs.h"
#include	"color.h"
#include	"transformn.h"
#include	"BasicFns.h"

extern IOBFILE *infile;

/*This file contains the functions which can be used to determine what color to
make a point under a given colormap.*/

void addcomponentcolor(ColorA * color, HPointN * pt, char *coordsys,
		       dir * plex, char *obj)
{
  TransformN *t;
  int i, j, smdim;
  float hgt, weight;
  HPointN *newpt;
  if (plex->np == 1) {		/*If there is only one mark color the point that color */
    color->r += (*(plex->p))->c.r;
    color->g += (*(plex->p))->c.g;
    color->b += (*(plex->p))->c.b;
    color->a += (*(plex->p))->c.a;
    return;
  }
  /*Find smaller of dimensions of the point and the direction */
  smdim = (pt->dim < plex->dim) ? pt->dim : plex->dim;
  /*If map is specified with respect to the object make an identify matrix */
  if (strcmp(coordsys, obj) == 0) {
    t = TmNIdentity(TmNCreate(pt->dim, pt->dim, NULL));
  }
  /*Otherwise get the appropriate transform */
  else {
    printf("(echo (ND-xform-get %s %s))\n", obj, coordsys);
    fflush(stdout);
    t = TmNRead(infile, 0);
  }
  newpt = HPtNTransform(t, pt, NULL);	/*apply transform */
  hgt = InnerProductN(newpt->v + 1, plex->direction, smdim);	/*get point's height */
  hgt /= newpt->v[0];		/* just in case */
  free(newpt);
  /*Find index of smallest height larger than ours */
  for (i = 0; (i < plex->np) && (hgt > ((plex->p)[i])->height); i++);
  if (i == 0) {			/*If none smaller use smallest */
    color->r += (*(plex->p))->c.r;
    color->g += (*(plex->p))->c.g;
    color->b += (*(plex->p))->c.b;
    color->a += (*(plex->p))->c.a;
    return;
  }
  if (i == plex->np) {		/*If none larger use largest */
    /* cH: that means i - 1, not i. Gnah. Go and code
       Fortran.
     */
    color->r += (*(plex->p + i - 1))->c.r;
    color->g += (*(plex->p + i - 1))->c.g;
    color->b += (*(plex->p + i - 1))->c.b;
    color->a += (*(plex->p + i - 1))->c.a;
    return;
  }				/*Otherwise interpolate in between */
  weight = (hgt - (*(plex->p + i - 1))->height) /
      ((*(plex->p + i))->height - (*(plex->p + i - 1))->height);
  color->r += (*(plex->p + i - 1))->c.r + weight * ((*(plex->p + i))->c.r -
						    (*(plex->p + i - 1))->
						    c.r);
  color->g +=
      (*(plex->p + i - 1))->c.g + weight * ((*(plex->p + i))->c.g -
					    (*(plex->p + i - 1))->c.g);
  color->b +=
      (*(plex->p + i - 1))->c.b + weight * ((*(plex->p + i))->c.b -
					    (*(plex->p + i - 1))->c.b);
  color->a +=
      (*(plex->p + i - 1))->c.a + weight * ((*(plex->p + i))->c.a -
					    (*(plex->p + i - 1))->c.a);
  return;
}

/*Takes a pointer to an HPointN, a pointer to a colormap, and a string identifying
the object to which the point belongs.*/

ColorA *colorpoint(HPointN * pt, colormap * map, char *obj)
{
  ColorA *color;
  int i;
  color = (ColorA *) malloc(sizeof *color);	/*Makes space for colors */
  color->r = color->g = color->b = color->a = 0.0;	/*Starts off with 0's */
  for (i = 0; i < map->numdirs; i++)	/*Color along each direction */
    addcomponentcolor(color, pt, (map->coordsys)[i], (map->plex)[i], obj);
  if (color->r < 0.0)		/*if any color is less than 0 or bigger */
    color->r = 0.0;		/*than 1 then make 0 or 1 respectively */
  if (color->r > 1.0)
    color->r = 1.0;
  if (color->g < 0.0)
    color->g = 0.0;
  if (color->g > 1.0)
    color->g = 1.0;
  if (color->b < 0.0)
    color->b = 0.0;
  if (color->b > 1.0)
    color->b = 1.0;
  if (color->a < 0.0)
    color->a = 0.0;
  if (color->a > 1.0)
    color->a = 1.0;
  return color;
}
