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

#ifndef STUBDEFS
#define STUBDEFS

#include "geomclass.h"

extern GeomClass *StubMethods();
void StubRoutine();


/*
 * This file is #include'd by aspiring per-class stub routines.
 * Each STUBMETHODS() macro creates a Present tag indicating that the
 * corresponding library is not loaded, and a Methods() routine
 * which complains if called.
 * They're useful as components of a libstub.a library used to link with
 * applications that only need a few of the OOGL object-types.
 * Assuming classA, classB, etc. are needed, link as in
 *    -lgeom -lclassA -lclassB ... -lgeom -lstub   -lbbox -l3d -lm
 *
 * Use of the method-specific stubs (e.g. libstubdraw) is quite different;
 * see stubdraw.c.
 */

#if defined(__STDC__) || defined(__ANSI_CPP__)

#define STUBMETHODS(prefix, meth, lib) \
	int prefix##Present = 0; \
	GeomClass *prefix##Methods() { return StubMethods(meth, lib); }

#else /*plain C*/

#define	STUBMETHODS(prefix, meth, lib) \
	int prefix/**/Present = 0; \
	GeomClass *prefix/**/Methods() { return StubMethods(meth, lib); }
#endif
#endif
