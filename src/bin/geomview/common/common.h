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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef COMMON_H
#define COMMON_H

#include "mg.h"
#include "geom.h"
#include "pick.h"
#include "lispext.h"
#include "clang.h"

typedef int (*PFI)();
typedef void (*PFV)();

typedef struct Event {
  int dev;
  int val;
  int x,y;
  unsigned long int t;
  Pick *p;
} Event;

#if D1_DEBUG
void print_event(char *s, Event *e);
#define PRINT_EVENT(args) print_event args
#else
#define PRINT_EVENT(args) 
#endif

#endif /* ! COMMON_H */
