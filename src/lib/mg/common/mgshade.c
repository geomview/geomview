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

/*
 * Software shaders.
 * These operate in the mg environment.
 */
#include "mgP.h"

#define	DOT(a,b)  ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)

#define	CCMUL(A,B,C)  ((C).r=(A).r*(B).r, (C).g=(A).g*(B).g, (C).b=(A).b*(B).b)
#define	CCADD(A,B,C)  ((C).r=(A).r+(B).r, (C).g=(A).g+(B).g, (C).b=(A).b+(B).b)

#define CSCALE(s, A, B) ( (B).r = (A).r * s, (B).g = (A).g * s, (B).b = (A).b * s, (B).a = (A).b * s)


#define	CCSADD(A,B,s,C)	\
	 ((C).r += (s)*(A).r*(B).r, \
	  (C).g += (s)*(A).g*(B).g, \
	  (C).b += (s)*(A).b*(B).b)

/*
 * Transform a vector covariantly
 */
static void
cotransform(Transform Tinv, Point3 *in, Point3 *out)
{
    out->x = Tinv[0][0]*in->x + Tinv[0][1]*in->y + Tinv[0][2]*in->z;
    out->y = Tinv[1][0]*in->x + Tinv[1][1]*in->y + Tinv[1][2]*in->z;
    out->z = Tinv[2][0]*in->x + Tinv[2][1]*in->y + Tinv[2][2]*in->z;
}

static float euck1 = 1.0;		/* inverse power falloff */
static float euck2 = 0.0;		/* safety zone around light */
static float fog = 0.0;			/* fog control */
#if 0
static float hypk1 = .5;		/* controls exponential falloff */
static float hypk2 = .0;		/* controls exponential falloff */
static float sphk1 = .5;		/* controls exponential falloff */
static float sphk2 = 0.0;		/* safety zone around light */
#endif

/*
 * Euclidean shader
 *
 * We depend on the following mg features:
 *	_mgc->xstk->hasinv (cleared whenever transform changes)
 *	_mgc->W2C, C2W (initialized from camera at mgworldbegin())
 */
 
