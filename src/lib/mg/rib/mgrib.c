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

#include "mg.h"
#include "mgP.h"
#include "mgribP.h"
#include "mgribshade.h"
#include "mgribtoken.h"
#include "window.h"
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

mgcontext * mgrib_ctxcreate(int a1, ...);
int	    mgrib_ctxset( int a1, ...  );
int	    mgrib_feature( int feature );
void	    mgrib_ctxdelete( mgcontext *ctx );
int	    mgrib_ctxget( int attr, void* valueptr );
int	    mgrib_ctxselect( mgcontext *ctx );
void	    mgrib_sync( void );
void	    mgrib_worldbegin( void );
void	    mgrib_worldend( void );
void	    mgrib_reshapeviewport( void );
void	    mgrib_identity( void );
void	    mgrib_transform( Transform T );
int	    mgrib_pushtransform( void );
int	    mgrib_poptransform( void );
void	    mgrib_settransform( Transform T );
int	    mgrib_pushappearance( void );
int	    mgrib_popappearance( void );
const Appearance *mgrib_setappearance(const Appearance* app, int merge);
int	    mgrib_setcamera( Camera* cam );
mgribcontext *mgrib_newcontext( mgribcontext *ctx );

extern void  mgrib_polygon();
extern void  mgrib_mesh();
extern void  mgrib_line();
extern void  mgrib_polyline();
extern void  mgrib_polylist();
extern void  mgrib_bezier();

int _mgrib_ctxset(int a1, va_list *alist);

WnWindow *mgribwindow(WnWindow *win);

struct mgfuncs mgribfuncs = {
  MGD_RIB,
  mgdevice_RIB,
  mgrib_feature,
  (mgcontext *(*)())mgrib_ctxcreate,
  mgrib_ctxdelete,
  (int (*)())mgrib_ctxset,
  mgrib_ctxget,
  mgrib_ctxselect,
  mgrib_sync,
  mgrib_worldbegin,
  mgrib_worldend,
  mgrib_reshapeviewport,
  /* geometry transform */
  mgrib_settransform,
  mg_gettransform,	/* Generic */
  mgrib_identity,
  mgrib_transform,
  mgrib_pushtransform,
  mgrib_poptransform,
  /* texture transform */
  mg_settxtransform,
  mg_gettxtransform,
  mg_txidentity,
  mg_txtransform,
  mg_pushtxtransform,
  mg_poptxtransform,
  /* appearance stack */
  mgrib_pushappearance,
  mgrib_popappearance,
  mgrib_setappearance,
  mg_getappearance,
  /********************/
  mgrib_setcamera,
  mgrib_polygon,
  mgrib_polylist,
  mgrib_mesh,
  mgrib_line,
  mgrib_polyline,
  mg_quads,
  mgrib_bezier,
  mg_bsptree,
  mg_tagappearance,
  mg_untagappearance,
  mg_taggedappearance
};

mgribcontext *_MGRIB_;	/* For debugging */

