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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Note: we do not use OOGLNew() & friends because libc would neither do so. */
static inline void *xmalloc(size_t size)
{
  void *ptr;
  
  if ((ptr = malloc(size)) == NULL) {
    fprintf(stderr, "malloc(%d) failed in strdup().\n", size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

#undef strdup
char *strdup(const char *str )
{
    char *s;
    int len;

    if(str == NULL)
	return NULL;
    len = strlen(str);
    s = xmalloc(len+1);
    memcpy(s, str, len+1);
    return s;
}
/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
