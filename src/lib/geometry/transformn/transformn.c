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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips, Olaf Holt */

#include "hpointn.h"	/* Defines HPointN and TransformN */
#include "transformn.h"
#include <ooglutil.h>
#include <transform3.h>
#include <math.h>

	/* Construct a transform.  NULL a => identity */
TransformN *
TmNCreate( int idim, int odim, HPtNCoord *a )
{
	TransformN *T = OOGLNewE(TransformN, "new TransformN");
	RefInit((Ref *)T, TMNMAGIC);
	if( idim <= 0 ) idim = 1;
	if( odim <= 0 ) odim = 1;
	T->idim = idim;	 T->odim = odim;
	T->a = OOGLNewNE(HPtNCoord, idim*odim, "new TransformN data");
	if( a == NULL )
		memset(T->a, 0, idim*odim*sizeof(HPtNCoord));
	else
		memcpy(T->a, a, idim*odim*sizeof(HPtNCoord));
	return(T);
}

	/* Destroy */
void 
TmNDelete( TransformN *T )
{
	if(T && RefDecr((Ref*)T) == 0) {
		if(T->a) OOGLFree(T->a);
		OOGLFree(T);
	}
}

	/* Get and set space */
int 
TmNSpace( const TransformN *T );

TransformN *
TmNSetSpace( TransformN *T, int space );

	/* Invert */
TransformN * 
TmNInvert( const TransformN *T, TransformN *Tinv )
{
	register int i, j, k;
	HPtNCoord x;
	HPtNCoord f;
	short dim = T->idim; 
	TransformN *t = TmNCreate(dim,dim,T->a);

	if(T->odim != dim) {
		OOGLError(1,"Matrix for inversion is not square");
		return(0);
	}
	if(Tinv == NULL)
		Tinv = TmNCreate(dim,dim,NULL);
	else if (Tinv->idim != dim || Tinv->odim != dim) {
		Tinv->a = OOGLRenewNE(HPtNCoord,Tinv->a,dim*dim, "renew TransformN");
		Tinv-> idim = dim; Tinv->odim = dim;
	}

	TmNIdentity(Tinv);

	/* Components of unrolled inner loops: */
#define	SUB(v,k)  v[j*dim+k] -= f*v[i*dim+k]
#define	SWAP(v,k) x = v[i*dim+k], v[i*dim+k] = v[largest*dim+k], v[largest*dim+k] = x


	for (i = 0; i < dim; i++) {
		int largest = i;
		HPtNCoord largesq = t->a[i*dim+i]*t->a[i*dim+i];
		for (j = i+1; j < dim; j++)
			if ((x = t->a[j*dim+i]*t->a[j*dim+i]) > largesq)
				largest = j,  largesq = x;

		/* swap t->a[i][] with t->a[largest][] */
		for(k = 0; k < dim; k++) {
		    SWAP(t->a,k);
		    SWAP(Tinv->a,k);
		}

		for (j = i+1; j < dim; j++) {
			f = t->a[j*dim+i] / t->a[i*dim+i];
			/* subtract f*t->a[i][] from t->a[j][] */
			for(k = 0; k < dim; k++) {
			    SUB(t->a,k);
			    SUB(Tinv->a,k);
			}
		}
	}
	for (i = 0; i < dim; i++) {
		f = t->a[i*dim+i];
		for (k = 0; k < dim; k++) {
			t->a[i*dim+k] /= f;
			Tinv->a[i*dim+k] /= f;
		}
	}
	for (i = dim-1; i >= 0; i--)
		for (j = i-1; j >= 0; j--) {
			f = t->a[j*dim+i];
			for(k = 0; k < dim; k++) {
			    SUB(t->a,k);
			    SUB(Tinv->a,k);
			}
		}
#undef SUB
#undef SWAP
				
	return Tinv;
}

	/* Transpose */
