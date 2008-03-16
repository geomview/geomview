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
#ifndef _GV_VVVEC_H_
#define _GV_VVVEC_H_

#include <ooglutil.h>

/*
 * Variable-sized arrays ("vectors").
 */
static inline void vvinit(vvec *v, int elsize, int minelems);
static inline void vvuse(vvec *v, void *buf, int allocated);
static inline void vvzero(vvec *v);
static inline void vvtrim(vvec *v);
static inline void vvfree(vvec *v);
static inline void vvneeds(vvec *v, int needed);
static inline void *vvindex(vvec *v, int index);
static inline void vvcopy(vvec *src, vvec *dest);

static inline void
vvinit(vvec *v, int elsize, int minelems)
{
    v->elsize = elsize;
    v->count = 0;
    v->malloced = 0;
    v->dozero = 0;
    v->allocated = -minelems;
    v->base = NULL;
}

static inline void
vvuse(vvec *v, void *buf, int allocated)
{
    vvfree(v);
    v->base = buf;
    v->allocated = allocated;
}

static inline void
vvzero(vvec *v)
{
    v->dozero = 1;
    if(v->allocated > v->count)
	memset(v->base + v->elsize*v->count, 0,
		v->elsize * (v->allocated - v->count));
}

/*
 * Trim vvec to minimum size; ensure its buffer is malloced if it wasn't.
 */
static inline void
vvtrim(vvec *v)
{
    int newalloc = (v->count > 0 ? v->count : 1);
    int want = newalloc * v->elsize;
    static char why[] = "trimming vvec";

    (void)why;

    if(!v->malloced) {
	void *base = OOGLNewNE(char, want, why);
	if(v->base) memcpy(base, v->base, want);
	else memset(base, '\0', want);
	v->base = base;
	v->malloced = 1;
    } else if(v->allocated > v->count) {
	v->base = (void *)OOGLRenewNE(char, v->base, want, why);
    } else
	return;
    v->allocated = newalloc;
}

static inline void
vvfree(vvec *v)
{
    if(v->malloced) {
	OOGLFree(v->base);
	v->base = NULL;
	v->malloced = 0;
    }
}

static inline void
vvneeds(vvec *v, int needed)
{
    if(needed > v->allocated) {
	int had = v->allocated;
	int want = needed + (needed>>2) + 1;

	if(had < 0) {
	    if(want < -had)
		want = -had;
	    had = 0;
	} else {
	    int next = had + (had>>1) + 2;
	    if(next > needed)
		want = next;
	}

	if(v->malloced) {
	    v->base = OOGLRenewNE(char, v->base,
			want * v->elsize, "extending vvec");
	    if(had > v->count) had = v->count;
	} else {
	    void *was = v->base;
	    v->base = OOGLNewNE(char, want * v->elsize, "allocating vvec");
	    if(v->count > 0 && had > 0)
		memcpy(v->base, was, (v->count<had ? v->count:had) * v->elsize);
	}
	v->allocated = want;
	v->malloced = 1;
	if(v->dozero)
	    memset(v->base + v->elsize * had, 0,
		v->elsize * (want-had));
    }
}

static inline void *
vvindex(vvec *v, int index)
{
    if(index < 0) {
	OOGLError(1, "negative array index: %d", index);
	return v->base;
    }
    if(index >= v->allocated)
	vvneeds(v, index+1);
    if(index >= v->count)
	v->count = index+1;
    return v->base + index*v->elsize;
}

static inline void
vvcopy(vvec *src, vvec *dest) 
{
  char *newbase;
  if(src->base == NULL) {
    *dest = *src;
  } else {
    vvneeds(dest, src->allocated);
    newbase = dest->base;
    *dest = *src;
    dest->base = newbase;
    memcpy(dest->base, src->base, dest->allocated * dest->elsize);
  }
}

#endif