int
mgdevice_RIB()
{
  _mgf = mgribfuncs;
  if (_mgc != NULL && _mgc->devno != MGD_RIB)
    _mgc = NULL;

  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_ctxcreate
 * Date:	Thu Jul 18 18:55:18 1991
 * Author:	mbp
 * Notes:	see mg.doc for rest of spec
 */
mgcontext *
mgrib_ctxcreate(int a1, ...)
{
  va_list alist;
  time_t timedate = (time_t)time(0);
    
  _mgc =
    (mgcontext*)(_MGRIB_ = mgrib_newcontext( OOGLNewE(mgribcontext,
						      "mgrib_ctxcreate") ));

  /* Ensure some sensible default Window */
  WnSet(_mgc->win, WN_XSIZE, 450, WN_YSIZE, 450, WN_END);

  /* Default RIB1.0 structure info */
  sprintf(_mgribc->ribscene, "Generic RIB file");
  sprintf(_mgribc->ribcreator, "mgrib driver");
  strcpy(_mgribc->ribfor, getenv("USER"));
  strcpy(_mgribc->ribdate, ctime(&timedate));
  _mgribc->ribdate[24] = '\0'; /* remove the line feed */
  _mgribc->world = 0;

  va_start(alist, a1);
  if (_mgrib_ctxset(a1, &alist) == -1) {
    mgrib_ctxdelete(_mgc);
  }
  va_end(alist);

  return _mgc;
}

/*-----------------------------------------------------------------------
 * Function:	_mgrib_ctxset
 * Description:	internal ctxset routine
 * Args:	a1: first attribute
 *		*alist: rest of attribute-value list
 * Returns:	-1 on error, 0 on success
 * Author:	mbp
 * Date:	Fri Sep 20 11:08:13 1991
 * Notes:	mgrib_ctxcreate() and mgrib_ctxset() call this to actually
 *		parse and interpret the attribute list.
 */
int
_mgrib_ctxset(int a1, va_list *alist)
{
  int attr;
  FILE *ribfile = NULL;
  const char *ribdpy = NULL;
  struct stat st;
  char *dot;

  /* secure copy to fixed-length string */
#undef strNcpy
#define strNcpy(dst, src)						\
  strncpy((dst), (src), sizeof(dst)); (dst)[sizeof(dst)-1] = '\0'

  for (attr = a1; attr != MG_END; attr = va_arg (*alist, int)) {
    switch (attr) {
    case MG_ApSet: {
      Appearance *ap;
      
      ap = _ApSet(NULL, va_arg(*alist, int), alist);
      mgrib_setappearance(ap, MG_MERGE);
      ApDelete(ap);
      break;
    }
    case MG_WnSet:
      _WnSet( _mgc->win, va_arg(*alist, int), alist);
      break;
    case MG_CamSet:
      _CamSet( _mgc->cam, va_arg(*alist, int), alist);
      break;
    case MG_APPEAR:
      mgsetappearance(va_arg(*alist, Appearance *), MG_SET);
      break;
    case MG_WINDOW:
      if (_mgc->win) WnDelete(_mgc->win);
      _mgc->win = va_arg(*alist, WnWindow*);
      RefIncr((Ref*) (_mgc->win));
      break;
    case MG_CAMERA:
      mgrib_setcamera( va_arg(*alist, Camera*) );
      break;
    case MG_SETOPTIONS:
      _mgc->opts |= va_arg(*alist, int);
    case MG_UNSETOPTIONS:
      _mgc->opts &= ~va_arg(*alist, int);
      break;
    case MG_SHOW:
      _mgc->shown = va_arg(*alist, int);
      break;
    case MG_PARENT:
      _mgc->parent = va_arg(*alist, mgcontext*);
      break;
    case MG_BACKGROUND:
      _mgc->background = *va_arg(*alist, ColorA*);
      break;
    case MG_ZNUDGE:
      /* for now, read nothing */
      (void)va_arg(*alist, double);
      break;
    case MG_SPACE:
      _mgc->space = va_arg(*alist, int);
      break;
    case MG_NDCTX:
      _mgc->NDctx = va_arg(*alist, mgNDctx *);
      break;

      /* kind of RIB-specific */
    case MG_RIBFILE:
      /*        if(_mgribc->rib) fclose(_mgribc->rib); */
      ribfile = va_arg(*alist, FILE*);
      break;

      /* really RIB-specific */
    case MG_RIBLINEMODE:
      _mgribc->line_mode = (enum line_mode)va_arg(*alist, int);
      break;
    case MG_RIBFORMAT:
      switch( va_arg(*alist, int) ) {
      case MG_RIBASCII:  _mgribc->render_device = RMD_ASCII; break;
      case MG_RIBBINARY: _mgribc->render_device = RMD_BINARY; break;
      }
      break;
    case MG_RIBDISPLAY:
      _mgribc->display = (enum display)va_arg(*alist, int);
      break;
    case MG_RIBDISPLAYNAME:
      ribdpy = va_arg(*alist, char*);
      break;
    case MG_RIBBACKING:
      _mgribc->backing = (enum backing)va_arg(*alist, int);
      break;
    case MG_RIBSHADER:
      _mgribc->shader = (enum shader)va_arg(*alist, int);
      break;
    case MG_RIBSCENE:
      strNcpy(_mgribc->ribscene, va_arg(*alist, char*));
      break;
    case MG_RIBCREATOR:
      strNcpy(_mgribc->ribcreator, va_arg(*alist, char*));
      break;
    case MG_RIBFOR:
      strNcpy(_mgribc->ribfor, va_arg(*alist, char*));
      break;
    case MG_RIBDATE:
      strNcpy(_mgribc->ribdate, va_arg(*alist, char*));
      break;
    default:
      OOGLError(0, "_mgrib_ctxset: undefined option: %d\n", attr);
      return -1;
    }
  }

  if (ribfile != NULL && ribdpy == NULL) {
    OOGLError(0,
	      "_mgrib_ctxset: must not specify a file without a displayname");
    return -1;
  }

  if (ribdpy != NULL) {
    bool rfopened = false;

    if (ribfile == NULL) {
      ribfile = fopen(ribdpy, "w+");
      rfopened = true;
    }
    if (ribfile == NULL) {
      OOGLError(0, "_mgrib_ctxset: unable to open file for rib display \"%s\"",
		ribdpy);
      return -1;
    }
    if (fstat(fileno(ribfile), &st) < 0) {
      OOGLError(0, "_mgrib_ctxset: unable to stat file for rib display \"%s\"",
		ribdpy);
      if (rfopened) {
	fclose(ribfile);
      }
      return -1;
    }
    /* ok, now insert the stuff into the context */
    if (_mgribc->rib && _mgribc->rib_close) {
      fclose(_mgribc->rib);
    }
    _mgribc->rib = ribfile;
    strNcpy(_mgribc->displayname, ribdpy);
  }

  /* Now decide where to put texture data (anything else?)
   *
   * If ribfile is a regular file, then we asssume that
   * dirname(ribdpy) is its path-component; texture data goes there,
   * under the name basename(ribdpy)-txSEQ.tiff, where "SEQ" is an
   * ever increasing number (during the life-time of the context).
   *
   * If ribfile is not a regular file and dirname(ribdpy) equals ".",
   * then texture data goes to /tmp or ${TMPDIR}, in order not to
   * bloat the current directory which piping data around.
   *
   * If dirname(ribdpy) != ".", but dirname(ribdpy) does not exist,
   * then we loose. So what.
   */

  /* Extract the path-component from displayname */
  strcpy(_mgribc->displaypath, _mgribc->displayname);
  strcpy(_mgribc->displaypath, dirname(_mgribc->displaypath));
  if (!S_ISREG(st.st_mode) && strcmp(_mgribc->displaypath, ".") == 0) {
    strNcpy(_mgribc->displaypath, _mgribc->tmppath);
  }

  /* If displayname really is a path-name, then include only its
   * basename component into the rib-output. Otherwise one has to edit
   * the RIB-file each time one moves it around.
   */
  strcpy(_mgribc->displaybase, _mgribc->displayname);
  strcpy(_mgribc->displaybase, basename(_mgribc->displaybase));
  /* Strip the suffix; the RIB-file will contain displaybase.tiff,
   * texturenames will be generated as displaybase-SEQ.tiff, where SEQ
   * is an ever increasing number.
   */
  if ((dot = strrchr(_mgribc->displaybase, '.')) != NULL &&
      (strcmp(dot, ".tiff") == 0 || strcmp(dot, ".rib") == 0)) {
    *dot = '\0';
  }
  /* Finished. The displayname emitted to the RIB file now will always
   * be without path-component.
   */

  if (_mgc->shown && !_mgribc->born) {
    /* open the window */
    mgribwindow(_mgc->win);
    /* rib state is *not* in accordance with appearance state:
       don't set the appearance until worldbegin time */
  }
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_ctxget
 * Description:	get a context attribute value
 * Args:	attr: the attribute to get
 *		value: place to write attr's value
 * Returns:	1 for success; -1 if attr is invalid
 * Author:	mbp
 * Date:	Fri Sep 20 11:50:25 1991
 * Notes:
 */
int
mgrib_ctxget(int attr, void* value)
{
#define VALUE(type) ((type*)value)

  switch (attr) {

    /* Attributes common to all MG contexts: */
  case MG_APPEAR:
    *VALUE(Appearance*) = &(_mgc->astk->ap);
    break;
  case MG_CAMERA:
    *VALUE(Camera*) = _mgc->cam;
    break;
  case  MG_WINDOW:
    if(_mgribc->born) {
      /* IT DOESN'T MAKE SENSE TO DO THIS, DOES IT? */
    }
    *VALUE(WnWindow*) = _mgc->win;
    break;
  case MG_PARENT:
    *VALUE(mgcontext*) = _mgc->parent;
    break;
  case MG_SETOPTIONS:
  case MG_UNSETOPTIONS:
    *VALUE(int) = _mgc->opts;
    break;
  case MG_BACKGROUND:
    *VALUE(ColorA) = _mgc->background;
    break;
  case MG_ZNUDGE:
    /* not implemented now */
    *VALUE(int) = 0;
  case MG_SPACE: *VALUE(int) = _mgc->space; break;
  case MG_NDCTX: *VALUE(mgNDctx *) = _mgc->NDctx; break;

    /* Attributes specific to RIB contexts: */
  case MG_RIBWINID:
    *VALUE(int) = _mgribc->win;
    break;
  case MG_RIBFILE:
    *VALUE(FILE*) = _mgribc->rib;
    break;

    /* Attributes really specific to RI contexts: */
  case MG_RIBLINEMODE:
    *VALUE(int) = _mgribc->line_mode;
    break;
  case MG_RIBFORMAT:
    switch (_mgribc->render_device) {
    case RMD_ASCII: *VALUE(int) = MG_RIBASCII; break;
    case RMD_BINARY: *VALUE(int) = MG_RIBBINARY; break;
    }
    break;
  case MG_RIBDISPLAY:
    *VALUE(int) = _mgribc->display;
    break;
  case MG_RIBDISPLAYNAME:
    *VALUE(char *) = _mgribc->displayname;
    break;
  case MG_RIBBACKING:
    *VALUE(int) = _mgribc->backing;
    break;
  case MG_RIBSHADER:
    *VALUE(int) = _mgribc->shader;
    break;
  case MG_RIBSCENE:
    *VALUE(char *) = _mgribc->ribscene;
    break;
  case MG_RIBCREATOR:
    *VALUE(char *) = _mgribc->ribcreator;
    break;
  case MG_RIBFOR:
    *VALUE(char *) = _mgribc->ribfor;
    break;
  case MG_RIBDATE:
    *VALUE(char *) = _mgribc->ribdate;
    break;
  default:
    OOGLError (0, "mgrib_ctxget: undefined option: %d\n", attr);
    return -1;

  }
  return 1;

#undef VALUE
}

/*-----------------------------------------------------------------------
 * Function:	mgribwindow
 * Description:	create a RIB window
 * Args:	*win: the WnWindow structure to realize
 * Returns:	win if success, NULL if not
 * Author:	wisdom, gunn
 * Date:	Fri Sep 20 11:56:31 1991
 * Notes:	makes the RI calls necessary to create a RIB "window"
 *		which is really a virtual window defining Format, Display,
 *		etc.
 */
WnWindow *
mgribwindow(WnWindow *win)
{
  WnPosition wp;
  int xsize, ysize;
  char fullscene[280], fullcreator[280], fullfor[280], fulldate[280];
  char dpyname[PATH_MAX];

  /* RIB 1.0 structure comments */
  sprintf(fullscene,"Scene %s",_mgribc->ribscene);
  sprintf(fullcreator,"Creator %s",_mgribc->ribcreator);
  sprintf(fullfor,"For %s",_mgribc->ribfor);
  sprintf(fulldate,"CreationDate %s",_mgribc->ribdate);
  mrti(mr_header,"RenderMan RIB-Structure 1.0", mr_nl,
       mr_header, fullscene, mr_nl,
       mr_header, fullcreator, mr_nl,
       mr_header, fulldate, mr_nl,
       mr_header, fullfor, mr_nl,
       mr_header, "Frames 1", mr_nl,	/* always 1 frame*/
       mr_NULL);

  /* options */
  if(_mgribc->shader != RM_STDSHADE && _mgribc->shadepath) {
    mrti(mr_header, "CapabilitiesNeeded ShadingLanguage", mr_nl,
	 mr_embed, "version 3.03", mr_nl,
	 mr_option, mr_string, "searchpath", mr_string, "shader",
	 mr_nl, mr_embed, "[", mr_string, _mgribc->shadepath,
	 mr_embed, "]", mr_nl, mr_nl, mr_NULL);
  } else {
    mrti(mr_embed, "version 3.03", mr_nl, mr_nl, mr_NULL);
  }
        
  /* set display characteristics...*/
  snprintf(dpyname, PATH_MAX, "%s%s", _mgribc->displaybase, 
	   _mgribc->display == MG_RIBTIFF ? ".tiff" : ".rib");
  mrti(mr_display, mr_string, dpyname, 
       (_mgribc->display == MG_RIBFRAME) ? mr_framebuffer : mr_file, 
       (_mgribc->backing == MG_RIBDOBG) ? mr_rgb : mr_rgba, mr_NULL);

  /* window size */
  WnGet(_mgc->win, WN_CURPOS, &wp);
  xsize = wp.xmax - wp.xmin + 1;
  ysize = wp.ymax - wp.ymin + 1;
  mrti(mr_format, mr_int, xsize, mr_int, ysize, mr_float, 1., mr_NULL);

  _mgribc->born = true;
  return win;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_ctxset
 * Description:	set some context attributes
 * Args:	a1, ...: list of attribute-value pairs
 * Returns:	-1 on error, 0 on success
 * Author:	mbp
 * Date:	Fri Sep 20 12:00:18 1991
 */
int mgrib_ctxset( int a1, ...  )
{
  va_list alist;
  int result;
  
  va_start( alist, a1 );
  result = _mgrib_ctxset(a1, &alist);
  va_end(alist);
  return result;
}


/*-----------------------------------------------------------------------
 * Function:	mgrib_feature
 * Description:	report whether mgrib device has a particular feature
 * Args:	feature: the feature to report on
 * Returns:	an int giving info about feature
 * Author:	wisdom
 * Date:	Tue Aug  3 15:38:05 CDT 1993
 * Notes:	-1 means the feature is not present.
 *
 */
int mgrib_feature( int feature )
{
  if(feature == MGF_BEZIER) return 1;
  return(-1);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_ctxdelete
 * Description:	delete a GL context
 * Args:	*ctx: context to delete
 * Returns:	nothing
 * Author:	slevy
 * Date:	Tue Nov 12 10:29:04 CST 1991
 * Notes:	Deleting the current context leaves the current-context
 *		pointer set to NULL.
 */
void mgrib_ctxdelete( mgcontext *ctx )
{
  /*  if(_mgribc->rib) fclose(_mgribc->rib); */

  if(ctx->devno != MGD_RIB) {
    mgcontext *was = _mgc;
    mgctxselect(ctx);
    mgctxdelete(ctx);
    if(was != ctx) {
      mgctxselect(was);
    }
  } else {
    if(((mgribcontext *)ctx)->shadepath) {
      free(((mgribcontext *)ctx)->shadepath);
    }
    if (_mgribc->tx) {
      OOGLFree(_mgribc->tx);
    }
    mrti_delete(&_mgribc->worldbuf);
    mrti_delete(&_mgribc->txbuf);
    mg_ctxdelete(ctx);
    if(ctx == _mgc)
      _mgc = NULL;
  }
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_ctxselect
 * Description:	select a RIB context --- make it current
 * Args:	*ctx: the context to become current
 * Returns:	0 (why ???)
 * Author:	mbp, wisdom
 * Date:	Fri Sep 20 12:04:41 1991
 */
int
mgrib_ctxselect( mgcontext *ctx )
{
  if(ctx == NULL || ctx->devno != MGD_RIB) {
    return mg_ctxselect(ctx);
  }

  _mgc = ctx;
  _MGRIB_ = (mgribcontext *)ctx;

  return(0);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_sync
 * Description:	sync
 * Returns:	nothing
 * Author:	wisdom
 * Date:	Mon Aug  2 18:05:41 CDT 1993
 * Notes:	sync with PRMAN not necessary
 */
void
mgrib_sync( void )
{}

/*-----------------------------------------------------------------------
 * Function:	mgrib_worldbegin
 * Description:	prepare to draw a frame
 * Returns:	nothing
 * Author:	wisdom, slevy, gunn
 * Date:	Sat Mar  7 16:28:35 GMT-0600 1992
 */
void
mgrib_worldbegin( void )
{
  static Transform cam2ri = {{1, 0,0,0}, {0,1,0,0}, {0,0,-1,0},{0,0,0,1}};
  float halfxfield, halfyfield, aspect, cnear, cfar, fov;
  char str[256];
  HPoint3 look;
  Point3 lookat;
  LtLight **lp;
  int i;

  /* first, check to see if we need to open the default rib file    */
  /* IT'S NOW POSSIBLE THAT THIS WON'T GET SENT TO A FILE (streams) */
  /* IN WHICH CASE IT WOULD BE WRONG TO OPEN A FILE. MOVE THIS!!    */
  if(!_mgribc->rib) {
    if (mgrib_ctxset(MG_RIBDISPLAYNAME, DEFAULT_RIB_FILE, MG_END) == -1) {
      OOGLError(0, "mgrib_worldbeging(): unable to open default file \"%s\"",
		DEFAULT_RIB_FILE);
    }
  }
	
  /* interpret options...(none exist now) */

  mg_worldbegin();
  mg_findcam();

  LM_FOR_ALL_LIGHTS( _mgc->astk->ap.lighting, i,lp ) {
    (*lp)->changed = 1;	/* We'll need to emit all the lights once */
  }

  /* Camera is at (in its own coordinates)  (0,0,0).
   * Looking toward vector (0,0,-1)
   * nominally looking at a point (0,0,-focallen)
   */
  CamGet(_mgc->cam, CAM_FOCUS, &_mgribc->focallen);
  look.x = look.y = 0;  look.z = -_mgribc->focallen;   look.w = 1;
  /* Transform to world coordinates */
  HPt3TransPt3(_mgc->C2W, &look, &lookat);    
  /* Now the camera is at _mgc->cpos (this is a Point3), */
  /* looking at lookat (another Point3) */
   
  /* interpret camera ...*/
  CamGet( _mgc->cam, CAM_NEAR, &cnear);
  CamGet( _mgc->cam, CAM_FAR, &cfar);

  /* make our buffer the current one */
  mrti_makecurrent(&_mgribc->worldbuf);

  mrti(mr_clipping, mr_float, cnear, mr_float, cfar, mr_NULL);
  CamGet( _mgc->cam, CAM_PERSPECTIVE, &_mgribc->persp);
  mrti(mr_projection, mr_string,
       _mgribc->persp ? "perspective" : "orthographic", mr_NULL);
  CamGet( _mgc->cam, CAM_ASPECT, &aspect );
  CamGet( _mgc->cam, CAM_HALFYFIELD, &halfyfield );
  halfxfield = halfyfield * aspect;
  mrti(mr_screenwindow, mr_float, -halfxfield, mr_float, halfxfield,
       mr_float, -halfyfield, mr_float, halfyfield, mr_NULL);
  CamGet( _mgc->cam, CAM_FOV, &fov);
  sprintf(str, "Field of view %.8g", fov);
  mrti(mr_comment, str, mr_NULL);
  mrti(mr_framebegin, mr_int, 1, mr_nl, mr_NULL);
    
  if (_mgc->cpos.w != 0.0 && _mgc->cpos.w != 1.0) {
    sprintf(str, "CameraOrientation %.8g %.8g %.8g %.8g %.8g %.8g",
	    _mgc->cpos.x/_mgc->cpos.w,
	    _mgc->cpos.y/_mgc->cpos.w,
	    _mgc->cpos.z/_mgc->cpos.w,
	    lookat.x,lookat.y,lookat.z);
  } else {
    sprintf(str, "CameraOrientation %.8g %.8g %.8g %.8g %.8g %.8g",
	    _mgc->cpos.x,_mgc->cpos.y,_mgc->cpos.z,
	    lookat.x,lookat.y,lookat.z);
  }
  mrti(mr_header, str, mr_nl, mr_NULL);
  mrti(mr_identity, mr_NULL);
  mgrib_printmatrix(cam2ri);

  mgrib_printmatrix(_mgc->W2C);

  /* otherwise explicitly specified normals would be inverted */
  mrti(mr_reverseorientation, mr_NULL);

  /* declare "bgcolor" as uniform float, needed by the GVblend filters
   * (GL_BLEND texturing).
   *
   * We also declare At (\alpha_{texture}), which is used to switch
   * off texture introduced transparency when the appearance has
   * transparency disabled.
   */
  mrti(mr_declare, mr_string, "bgcolor", mr_string, "uniform color", mr_NULL);
  mrti(mr_declare, mr_string, "At", mr_string, "uniform float", mr_NULL);

  /* RiWorldBegin...*/
  mrti(mr_nl, mr_nl, mr_worldbegin, mr_NULL);

  _mgribc->world = 1;
    
  /* if the option is selected, add the background */
  if(_mgribc->backing == MG_RIBDOBG) {
    float halfxbg = cfar * halfxfield, halfybg = cfar * halfyfield;
    float farz = -.99 * cfar;
    Point3 bg[4];

    bg[0].x = -halfxbg; bg[0].y = -halfybg; bg[0].z = farz;
    bg[1].x = -halfxbg; bg[1].y =  halfybg; bg[1].z = farz;
    bg[2].x =  halfxbg; bg[2].y =  halfybg; bg[2].z = farz;
    bg[3].x =  halfxbg; bg[3].y = -halfybg; bg[3].z = farz;
	
    mrti(mr_comment, "simulate background color via polygon", mr_NULL);
    mrti(mr_attributebegin, mr_NULL);
    mgrib_printmatrix(_mgc->C2W);
    mrti(mr_surface, mr_constant, mr_NULL);
    mrti(mr_color, mr_parray, 3, &_mgc->background, mr_NULL);
    mrti(mr_polygon, mr_P, mr_parray, 4*3, bg, mr_NULL);
    mrti(mr_attributeend, mr_NULL);
  }			

  /* bring ri state into accordance with appearance state */
  
  {
    Appearance *ap = ApCopy( &(_mgc->astk->ap), NULL );
    mgrib_setappearance( ap, MG_SET );
    ApDelete(ap);
  }

  /* NOW DONE BY mg_worldbegin()
   *CamView(_mgc->cam, _mgribc->W2S);
   *TmTranslate(T, 1.0, 1.0, 0);
   *TmConcat(_mgribc->W2S,T, _mgribc->W2S);
   */
     

  /* NOW DONE BY mg_worldbegin()
   *mgrib_ctxget(MG_WINDOW, &win);
   *WnGet(_mgc->win, WN_CURPOS, &wp);
   *xsize = wp.xmax - wp.xmin + 1;
   *ysize = wp.ymax - wp.ymin + 1;
   *
   *TmScale(T, (double)xsize*.5, (double)ysize*.5, 1.0);
   *TmConcat(_mgribc->W2S,T, _mgribc->W2S);
   *
   *TmCopy(_mgribc->W2S, _mgribc->O2S);
   */
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_worldend
 * Description:	finish drawing a frame
 * Returns:	nothing
 * Author:	wisdom
 * Date:	Sat Mar  7 14:47:40 GMT-0600 1992
 */
void
mgrib_worldend( void )
{
  _mgribc->world = 0;
  mrti(mr_worldend, mr_nl, mr_NULL);
  mrti(mr_frameend, mr_nl, mr_NULL);
  /* now flush the buffer, if appropriate */
  /* mgrib_flushbuffer(); NOW DONE AT HIGHER LEVEL */

}

/*-----------------------------------------------------------------------
 * Function:	mgrib_reshapeviewport
 * Description:	adjust to a new window size
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:08:30 1991
 * Notes:	adjusts both GL's internal viewport setting, as well as
 *		MG context WnWindow's current position and camera's
 *		aspect ratio.
 */
void
mgrib_reshapeviewport( void )
{
  WnWindow *win;
  WnPosition wp;
  int xsize, ysize;

  mgrib_ctxget(MG_WINDOW, &win);	/* Get window; force it to ask
					 * NeXTStep how big the window is
					 */
  WnGet(win, WN_CURPOS, &wp);
  xsize = wp.xmax - wp.xmin + 1;
  ysize = wp.ymax - wp.ymin + 1;

  CamSet(_mgc->cam, CAM_ASPECT, (double)xsize/(double)ysize, CAM_END);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_identity
 * Description:	set the current object xform to identity
 * Returns:	nothing
 * Author:
 * Date:
 * Notes:
 *
 */
void
mgrib_identity( void )
{
  mgrib_settransform( TM_IDENTITY );
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_transform
 * Description:	premultiply the object xform by T
 * Args:	T
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:24:57 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 */
void
mgrib_transform( Transform T )
{
  mg_transform(T);
  mgrib_printmatrix(T);
  TmConcat(_mgc->xstk->T, _mgc->W2S, _mgc->O2S);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_pushtransform
 * Description:	push the object xform stack
 * Returns:	nothing (???)
 * Author:	mbp
 * Date:	Fri Sep 20 12:25:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in MVIEWING mode.
 */
int
mgrib_pushtransform( void )
{
  mg_pushtransform();
  mrti(mr_transformbegin, mr_NULL);
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_popransform
 * Description:	pop the object xform stack
 * Returns:	nothing (???)
 * Author:	mbp
 * Date:	Fri Sep 20 12:25:43 1991
 * Notes:	We use the GL ModelView matrix stack, not the mgcontext's
 *		stack.
 *
 *		This assumes we're already in MVIEWING mode.
 */
int
mgrib_poptransform( void )
{
  mg_poptransform();
  mrti(mr_transformend, mr_NULL);
  TmConcat(_mgc->xstk->T, _mgc->W2S, _mgc->O2S);
  return 0;
}


/*-----------------------------------------------------------------------
 * Function:	mgrib_settransform
 * Description:	set the current object xform to T
 * Args:	T
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:29:43 1991
 * Notes:	
 */
void
mgrib_settransform( Transform T )
{
  mrti(mr_identity, mr_NULL);
  mg_settransform( T );
  TmConcat(_mgc->xstk->T, _mgc->W2S, _mgc->O2S);
  if(T != TM_IDENTITY)
    mgrib_printmatrix(T);
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_pushappearance
 * Description:	push the MG context appearance stack
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 12:54:19 1991
 */
int
mgrib_pushappearance( void )
{
  mg_pushappearance();
  if (_mgribc->world)
    mrti(mr_comment, "push appearance", mr_attributebegin, mr_NULL);
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_popappearance
 * Description:	pop the MG context appearance stack
 * Returns:	nothing
 * Author:	
 * Date:	
 * Note:	Note that we do not have to do anything besides
 *		AttributeEnd, because the RenderMan driver retains
 *		the previous state information. 1/13/92 TMM
 */
int
mgrib_popappearance( void )
{
  struct mgastk *mastk = _mgc->astk;

  if (mastk->next == NULL) {
    OOGLError(0, "mggl_popappearance: appearance stack has only 1 entry.\n");
    return 0;
  }

  if (_mgribc->world)
    mrti(mr_comment, "pop appearance", mr_attributeend, mr_NULL);
  mg_popappearance();
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_setappearance
 * Author:	munzner, mbp, wisdom
 * Date:	Wed Aug  7 01:08:07 1991
 * Notes:	Tossed mgrib_material, just use mgrib_appearance
 *		since shaders depend on both appearance and material 
 *		settings. (4/1/93 TMM)
 */
const Appearance *
mgrib_setappearance(const Appearance* ap, int mergeflag )
{
  int changed, mat_changed, lng_changed;
  struct mgastk *mastk = _mgc->astk;
  Appearance *ma;

  ma = &(mastk->ap);

  /* Decide what changes */
  if (mergeflag == MG_MERGE) {
    changed = ap->valid & ~ma->override;
    mat_changed =
      ap->mat ? ap->mat->valid & ~ma->mat->override : 0;
    lng_changed =
      ap->lighting ? ap->lighting->valid & ~ma->lighting->override : 0;
  }
  else {
    changed = ap->valid;
    mat_changed = ap->mat ? ap->mat->valid : 0;
    lng_changed = ap->lighting ? ap->lighting->valid : 0;
  }

  mg_setappearance( ap, mergeflag );

  /* here is where everything gets set (sort of) */

  if(_mgribc->born) {
    mgrib_appearance( mastk, changed, mat_changed);

    /* interpret lights ... */
    mgrib_lighting(_mgc->astk, lng_changed);
  }

  return &_mgc->astk->ap;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_setcamera
 * Description:	set the context's camera (pointer)
 * Args:	*cam: the camera to use
 * Returns:	nothing
 * Author:	mbp
 * Date:	Fri Sep 20 13:07:31 1991
 * Notes:	The context stores a pointer to the camera, not a copy
 *		of it.
 */
int
mgrib_setcamera( Camera* cam )
{
  if (_mgc->cam) CamDelete(_mgc->cam);
  _mgc->cam = cam;
  RefIncr((Ref*) cam);
  return 0;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_newcontext
 * Description:	initialize a new mgricontext structure
 * Args:	*ctx: the struct to initialize
 * Returns:	ctx
 * Author:	mbp
 * Date:	Fri Sep 20 13:11:03 1991
 */
mgribcontext *
mgrib_newcontext( mgribcontext *ctx )
{
  static const char stdshaderpaths[] =
    ".:shaders:/usr/local/prman/prman/lib/shaders:/NextLibrary/Shaders";
  
  char *geomdata = getenv("GEOMDATA");

  memset(ctx, 0, sizeof(mgribcontext));

  mg_newcontext(&(ctx->mgctx));
  ctx->mgctx.devfuncs = &mgribfuncs;
  ctx->mgctx.devno = MGD_RIB;
  ctx->mgctx.astk->ap_seq = 1;
  ctx->mgctx.astk->mat_seq = 1;
  ctx->mgctx.astk->light_seq = 1;
  ctx->born = false;
  ctx->rib = NULL;
  ctx->rib_close = false;
  ctx->backing = RB_DOBG;
  ctx->shader = RM_EXTSHADE;
  ctx->shadepath = NULL;		/* should add context field */
  if (geomdata) {
    char path[512];
    sprintf(path, "%s:%s/shaders:&", stdshaderpaths, geomdata);
    ctx->shadepath = strdup(path);
  }

  if ((ctx->tmppath = getenv("TMPDIR")) == NULL) {
    ctx->tmppath = "/tmp";
  }

  /* initalize the token interface */
  mrti_init(&ctx->worldbuf);
  mrti_init(&ctx->txbuf);
  mrti_makecurrent(&ctx->worldbuf);

  ctx->render_device = RMD_ASCII;
  ctx->line_mode = RM_CYLINDER;

  return ctx;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_findctx
 * Description: Given a GL window ID, returns the associated mg context.
 * Returns:	mgcontext * for success, NULL if none exists.
 * Author:	slevy
 * Date:	Mon Nov 11 18:33:53 CST 1991
 * Notes:	This is a public routine.
 */
mgcontext *
mgrib_findctx( long winid )
{
  struct mgcontext *mgc;

  for(mgc = _mgclist; mgc != NULL; mgc = mgc->next) {
    if(mgc->devno == MGD_RIB && ((mgribcontext *)mgc)->win == winid)
      return mgc;
  }
  return NULL;
}

/*-----------------------------------------------------------------------
 * Function:	mgrib_flushbuffer
 * Description: flushed tokens from buffer to a file
 * Returns:	nothing
 * Author:	wisdom
 * Date:	Mon Jul 26 12:35:45 CDT 1993
 * Notes:	This is a public routine, prototyped in mgrib.h
 */
void
mgrib_flushbuffer()
{
  TokenBuffer *wbuf = &_mgribc->worldbuf;
  size_t size;

  if(!_mgribc->rib) {
    if (mgrib_ctxset(MG_RIBDISPLAYNAME, DEFAULT_RIB_FILE, MG_END) == -1) {
      return;
    }
  }
  if (_mgribc->tx) {
    TokenBuffer *txbuf = &_mgribc->txbuf;

    size = (size_t)wbuf->tkb_worldptr - (size_t)wbuf->tkb_buffer;
    if (size && fwrite(wbuf->tkb_buffer, size, 1, _mgribc->rib) != 1) {
      OOGLError(1, "Error flushing RIB tokenbuffer (prologue)");
    }
    mrti_makecurrent(txbuf);
    mrti(mr_nl, mr_nl, mr_NULL);
    size = (size_t)txbuf->tkb_ptr - (size_t)txbuf->tkb_buffer;
    if (size && fwrite(txbuf->tkb_buffer, size, 1, _mgribc->rib) != 1) {
      OOGLError(1, "Error flushing RIB tokenbuffer (textures)");
    }
    size = (size_t)wbuf->tkb_ptr - (size_t)wbuf->tkb_worldptr;
    if (size && fwrite(wbuf->tkb_worldptr, size, 1, _mgribc->rib) != 1) {
      OOGLError(1, "Error flushing RIB tokenbuffer (world)");
    }
    mrti_reset();
  } else {
    size = (size_t)wbuf->tkb_ptr - (size_t)wbuf->tkb_buffer;
    if (size && fwrite(wbuf->tkb_buffer, size, 1, _mgribc->rib) != 1) {
      OOGLError(1, "Error flushing RIB tokenbuffer");
    }
  }
  fflush(_mgribc->rib);
  mrti_makecurrent(wbuf);
  mrti_reset();
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
