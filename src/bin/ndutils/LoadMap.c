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
 * LoadMap.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ColorMapDefs.h"
#include "ooglutil.h"

/*The folowing code takes a file pointer and a file name (which it never uses)
and loads the contents of that file into a colormap.  This is not a very well
written function.  It depends on the entire file being a single line and the
spacing being in exactly a certain manner.  It works correctly on the results of
a query to Geomview of the the form "(echo(ND-color [camid])\n)" but it probably
should be rewritten to work more generally*/

colormap *NDcolorFLoad(IOBFILE * fp, char *name)
{
  int i, j, k;
  char *ptr, *findend;
  colormap *map;
  char input[1000];
  iobfgets(input, 1000, fp);
  map = (colormap *) malloc(sizeof *map);
  for (ptr = input, i = 0; *ptr != '\0'; ptr++)
    if (*ptr == '(')
      i++;
  i = (i - 1) / 2;
  if (i == 0) {
    map->numdirs = 0;
    map->coordsys = NULL;
    map->plex = NULL;
    return map;
  }
  map->numdirs = i;
  map->coordsys = (char **) malloc((sizeof *(map->coordsys) * i));
  map->plex = (dir **) malloc((sizeof *(map->plex) * i));
  for (k = 0; k < i; k++)
    (map->plex)[k] = (dir *) malloc(sizeof *((map->plex)[k]));
  ptr = input + 3;
  for (j = 0; j < map->numdirs; j++) {
    for (findend = ptr; *findend != '"'; findend++);
    (map->coordsys)[j] = (char *) malloc((sizeof *((map->coordsys)[j]))
					 *
					 (((size_t) findend -
					   (size_t) ptr) + 1));
    memcpy((map->coordsys)[j], ptr,
	   (sizeof *((map->coordsys)[j])) * ((size_t) findend -
					     (size_t) ptr));
    ((map->coordsys)[j])[(size_t) findend - (size_t) ptr] = '\0';
    ptr = findend + 2;
    for (; *ptr != '('; ptr++);
    ptr++;
    for (findend = ptr, i = 0; *findend != ')'; findend++)
      if (*findend == ' ')
	i++;
    ((map->plex)[j])->direction = (float *) malloc
      ((sizeof *((map->plex)[j])->direction) * (i + 1));
	 ((map->plex)[j])->dim = i + 1; for (k = 0; k < i + 1; k++) {
	 *((map->plex)[j]->direction + k) = atof(ptr);
	 for (; *ptr != ' '; ptr++); ptr++;}
	 ptr = findend + 2;
	 for (findend = ptr, k = 0; *findend != ')'; findend++)
	 if (*findend == ' ')
	 k++;
	 k = (k + 1) / 5;
	 ((map->plex)[j])->np = k;
	 ((map->plex)[j])->p = (mark **) malloc
	 ((sizeof *(((map->plex)[j])->p)) * k); for (i = 0; i < k; i++) {
	 (((map->plex)[j])->p)[i] = (mark *) malloc
	 (sizeof *((((map->plex)[j])->p)[i]));
	 ((((map->plex)[j])->p)[i])->height = atof(ptr);
	 for (; *ptr != ' '; ptr++);
	 ptr++;
	 ((((map->plex)[j])->p)[i])->c.r = atof(ptr);
	 for (; *ptr != ' '; ptr++);
	 ptr++;
	 ((((map->plex)[j])->p)[i])->c.g = atof(ptr);
	 for (; *ptr != ' '; ptr++);
	 ptr++;
	 ((((map->plex)[j])->p)[i])->c.b = atof(ptr);
	 for (; *ptr != ' '; ptr++);
	 ptr++;
	 ((((map->plex)[j])->p)[i])->c.a = atof(ptr);
	 for (; *ptr != ' '; ptr++); ptr++;}
	 ptr = findend + 4;}
	 return map;}
