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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef APPEARANCEDEF
#define APPEARANCEDEF

/*
  #define AP_IDEBUG before #including this file to compile for
  debugging
*/

#include <stdarg.h>

#include "ooglutil.h"
#include "transobj.h"
#include "3d.h"
#include "color.h"
#include "streampool.h"
#include "dbllist.h"

#define DONE 1
#define ANOTHER 2

typedef struct LtLight {
    REFERENCEFIELDS;
    Color ambient;
    Color color;
    Point position;             /* application-specified light position */
    Point globalposition;       /* real light position; set & used only by mg */
    float intensity;

    int Private;                /* private part; used only by mg */

    short location;             /* How to interpret the "position": */
#define LTF_GLOBAL      0x0             /* global coordinate system */
#define LTF_CAMERA      0x1             /* camera coordinates */
#define LTF_LOCAL       0x2             /* local coordinates
                                         * where appearance is attached. */

    short changed;
} LtLight;


#define AP_MAXLIGHTS    8

typedef struct LmLighting {
    REFERENCEFIELDS;
    int valid, override;         /* LMF_* inheritance bits */
    Color ambient;
    int localviewer;
    float attenconst, attenmult, attenmult2;
    LtLight *lights[AP_MAXLIGHTS];

    int Private;
    int changed;
} LmLighting;

#define LM_ANY_LIGHTS(lm)  ((lm)->lights[0] != NULL)
#define LM_FOR_ALL_LIGHTS(lm, i,lp)  for((i)=0, (lp) = &(lm)->lights[0]; \
					 (i)<AP_MAXLIGHTS && *(lp) != NULL; (i)++, (lp)++)

typedef struct Material {
    REFERENCEFIELDS;
    int valid, override;        /* MTF_* inheritance bits */
    Color emission;
    Color ambient;
    ColorA diffuse;             /* diffuse.a === material alpha */
    Color specular;
    float shininess;
    float ka, kd, ks;
    Color edgecolor;            /* Color for polygon edges and other vectors */
    Color normalcolor;          /* Color for surface-normal vectors */

    int Private;
    int changed;
} Material;

/* The pixmap image underlying a texture. We define a separate
 * Lisp-object such that sharing of image data is possible (and can be
 * implemented more cleanly than before).
 *
 * The image format does not interprete the image data, it just
 * records the number of channels. However: the corresponding I/O
 * routines always read data in RGB(A) format, or LUMINANCE(-ALPHA)
 * format.
 *
 * The pixmap is in Cartesian coordinates, i.e. row 0/col 0
 * corresponds to the LOWER-LEFT corner.
 */
typedef struct Image {
    REFERENCEFIELDS;
    int width;      /* width in pixels */
    int height;     /* height in pixels */
    int channels;   /* number of channels */
    int maxval;     /* maximum intensity value of a channel */
    char *data;     /* image data, row by row, if depth > 8 then the
                     * most significant byte comes first (BIG-ENDIAN).
                     */
} Image;

typedef struct TxUser TxUser;

/* Textures are bulky and need active management! */
struct TxUser {
    struct TxUser *next;        /* Next user in tx->users list */
    struct Texture *tx;         /* Which texture is ours? */
    int id;                     /* Slot for e.g. graphics-system texture id */
    void *ctx;                  /* Slot for e.g. mg context */
    void *data;                 /* Any other data the user needs */
    int flags;                  /* flags for user to determine usage */
    int (*needed)(TxUser *);    /* user answers "d'you still need this?" */
    void (*purge)(TxUser *);    /* request to user to purge this texture */
};

typedef struct Texture {
    REFERENCEFIELDS;
    Image *image;         /* underlying pixmap */
    Handle *imghandle;
    Transform tfm;        /* texture-coord transformation */
    Handle *tfmhandle;
    unsigned int flags;   /* clamp, etc. */
    enum apply_enum {
        tx_modulate,
        tx_decal,
        tx_blend,
        tx_replace
    } apply;              /* Application style
                           * (TXF_DECAL, TXF_MODULATE, TXF_BLEND, TXF_REPLACE)
                           */
    Color background;     /* background color for TXF_BLEND */
    TxUser *users;        /* Users of this texture register here */
#if 0
    int coords;           /* Texture-coord auto generation (not implemented) */
#endif
    /*****************************************/
    /* the old and deprecated way to do stuff */
    char *filename;       /* ppm or pgm (.Z) file */
    char *alphafilename;  /* If present, this is a .pgm (.Z) file */
    /*****************************************/
    DblListNode loadnode;
} Texture;

