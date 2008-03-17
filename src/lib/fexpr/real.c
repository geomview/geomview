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

#include <math.h>
#include "fexpr.h"

double freal_cot(double a)
{
  return cos(a)/sin(a);
}

double freal_sec(double a)
{
  return 1/cos(a);
}

double freal_csc(double a)
{
  return 1/sin(a);
}

double freal_re(double a)
{
  return a;
}

double freal_im(double a)
{
  (void)a;
  return 0;
}

double freal_monstub(double a)
{
  double b = 0.0;
  double c = 0.0;
  (void)a;
  return b/c;
}

double freal_binstub(double a1, double a2)
{
  double a = 0.0;
  double b = 0.0;
  (void)a1;
  (void)a2;
  return a/b;
}
