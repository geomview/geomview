/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#include "mgP.h"
#include "mgribP.h"
#include "mgribshade.h"
#include "mgribtoken.h"

/*
 * Notes:	Tossed mgrib_material, just use mgrib_appearance
 *		since shaders depend on both appearance and material 
 *		settings. (4/1/93 TMM)
 */
void
mgrib_appearance( struct mgastk *astk, int ap_mask, int mat_mask)
{
    Appearance *ap = &(astk->ap);
    Material *mat = &astk->mat;

    mrti(mr_section, "Interpreting Material", mr_NULL);

    if (mat_mask & MTF_DIFFUSE)
        mrti(mr_color, mr_parray, 3, &mat->diffuse, mr_NULL);

    if ((ap_mask & APF_TRANSP || mat_mask & MTF_ALPHA) && 
	 ap->valid & APF_TRANSP && ap->flag & APF_TRANSP) {
        /* presumably, we want this here as well as per vertex opacity
         * specification
	 * 
	 * Actually, if the material alpha override bit is set, we
	 * don't output the per vertex opacity information, which
	 * would take precedence. 4/16/93 TMM
         */
        float alpha = astk->mat.diffuse.a;
	mrti(mr_opacity, mr_array, 3, alpha, alpha, alpha, mr_NULL);
    }

    if (     (ap_mask & (APF_SHADING|APF_TEXTURE))
	 ||  (mat_mask & (MTF_SPECULAR|MTF_SHININESS|MTF_Kd|MTF_Ka|MTF_Ks))
         ||  (    (ap->tex != NULL)
               && (ap->flag & APF_TEXTURE)
               && (ap->tex != astk->next->ap.tex) )
       ) {
#if 1 /* cH: I'm not so sure 'bout the following anymore */
	    /* NOTE: the factor "8.0" is to compensate for some of the
	     * usual rib shaders, i.e. BMRT, aqsis etc.. Don't know
	     * why they started this affair, but without the output
	     * from rendrib or aqsis is just _very_ different from
	     * that what is displayed on the screen -- cH.
	     *
	     */
        float roughness = (mat->shininess)? 8.0/mat->shininess : 8.0/1.0;
#else
        float roughness = (mat->shininess)? 1.0/mat->shininess : 1.0/1.0;
#endif
	enum tokentype shader = mr_plastic;

        if (ap->shading == APF_CONSTANT || ap->shading == APF_CSMOOTH) {
	    if (_mgribc->shader == MG_RIBSTDSHADE) {
		shader = mr_constant;
	    } else if ((ap->flag & APF_TEXTURE) && ap->tex != NULL) {
		switch (ap->tex->apply) {
		case TXF_MODULATE: shader = mr_GVmodulateconstant; break;
		case TXF_DECAL: shader = mr_GVdecalconstant; break;
		case TXF_BLEND: shader = mr_GVblendconstant; break;
		case TXF_REPLACE: shader = mr_GVreplaceconstant; break;
		}
	    }
	    mrti(mr_shadinginterpolation,
		 mr_string, ap->shading == APF_CONSTANT ? "constant" : "smooth",
		 mr_surface, shader, mr_NULL);
	} else {
	    /* determine shader */
	    if (_mgribc->shader == MG_RIBSTDSHADE) {
		if ((ap->flag & APF_TEXTURE) && ap->tex != NULL) {
		    shader = mr_paintedplastic;
		} else {
		    shader = mr_plastic;
		}
	    } else {
	        if (_mgc->space & TM_HYPERBOLIC) {
		    shader = mr_hplastic;
		} else if ((ap->flag & APF_TEXTURE) && ap->tex != NULL) {
		    switch (ap->tex->apply) {
		    case TXF_MODULATE: shader = mr_GVmodulateplastic; break;
		    case TXF_DECAL: shader = mr_GVdecalplastic; break;
		    case TXF_BLEND: shader = mr_GVblendplastic; break;
		    case TXF_REPLACE: shader = mr_GVreplaceplastic; break;
		    }
		} else {
		    shader = mr_plastic;
		}
	    }

	    /* define surface */
	    if (!IS_SMOOTH(ap->shading)) {
		mrti(mr_shadinginterpolation, mr_constant,
		     mr_surface, shader, mr_Ka, mr_float, mat->ka,
		     mr_Kd, mr_float, mat->kd, mr_Ks, mr_float, mat->ks,
		     mr_specularcolor, mr_parray, 3, &(mat->specular),
		     mr_roughness, mr_float, roughness, mr_NULL);
	    } else {
		mrti(mr_shadinginterpolation, mr_string, "smooth",
		     mr_surface, shader, mr_Ka, mr_float, mat->ka,
		     mr_Kd, mr_float, mat->kd, mr_Ks, mr_float, mat->ks,
		     mr_specularcolor, mr_parray, 3, &(mat->specular),
		     mr_roughness, mr_float, roughness, mr_NULL);
	    }
	}

	/* define texture if appropriate */
	if ((ap->flag & APF_TEXTURE) &&
	    ap->tex != NULL && ap->tex->image != NULL) {
	    char txtxname[PATH_MAX];
	    char tifftxname[PATH_MAX];
	    char filter[PATH_MAX];
	    int i;
	    unsigned chmask = 0;
	    
	    if (_mgribc->shader == MG_RIBSTDSHADE) {
		static bool was_here = false;
		
		if (!was_here) {
		    OOGLWarn("textures with apply != modulate will not work "
			     "when using the standard shaders.\n");
		    was_here = true;
		}
	    }

	    for (i = 0; i < _mgribc->n_tximg; i++) {
		if (_mgribc->tx[i]->image == ap->tex->image &&
		    (_mgribc->tx[i]->flags & (TXF_SCLAMP|TXF_TCLAMP))
		    ==
		    (ap->tex->flags & (TXF_SCLAMP|TXF_TCLAMP))) {
		    break;
		}
	    }

	    mgrib_mktexname(txtxname, i, _mgribc->tmppath, "tiff.tx");

	    if (i == _mgribc->n_tximg) {
		if (_mgribc->n_tximg % 10 == 0) {
		    _mgribc->tx = OOGLRenewNE(Texture *,
					      _mgribc->tx,
					      _mgribc->n_tximg + 10,
					      "New RIB texture images");
		}
		_mgribc->tx[i] = ap->tex;
		_mgribc->n_tximg++;
		/* try to dump the image to disk */
		mgrib_mktexname(tifftxname, i, _mgribc->displaypath, "tiff");
#ifdef HAVE_PAMTOTIFF 
		chmask = (1 << ap->tex->image->channels) - 1;
		sprintf(filter, "pamtotiff -lzw -truecolor > %s 2> /dev/null",
			tifftxname);
#else
		chmask = ap->tex->image->channels > 2 ? 0x7 : 0x1;
		sprintf(filter, "pnmtotiff -lzw -truecolor > %s 2> /dev/null",
			tifftxname);
#endif
		if (!ImgWriteFilter(ap->tex->image, chmask, filter)) {
		    _mgribc->tx[i] = NULL;
		    --_mgribc->n_tximg;
		}
		mgrib_mktexname(tifftxname, i, NULL, "tiff");
		mrti_makecurrent(&_mgribc->txbuf);
		mrti(mr_maketexture, mr_string, tifftxname, mr_string, txtxname,
		     mr_string,
		     ap->tex->flags & TXF_SCLAMP ? "clamp" : "periodic",
		     mr_string,
		     ap->tex->flags & TXF_TCLAMP ? "clamp" : "periodic",
		     mr_string, "gaussian", mr_float, 2.0, mr_float, 2.0,
		     mr_NULL);
		mrti_makecurrent(&_mgribc->worldbuf);
	    }
	    if (i < _mgribc->n_tximg) {
		mrti(mr_texturename, mr_string, txtxname, mr_NULL);
	    }
	    if (ap->tex->apply == TXF_BLEND) {
		mrti(mr_string, "bgcolor",
		     mr_parray, 3, &(ap->tex->background), mr_NULL);
	    }
	    if (ap->tex->apply != TXF_DECAL) {
		mrti(mr_string, "At",
		     mr_float, (ap->flag & APF_TRANSP) ? 1.0 : 0.0, mr_NULL);
	    }
	}
    }
}

