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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "geom.h"
#include "discgrpP.h"
#include "instP.h"
#include "maniview.h"

#include "forms.h"
#include "xforms-compat.h"

char gvinitstr[] =
#include "gvinit.h"
;

char maniviewhelpstr[] =
#include "maniviewhelp.h"
;

char *getline(char *s);

/* globals */
DiscGrp *dg = NULL;
int changed = 0;
int space = DG_EUCLIDEAN; 
int myindex = 1;
int softshade = 1;
int loadtype = LOAD_GROUP;
int tilemode = DIRDOM_MODE;
float myscale = .2;

int mainplacement = FL_PLACE_MOUSE;
int displayplacement = FL_PLACE_MOUSE;
int loadplacement = FL_PLACE_MOUSE;
int saveplacement = FL_PLACE_MOUSE;
int enumplacement = FL_PLACE_MOUSE;
int dirdomplacement = FL_PLACE_MOUSE;
int helpplacement = FL_PLACE_MOUSE;
int infoplacement = FL_PLACE_MOUSE;
int fileplacement = FL_PLACE_MOUSE;

/* magic values for attenuation constants for software shaders 
 * entries: 
	0	atten1
	1	atten2
	2,3	bounds on atten1
	3,4	bounds on atten2
 */
float atk[3][7] = {
		{.5, 1.0,0.0,1.0,0.0,6.0, 0.0}, 	/* hyperbolic */
		{0.5, 0.5, 0.0, 2.0, 0.0, 1.0, 0.0}, /* euclidean */
		{0.5, .3, 0.0, 1.0, 0.0, 1.57, 0.0}}; /*spherical */
float bkgdblend = 0.0;  /* no background blend */
float radius[3] = {6.0, 15.0, 4.0};
float drawradius[3] = {6.0, 15.0, 4.0};
float depth[3] = {6.0, 15.0, 4.0};

void
fl_update_from_dg()
{
  if (dg == NULL ) return;
  fl_set_positioner_xvalue(DDXYPositioner, dg->cpoint.x);
  fl_set_positioner_yvalue(DDXYPositioner, dg->cpoint.y);
  fl_set_dial_value(DDZDial, dg->cpoint.z);
  fl_set_counter_value(WorddepthCounter, dg->enumdepth);
  fl_set_slider_value(RadiusSlider, dg->enumdist);
  fl_set_slider_value(DrawRadiusSlider, dg->drawdist);
  fl_set_slider_value(DDScaleSlider, myscale);
  fl_set_slider_value(Attenuation1Slider, atk[myindex][0]);
  fl_set_slider_value(Attenuation2Slider, atk[myindex][1]);
  fl_set_slider_value(Attenuation3Slider, atk[myindex][6]);
  fl_set_button(DirdomButton, dg->flag & DG_DRAWDIRDOM);
  fl_set_button(DrawGeomButton, dg->flag & DG_DRAWGEOM);
  fl_set_button(CentercamButton, dg->flag & DG_CENTERCAM);
  fl_set_button(ShowcamButton, dg->flag & DG_DRAWCAM);
  fl_set_button(CullzButton, dg->flag & DG_ZCULL);
  fl_set_button(SaveGeomButton, dg->flag & DG_SAVEDIRDOM);
  fl_set_button(SaveGroupButton, dg->flag & DG_SAVEBIGLIST);
  fl_set_button(DirichletDomainButton, tilemode & DIRDOM_MODE );
  fl_set_button(UsergeometryButton, tilemode & USER_GEOM );
  fl_set_button(LoadGroupButton, loadtype & LOAD_GROUP );
  fl_set_button(LoadGeomButton, loadtype & LOAD_GEOM );
  fl_set_button(LoadCameraGeomButton, loadtype & LOAD_CAMGEOM );
  fl_set_button(SoftshadeButton, softshade);
}

void
fl_set_bounds()
{
  fl_set_slider_bounds(Attenuation1Slider, atk[myindex][2], atk[myindex][3]);
  fl_set_slider_bounds(Attenuation2Slider, atk[myindex][4], atk[myindex][5]);
  fl_set_slider_bounds(Attenuation3Slider, 0.0, 1.0);
  fl_set_slider_bounds(RadiusSlider, 0.0, radius[myindex]);
  fl_set_slider_bounds(DrawRadiusSlider, 0.0, drawradius[myindex]);
  fl_set_counter_bounds(WorddepthCounter, 0.0, depth[myindex]);
  fl_set_positioner_xbounds(DDXYPositioner, -.5, .5);
  fl_set_positioner_ybounds(DDXYPositioner, -.5, .5);
  fl_set_dial_bounds(DDZDial, -.5, .5);
  fl_update_from_dg();
}