int mg_eucshade(int nv, HPoint3 *v, Point3 *n, ColorA *c, ColorA *cs)
{
    struct mgxstk *mx = _mgc->xstk;
    struct mgastk *ma = _mgc->astk;
    struct LtLight *l, **lp;
    int i, lno;
    HPoint3 V;
    Point3 N, I;
    float s;
    Color Ca, Ci;

    euck1 = ma->lighting.attenmult;
    euck2 = ma->lighting.attenmult2;
    fog = ma->lighting.attenconst > 1 ? ma->lighting.attenconst-1 : 0;
#define	EYE	((HPoint3 *) (_mgc->C2W[3]))	/* Eye (camera) position as an HPoint3 */

    if(!mx->hasinv) {
	TmInvert(mx->T, mx->Tinv);
	mx->hasinv = 1;
    }

    /* "ambient" color */
    
    Ca.r = Ca.g = Ca.b = 0;
    CCSADD(ma->mat.ambient, ma->lighting.ambient, ma->mat.ka, Ca);

    for(i = 0; i < nv; i++, v++, n++, c++, cs++) {
	ColorA *thiscolor = c;
	float num;

					/* Transform point by T */
	HPt3Transform(mx->T, v, &V);	/* V = v in world coords */
	HPt3SubPt3(EYE, &V, &I);	/* I = EYE - V (point-to-eye vector) */

	/* Is this vertex behind our eye?  If so, let's not shade it.
	 * Speed up Shape of Space rendering.  This code may be questionable:
	 * what if this is a vertex of a smooth-shaded polygon that extends
	 * in front of our eye, too?
	 */
	if(DOT(I, *((HPoint3 *)_mgc->C2W[2])) < 0) {
	    /* Dummy -- just Ci = Cs. */
	    *cs = *c;
	    continue;
	}

	Pt3Unit(&I);

					/* Transform normal by T'^-1 */
	cotransform(mx->Tinv, n, &N);
					/* check if normal has 0 length! */
	num = sqrt(DOT(N,N));
	if (num == 0.0)
	  num = 1;
	s = 1/num;

	if(DOT(N,I) < 0) {
	    s = -s;			/* Flip normal to face the viewer */

		/* Hack: use otherwise-useless emission color, if defined,
		 * as the back-side material color.
		 */
	    if(ma->mat.valid & MTF_EMISSION)
		    thiscolor = (ColorA*)(void *)&ma->mat.emission;
	}

	N.x *= s;  N.y *= s;  N.z *= s;
	
					/* Ambient term */
	Ci = Ca;

	lp = &ma->lighting.lights[0];
	for(lno = 0; lno<AP_MAXLIGHTS && (l = *lp) != NULL; lno++, lp++) {
	    Point3 *L;
	    Point3 Lt;
	    float bright, ls, ll, ln, power;

					/* Diffuse term */
	    bright = l->intensity;
	    if(l->globalposition.w == 0) {
		    L = (Point3 *)(void *)&l->globalposition;
		ll = DOT(*L,*L);
	    } else {
		HPt3SubPt3(&l->globalposition, &V, &Lt);
		L = &Lt;
		ll = DOT(Lt,Lt);
		/* ll = (ll > euck2) ? ll : euck2; */
		if (ll > 0.0)
 		  power = pow(ll, -euck1/2.0);
		else
		  power = 1.0;
			/* Inverse square falloff */
 		bright *= power;
	    }

	    ln = DOT(*L,N);
	    if(ln <= 0)			/* Ignore lights shining from behind */
		continue;
	    if (ll > 0.0)
	      ls = sqrt(ll);
	    else
	      ls = 1.0;

	    if (ls == 0.0) ls = 1.0;

	    s = ma->mat.kd * bright * (ln/ls);
					/* cosine law: L.N diffuse shading */

	    CCSADD(l->color, *thiscolor, s, Ci);

	    if(ma->mat.ks > 0) {	/* Specular term */
		Point3 H;		/* H: halfway between L and I */
		float num;

		H.x = L->x/ls + I.x, H.y = L->y/ls + I.y, H.z = L->z/ls + I.z;
		num = DOT(H,H);
		if (num <= 0.0)
		  num = 1.0;
		else
		  num = sqrt(num);

		s = DOT(H,N) / num;
					/* cos angle(H,N) */

			/* Note we need s>=0 for specular term to make sense.
			 * This should be true since we checked
			 * that both L.N and I.N are positive above.
			 */

					/* shininess = specular exponent */
		if (s > 0.0)
		  s = ma->mat.ks * bright * pow(s, ma->mat.shininess);
		else
		  s = ma->mat.ks * bright;

		CCSADD(l->color, ma->mat.specular, s, Ci);
	    }
	}
	/* insert fog code */
 	if (fog)	{
	    float k1, k2;
	    float d = HPt3Distance(&V, EYE);
	    ColorA surfcolor, fogcolor;
	    d = d - euck2;		/* fog-free sphere of radius euck2 */
	    if (d < 0) d = 0;
	    k1 = exp( -fog * d);
	    k2 = 1.0 - k1;
	    CSCALE(k1, Ci, surfcolor); 
	    CSCALE(k2, _mgc->background, fogcolor); 
	    CCADD(surfcolor, fogcolor, Ci);
	}
	     
	if(Ci.r < 0) Ci.r = 0; else if(Ci.r > 1) Ci.r = 1;
	if(Ci.g < 0) Ci.g = 0; else if(Ci.g > 1) Ci.g = 1;
	if(Ci.b < 0) Ci.b = 0; else if(Ci.b > 1) Ci.b = 1;
	*(Color *)cs = Ci;
	cs->a = c->a;
    }
    return 0;
}
