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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "transform3.h"

void
Ctm3AlignX( Transform3 T, Point3 *axis )
{
    Ctm3Align( T, axis, &TM3_XAXIS );
}

void
Ctm3AlignY( Transform3 T, Point3 *axis )
{
    Ctm3Align( T, axis, &TM3_YAXIS );
}

void
Ctm3AlignZ( Transform3 T, Point3 *axis )
{
    Ctm3Align( T, axis, &TM3_ZAXIS );
}

void
Ctm3Align( Transform3 T, Point3 *axis, Point3 *newaxis )
{
    Transform3 Ta;

    Tm3Align( Ta, axis, newaxis );
    Tm3Concat(Ta, T, T);
}
