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

#ifndef _GEOMCLASS_SM_
#define _GEOMCLASS_SM_

#define	IsInSM(addr, sm)   ((sm)->sm_base <= (char *)(addr) && \
			     (char *)(addr) < (sm)->sm_base + (sm)->sm_mapped)

#define	GeomRefreshSM(sm)  ((sm)->sm_mapped < ROOTSYM(sm)->s_size ? GeomRemapSM(sm) : (sm)->sm_mapped)

#define	ROOTSYM(sm)	((GeomSMSym *)(sm)->sm_base)

#define	GOffset(sym, type, ptr)  ((type *)(((char *)(ptr)) + \
				   (((char *)(sym)) - (sym)->s_here)))

#define	MAXSM		4	/* Max number of simultaneous shared mem's */

#define	SM_MAGIC  0x4AB3B8B9	/* s_version field for SM root symbol */

#endif _GEOMCLASS_SM_