TransformN *
TmNTranspose( const TransformN *from, TransformN *to )
{
	register int i,j;
	short idim = from->idim, odim = from->odim;
	HPtNCoord t;

	if( from != to) {
		if(to == NULL)
			to = TmNCreate(odim,idim,NULL);
		else if (to->idim != odim || to->odim != idim) {
			to->a = OOGLRenewNE(HPtNCoord,to->a,idim*odim, "renew TransformN");
			to-> idim = odim; to->odim = idim;
		}
		for( i=0; i<idim; i++)
			for( j=0; j<odim; j++)
				to->a[j*idim+i] = from->a[i*odim+j];
	} else {

#define SWITCH(a,b)		 t = a; a = b; b = t;						

		if (idim == odim) {
			for( i=0; i<idim; i++)
				for( j= 0; j<i; j++) {
					SWITCH(to->a[i*odim+j], to->a[j*odim+i]);
				}
		} else {
			int remainder, dividend;
			to->idim = odim; to->odim = idim;
			for( i=0; i<idim; i++)
				for( j=0; j<odim; j++) {
					remainder = (i*idim +j)%odim;
					dividend = (i*idim + j -remainder)/odim;
					SWITCH(to->a[i*odim+j],to->a[dividend*odim+remainder]);
				}
		}
#undef SWITCH
	}

	return(to);
}

	/* Multiply transforms */
TransformN *
TmNConcat( const TransformN *A, const TransformN *B, TransformN *result )
{
	register int i,j,k;
	short dim1 = A->idim, dim2 = A->odim, dim3 = B->odim;

#define MAKEPRODUCT(T,A,B)											\
	T->idim = dim1; T->odim = dim3;									\
	for( i=0; i<dim1; i++)											\
		for( j=0; j<dim3; j++) {									\
			T->a[i*dim3+j] = 0;										\
			for( k=0; k<dim2; k++ )									\
				T->a[i*dim3+j] += A->a[i*dim2+k] * B->a[k*dim3+j];	\
		}

	if(B->idim != dim2) {
	  if ( B->idim > dim2) {
		TransformN *Atmp = TmNCreate(dim1,dim2,NULL);
		dim2 = B->idim;
		TmNPadZero((TransformN *)A,dim1,dim2,Atmp);
		if( B == result ) {
			TransformN *T = TmNCreate( dim1, dim3, NULL);

			MAKEPRODUCT(T,Atmp,B);
			TmNCopy(T,result);
		} else {
			if(result == NULL)
				result = TmNCreate(dim1,dim3,NULL);
			else if (result->idim != dim1 || result->odim != dim3) {
				result->a = OOGLRenewNE(HPtNCoord,result->a,dim1*dim3, "renew TransformN");
				result-> idim = dim1; result->odim = dim3;
			}
			MAKEPRODUCT(result,Atmp,B);
		}
	  } else {  /* B->idim < dim2 */
		TransformN *Btmp = TmNCreate(dim2,dim3,NULL);
		TmNPadZero((TransformN *)B,dim2,dim3,Btmp);
		if( A == result ) {
			TransformN *T = TmNCreate( dim1, dim3, NULL);

			MAKEPRODUCT(T,A,Btmp);
			TmNCopy(T,result);
		} else {
			if(result == NULL)
				result = TmNCreate(dim1,dim3,NULL);
			else if (result->idim != dim1 || result->odim != dim3) {
				result->a = OOGLRenewNE(HPtNCoord,result->a,dim1*dim3, "renew TransformN");
				result-> idim = dim1; result->odim = dim3;
			}
			MAKEPRODUCT(result,A,Btmp);
		}
	  }
	} else {

		if( A == result || B == result ) {
			TransformN *T = TmNCreate( dim1, dim3, NULL);

			MAKEPRODUCT(T,A,B);
			TmNCopy(T,result);
		} else {
			if(result == NULL)
				result = TmNCreate(dim1,dim3,NULL);
			else if (result->idim != dim1 || result->odim != dim3) {
				result->a = OOGLRenewNE(HPtNCoord,result->a,dim1*dim3, "renew TransformN");
				result-> idim = dim1; result->odim = dim3;
			}
			MAKEPRODUCT(result,A,B);
		}
	}

#undef MAKEPRODUCT

	return(result);
}

	/* Copy */
