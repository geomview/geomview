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

#include "color.h"
#include <stdlib.h>

void
CoPrint( Color *c )
{
  (void)c;
}

void
CoCopy( Color *c1, Color *c2 )
{
    *c2 = *c1;
}

void
CoACopy( ColorA *c1, ColorA *c2 )
{
    *c2 = *c1;
}

void
CoFrom( Color *c, float r, float g, float b )
{
    c->r = r;
    c->g = g;
    c->b = b;
}

void
CoAdd( Color *c1,Color *c2, Color *c3 )
{
    c3->r = c1->r + c2->r;
    c3->g = c1->g + c2->g;
    c3->b = c1->b + c2->b;
}

void
CoSub( Color *c1, Color *c2, Color *c3 )
{
    c3->r = c1->r - c2->r;
    c3->g = c1->g - c2->g;
    c3->b = c1->b - c2->b;
}
void
CoFilter( Color *c1, Color *c2, Color *c3 )
{
    c3->r = c1->r * c2->r;
    c3->g = c1->g * c2->g;
    c3->b = c1->b * c2->b;
}


#define vH (hsv->r)
#define vS (hsv->g)
#define vV (hsv->b)

/* HSV to RGB conversion from Ken Fishkin, pixar!fishkin */
void hsv2rgb(Color *hsv, Color *rgb)
{
    float h = 6.0 * (vH < 0 ? vH + (1 - (int)vH) : vH - (int)vH);
    int sextant = (int) h; /* implicit floor */
    float fract = h - sextant;
    float vsf = vS*vV*fract;
    float min = (1-vS)*vV;
    float mid1 = min + vsf;
    float mid2 = vV - vsf;
    switch (sextant%6) {
    case 0: rgb->r = vV;   rgb->g = mid1; rgb->b = min; break;
    case 1: rgb->r = mid2; rgb->g = vV;	  rgb->b = min; break;
    case 2: rgb->r = min;  rgb->g = vV;	  rgb->b = mid1; break;
    case 3: rgb->r = min;  rgb->g = mid2; rgb->b = vV; break;
    case 4: rgb->r = mid1; rgb->g = min;  rgb->b = vV; break;
    case 5: rgb->r = vV;   rgb->g = min;  rgb->b = mid2; break;
    }
}

void rgb2hsv(Color *rgb, Color *hsv)
{

#define cRGB (&rgb->r)

    int min, max;
    float dv;
    if(cRGB[0] < cRGB[1])
	min = 0, max = 1;
    else
	min = 1, max = 0;
    if(cRGB[min] > cRGB[2]) min = 2;
    else if(cRGB[max] < cRGB[2]) max = 2;

    vV = cRGB[max];
    dv = cRGB[max] - cRGB[min];
    if(dv == 0) {
	vH = 0;	/* hue undefined, use 0 */
	vS = 0;
    } else {
	float dh = (cRGB[3 - max - min] - cRGB[min]) / (6*dv);
	vH = (3+max-min)%3==1 ? max/3.0 + dh : max/3.0 - dh;
	if(vH < 0) vH += 1 + (int)vH;
	if(vH > 1) vH -= (int)vH;
	vS = dv / cRGB[max];
    }
}