extern DblListNode AllLoadedTextures;   /* List of em */

typedef struct Appearance {
    REFERENCEFIELDS;
    Material    *mat, *backmat;     /* material properties */
    LmLighting  *lighting;  /* attached lighting */
    Texture     *tex;       /* Texture-map object */
    unsigned int flag;      /* APF_* shading,faces/edges,normals,transparency */
    unsigned int valid;     /* APF_* bits control inheritance of these fields */
    unsigned int override;  /* same APF_* flags: parent overrides child */
    float nscale;           /* scale factor for drawing normals */
    int linewidth;          /* how wide do lines appear (pixels) ? */
    enum {
        apf_constant = 0,
	apf_flat,
	apf_smooth,
	apf_csmooth,
	apf_vcflat,
	apf_num_shadings
    } shading;
    enum {
	apf_alpha_blending = 0, /* translucency using alpha-blending. */
	apf_screen_door,        /* translucency by stipple patterns. */
	apf_naive_blending,
	apf_num_translucencies
    } translucency;
    int dice[2];          /* u, v patch dicing parameters */
}  Appearance;

extern HandleOps AppearanceOps;
extern HandleOps ImageOps;
extern HandleOps TextureOps;

/* the following tokens are used in LtSet and LtGet */
#define LT_END          700
#define LT_AMBIENT      701     /* Color ambient */
#define LT_COLOR        702     /* Color color */
#define LT_POSITION     703     /* Point position */
#define LT_INTENSITY    704     /* double intensity */
#define LT_LOCATION     705     /* int location: LTF_{GLOBAL,CAMERA,LOCAL} */

#define LT_ABLOCK       706     /* void **ablock ??? cH: unimplemented ??? */


/* the following tokens are used in LmSet and LmGet */
#define LM_END            600   
#define LM_AMBIENT        601   /* Color ambient */
#define LM_LOCALVIEWER    602   /* int localviewer */
#define LM_ATTENC         603   /* double attenconst */
#define LM_ATTENM         604   /* double attenmult */
#define LM_ATTEN2         612   /* double attenmult2 */
#define LM_LtSet          605   /* ... */
#define LM_LIGHT          606   /* LtLight *light */
#define LM_VALID          607   /* int mask (Get only) */
#define LM_INVALID        608   /* int mask (unsets valid bits) */
#define LM_OVERRIDE       609   /* int mask (sets override bits) */
#define LM_NOOVERRIDE     610   /* int mask (unsets override bits) */
#define LM_REPLACELIGHTS  611   /* int replace */

#define LM_ABLOCK         613   /* void **ablock ??? cH: unimplemented ??? */


/* the following tokens are used in MtSet and MtGet */
#define MT_END          500     
#define MT_EMISSION     501     /* Color *emission */
#define MT_AMBIENT      502     /* Color *ambient */
#define MT_DIFFUSE      503     /* Color *diffuse */
#define MT_SPECULAR     504     /* Color *specular */
#define MT_Ka           505     /* double ka */
#define MT_Kd           506     /* double kd */
#define MT_Ks           507     /* double ks */
#define MT_ALPHA        508     /* double alpha */
#define MT_SHININESS    509     /* double shininess */
#define MT_EDGECOLOR    510     /* Color *edgecolor (for edges & vectors) */
#define MT_NORMALCOLOR  511     /* Color *normalcolor (for surface normals) */
#define MT_VALID        512     /* int mask (Get only) */
#define MT_INVALID      513     /* int mask (unsets valid bits) */
#define MT_OVERRIDE     514     /* int mask (sets override bits) */
#define MT_NOOVERRIDE   515     /* int mask (unsets override bits) */

#define MT_ABLOCK       516     /* void **ablock ??? cH: unimplemented ??? */


/* the following tokens are used in ApSet and ApGet */
#define AP_END          400     
#define AP_DO           401 /* int mask (set flag bits) */
#define AP_DONT         402 /* int mask (unset flag bits) */
#define AP_MAT          403 /* Material *mat */
#define AP_MtSet        404 /* ... */
#define AP_LGT          405 /* LmLighting *lgt */
#define AP_LmSet        406 /* ... */
#define AP_NORMSCALE    407 /* float nscale */
#define AP_LINEWIDTH    408 /* int linewidth */
#define AP_VALID        409 /* int mask (Get only) */
#define AP_INVALID      410 /* int mask (unsets valid bits) */
#define AP_OVERRIDE     411 /* int mask (sets override bits) */
#define AP_NOOVERRIDE   412 /* int mask (unsets override bits) */
#define AP_SHADING      413 /* int shading (set to APF_{CONSTANT,FLAT,SMOOTH} */
#define AP_BACKMAT      414 /* Material *backmaterial */

