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

#include <stdlib.h>
#include "ooglutil.h"
#include "windowP.h"

#define SETFLAG(flag, bit, value)	\
      if (value) flag |= bit;		\
      else	 flag &= ~bit

#define GETFLAG(flag, bit)	( (flag & bit) != 0 )


static void WnSetName(WnWindow *win, char *name);

WnWindow *
WnCreate(int firstattr, ...)
{
    WnWindow *win;
    va_list a_list;

    win = OOGLNewE ( WnWindow, "WnCreate WnWindow" );
    RefInit((Ref *)win, WINDOWMAGIC);
    win->xsize = -1;
    win->ysize = -1;
    win->pixaspect = 1.0;
    win->win_name = NULL;
    win->changed = win->flag = WNF_ENLARGE | WNF_SHRINK;

    va_start(a_list, firstattr);
    win = _WnSet(win, firstattr, &a_list);
    va_end( a_list );
    return win;
}

static void
WnSetName(WnWindow *win, char *name)
{
  if (win->win_name != NULL) free(win->win_name);
  win->win_name = name ? strdup(name) : NULL;
}

WnWindow *
WnSet(WnWindow *win, int firstattr, ...)
{
    va_list a_list;

    va_start(a_list, firstattr);
    win = _WnSet(win, firstattr, &a_list);
    va_end( a_list );
    return win;
}

WnWindow *
_WnSet(WnWindow *win, int firstattr, va_list *alist)
{
    int attr;
    WnPosition *pos;
    int oldflag;
    char *name;
    char **ablock = NULL;

#define NEXT(type) OOGL_VA_ARG(type,alist,ablock)

    /* Parse args */
    for(attr = firstattr; attr != WN_END; attr = NEXT(int)) {
      switch (attr) {
        case WN_ABLOCK:
	    ablock = NEXT(char **);
	    break;
	case WN_XSIZE:
	    win->xsize = NEXT(int);
	    goto sized;

	case WN_YSIZE:
	    win->ysize = NEXT(int);
	  sized:
	    win->flag |= WNF_HASSIZE; 
	    win->changed |= WNF_HASSIZE;
	    break;

	case WN_CURPOS:
	    pos = NEXT(WnPosition *);
	    if(pos) {
		win->cur = *pos;
		win->aspect = (pos->ymax != pos->ymin) ? 
		  (double)(pos->xmax - pos->xmin)
		    / (double)(pos->ymax - pos->ymin) : 1.0;
		win->xsize = win->cur.xmax - win->cur.xmin + 1;
		win->ysize = win->cur.ymax - win->cur.ymin + 1;
		win->flag |= WNF_HASCUR|WNF_HASSIZE;
	    } else {
		win->flag &= ~WNF_HASCUR;
	    }
	    win->changed |= WNF_HASCUR;
	    break;

	case WN_PREFPOS:
	    pos = NEXT(WnPosition *);
	    if(pos) {
		win->pref = *pos;
		win->flag |= WNF_HASPREF; 
	    } else
		win->flag &= ~WNF_HASPREF;
	    win->changed |= WNF_HASPREF;
	    break;

	case WN_VIEWPORT:
	    pos = NEXT(WnPosition *);
	    if(pos) {
		win->viewport = *pos;
		win->flag |= WNF_HASVP;
	    } else {
		win->flag &= ~WNF_HASVP;
	    }
	    win->changed |= WNF_HASVP;
	    break;

	case WN_NAME:
	    name = NEXT(char *);
	    WnSetName(win, name);
	    if (name != NULL)
	      win->flag |= WNF_HASNAME;
	    else
	      win->flag &= ~WNF_HASNAME;
	    win->changed |= WNF_HASNAME;
	    break;

	case WN_ENLARGE:
	    oldflag = win->flag;
	    SETFLAG(win->flag, WNF_ENLARGE, NEXT(int));
	    if (oldflag != win->flag) win->changed |= WNF_ENLARGE;
	    break;

	case WN_SHRINK:
	    oldflag = win->flag;
	    SETFLAG(win->flag, WNF_SHRINK, NEXT(int));
	    if (oldflag != win->flag) win->changed |= WNF_SHRINK;
	    break;

	case WN_NOBORDER:
	    oldflag = win->flag;
	    SETFLAG(win->flag, WNF_NOBORDER, NEXT(int));
	    if (oldflag != win->flag) win->changed |= WNF_NOBORDER;
	    break;

	case WN_PIXELASPECT:
	    win->pixaspect = NEXT(double);
	    win->changed |= WNF_HASPIXASPECT;
	    win->flag |= WNF_HASPIXASPECT;
	    break;

	default:
	    OOGLError (0, "_WnSet: Undefined attribute: %d", attr);
	    return NULL;
	}
    }
    return(win);
#undef NEXT
}


/*
 * Get a value.
 * Returns -1 for invalid attribute, 0 if no value set, 1 if valid value.
 */
