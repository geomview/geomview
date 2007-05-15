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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * porting.h: Declarations for porting to brain-damaged operating systems.
 *
 *  ooglutil.h includes this file
 */
#ifndef GV_PORTING_H
#define GV_PORTING_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_SYS_TYPE_H
# include <sys/types.h>
#endif
#if HAVE_STDIO_H
# include <stdio.h>
#endif
#if HAVE_GETOPT_H
# include <getopt.h>
#endif
#if HAVE_UNISTD_H /* getopt() on AIX */
# include <unistd.h>
#endif

#if !HAVE_M_PI
# define M_PI 3.14159265358979323846	/* pi */
#endif

#if !HAVE_BCOPY
static inline void bcopy(char *src, char *dst, int len)
{
  memcpy(dst, src, len);
}
#endif

#if !HAVE_BZERO
static inline void bzero(char *mem, int len)
{
  memset(mem, 0, len);
}
#endif

#if !HAVE_FINITE
static inline int finite(double v)
{
  return (! (v <= 0.0 || v > 0.0));
}
#endif

#if !HAVE_DECL_STRCASECMP
extern int strcasecmp(char *s1, char *s2);
#endif

#if !HAVE_DECL_STRNCASECMP
extern int strncasecmp(char *a, char *b, int n);
#endif

#if !HAVE_DECL_ACOSH
double acosh(double c);
#endif

#if !HAVE_DECL_STRDUP
extern char *strdup(const char *);
#endif

#if !HAVE_DECL_PUTENV
extern putenv(char *name);
#endif

/* supply missing declaration for fmemopen */
#if !HAVE_DECL_FMEMOPEN
extern FILE *fmemopen(void *buf, size_t buflen, char *mode);
#endif

#if !HAVE_DECL_GETOPT
extern int getopt(int argc, char * const argv[], const char *optstring);
#endif

#if !HAVE_DECL_OPTARG
extern char *optarg;
#endif

#if !HAVE_DECL_OPTIND
extern int optind;
#endif

#if !HAVE_DECL_OPTERR
extern int opterr;
#endif

#if !HAVE_DECL_OPTOPT
extern int optopt;
#endif

#endif /* GV_PORTING_H */
