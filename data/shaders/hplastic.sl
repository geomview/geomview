/*
 * Copyright (c) 1990, Geometry Supercomputer Project
 *                     University of Minnesota
 *                     1200 Washington Ave. S
 *                     Minneapolis, MN  55415
 *
 * email address: software@geomview.org
 *
 * This software is copyrighted as noted above.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the authors, who may or may not act on them as they desire.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 */

/* 
 *  hplastic.sl:	a plastic shader for hyperbolic space
 *  Author:		Charlie Gunn
 *  Description:
 *	The parameters to this shader are the same as the parameters to the regular plastic
 *	shader, but this shader computes angles and distances using the hyperbolic metric.
 *   	This metric is defined on homogeneous coordinates (x,y,z,w) and is induced by the 
 *	quadratic form x*x + y*y +z*z - w*w.
 *	This metric is valid on the interior of the unit ball; the isometries of this metric
 *	are projective transformations that preserve the unit sphere and map the interior to 
 *	itself.  These are represented by 4x4 matrices hence can be implemented using the 
 *	regular geometry/viewing pipeline provided by Renderman (and other rendering systems).
 *  Features:
 *	It would be much easier to implement if there were more general datatypes in the 
 *	shading language.  I mean if I could use 4-tuples as points instead of just 3-tuples
 *	the code would be much more compact.  As it is, I have to drag around a "w" coordinate
 *	for each point through all the computations. Ugly.
 */

surface
hplastic(
	float Ka	= 1,
		Kd	= 1,
		Ks	= 1,
		roughness = .1;
	color	specularcolor = color (1,1,1);
	)
{
	point	Nh, Ih, M,Lh;
	float	Ihw,Nhw, Mw, Lhw;
	float	a,b,d,ss, nn, mag2,PdotP;
	point	sP;
	color	total;
	uniform float spec;

	sP = P;
	PdotP = sP.sP;	
	/* turn it off if outside unit ball */
	if (PdotP >= 1.0)  {
		Ci = 0;
		Oi = 1.0;
		}

	else{
	spec = 1.0/roughness;
	Nh = faceforward(normalize(N),I);
	/* make N be "tangent" to P i.e. <P, Nh> = 0*/
	Nhw = sP.Nh;
	nn = sqrt((Nh.Nh) - (Nhw*Nhw));

	/* as a difference vector, I has w cord = 0 */
	/* also, we want the L which points at the eye, not the surface */
	Ih = -I;
	b = -((Ih.sP) - 0)/((PdotP) - 1);
	Ih = Ih + b*sP;		/* hyperbolic eye vector is lin comb of I,P */
	Ihw = 0 + b;
	/* normalize this light vector */
	mag2 = sqrt(abs((Ih.Ih) - (Ihw*Ihw)));
	if (mag2 != 0.0) mag2 = 1.0/mag2;
	else mag2 = 1.0;
	Ih = mag2*Ih;
	Ihw = mag2*Ihw;

    total = 0;
    illuminance(sP)
    {
	/* compute specular */
	/* first adjust light vector to be tangent at P */
	/* L is also a difference vector, hence its w-cord = 0 */
	b = -((L.sP) - 0)/((PdotP) - 1);
	Lh = L + b*sP;		
	Lhw = 0 + b;
	/* normalize Lh */
	mag2 = sqrt(abs((Lh.Lh) - (Lhw*Lhw)));
	if  (mag2 != 0.0) 	mag2 = 1.0/mag2;
	else mag2 = 1.0;
	Lh = mag2*Lh;
	Lhw = mag2*Lhw;
	d = ((Lh.Nh) - (Lhw*Nhw))/nn;
	if (d > 1.0)	/* printf("big d: %f\n",d); */
		d = 1.0;

	/* now compute bisector of angle between L and I */
	/* important for Lh, Ih to be unit length */
	M = Ih - Lh;
	Mw = Ihw - Lhw;
	a = -((Ih.M) - (Ihw*Mw))/((Lh.M) - (Lhw*Mw));
	M = Ih + a*Lh;
	Mw = Ihw + a*Lhw;	
	/* detect very small vectors, reject them */
	if (abs(M.M - Mw*Mw) < .0001)	{
	    M = Lh;
	    Mw = Lhw;
	    }
	if (Mw < 0.0)	{
	    M = -M;
	    Mw = -Mw;
	    }

	/* compute cos(angle between normal and mid-vector H) */
	ss = ((M.Nh) - (Mw*Nhw))/(sqrt(abs((M.M - Mw*Mw))) * nn);

	if (ss < 0.0)	ss = -ss;
	if (ss > 1.0)	ss = 1.0;

	total = total + Os * Cs * Cl * (Ka*ambient() + Kd*d);
	total = total + Os * specularcolor * Cl * Ks*pow(ss,spec);
    }
	Ci = total;
    }
	Oi = Os; 
}		
	
