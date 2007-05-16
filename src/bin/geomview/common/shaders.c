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
#include "drawer.h"
#include "mgP.h"
#include "3d.h"

#define	DOT(a,b)  ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)

#define	CCMUL(A,B,C)  ((C).r=(A).r*(B).r, (C).g=(A).g*(B).g, (C).b=(A).b*(B).b)
#define	CCADD(A,B,C)  ((C).r=(A).r+(B).r, (C).g=(A).g+(B).g, (C).b=(A).b+(B).b)

#define CSCALE(s, A, B) ( (B).r = (A).r * s, (B).g = (A).g * s, (B).b = (A).b * s, (B).a = (A).b * s)


#define	CCSADD(A,B,s,C)	\
	 ((C).r += (s)*(A).r*(B).r, \
	  (C).g += (s)*(A).g*(B).g, \
	  (C).b += (s)*(A).b*(B).b)

	/* Eye position as an HPoint3 */
#define	EYE	((HPoint3 *) (_mgc->C2W[3]))



	/* Shading parameters, left here for easy dbx viewing. */
static float fog = 0.0;			/* fog control */
static float hypk1 = .5;		/* controls exponential falloff */
static float hypk2 = .0;		/* controls exponential falloff */
static float sphk1 = .5;		/* controls exponential falloff */
static float sphk2 = 0.0;		/* safety zone around light */

/*
 * Note: code for the Euclidean shader now lives in src/lib/mg/common/mgshade.c.
 * This file contains only the non-Euclidean ones.
 */

#define HPt3R31GramSchmidt	HPt3HypGramSchmidt
#define HPt3R31Distance		HPt3HypDistance

#define HPt3R40GramSchmidt	HPt3SphGramSchmidt
#define HPt3R40Distance		HPt3SphDistance

/*
 * A routine to determine the bisector of two tangent vectors 
 */
void HPt3R31Bisector(HPoint3 *Lt, HPoint3 *I, HPoint3 *H)
{
    HPoint3 tmp1, tmp2;
    float a,b;

    HPt3Sub(I, Lt, &tmp1);
    a = HPt3R31Dot(I, &tmp1); 
    b = HPt3R31Dot(Lt, &tmp1); 
    if ( b == 0) b = 1;
    HPt3Scale(-(a/b), Lt, &tmp2);
    HPt3Add(I, &tmp2, H); 
    if (H->w < 0)   HPt3Scale(-1, H, H);
    return;
}

void HPt3R40Bisector(HPoint3 *Lt, HPoint3 *I, HPoint3 *H)
{
    HPoint3 tmp1, tmp2;
    float a,b;

    HPt3Sub(I, Lt, &tmp1);
    a = HPt3R40Dot(I, &tmp1); 
    b = HPt3R40Dot(Lt, &tmp1); 
    if ( b == 0) b = 1;
    HPt3Scale(-(a/b), Lt, &tmp2);
    HPt3Add(I, &tmp2, H); 
    return;
}

/*
 * Hyperbolic shader
 */

