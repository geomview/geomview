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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */


#ifdef IRIX3	/* Define -DIRIX3 in makefiles/mk.sgi  to enable this.  */

/*
 * Define some routines not present in Irix 3.x:
 *	GLXwinset()	(used by mg libraries, if they're told to)
 */

#include "ooglutil.h"

GLXwinset() {
	OOGLError(1, "Hey, somebody called GLXwinset()!");
}

#endif /*IRIX3*/


#if (defined(__hpux) && defined(__GNUC__))

#include <math.h>

	/* HP-UX native cc does have finite() and acosh() in its library,
	 * but gcc (at least our 2.5.8) doesn't find it there.
	 */
int finite(double v) { return (!isinf(v) && !isnan(v)); }

double acosh(double c) {
    /* cosh(x) = (e^x + e^-x)/2
     * u = e^x
     * u^2 - 2c u + 1 = 0
     * u = c +/- sqrt(c^2 - 1)
     */
    return log(c + sqrt(c*c - 1));
}
#endif /*hpux gcc*/

#ifdef SVR4
  /* Solaris lacks bcopy, bzero */

void bcopy(char *src, char *dst, int len) { memcpy(dst, src, len); }
void bzero(char *mem, int len)            { memset(mem, 0, len); }

#endif


#ifdef _WIN32
/* Visual C++ lacks strcasecmp()! */

int strcasecmp(char *a, char *b)
{
  int ca, cb;
  while((ca = *a) && (cb = *b)) {
    if(ca != cb) {
	if(ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
	if(cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
	if(ca != cb)
	    return ca - cb;
    }
    a++, b++;
  }
  return 0;
}

int strncasecmp(char *a, char *b, int n)
{
  int ca, cb;
  while(--n >= 0 && (ca = *a) && (cb = *b)) {
    if(ca != cb) {
	if(ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
	if(cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
	if(ca != cb)
	    return ca - cb;
    }
    a++, b++;
  }
  return 0;
}

int finite(double v)
{
  return (! (v <= 0.0 || v > 0.0));
}

double acosh(double c) {
    /* cosh(x) = (e^x + e^-x)/2
     * u = e^x
     * u^2 - 2c u + 1 = 0
     * u = c +/- sqrt(c^2 - 1)
     */
    return log(c + sqrt(c*c - 1));
}

FILE *popen(char *name, char *mode) {
    fprintf(stderr, "Call to stub popen(%s, %s)!\n", name, mode);
    return NULL;
}

#endif /*_WIN32*/
