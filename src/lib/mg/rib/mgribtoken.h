/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

/* minnegraphics renderman token interface */
void mrti_init();
void mrti( int a1, ... );
void mrti_reset();

extern unsigned char *tokenbuffer;
extern unsigned char *ptr;

#define STRINGBASE	50     /* strings start at 50 */

enum tokentype {
    mr_NULL=0,

    /* RI Requests */
    mr_attributebegin,
    mr_attributeend,
    mr_transformbegin,
    mr_transformend,
    mr_identity,
    mr_concattransform, /* Transform T */
    mr_surface,
    mr_shadinginterpolation,
    mr_color,	    /* float *ptr (mr_array w/size=3) */
    mr_opacity,	    /* float alpha (will become an array) */
    mr_polygon,	    /* following arrays must be decalred explicitly */
    mr_patchmesh,   /* Ditto */
    mr_format,	    /* follow with mr_intseq */
    mr_projection,
    mr_clipping,
    mr_worldbegin,
    mr_worldend,
    mr_display,
    mr_screenwindow,
    mr_lightsource,
    mr_sphere,
    mr_translate,
    mr_rotate,
    mr_cylinder,
    mr_nupatch,
    mr_shadingrate,
    mr_option,
    mr_illuminate,
    mr_framebegin,
    mr_frameend,

    /* Strings */
    mr_P=STRINGBASE,
    mr_N,
    mr_Cs,
    mr_Pw,
    mr_Os,
    mr_st,
    mr_plastic,
    mr_hplastic,
    mr_eplastic,
    mr_heplastic,
    mr_constant,
    mr_ambientlight,
    mr_lightcolor,
    mr_distantlight,
    mr_intensity,
    mr_file,
    mr_rgb,
    mr_rgba,
    mr_Ka,
    mr_Kd,
    mr_Ks,
    mr_specularcolor,
    mr_roughness,
    mr_fov,
    mr_perspective,
    mr_to,
    mr_framebuffer,

    /* SPECIAL */    
    mr_array,		/* (full array) size, f,f,.. */
    mr_buildarray,	/* size (subarys follow w/total subsizes==size) */
    mr_parray,  	/* subsize, *float, *float, .. */
    mr_subarray3,	/* ptr to aggregate of 3 floats (most common) */
    mr_section,		/* char *string */
    mr_comment,		/* char *string */
    mr_nl,		/* new line */
    mr_int,
    mr_intseq,		/* int numberOfInts, int, int, .. (NOT AN ARRAY!)*/
    mr_float,
    mr_string,
    mr_embed,
    mr_header		/* ## 'header' comments for RIB 1.0 structure */
};

struct _table {
    char *name;		 /* string representing token, or null */
    int  len;		 /* length of string */
    unsigned char reqn;  /* number used to define request/string to renderer */
    int  defined;	 /* flags weather request/string has been defined */
};

