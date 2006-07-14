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

/*
 * bdy.c
 * author:  Celeste Fowler, Mark Phillips
 * date:  June 12, 1992
 */

#include <stdio.h>
#include <stdlib.h>

#include "geom.h"
#include "bdy.h"

int main(int argc, char *argv[]) {
  Geom *o, *v;
  float p;

  if (argc > 1) sscanf(argv[1], "%f", &p);
  else p = 0.0;

  o = GeomFLoad(iobfileopen(stdin), NULL);
  v = Bdy(o, p);

  GeomFSave(v, stdout, NULL);

  fflush(stdout);

  GeomDelete(v);
  GeomDelete(o);

  fclose(stdin);
  fclose(stdout);
  exit(0);
}