#define AP_ABLOCK       415 /* void **ablock ??? cH: unimplemented ??? */
#define AP_DICE         416 /* int udice, vdice (set); int dice[2] (get) */
#define AP_TEXTURE      417 /* Texture *tx */
#define AP_TxSet        418 /* ... */

/* int translucency (set to APF_{ALPHA_BLENDING,SCREEN_DOOR,NAIVE_BLENDING} */
#define AP_TRANSLUCENCY 419

/* Flags to ApMerge, etc. */
#define APF_INPLACE      0x1  /* Merge in place */
#define APF_OVEROVERRIDE 0x2  /* src replaces dst even without src override */



/* This model of texture-mapping shamelessly stolen
 * from GL/Open-GL
 */

#define TXMAGIC         OOGLMagic('t', 1)

#define TX_DOCLAMP  450
#define TXF_SCLAMP  0x1  /* Clamp if s outside 0..1 (else wrap) */
#define TXF_TCLAMP  0x2  /* Clamp if t outside 0..1 (else wrap) */

#define TXF_LOADED  0x4  /* Has this texture been loaded?
                          * (tried to read those files yet?)
                          */
#define TXF_RGBA    0x8  /* In loaded data, is R first byte? (else ABGR) */
#define TXF_USED    0x10 /* "Recently rendered a geom containing this texture"*/

#define TX_APPLY        451     /* Interpret texture values to... */
#define   TXF_MODULATE tx_modulate
#define   TXF_DECAL    tx_decal
#define   TXF_BLEND    tx_blend
#define   TXF_REPLACE  tx_replace

#define TX_HANDLE_IMAGE     452
#define TX_HANDLE_TRANSFORM 453
#define TX_BACKGROUND       454

#define TX_FILE             455 /* deprecated (really?) */
#define TX_ALPHAFILE        456 /* deprecated (really?) */
#define TX_CHANNELS         457 /* deprecated (really?) */
#define TX_XSIZE            458 /* deprecated (really?) */
#define TX_YSIZE            459 /* deprecated (really?) */

#if 0 /* not implemented */
#define TX_COORDS         458 /* Texture coordinates come from... */
#define TXF_COORD_GIVEN     0 /* given as part of object (default) In
                               * fact, only TXF_COORD_GIVEN works now.
                               */
# define TXF_COORD_LOCAL    1 /* In coord system of texture map */
# define TXF_COORD_CAMERA   2 /* In camera coords */
# define TXF_COORD_NORMAL   3 /* Taken from surface-normal, for env map */
#endif

#define TX_ABLOCK 464 /* ??? cH: unimplemented ??? */
#define TX_END    465

#define IMGMAGIC OOGLMagic('i', 1)

/* ImgSet() commands */
#define IMG_WIDTH          1000
#define IMG_HEIGHT         1001
#define IMG_CHANNELS       1002
#define IMG_MAXVAL         1003
#define IMG_DATA           1004
#define IMG_DATA_CHAN_FILE 1005 /* CHMASK, FILTER/NULL, FILENAME */
#define IMG_DATA_CHAN_DATA 1006 /* CHMASK, FILTER/NULL, DATA, DATASZ */
#define   IMGF_LUMINANCE       0x1
#define   IMGF_LUMINANCE_ALPHA 0x3
#define   IMGF_RGB             0x7
#define   IMGF_RGBA            0xf
#define   IMGF_ALPHA           0x100
#define   IMGF_AUTO            0x200
#define IMG_END            1042

typedef struct TxST 
{
    float s;
    float t;
} TxST;

/* Transformation of (s,t)-texture co-ordinates. (s,t,r,q) with r = 0,
 * q = 1
 */
