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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * mouse.c: mouse computations
 */

#include "window.h"
#include "common.h"
#include "mouse.h"

Event oldmouse;
static float olddist = 0;

void
mousezero(Event *event,  WnPosition *wp)
{
  oldmouse = *event;
}


void
mousedisp(Event *event, float *dx, float *dy, unsigned long int *dt, WnPosition *wp)
{
  *dx = 2. * (event->x - oldmouse.x) / (wp->xmax - wp->xmin + 1);
  *dy = 2. * (event->y - oldmouse.y) / (wp->ymax - wp->ymin + 1);
  *dt = event->t - oldmouse.t;
D1PRINT(("mousedisp: dx <- %10f, dy <- %10f, dt <- %ld\n", *dx, *dy, *dt));
  mousezero(event, wp);
}

void
radialmousezero(Event *event, WnPosition *wp)
{
  float x,y;
  float xw = wp->xmax - wp->xmin + 1;
  float yw = wp->ymax - wp->ymin + 1;

  x = (event->x - wp->xmin) / xw - 0.5;
  y = (event->y - wp->ymin) / yw - 0.5;
  olddist = sqrt( x*x + y*y );
}

float
radialmousedisp(Event *event, WnPosition *wp)
{
  float x, y, dist, val;
  float xw = wp->xmax - wp->xmin + 1;
  float yw = wp->ymax - wp->ymin + 1;

  x = (event->x - wp->xmin) / xw - 0.5;
  y = (event->y - wp->ymin) / yw - 0.5;

  dist = sqrt( x*x + y*y );
  val = dist - olddist;
  olddist = dist;
  return 2 * val;
}

void
mousemap(int x, int y, float *rx, float *ry, WnPosition *wp)
{
  *rx = 2. * (x - wp->xmin) / (wp->xmax - wp->xmin + 1) - 1;
  *ry = 2. * (y - wp->ymin) / (wp->ymax - wp->ymin + 1) - 1;
}
