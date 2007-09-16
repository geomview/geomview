/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Claus-Justus Heine
 */

/*
 * Handle NULL or uninitialized times.
 */
struct timeval *timeof(struct timeval *when)
{
   static struct timeval now;
   if ((when == NULL && (when = &now)) || !timerisset(when))
	gettimeofday(when, NULL);
   return when;
}
    
void addtime(struct timeval *result, const struct timeval *base, double offset)
{
    double osec = floor(offset);
    result->tv_sec = base->tv_sec + osec;
    result->tv_usec = base->tv_usec + (int)((offset - osec)*1000000);
    while(result->tv_usec >= 1000000) {
	result->tv_sec++;
	result->tv_usec -= 1000000;
    }
}

double timeoffset(struct timeval *when, struct timeval *base)
{
  return
    (double)(when->tv_sec - base->tv_sec)
    +
    1e-6*(double)(when->tv_usec - base->tv_usec);
}