static inline void TxSTTransform(Transform T, TxST *in, TxST *out)
{
    float s, t, q;

    if (T == NULL || T == TM_IDENTITY) {
        *out = *in;
        return;
    }

    s = in->s;
    t = in->t;
    q = T[TMX][TMW]*s + T[TMY][TMW]*t + T[TMW][TMW];

    if(q != 1.0) {
        q = 1.0 / q;
        out->s = q * (s*T[TMX][TMX] + t*T[TMY][TMX] + T[TMW][TMX]);
        out->t = q * (s*T[TMX][TMY] + t*T[TMY][TMY] + T[TMW][TMY]);
    } else {
        out->s = s*T[TMX][TMX] + t*T[TMY][TMX] + T[TMW][TMX];
        out->t = s*T[TMX][TMY] + t*T[TMY][TMY] + T[TMW][TMY];
    }
}

Appearance *    ApCreate( int attr, ... );
Appearance *    ApSet( Appearance *ap, int attr, ... );
Appearance *    _ApSet(Appearance *ap, int attr1, va_list *alist);
int             ApGet( Appearance *ap, int attr, void *valuep );
void            ApDelete( Appearance *ap );
Appearance *    ApDefault( Appearance *ap );
Appearance *    ApCopy(const Appearance *from, Appearance *into );
Appearance *    ApMerge(const Appearance *src, Appearance *dst, int inplace );
Appearance *    ApFLoad(IOBFILE *inf, char *fname);
Appearance *    ApCopyShared(const Appearance *ap, Appearance *into );
Appearance *    ApFSave(Appearance *ap, FILE *f, char *fname);
int ApStreamIn(Pool *p, Handle **hp, Appearance **app);
int ApStreamOut(Pool *p, Handle *h, Appearance *ap);

/* Force 'override' bits on (for all valid fields)
 * or off, in an Appearance.
 */
void    ApUseOverrides( Appearance *ap, int use_overrides );

/* Erase fields in 'dst' corresponding to any in 'src';
 * allows 'src' settings to propagate through 'dst',
 * when 'dst' is its child.
 */
void    ApLetPropagate( Appearance *src, Appearance *dst );

int     TxStreamIn( Pool *, Handle **, Texture ** );
int     TxStreamOut( Pool *, Handle *, Texture * );
void    TxDelete( Texture * );

int ImgStreamIn(Pool *p, Handle **hp, Image **imgp);
int ImgStreamOut(Pool *p, Handle *h, Image *tx);
Image *ImgFSave(Image *img, FILE *outf, char *fname);
void ImgDelete(Image *img);
Image *ImgCreate(int a1, ...);
int ImgWritePGM(Image *img, int channel, bool compressed, char **buffer);
int ImgWritePNM(Image *img, unsigned chmask, bool compressed, char **buffer);
int ImgWritePAM(Image *img, unsigned chmask, bool compressed, char **buffer);
bool ImgWriteFilter(Image *img, unsigned chmask, const char *filter);

Texture * TxFLoad( IOBFILE *inf, char *fname );
Texture * TxFSave(Texture *tx, FILE *outf, char *fname);
Texture * TxSet( Texture *, int attr, ... );
Texture * TxCreate( int attr, ... );
Texture * TxCopy( Texture *src, Texture *dst );
Texture * TxMerge( Texture *src, Texture *dst, int mergeflags );

void    TxPurge( Texture * );
TxUser *TxAddUser(Texture *, int id,
                  int (*needed)(TxUser *), void (*purge)(TxUser *));
void    TxRemoveUser(TxUser *user);

#ifndef AP_IDEBUG
Material *      MtCreate( int attr, ... );
Material *      MtSet( Material *, int attr, ... );
#endif
int             MtGet( Material *, int attr, void *valuep );

void            MtDelete( Material * );
Material *      MtDefault( Material * );
Material *      MtLoad( Material *, char *filename );
Material *      MtCopy( Material *from, Material *into );
Material *      MtMerge( Material *src, Material *dst, int inplace );
Material *      MtFLoad( Material *into, IOBFILE *stream, char *filename );
int             MtSave( Material *, char *filename );
int             MtFSave( Material *mat, FILE *stream, Pool *p );
void            MtPrint( Material * );


#ifndef AP_IDEBUG
LtLight *       LtCreate( int attr, ... );
LtLight *       LtSet( LtLight *, int attr, ... );
#endif
int             LtGet( LtLight *, int attr, void *valuep );
void            LtDelete( LtLight * );
LtLight *       LtCopy( LtLight *, LtLight * );
LtLight *       LtMerge( LtLight *, LtLight * );
LtLight *       LtDefault( LtLight * );
void            LtProperties( LtLight *, float, Color *, Point * );
LtLight *       LtFLoad( LtLight *, IOBFILE *, char *filename);

