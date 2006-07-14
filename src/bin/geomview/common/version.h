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
/*
 * GEOMVIEW_VERSION is a string describing the current version of
 * geomview.  As new versions are created this string should be
 * modified in a lexicographically increasing manner.  This is the
 * string that appears in various panels and is what the
 * (geomview-version) lisp command returns.
 *
 * NOTE: As of verison 1.7.5 the GEOMVIEW_VERSION string now comes from
 * the VERSION macro defined in the configure-generated header file
 * config.h.  This number is specified in "configure.in" in the top-level
 * directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef GEOMVIEW_VERSION
#define GEOMVIEW_VERSION VERSION
#endif