TransformN *
TmNCopy( const TransformN *Tsrc, TransformN *Tdst )
{
	if(Tdst == NULL) {
		Tdst = TmNCreate(Tsrc->idim, Tsrc->odim, Tsrc->a);
	} else {
		if(Tdst->idim != Tsrc->idim || Tdst->odim != Tsrc->odim) {
			Tdst->a = OOGLRenewNE(HPtNCoord,Tdst->a,Tsrc->idim*Tsrc->odim, "renew TransformN");
			Tdst->idim = Tsrc->idim; Tdst->odim = Tsrc->odim;
		}
		memcpy(Tdst->a, Tsrc->a, Tsrc->idim*Tsrc->odim*sizeof(HPtNCoord));
	}
	return(Tdst);
}

	/* Set to identity */
TransformN * 
TmNIdentity( TransformN *T )
{


	if(T == NULL) {
		T = TmNCreate(1,1,NULL);
		T->a[0] = 1;
	} else {
  		int i;
	  short idim = T->idim, odim = T->odim;
		memset(T->a, 0, idim*odim*sizeof(HPtNCoord));
		if (idim == odim || idim < odim) {
			for(i = 0; i < idim; i++)
				T->a[i*odim+i] = 1;
		} else {  /*  idim > odim */
			for(i = 0; i < odim; i++)
				T->a[i*odim+i] = 1;
		}
	}
	return(T);
}


	/* Euclidean translations */
TransformN * 
TmNTranslateOrigin( TransformN *T, const HPointN *p )
{
/* old version, assumes no "w" coordinate at end of p  */
	short dim = p->dim;
	HPointN *pt = HPtNCreate( 1+dim, NULL );
	int i;

	for ( i=0; i< dim; i++)
		pt->v[i] = p->v[i];
	pt->v[dim] = 1;

	TmNTranslate(T,pt);
	return(T);
}

TransformN * 
TmNTranslate( TransformN *T, const HPointN *pt )
{
	register int i;
	HPointN *newpt;
	short dim = pt->dim;

	newpt = HPtNCreate(dim,pt->v);
	HPtNDehomogenize(newpt,newpt);

  if ( T == NULL ) {
	T = TmNCreate(dim,dim,NULL);
	TmNIdentity(T);
	for( i=0; i<dim - 1; i++)
		T->a[(dim-1)*dim+i] = newpt->v[i];
  } else {
	if (T->odim == dim) {
		TmNIdentity(T);
		for( i=0; i<dim - 1; i++)
			T->a[(dim-1)*dim+i] = newpt->v[i];
	} else if (T->odim > dim) {
		short idim = T->idim, odim = T->odim;
		TmNIdentity(T);
		for( i=0; i<dim - 1; i++)
			T->a[(idim-1)*odim+i] = newpt->v[i];
	} else if (T->odim < dim) {
		short idim = T->idim, odim = dim;
		TmNPad(T,idim,odim,T);
		for( i=0; i<dim - 1; i++)
			T->a[(idim-1)*odim+i] = newpt->v[i];
	}
  }

  HPtNDelete(newpt);
  return(T);
}

	/* Pad the transform, with ones down the main diagonal, moving the
		last row down as if it is a translation */
TransformN *
TmNPad(TransformN *Tin, short idim, short odim, TransformN *Tout)
{
	if(Tin == NULL) {
		Tout = TmNCreate(idim,odim,NULL);
		TmNIdentity(Tout);
	} else if ( odim <= 0 || idim <= 0 ) {
		;  /* do nothing */
	} else {

	short oldidim = Tin->idim, oldodim = Tin->odim;
	int i;

	/* first pad the transform with no consideration for the last row */
	TmNPadSimple(Tin, idim, odim, Tout);
	/* then move the translation row to the right place, if we're upping
		the dimension */

  if( oldidim < idim && oldodim < odim) {
	for ( i =0; i < oldodim-1; i++ ) {
		Tout->a[(idim-1)*odim + i] = Tout->a[(oldidim-1)*odim + i];
		Tout->a[(oldidim-1)*odim + i] = 0;
	}
  }

	}
	return(Tout);
}

	/* Pad the transform, with ones down the main diagonal, not assuming
		a homogeneous coordinate that needs special attention */