#define LTMAGIC OOGLMagic('l', 2)

#ifndef AP_IDEBUG
LmLighting *    LmCreate( int attr, ... );
LmLighting *    LmSet( LmLighting *, int attr, ... );
#endif
int             LmGet( LmLighting *, int attr, void *valuep );
void            LmDelete( LmLighting * );
void            LmDelete( LmLighting * );
void            LmDefault( LmLighting * );
LmLighting *    LmLoad( LmLighting *, char *filename );
LmLighting *    LmFLoad( LmLighting *, IOBFILE *, char *filename );
void            LmSave( LmLighting *, char *filename );
void            LmFSave( LmLighting *, FILE *, char *filename, Pool *p );
LmLighting *    LmMerge( LmLighting *src, LmLighting *dst, int inplace );
LmLighting *    LmCopy( LmLighting *src, LmLighting *dst );
void            LmAddLight( LmLighting *dst, LtLight *light );
void            LmRemoveLight( LmLighting *dst, LtLight *light );
void            LmCopyLights( LmLighting *src, LmLighting *dst );
void            LmDeleteLights( LmLighting *lm );

#define LIGHTINGMAGIC   OOGLMagic('l', 1)

#define LMF_LOCALVIEWER 0x1   /* Local viewer (flag valid) */
#define LMF_AMBIENT     0x2   /* Ambient light color */
#define LMF_ATTENC      0x4   /* attenuation constant factor */
#define LMF_ATTENM      0x8   /* attenuation linear factor */
#define LMF_ATTEN2      0x20  /* 1/r^2 attenuation factor */
#define LMF_REPLACELIGHTS 0x10  /* When merging, use only new lights, not union */

#define MATMAGIC        OOGLMagic('m', 1)

#define MTF_EMISSION    0x1
#define MTF_AMBIENT     0x2
#define MTF_DIFFUSE     0x4
#define MTF_SPECULAR    0x8
#define MTF_Ka          0x10
#define MTF_Kd          0x20
#define MTF_Ks          0x40
#define MTF_ALPHA       0x80
#define MTF_SHININESS   0x100
#define MTF_EDGECOLOR   0x200
#define MTF_NORMALCOLOR 0x400



#define APMAGIC         OOGLMagic('a', 1)

/* The following bits are used in 'flag', 'valid', 'override' */
#define APF_FACEDRAW    0x2     /* Draw faces */
#define APF_EDGEDRAW    0x10    /* Draw edges */
#define APF_TRANSP      0x20    /* Enable transparency */
#define APF_TRANSLUCENT APF_TRANSP /* "transparent" is the wrong word */
#define APF_EVERT       0x40    /* Evert surface normals */
#define APF_NORMALDRAW  0x80    /* Draw surface normals */
#define APF_VECTDRAW    0x100   /* Draw vectors/points */
#define APF_KEEPCOLOR   0x200   /* Not susceptible to N-D coloring */
#define APF_TEXTURE     0x400   /* Enable texture mapping */
#define APF_BACKCULL    0x800   /* Enable back-face culling */
#define APF_SHADELINES  0x2000  /* Enable line (edge & vect) lighting&shading */
#define APF_CONCAVE     0x4000  /* Expect concave polygons */
#define APF_TXMIPMAP    0x8000  /* Mip-mapped textures */
#define APF_TXMIPINTERP 0x10000 /* Interpolate between mipmaps */
#define APF_TXLINEAR    0x20000 /* Interpolate between texture pixels */

/* The following bits are used in 'valid', 'override' */
#define APF_SHADING     0x1     /* Use 'shading' value */
#define APF_NORMSCALE   0x4     /* Use 'nscale' value to draw normals */
#define APF_LINEWIDTH   0x8     /* Use 'linewidth' value  */
#define APF_DICE        0x1000  /* Dicing (use udice, vdice fields) */

/* Possible values for ap->shading field; these MUST be consecutive !!
   (code outside the appearance library depends on this fact) */
#define APF_CONSTANT apf_constant /* constant-colored (unlighted) faces */
#define APF_FLAT     apf_flat     /* Flat-shaded, lighted faces */
#define APF_SMOOTH   apf_smooth   /* Gouraud-shaded faces, with lighting */
#define APF_CSMOOTH  apf_csmooth  /* Gouraud-shaded faces, without lighting */
#define APF_VCFLAT   apf_vcflat   /* Gouraud-coloured, but flat faces */
#define APF_NUM_SHADINGS apf_num_shadings