int hypshade(int nv, HPoint3 *v, Point3 *n, ColorA *c, ColorA *cs)
{
    struct mgxstk *mx = _mgc->xstk;
    struct mgastk *ma = _mgc->astk;
    struct LtLight *l;
    int i, lno;
    HPoint3 V, n4, N, I;
    float s, ni;
    Color Ci;
    static LtLight *cachedlights[AP_MAXLIGHTS];
    static int ncached = 0;
    static short cached_light_seq = -1;

    hypk1 = ma->lighting.attenmult;
    hypk2 = ma->lighting.attenmult2;
    fog = ma->lighting.attenconst>1 ? ma->lighting.attenconst-1 : 0;

    Ci.r = Ci.g = Ci.b = 0;
    CCSADD(ma->mat.ambient, ma->lighting.ambient, ma->mat.ka, Ci);

    /* for now, move lights to be inside hyperbolic space */
    
    if(ma->light_seq != cached_light_seq) {
	LtLight **lp;
	float d;
	HPoint3 *L;

	for(i = 0; i < ncached; i++)
	    LtDelete(cachedlights[i]);
	ncached = 0;
	LM_FOR_ALL_LIGHTS(&ma->lighting, i,lp) {
	    if(ncached >= AP_MAXLIGHTS)
		break;

	    cachedlights[ncached++] = l = LtCopy(*lp, NULL);

	    /* use euclidean distance */

	    L = &l->globalposition;
	    d = HPt3R30Dot(L, L);
	    if (d >= 1) 	{ 	/* move light to sphere of radius .8 */
		d = .8/sqrt(d);	
		L->x *= d;
		L->y *= d;
		L->z *= d;
		L->w = 1.0;
	    }
	}
	cached_light_seq = ma->light_seq;
    }

    for(i = 0; i < nv; i++, v++, n++, c++, cs++) {

					/* Transform point by T */
	HPt3Transform(mx->T, v, &V);	/* V = v in world coords */
	if (HPt3R31Dot(&V, &V) >= 0)    /* point at infinity, or beyond */
		continue;		/* outside hyperbolic space */
	HPt3R31Normalize(&V);		/* lie on surface < , > = -1 */

	HPt3Sub( (HPoint3 *) EYE, &V, &I );	/* I = EYE - V (point-to-eye vector) */
	HPt3R31GramSchmidt(&V, &I);
	HPt3R31Normalize(&I);

        n4.x = n->x; n4.y = n->y; n4.z = n->z; n4.w = 1.0;
    	HPt3R31GramSchmidt( v, &n4);
    	HPt3R31Normalize(&n4);

	HPt3Transform(mx->T, &n4, &N);/* transform by inverse adjoint */

	/* tangent space at V has a Euclidean (positive definite) structure */
	ni = HPt3R31Dot(&N,&I);
	if (ni < 0)  
	    {		
	    float s = -1.0;
	    HPt3Scale(s, &N, &N);
	    }
		

	for(lno = ncached; --lno >= 0; ) {
	    HPoint3 *L;
	    HPoint3 Lt;
	    float bright, ll, ln, light_intensity, d;

	    l = cachedlights[lno];
	    L = &l->globalposition;
					/* Diffuse term */
	    /* first compute the color of arriving light */
	    bright = l->intensity;
	    ll = HPt3R31Dot(L,L);
	    if (ll >= 0)		/* ignore lights outside of hspace */
		continue;
	    ln = HPt3R31Dot(L,&N);
	    if(ln <= 0)			/* Ignore lights shining from behind */
		continue;
	    /* compute the distance for computation of falloff of light */
	    d = HPt3R31Distance(L, &V);
	    /* the following models the exponential falloff of intensity */
	    light_intensity = exp(-hypk1*d);
	
	    /* now compute the cosine for diffuse shading */
	    Lt = l->globalposition;	/* we'll change the values here */
	    HPt3Sub(&Lt, &V, &Lt);	/* make it a difference vector */
	    HPt3R31GramSchmidt(&V, &Lt);
	    HPt3R31Normalize(&Lt);
	    d = HPt3R31Dot(&Lt, &N);	/* cos of angle between L and N */

	    s = ma->mat.kd * bright * d * light_intensity;
					
	    CCSADD(l->color, *c, s, Ci);

	    if(ma->mat.ks > 0) {	/* Specular term */
		HPoint3 H;		/* H: halfway between L and I */

	 	HPt3R31Bisector(&Lt, &I, &H);
		HPt3R31Normalize(&H);

		s = HPt3R31Dot(&H, &N); /* cos of angle between H and N */
		if (s < 0) continue;

		s = ma->mat.ks * bright * pow(s, ma->mat.shininess);

		CCSADD(l->color, ma->mat.specular, s, Ci);
	    }
	} 
	/* insert fog code */
 	if (fog)	{
	    float k1, k2;
	    float d = HPt3HypDistance(&V, EYE);
	    ColorA surfcolor, fogcolor;
	    d = d - hypk2;		/* fog-free sphere of radius euck2 */
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

int sphshade(int nv, HPoint3 *v, Point3 *n, ColorA *c, ColorA *cs)
{
    struct mgxstk *mx = _mgc->xstk;
    struct mgastk *ma = _mgc->astk;
    struct LtLight *l;
    int i, lno;
    HPoint3 V, n4, N, I;
    float s, ni;
    Color Ci;
    static LtLight *cachedlights[AP_MAXLIGHTS];
    static int ncached = 0;
    static short cached_light_seq = -1;

    sphk1 = ma->lighting.attenmult;
    sphk2 = ma->lighting.attenmult2;
    fog = ma->lighting.attenconst>1 ? ma->lighting.attenconst-1 : 0;

    /* "ambient" color */
    
    Ci.r = Ci.g = Ci.b = 0;
    CCSADD(ma->mat.ambient, ma->lighting.ambient, ma->mat.ka, Ci);

    /* for now, move lights to be inside spherical space */
    if(ma->light_seq != cached_light_seq) {
	float d;
	HPoint3 *L;
	LtLight **lp;

	for(i = 0; i < ncached; i++)
	    LtDelete(cachedlights[i]);
	ncached = 0;
	LM_FOR_ALL_LIGHTS(&ma->lighting, i,lp) {
	  if(ncached >= AP_MAXLIGHTS)
	    break;

	  cachedlights[ncached++] = l = LtCopy(*lp, NULL);

	  /* use euclidean distance */

	  L = &l->globalposition;
	  /* use euclidean distance */
	  d = HPt3R40Dot(L, L);
	  if (d >= 1.0) { 	/* move light to be in S3 */
	      d = 1.0/sqrt(d);	
	      L->x *= d;
	      L->y *= d;
	      L->z *= d;
	      L->w *= d;
	  }
	}
	cached_light_seq = ma->light_seq;
    }

    for(i = 0; i < nv; i++, v++, n++, c++, cs++) {

					/* Transform point by T */
	HPt3Transform(mx->T, v, &V);	/* V = v in world coords */
	HPt3R40Normalize(&V);		/* lie on surface < , > = -1 */
					/* I = EYE - V (point-to-eye vector) */
	HPt3Sub(  (HPoint3 *) EYE, &V, &I );	
	HPt3R40GramSchmidt(&V, &I);
	HPt3R40Normalize(&I);

        n4.x = n->x; n4.y = n->y; n4.z = n->z; n4.w = 1.0;
    	HPt3R40GramSchmidt( v, &n4);
    	HPt3R40Normalize(&n4);

	HPt3Transform(mx->T, &n4, &N);/* transform by inverse adjoint */

	/* tangent space at V has a Euclidean (positive definite) structure */
	ni = HPt3R40Dot(&N,&I);
	if (ni < 0)  
	    {		
	    float s = -1.0;
	    HPt3Scale(s, &N, &N);
	    }
		
	for(lno = ncached; --lno >= 0; ) {
	    HPoint3 *L;
	    HPoint3 Lt;
	    float bright, ln, d, radius, light_intensity;

	    l = cachedlights[lno];
	    L = &l->globalposition;
					/* Diffuse term */
	    /* first compute the color of arriving light */
	    bright = l->intensity;
	    /*ll = HPt3R40Dot(L,L);*/	/* this should be 1.0 by above */
	    ln = HPt3R40Dot(L,&N);
	    if(ln <= 0)			/* Ignore lights shining from behind */
		continue;
	    /* compute the distance for computation of falloff of light */
	    d = HPt3R40Distance(L, &V);
	    /* the amount of light from the source which arrives at
	    the surface is inversely proportional to the area 
	    of the sphere centered at the light which contains 
	    the surface point. The radius of this sphere is the sin
	    of the distance between L and V */
	    radius = sin((double) d);
	    if (radius > sphk2)
		{
		float kk = (sphk2/radius);	/* 0 <= sphk2 <= 1.0 */
	        light_intensity = pow(kk, -sphk1);
		}
	    else light_intensity = 1.0;
	    /* we model the atmosphere as a murky fluid that has exponential
	    dissipation; sphk1 controls exponential drop off; value of 0
	    avoids any drop off */
	    d = HPt3R40Distance((HPoint3 *) EYE, &V);
	    light_intensity *= exp(-sphk1 * d);
	
	    /* now compute the cosine for diffuse shading */
	    Lt = l->globalposition;	/* we'll change the values here */
	    HPt3Sub(&Lt, &V, &Lt);	/* make it a difference vector */
	    HPt3R40GramSchmidt(&V, &Lt);
	    HPt3R40Normalize(&Lt);
	    d = HPt3R40Dot(&Lt, &N);	/* cos of angle between L and N */

	    s = ma->mat.kd * bright * d * light_intensity;
					
	    CCSADD(l->color, *c, s, Ci);

	    if(ma->mat.ks > 0) {	/* Specular term */
		HPoint3 H;		/* H: halfway between L and I */

	 	HPt3R40Bisector(&Lt, &I, &H);
		HPt3R40Normalize(&H);

		s = HPt3R40Dot(&H, &N); /* cos of angle between H and N */
		if (s < 0) continue;

		s = ma->mat.ks * bright * pow(s, ma->mat.shininess);

		CCSADD(l->color, ma->mat.specular, s, Ci);
	    }
	}
	/* insert fog code */
 	if (fog)	{
	    float k1, k2;
	    float d = HPt3SphDistance(&V, EYE);
	    ColorA surfcolor, fogcolor;

	    d = d - sphk2;		/* fog-free sphere of radius euck2 */
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


mgshadefunc
softshader(int camid)
{
    switch(spaceof(camid)) {
    case TM_HYPERBOLIC: return hypshade;
    case TM_EUCLIDEAN: return mg_eucshade;
    case TM_SPHERICAL: return sphshade;
    default: return mg_eucshade;
    }
}