TransformN *
TmNPadSimple(TransformN *Tin, short idim, short odim, TransformN *Tout)
{
	if(Tin == NULL) {
		Tout = TmNCreate(idim,odim,NULL);
		TmNIdentity(Tout);
	} else if ( odim <= 0 || idim <= 0 ) {
		;  /* do nothing */
	} else {


	short oldidim = Tin->idim, oldodim = Tin->odim;
	int i,j;
	
	if(Tin != Tout) {
		if (Tout == NULL) {
			Tout = TmNCreate(idim,odim,NULL);
		} else if(Tout->idim != idim || Tout->odim != odim) {
			Tout->a = OOGLRenewNE(HPtNCoord,Tout->a,idim*odim, "renew TransformN");
			Tout->idim = idim; Tout->odim = odim;
		}

	  if( oldidim < idim && oldodim < odim) {
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<oldodim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
			for( j=oldodim; j<odim; j++)
				if(i==j)
					Tout->a[i*odim+j] = 1;
				else
					Tout->a[i*odim+j] = 0;
		}
		for ( i=oldidim; i<idim; i++) {
			for( j=0; j<odim; j++)
				if(i==j)
					Tout->a[i*odim+j] = 1;
				else
					Tout->a[i*odim+j] = 0;
		}
	  } else if ( oldidim < idim && oldodim >= odim) {	
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<odim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
		for ( i=oldidim; i<idim; i++) {
			for( j=0; j<odim; j++)
				if(i==j)
					Tout->a[i*odim+j] = 1;
				else
					Tout->a[i*odim+j] = 0;
		}
	  } else if ( oldidim >= idim && oldodim < odim) {	
		for ( i=0; i<idim; i++) {
			for( j=0; j<oldodim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
			for( j=oldodim; j<odim; j++)
				if(i==j)
					Tout->a[i*odim+j] = 1;
				else
					Tout->a[i*odim+j] = 0;
		}
	  } else if ( oldidim >= idim && oldodim >= odim) {	
		for ( i=0; i<idim; i++)
			for( j=0; j<odim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	  }
	} else {
		TransformN *Tnew = TmNCreate(idim,odim,NULL);
	  if( oldidim < idim && oldodim < odim) {
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<oldodim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
			for( j=oldodim; j<odim; j++)
				if(i==j)
					Tnew->a[i*odim+j] = 1;
		}
		for ( i=oldidim; i<idim; i++) {
			for( j=0; j<odim; j++)
				if(i==j)
					Tnew->a[i*odim+j] = 1;
		}
	  } else if ( oldidim < idim && oldodim >= odim) {	
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<odim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
		for ( i=oldidim; i<idim; i++) {
			for( j=0; j<odim; j++)
				if(i==j)
					Tnew->a[i*odim+j] = 1;
		}
	  } else if ( oldidim >= idim && oldodim < odim) {	
		for ( i=0; i<idim; i++) {
			for( j=0; j<oldodim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
			for( j=oldodim; j<odim; j++)
				if(i==j)
					Tnew->a[i*odim+j] = 1;
		}
	  } else if ( oldidim >= idim && oldodim >= odim) {	
		for ( i=0; i<idim; i++)
			for( j=0; j<odim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	  }
		TmNCopy(Tnew,Tout);
	}
  }
	return(Tout);
}
	
	/* Pad with zeroes */
TransformN *
TmNPadZero(TransformN *Tin, short idim, short odim, TransformN *Tout)
{
	if(Tin == NULL) {
		Tout = TmNCreate(idim,odim,NULL);
		TmNIdentity(Tout);
	} else if ( odim <= 0 || idim <= 0 ) {
		;  /* do nothing */
	} else {


	short oldidim = Tin->idim, oldodim = Tin->odim;
	int i,j;
	
	if(Tin != Tout) {
		if (Tout == NULL) {
			Tout = TmNCreate(idim,odim,NULL);
		} else if(Tout->idim != idim || Tout->odim != odim) {
			Tout->a = OOGLRenewNE(HPtNCoord,Tout->a,idim*odim, "renew TransformN");
			Tout->idim = idim; Tout->odim = odim;
		}

	  memset(Tout->a, 0, idim*odim*sizeof(HPtNCoord) );

	  if( oldidim < idim && oldodim < odim) {
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<oldodim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim < idim && oldodim >= odim) {	
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<odim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim >= idim && oldodim < odim) {	
		for ( i=0; i<idim; i++) {
			for( j=0; j<oldodim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim >= idim && oldodim >= odim) {	
		for ( i=0; i<idim; i++)
			for( j=0; j<odim; j++) 
				Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	  }
	} else {
		TransformN *Tnew = TmNCreate(idim,odim,NULL);
	  if( oldidim < idim && oldodim < odim) {
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<oldodim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim < idim && oldodim >= odim) {	
		for ( i=0; i<oldidim; i++) {
			for( j=0; j<odim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim >= idim && oldodim < odim) {	
		for ( i=0; i<idim; i++) {
			for( j=0; j<oldodim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
		}
	  } else if ( oldidim >= idim && oldodim >= odim) {	
		for ( i=0; i<idim; i++)
			for( j=0; j<odim; j++) 
				Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	  }
		TmNCopy(Tnew,Tout);
	}
  }
	return(Tout);
}
	

	/* Translations by the space of 'pt' */
TransformN * 
TmNSpaceTranslate( TransformN *T, HPointN *pt ) 
{
	/* this has not yet been implemented
	 *  for now, just call the euclidean one */
	return(TmNTranslate( T, pt ));
}

TransformN * 
TmNSpaceTranslateOrigin( TransformN *T, HPointN *pt )
{
	/* this has not yet been implemented
	 *  for now, just call the euclidean one */
	return(TmNTranslate( T, pt ));
}

	/* Scale by the components of 'amount' */
TransformN *
TmNScale( TransformN *T, const HPointN *amount )
{
	int i;
	short dim = amount->dim;

	if(T == NULL) {
		T = TmNCreate(dim,dim,NULL);
		TmNIdentity(T);
		for( i=0; i<dim-1; i++)
			T->a[i*dim+i] = amount->v[i];
	} else if( dim != T->idim || dim != T->odim ) {
		short idim = T->idim, odim = T->odim;
		switch( (dim > idim) + 2*(dim>odim) + 4*(idim>odim) ) {
			case 0:  /* odim > idim > dim */
				idim = odim;
				break;
			case 1:  /* odim > dim > idim */
				idim = odim;
				break;
			case 2:  /* impossible */
				OOGLError(1,"incorrect switch");
				break;
			case 3:  /* dim > odim > idim */
				odim = dim; idim = dim;
				break;
			case 4:  /* idim > odim > dim */
				odim = idim;
				break;
			case 5:  /* impossible */
				OOGLError(1,"incorrect switch");
				break;
			case 6:  /* idim > dim > odim */
				odim = idim;
				break;
			case 7:  /* dim > idim > odim */
				odim = dim; idim = dim;
				break;
		}
		TmNPadZero(T,idim,odim,T);
		TmNIdentity(T);
		for( i=0; i<dim-1; i++)
			T->a[i*odim+i] = amount->v[i];
	} else {
		TmNIdentity(T);
		for( i=0; i<dim-1; i++)
			T->a[i*dim+i] = amount->v[i];
	}

	return(T);
}

	/* Construct a geodesic rotation taking vector 'from' to 'toward' */
TransformN *
TmNRotate( TransformN *T, const HPointN *from, const HPointN *toward )
{
	register int i,j,k;
	short dim = from->dim;
	register HPtNCoord len, proj, cosA, sinA;
	HPtNCoord *planebasis1, *planebasis2; /* not homogeneous coords!*/
	HPtNCoord planecoord1, planecoord2;

	planebasis1 = OOGLNewNE(HPtNCoord, 2*(dim-1), "TmNRotate data");
	planebasis2 = planebasis1 + dim-1;

#define NORMALIZE(v,vout)								\
	len = 0;											\
	for( k=0; k<dim-1; k++)								\
		len += v[k] * v[k];								\
	if ( len == 0 ) {									\
		len = 1;										\
	} else												\
		len = sqrt(len);								\
	for( k=0; k<dim-1; k++)								\
		vout[k] = v[k] / len;

#define DOTPRODUCT(v1,v2,result)						\
	result = 0;											\
	for( i = 0; i<dim -1; i++)                          \
		result += v1[i] * v2[i];
		

  if(dim != toward->dim) {
	if(toward->dim < dim) {
		HPointN *towardtmp = HPtNCreate(dim,NULL);
		HPtNPad((HPointN *)toward,dim,towardtmp);
		if (T == NULL)
			T = TmNCreate(dim,dim,NULL);
		else if (T->idim != dim || T->odim != dim) {
			T->a = OOGLRenewNE(HPtNCoord,T->a,dim*dim, "renew TransformN");
			T->idim = dim;  T->odim = dim;
		}
		memset(T->a, 0, dim*dim*sizeof(HPtNCoord));
		T->a[dim*dim-1] = 1;
		NORMALIZE(from->v,planebasis1);
		DOTPRODUCT(towardtmp->v,planebasis1,proj);
		for( i=0; i<dim-1; i++)
			planebasis2[i] = towardtmp->v[i] - proj * planebasis1[i];
		NORMALIZE(planebasis2, planebasis2);
		DOTPRODUCT(towardtmp->v,towardtmp->v,len);
		if( (len = sqrt(len)) == 0 ) {
			OOGLError(1,"zero vector unexpected");
			return(NULL);
		}
		cosA = proj / len;
		sinA = sqrt(1-cosA*cosA);
		for( i=0; i<dim-1; i++) {  /* each basis vector */
			planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
			planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
			for( j=0; j<dim-1; j++)
				if ( i == j ) T->a[j*dim+i] = 1 + 
						planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
				else T->a[j*dim+i] = planecoord1*planebasis1[j] + 
						planecoord2*planebasis2[j];
		}
	} else {
		HPointN *fromtmp = HPtNCreate(dim,NULL);
		dim = toward->dim;
		HPtNPad((HPointN *)from,dim,fromtmp);
		if (T == NULL)
			T = TmNCreate(dim,dim,NULL);
		else if (T->idim != dim || T->odim != dim) {
			T->a = OOGLRenewNE(HPtNCoord,T->a,dim*dim, "renew TransformN");
			T->idim = dim;  T->odim = dim;
		}
		memset(T->a, 0, dim*dim*sizeof(HPtNCoord));
		T->a[dim*dim-1] = 1;
		NORMALIZE(fromtmp->v,planebasis1);
		DOTPRODUCT(toward->v,planebasis1,proj);
		for( i=0; i<dim-1; i++)
			planebasis2[i] = toward->v[i] - proj * planebasis1[i];
		NORMALIZE(planebasis2, planebasis2);
		DOTPRODUCT(toward->v,toward->v,len);
		if( (len = sqrt(len)) == 0 ) {
			OOGLError(1,"zero vector unexpected");
			return(NULL);
		}
		cosA = proj / len;
		sinA = sqrt(1-cosA*cosA);
		for( i=0; i<dim-1; i++) {  /* each basis vector */
			planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
			planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
			for( j=0; j<dim-1; j++)
				if ( i == j ) T->a[j*dim+i] = 1 + 
						planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
				else T->a[j*dim+i] = planecoord1*planebasis1[j] + 
						planecoord2*planebasis2[j];
		}
	}
  } else {
	if (T == NULL)
		T = TmNCreate(dim,dim,NULL);
	else if (T->idim != dim || T->odim != dim) {
		T->a = OOGLRenewNE(HPtNCoord,T->a,dim*dim, "renew TransformN");
		T->idim = dim;  T->odim = dim;
	}
	memset(T->a, 0, dim*dim*sizeof(HPtNCoord));
	T->a[dim*dim-1] = 1;

	/* form orthonormal basis for plane */

	NORMALIZE(from->v,planebasis1);
	DOTPRODUCT(toward->v,planebasis1,proj);
	for( i=0; i<dim-1; i++)
		planebasis2[i] = toward->v[i] - proj * planebasis1[i];
	NORMALIZE(planebasis2, planebasis2);
	

	/* now that we have the basis vectors for the plane,
		calculate the rotation matrix */

	DOTPRODUCT(toward->v,toward->v,len);
	if( (len = sqrt(len)) == 0 ) {
		OOGLError(1,"zero vector unexpected");
		return(NULL);
	}
	cosA = proj / len;
	sinA = sqrt(1-cosA*cosA);
	
	for( i=0; i<dim-1; i++) {  /* each basis vector */
		planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
		planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
		for( j=0; j<dim-1; j++)
			if ( i == j ) T->a[j*dim+i] = 1 + 
					planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
			else T->a[j*dim+i] = planecoord1*planebasis1[j] + 
					planecoord2*planebasis2[j];
	}
  }
#undef DOTPRODUCT
#undef NORMALIZE

	OOGLFree(planebasis1);
	return(T);
}


	/* special operation for computing n-d counterpart for movement
		in a subspace */
/* permute tells which dimensions are projected */
/* delta is the usual 4x4 matrix used in Geomview */
TransformN *
TmNApplyDN( TransformN *mat, int *perm, Transform3 delta)
{
	register HPtNCoord sum;
	short n = mat->idim, d = 4;  /* this is the dimension of the delta matrix */
	short nprime = mat->odim;
	HPtNCoord *tmp;
	int i,j,k;
	int permute[4];

	if( mat->odim != n) {
		if(nprime > n) {
			n = nprime;
			TmNPad(mat,n,n,mat);    /* Note: the input matrix is changed here */
		} else {
			TmNPad(mat,n,n,mat);
		}
	}

	/* Map "-1" in perm[] array to dimension N-1 (homogeneous divisor) */
	for(i = 0; i < 4; i++)
	   permute[i] = (perm[i] >= 0 && perm[i] < n) ? perm[i] : n-1;

	tmp = OOGLNewNE(HPtNCoord, n*d, "TmNApplyDN data");

	for(i = 0; i<n; i++) {
		for(j= 0; j<d; j++) {
			tmp[i*d+j] = mat->a[i*n+permute[j] ];
		}
	}

	for(i = 0; i<n; i++) {
		for (j= 0; j<d; j++) {
			sum = 0;
			for(k = 0; k<d; k++) {
				sum += tmp[i*d+k] * delta[k][j];
			}
			mat->a[i*n + permute[j] ] = sum;
		}
	}

	OOGLFree(tmp);
	return(mat);
}

    /* Return dimensions of a TransformN.  Value is first dimension. */
    /* idim and/or odim may be NULL, in which case they're not returned */
int 
TmNGetSize(const TransformN *T, int *idim, int *odim)
{
	if ( T == NULL)
		return(0);

	if(idim) *idim = T->idim;
	if(odim) *odim = T->odim;
	return(T->idim);
}

void
TmNPrint(FILE *f, const TransformN *T) 
{
    int i, j;
    short idim = T->idim, odim = T->odim;

	if( f == NULL)
		return;
    fprintf(f, "ntransform { %d %d\n", idim, odim);
    for(i = 0; i < idim; i++) {
        for(j = 0; j < odim; j++)
            fprintf(f, "%10.7f ", T->a[i*odim+j]);
        fprintf(f, "\n");
    }
    fprintf(f, "}\n");
}

TransformN *
TmNRead(IOBFILE *f)
{
    HPtNCoord *a;
    int got, n, brack = 0;
    int idim, odim;

    iobfexpecttoken(f, "ntransform");

    if(iobfnextc(f,0) == '{')
	brack = iobfgetc(f);

    if(iobfgetni(f,1,&idim, 0) <= 0 || iobfgetni(f, 1, &odim, 0) <= 0 || idim <= 0 || odim <= 0) {
	OOGLSyntax(f, "Expected dimensions of N-D transform");
	return NULL;
    }
    n = idim*odim;
    a = OOGLNewNE(HPtNCoord, n, "new TransformN data");

    got = iobfgetnf(f, n, a, 0);
    if(got < n) {
	OOGLSyntax(f, "N-D transform: error reading %d'th of %d values.", got, n);
	return NULL;
    }

    if(brack)
	iobfexpecttoken(f, "}");

    return TmNCreate(idim,odim,a);
}

TransformN *
CtmNScale( HPtNCoord s, TransformN *in, TransformN *out)
{
	short idim = in->idim;
	short odim = in->odim;
	int i,j;

	if(out == in) {
		for( i=0; i<idim-1; i++)
			for( j = 0; j<odim-1; j++)
				out->a[i*odim+j] *= s;
	} else {
		out = TmNCopy(in, out);
		for( i=0; i<idim-1; i++)
			for( j = 0; j<odim-1; j++)
				out->a[i*odim+j] = s*in->a[i*odim+j];
	}
	return(out);
}