#define IS_SMOOTH(shading)  ((shading) >= APF_SMOOTH)
#define IS_SHADED(shading)						\
    ((1<<(shading)) & ((1<<APF_FLAT)|(1<<APF_SMOOTH)|(1<<APF_VCFLAT)))

/* Possible values for ap->translucency; ALPHA_BLENDING requests
 * alpha-blending with correct ordering of faces, SCREEN_DOOR requests
 * masking by a stipple patterns, NAIVE_BLENDING requests
 * alpha-blending without sorting the polygons.
 */
#define APF_ALPHA_BLENDING apf_alpha_blending
#define APF_SCREEN_DOOR    apf_screen_door
#define APF_NAIVE_BLENDING apf_naive_blending

#ifdef AP_IDEBUG

Appearance *    ApCreate();
Appearance *    ApSet();
Material *      MtCreate();
Material *      MtSet();
LtLight *       LtCreate();
LtLight *       LtSet();
LmLighting *    LmCreate();
LmLighting *    LmSet();

int lt_end = LT_END;
int lt_ambient = LT_AMBIENT;
int lt_color = LT_COLOR;
int lt_position = LT_POSITION;
int lt_intensity = LT_INTENSITY;

int lm_end = LM_END;
int lm_ambient = LM_AMBIENT;
int lm_localviewer = LM_LOCALVIEWER;
int lm_attenc = LM_ATTENC;
int lm_attenm = LM_ATTENM;
int lm_ltset = LM_LtSet;
int lm_light = LM_LIGHT;
int lm_valid = LM_VALID;
int lm_invalid = LM_INVALID;
int lm_override = LM_OVERRIDE;
int lm_nooverride = LM_NOOVERRIDE;

int mt_end = MT_END;
int mt_emission = MT_EMISSION;
int mt_ambient = MT_AMBIENT;
int mt_diffuse = MT_DIFFUSE;
int mt_specular = MT_SPECULAR;
int mt_ka = MT_Ka;
int mt_kd = MT_Kd;
int mt_ks = MT_Ks;
int mt_alpha = MT_ALPHA;
int mt_shininess = MT_SHININESS;
int mt_valid = MT_VALID;
int mt_invalid = MT_INVALID;
int mt_override = MT_OVERRIDE;
int mt_nooverride = MT_NOOVERRIDE;

int ap_end = AP_END;
int ap_do = AP_DO;
int ap_dont = AP_DONT ; 
int ap_mat = AP_MAT;
int ap_mtset = AP_MtSet;
int ap_lgt = AP_LGT;
int ap_lmset = AP_LmSet;
int ap_normscale = AP_NORMSCALE;
int ap_linewidth = AP_LINEWIDTH;
int ap_valid = AP_VALID;
int ap_invalid = AP_INVALID;
int ap_override = AP_OVERRIDE;
int ap_nooverride = AP_NOOVERRIDE;
int ap_shading = AP_SHADING;

int lmf_localviewer = LMF_LOCALVIEWER;
int lmf_ambient = LMF_AMBIENT;
int lmf_attenc = LMF_ATTENC;
int lmf_attenm = LMF_ATTENM;
int lmf_replacelights = LMF_REPLACELIGHTS;

int mtf_emission = MTF_EMISSION;
int mtf_ambient = MTF_AMBIENT;
int mtf_diffuse = MTF_DIFFUSE;
int mtf_specular = MTF_SPECULAR;
int mtf_ka = MTF_Ka;
int mtf_kd = MTF_Kd;
int mtf_ks = MTF_Ks;
int mtf_alpha = MTF_ALPHA;
int mtf_shininess = MTF_SHININESS;

int apf_facedraw = APF_FACEDRAW;
int apf_edgedraw = APF_EDGEDRAW;
int apf_transp = APF_TRANSP;
int apf_transmaskp = APF_TRANSMASKP;
int apf_evert = APF_EVERT;

int apf_shading = APF_SHADING;
int apf_normscale = APF_NORMSCALE;
int apf_linewidth = APF_LINEWIDTH;

int apf_constant = APF_CONSTANT;
int apf_flat = APF_FLAT;
int apf_smooth = APF_SMOOTH;

#endif /* AP_IDEBUG */

#endif /* APPEARANCEDEF */

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
