/* hlight.sl:	shader for point lights in hyperbolic space
 * Author:	Charlie Gunn
 * Description: 
 *	See hplastic.sl also. 
 *	This simulates a point light in hyperbolic space.  The brightness of the light
 *	decays exponentially since the surface area of the sphere of radius r in hyperbolic
 *	space is exp(kr) for some constant k.    Distance is given by Arccosh({p1,p1}),
 *	where {p1,p0} is the Minkowski inner product on projective space.
 */


light
hlight(
	float k1 = 0.5,
	intensity = 1;
	color lightcolor = color (1, 1, .1);
        point from = point "camera" (0,0,0);	/* light position */
	)
{
	float	inpro, hdis, d;		/* hyperbolic distance (sort of) */
	float  PP, QQ, PQ, tmp, PdotP;
	point sP;

	Cl = 0;
	illuminate(from)	{
		sP = from + L;		/* need to reconstruct the position of light */
		PdotP = sP.sP;
		if (PdotP >= 1.0)	{
		    d = 1.0/sqrt(PdotP);
		    setxcomp(sP, d * xcomp(sP));
		    setycomp(sP, d * ycomp(sP));
		    setzcomp(sP, d * zcomp(sP));
		    PdotP = sP.sP;	
		    }
		PP =  1.0 - PdotP;
		if (PP < 0 ) PP = 0;
		QQ =  1.0 - from.from;
		/* if (QQ < 0 ) QQ = 0; if the light's bad, we're doomed */
		PQ =  1.0 - sP.from;
		/* if (PQ < 0 ) PQ = 0; ditto */
		
		d = PP * QQ;
		if (d > 0) {
		    inpro = PQ / sqrt (d);
		    hdis = log(inpro + sqrt((abs(inpro*inpro - 1))));
		    Cl += intensity * lightcolor * (exp(-k1*hdis));
		    }	
		else Cl = 0;
		}
}