void
mgrib_lighting(struct mgastk *astk, int mask)
{
  LmLighting *lm = &astk->lighting;

  if (lm->valid & mask & LMF_AMBIENT) {
      mrti(mr_section, "Interpreting Lights", mr_NULL);
      /* output the ambient light */
      mrti(mr_comment, "Ambient Light", mr_NULL);
      mrti(mr_lightsource, mr_ambientlight, mr_int, 0,
	   mr_lightcolor, mr_parray, 3, &(lm->ambient), mr_NULL);
    }    
  /* We must be in global coordinate space to place lights correctly. */
  mrti(mr_transformbegin, mr_identity, mr_NULL);
  mgrib_lights( lm, astk );
  mrti(mr_transformend, mr_NULL);
}

/* In GL, we want unique light numbers. In RenderMan we want to re-use
light numbers, so we always start with light "1".  In GL, we define
then bind lights. Defining is expensive and we avoid it as much as
possible. In RenderMan, it's a one-step process. Also, once a light is
defined it cannot be deleted. Thus we might need to explicitly turn
off lights if we are replacing more lights with less lights. So we
keep track of how many lights we've used so far, and turn off the
extras when we need to.
1/13/92 TMM */

void mgrib_lights( LmLighting *lm, struct mgastk *astk ) 
{ 
  int i, lightsused; 
  LtLight *light, **lp;
  static int prevused = 0;

  lightsused = 0;
  LM_FOR_ALL_LIGHTS(lm, i,lp) {
    light = *lp;
    ++lightsused;

    if (light->Private == 0) {
      /* this is a new light */
      light->Private = lightsused;
      light->changed = 1;
    }
    if (light->changed) {
      if (light->position.w == 0.0) {
	/* directional light */
	/* We used to output the lights as "to" instead of as "from".
	   This is like negating them, which is why we then did
	   explicitly negate them to get the right result!
	   So, leave the lights alone now. Do NOT negate them.
	   It's NOT about handedness of coordinate systems.
         */
	mrti(mr_comment, "Directional Light",
	     mr_lightsource, mr_distantlight, mr_int, light->Private,
	     mr_intensity, mr_float, light->intensity,
	     mr_lightcolor, mr_parray, 3, &(light->color),
	     mr_string, "from", mr_parray, 3, &(light->globalposition),
	     mr_string, "to", mr_array, 3, 0.,0.,0., /* aim at origin */
	     mr_NULL);
      } else {
	/* point light */
	mrti(mr_lightsource, mr_string, "pointlight", mr_int, light->Private,
	     mr_intensity, mr_float, light->intensity,
	     mr_lightcolor, mr_parray, 3, &(light->color),
	     mr_string, "from", mr_parray, 3, &(light->globalposition),
	     mr_NULL);
      }
      light->changed = 0;
    }
  }
  for (i = lightsused +1; i <= prevused; i++)
    mrti(mr_illuminate, mr_int, i, mr_int, 0, mr_NULL);
  if (prevused < lightsused) prevused = lightsused;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