int
WnGet(WnWindow *win, int attr, void *valp)
{
    int bit;

#define WnPos  WnPosition

    switch(attr) {
    case WN_XSIZE:	
    case WN_YSIZE:
	bit = WNF_HASSIZE;
	if(win->flag & WNF_HASCUR) {
	    win->xsize = win->cur.xmax - win->cur.xmin + 1;
	    win->ysize = win->cur.ymax - win->cur.ymin + 1;
	    bit = WNF_HASCUR;
	} else if((win->flag & (WNF_HASPREF|WNF_HASSIZE)) == WNF_HASPREF) {
	    win->xsize = win->pref.xmax - win->pref.xmin + 1;
	    win->ysize = win->pref.ymax - win->pref.ymin + 1;
	    bit = WNF_HASPREF;
	}
	*(int *)valp = attr == WN_XSIZE ? win->xsize : win->ysize;
	break;
    case WN_CURPOS:
	if(!(win->flag & WNF_HASCUR)) {
	    win->cur.xmin = win->cur.ymin = 0;
	    win->cur.xmax = win->xsize + 1;
	    win->cur.ymax = win->ysize + 1;
	}
	*(WnPos *)valp = win->cur;	bit=WNF_HASCUR;	 break;
    case WN_PREFPOS:	*(WnPos *)valp = win->pref;	bit=WNF_HASPREF; break;
    case WN_VIEWPORT:
	if(!(win->flag & WNF_HASVP)) {
	    /* Fill in viewport structure with something reasonable, if we can.
	     */
	    win->viewport.xmin = 0;  win->viewport.ymin = 0;
	    if(win->flag & WNF_HASCUR) {
		win->viewport.xmax = win->cur.xmax - win->cur.xmin + 1;
		win->viewport.ymax = win->cur.ymax - win->cur.ymin + 1;
	    } else if(win->flag & WNF_HASSIZE) {
		win->viewport.xmax = win->xsize;
		win->viewport.ymax = win->ysize;
	    } else {
		win->viewport.xmax = win->pref.xmax - win->pref.xmin + 1;
		win->viewport.ymax = win->pref.ymax - win->pref.ymin + 1;
	    }
	}
	*(WnPos *)valp = win->viewport; bit=WNF_HASVP; break;
    case WN_NAME:	*(char**)valp = win->win_name;	bit=WNF_HASNAME; break;
    case WN_ENLARGE:	*(int*)valp = GETFLAG(win->flag,WNF_ENLARGE); return 1;
    case WN_SHRINK:	*(int*)valp = GETFLAG(win->flag,WNF_SHRINK); return 1;
    case WN_NOBORDER:	*(int*)valp = GETFLAG(win->flag,WNF_NOBORDER); return 1;
    case WN_ASPECT:	*(float*)valp = win->aspect*win->pixaspect; return 1;
    case WN_PIXELASPECT: *(float*)valp = win->pixaspect; return 1;

    default:		return -1;
    }
    return win->flag & bit ? 1 : 0;
}

void
WnDelete(WnWindow *win)
{
    if(win == NULL)
	return;

    if(win->magic != WINDOWMAGIC) {
	OOGLWarn("Internal warning: WnDelete on non-WnWindow %x (%x != %x)",
		win, win->magic, WINDOWMAGIC);
	return;
    }
    if(RefDecr((Ref *)win) > 0)
	return;
    win->magic ^= 0x80000000;
    OOGLFree(win);
}

WnWindow *
WnCopy(WnWindow *win)
{
    WnWindow *nw = WnCreate(WN_END);

    if(win) {
	*nw = *win;
	RefInit((Ref *)nw, WINDOWMAGIC);
	/*
	 * This code assumes there are no (other) pointers in a WnWindow,
	 * i.e. a bitwise copy is enough.
	 */
	if(nw->win_name) nw->win_name = strdup(nw->win_name);
    }
    return nw;
}

WnWindow *
WnMerge(WnWindow *src, WnWindow *dst)
{
    int chg;

    if(src == NULL) return dst;
    if(dst == NULL) return NULL;

    chg = src->changed;

    dst->flag = (dst->flag & ~chg) | (src->flag & chg);
    dst->changed |= chg;

    if(chg & WNF_HASPREF) dst->pref = src->pref;
    if(chg & WNF_HASCUR) dst->cur = src->cur, dst->aspect = src->aspect;
    if(chg & WNF_HASSIZE) {
	dst->xsize = src->xsize;
	dst->ysize = src->ysize;
	/* If "size" is set and not cur/pref/vp, invalidate those. */
	dst->flag &= ~((WNF_HASCUR|WNF_HASPREF|WNF_HASVP) & ~chg);
    }
    if(chg & WNF_HASVP) dst->viewport = src->viewport;
    if(chg & src->flag & WNF_HASNAME) WnSetName(dst, src->win_name);
    if(chg & WNF_HASPIXASPECT) dst->pixaspect = src->pixaspect;
    return dst;
}