ui_init()
{
  char buf[120];
  foreground();
  FL_INITIALIZE("Maniview");
  create_the_forms();
  fl_set_bounds();

  {
    FILE *hf = fopen("maniviewhelp", "rb");

    if (hf == NULL) {
      char *line, *delims = "\n";
      line = getline(maniviewhelpstr);
      while (line) {
        fl_add_browser_line( HelpBrowser, line );
        line = getline(NULL);
      }
      fl_set_browser_topline( HelpBrowser, 1 );
    } else {
      if (hf != NULL) {
        while (fgets(buf, 120, hf) != NULL)
          fl_add_browser_line( HelpBrowser, buf );
        fl_set_browser_topline( HelpBrowser, 1 );
        fclose(hf);
      }
    }
  }
}

ui_main_loop()
{
  fl_show_form(MainForm, mainplacement, TRUE, "Maniview");
  while(1)	{
    fl_check_forms();
    if (ifp == stdin)
      if (async_fnextc(ifp, 0) != NODATA) {
        loadstuff(ifp, NULL, LOAD_GROUP);
	}
    if (changed)	{
	update_gv();
	fl_update_from_dg();
	changed = 0;
	}
    }
}

char *getline(char *s)
{
  static char *p;
  char *first;

  if (s != NULL) {
    p = s;
  } else {
    *p = '\n';
  }
  first = p;
  ++p; 
  while (*p != '\n' && *p != '\0') ++p;
  if (*p == '\n') {
    *p = '\0';
    return first;
  }
  return NULL;
}

/* get the default setup up to send over to Geomview at startup */
void
gvinit()
{
    extern char *getline();
    FILE *outf = stdout;
    char *line, *delims = "\n";
    line = getline(gvinitstr);
    while (line) {
        fprintf(outf,"%s",line);
        line = getline(NULL);
    }
    fflush(outf);
}

static char geomname[] = "maniview";

update_gv()
{
    FILE *outf = stdout;

    if (!dg) return;
    fprintf(outf,"(progn\n");

    /* this is a hack to avoid bad results when trying to fly around
     * inside a manifold: geomview's default setting makes the geometry,
     * and not the camera, the "center" of the motion */
    if (dg->flag & DG_CENTERCAM) 
	fprintf(outf,"(ui-target c0)\n");

    if (changed & NEW_AP) {
	/* send the light attenuation */
	fprintf(outf,"(merge-ap %s {lighting attenmult %g attenmult2 %g attenconst %g})\n",geomname, atk[myindex][0], atk[myindex][1], atk[myindex][6]);
	}
    if (changed & NEW_SPACE) {
	if (dg->attributes & DG_HYPERBOLIC) 
		fprintf(outf,"(space hyperbolic)\n");
	else if (dg->attributes & DG_EUCLIDEAN) 
		fprintf(outf,"(space euclidean)\n");
	else if (dg->attributes & DG_SPHERICAL) 
		fprintf(outf,"(space spherical)\n");
	/* fall through to the next case... */
	fprintf(outf,"(merge-ap %s {lighting attenmult %g attenmult2 %g attenconst %g})\n",geomname, atk[myindex][0], atk[myindex][1], atk[myindex][6]);
	changed |= CHANGED;
	}
    if (changed & DIRDOM)	{
      if (tilemode & USER_GEOM)
	if (dg->geom && dg->geom != dg->ddgeom)	{
	    Transform t1, t2;
	    Inst *inst;
	    TmTranslate(t1, dg->cpoint.x, dg->cpoint.y, dg->cpoint.z);
	    TmScale(t2, myscale, myscale, myscale);
	    TmConcat(t2, t1, t1);
	    /* HACK!! */
	    inst = (Inst *) dg->geom;
	    TmCopy(t1, inst->axis);
	    } 
      else dg->scale = myscale;
      }
    if (changed & CHANGED)	{
	fprintf(outf,"(read geometry {define dghandle {\n");
    	GeomFSave((Geom *) dg, outf, (char *)NULL);
    	fprintf(outf,"}})\n");
	}
    /* this needs to go last */
    if (changed & SOFTSHADE)	{
	fprintf(outf,"(soft-shader focus %s )\n",softshade ? "on" : "off");
	}
    fprintf(outf,")\n");
    fflush(outf);
}

   char defaultfilename[] = "3torus.dgp"; 

    FILE *ifp;		/* input file pointer; may be stdin */
main(int argc, char **argv)	
{
    char *filename = NULL;

    ui_init();

    if (argc > 1) {
	if (argv[1][0] == '-')	{
	    ifp = stdin;
	    }
	else {
	    filename = argv[1];
	    }
	}
    else filename = defaultfilename;

    if (filename) ifp = get_input_fp(filename, loadtype);
    loadstuff(ifp, filename, loadtype);
    if (dg == NULL) {
	filename = defaultfilename;
        ifp = get_input_fp(filename, loadtype);
	loadstuff(ifp,  filename, loadtype);
	}
    gvinit();

    update_gv();
    fl_update_from_dg();
    ui_main_loop();

}


