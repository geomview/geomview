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
#define PICKFUNC(name, maxfaceverts, maxpathlen, body)		\
LObject *name(Lake *lake, LList *args)				\
{								\
  char *coordsys = NULL, *id = NULL;				\
  HPoint3 point;  int pn = 4;					\
  HPoint3 vertex; int vn = 4;					\
  HPoint3 edge[2]; int en = 8;					\
  HPoint3 face[maxfaceverts]; int fn = maxfaceverts*4;		\
  int ppath[maxpathlen]; int ppn = maxpathlen;			\
  int vi;							\
  int ei[2]; int ein = 2;					\
  int fi;							\
								\
  LDECLARE(("pick", LBEGIN,					\
	    LSTRING,			&coordsys,		\
	    LSTRING,			&id,			\
	    LHOLD, LARRAY, LFLOAT,	&point, &pn,		\
	    LHOLD, LARRAY, LFLOAT,	&vertex, &vn,		\
	    LHOLD, LARRAY, LFLOAT,	edge, &en,		\
	    LHOLD, LARRAY, LFLOAT,	face, &fn,		\
	    LHOLD, LARRAY, LINT,	ppath, &ppn,		\
	    LINT,			&vi,			\
	    LHOLD, LARRAY, LINT,	ei, &ein,		\
	    LINT,			&fi,			\
	    LEND));						\
								\
  if (1) { body };						\
  return Lt;							\
}

/*
  Note: the "if (1)" business above is to prevent a warning
  about the following statement ("return Lt") not being reached
  if the body itself returns.
*/

/* Note: Don't use DEFPICKFUNC any more.  Use the newer PICKFUNC
 * instead.  DEFPICKFUNC is provided for backward compatibility.
 */
#define DEFPICKFUNC(helpstr,					\
		    coordsys,					\
		    id,						\
		    point, pn,					\
		    vertex, vn,					\
		    edge, en,					\
		    face, fn, maxfaceverts,			\
		    ppath, ppn, maxpathlen,			\
		    vi,						\
		    ei, ein,					\
		    fi,						\
		    body)					\
LDEFINE(pick, LVOID, helpstr)					\
{								\
  char *coordsys = NULL, *id = NULL;				\
  HPoint3 point;  int pn = 4;					\
  HPoint3 vertex; int vn = 4;					\
  HPoint3 edge[2]; int en = 8;					\
  HPoint3 face[maxfaceverts]; int fn = maxfaceverts*4;		\
  int ppath[maxpathlen]; int ppn = maxpathlen;			\
  int vi;							\
  int ei[2]; int ein = 2;					\
  int fi;							\
								\
  LDECLARE(("pick", LBEGIN,					\
	    LSTRING,			&coordsys,		\
	    LSTRING,			&id,			\
	    LHOLD, LARRAY, LFLOAT,	&point, &pn,		\
	    LHOLD, LARRAY, LFLOAT,	&vertex, &vn,		\
	    LHOLD, LARRAY, LFLOAT,	edge, &en,		\
	    LHOLD, LARRAY, LFLOAT,	face, &fn,		\
	    LHOLD, LARRAY, LINT,	ppath, &ppn,		\
	    LINT,			&vi,			\
	    LHOLD, LARRAY, LINT,	ei, &ein,		\
	    LINT,			&fi,			\
	    LEND));						\
								\
  if (1) { body };						\
  return Lt;							\
}

