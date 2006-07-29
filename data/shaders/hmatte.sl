surface
hmatte(
	float Ka	= 0.05,
		Kd	= 1;
	)
{
	float	Lhw,Nhw;	/* w-cord of hyperbolic normal vector to P */
	point	Nh, Lh;	/* first 3 cords of hyperbolic normal vector to P */
	point   Px;
	float	b,d,nn;

	Nh = faceforward(normalize(N),I);
	Px = transform("shader",P);

	/* compute plane equation */
    Ci = 0;
    illuminance(Px)
	{
	Nhw = Px.Nh;
	b = -(L.Px - 0)/(Px.Px - 1);
	Lh = L + b*Px;		/* hyperbolic light vector is lin comb of L,P */
	Lhw = 0 + b;
	d = (Lh.Nh - Lhw*Nhw)/sqrt(abs((Lh.Lh - Lhw*Lhw)*(Nh.Nh - Nhw*Nhw)));

	if (d < 0.0)	d = -d;
	if (d > 1.0)	{
/*
	    printf("Light:%f %f %f %f\n",xcomp(Lh),ycomp(Lh),zcomp(Lh),Lhw);
	    printf("Normal:%f %f %f %f\n",xcomp(Nh),ycomp(Nh),zcomp(Nh),Nhw);
	    printf("Posi:%f %f %f 1.0\n",xcomp(Px),ycomp(Px),zcomp(Px));
*/
	    d = 1.0; 
	    }

	Ci += Os * Cs * (Ka*ambient() + Cl*Kd*d );
	}
    Oi = Os;
}		
	
