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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include "forms.h"
#include "xforms-compat.h"

#include "ginsuF.h"

int resworld=0;

static char set_clip_plane[] = "( geometry clip_plane {\n\
CQUAD\n\
	1 1 0	1 0 0 1\n\
	-1 1 0	1 0 0 1\n\
	-1 -1 0	0.2 0.2 0.5 1\n\
	1 -1 0	0.2 0.2 0.5 1\n\
})\n";

void csleep(int centisecs) 
{
  struct timeval tv;
  tv.tv_sec = centisecs / 100;
  tv.tv_usec = centisecs % 100;
  select(0, NULL, NULL, NULL, &tv);
}

void set_transform(float M[4][4], float a, float b, float c, float d)
{
  float tmp, sc, sc2;
  float xa, ya, za;
  float xb, yb, zb;
  

  tmp = (float)sqrt((double)(a*a + b*b + c*c));
  sc = sqrt(tmp)/2.0;
  sc2 = 1.0/(tmp*tmp);
  M[0][0] = a/tmp; M[1][0] = b/tmp; M[2][0] = c/tmp; M[3][0] = 0.0;

  xa = M[0][0]; ya = M[1][0]; za = M[2][0];
  xb = M[0][0]; yb = M[1][0]; zb = M[2][0];

  if (xa>-.00001 && xa<.00001)
    xb += 10.0;
  else
    yb += 10.0;

  tmp = xa*xb + ya*yb + za*zb;
  xb -= tmp*xa; yb -= tmp*ya; zb -= tmp*za;
  tmp = (float)sqrt((double)(xb*xb + yb*yb + zb*zb));
  xb = xb/tmp; yb = yb/tmp; zb = zb/tmp;
  M[0][1] = xb; M[1][1] = yb; M[2][1] = zb; M[3][1] = 0.0;

  M[0][2] = ya*zb- za*yb;
  M[1][2] = za*xb - xa*zb;
  M[2][2] = xa*yb - ya*xb;
  tmp = (float)sqrt((double)(M[0][2]*M[0][2] + M[1][2]*M[1][2]
		+ M[2][2]*M[2][2]));
  M[0][2] = M[0][2]/tmp;
  M[1][2] = M[1][2]/tmp;
  M[2][2] = M[2][2]/tmp;

  M[0][3] = d*a; M[1][3] = d*b; M[2][3] = d*c; M[3][3] = 1.0;

  fprintf(stdout,"(xform-set clip_plane { ");
  fprintf(stdout,"%f %f %f %f ",M[0][1]*sc,M[1][1]*sc,M[2][1]*sc,M[3][1]);
  fprintf(stdout,"%f %f %f %f ",M[0][2]*sc,M[1][2]*sc,M[2][2]*sc,M[3][2]);
  fprintf(stdout,"%f %f %f %f ",M[0][0]*sc,M[1][0]*sc,M[2][0]*sc,M[3][0]);
  fprintf(stdout,"%f %f %f %f })\n",M[0][3]*sc2,M[1][3]*sc2,
			M[2][3]*sc2,M[3][3]);
  fflush(stdout);
}

void update_transform(int skip)
{
  char str[100];
  float T[4][4];
  float x[3],y[3],z[3];
  float av,bv,cv,dv;
  static struct timeval notime = {0, 0};
  static struct timeval timeout = {0, 200000};
  FILE *in = stdin;
  char c = ' ';
  int   count=0;

  c = ' ';
  if (skip)
  {
   fprintf(stdout,"( write transform - clip_plane wrap )\n");
   fprintf(stdout,"( echo q )\n");
   fflush(stdout);
  }
  while ((c!='q')&&(!feof(stdin)))
  {
   while ((c!='\n')&&(c!='q')&&(!feof(stdin)))
   {
    c = (char) fgetc(stdin);
    /*fputc((int)c,stderr);
    fputc((int)':',stderr);
    fprintf(stderr,"%d",(int)c);*/
   }
   if ((c=='q')||(feof(stdin))) exit(0);
   fscanf(stdin," %f %f %f %f\n",&T[0][0],&T[1][0],&T[2][0],&T[3][0]);
   fscanf(stdin," %f %f %f %f\n",&T[0][1],&T[1][1],&T[2][1],&T[3][1]);
   fscanf(stdin," %f %f %f %f\n",&T[0][2],&T[1][2],&T[2][2],&T[3][2]);
   fscanf(stdin," %f %f %f %f\n",&T[0][3],&T[1][3],&T[2][3],&T[3][3]);
   count=0;
   while ((c!='q')&&(!feof(stdin)))
   {
    c = (char) fgetc(stdin);
    /*fputc((int)c,stderr);
    fputc((int)'!',stderr);*/
   }
  if (c!='q') exit(0);
  }
  x[0] = T[0][0]*1.0+T[0][1]*1.0+T[0][2]*0.0+T[0][3]*1.0;
  y[0] = T[1][0]*1.0+T[1][1]*1.0+T[1][2]*0.0+T[1][3]*1.0;
  z[0] = T[2][0]*1.0+T[2][1]*1.0+T[2][2]*0.0+T[2][3]*1.0;
  x[1] = T[0][0]*1.0+T[0][1]*(-1.0)+T[0][2]*0.0+T[0][3]*1.0;
  y[1] = T[1][0]*1.0+T[1][1]*(-1.0)+T[1][2]*0.0+T[1][3]*1.0;
  z[1] = T[2][0]*1.0+T[2][1]*(-1.0)+T[2][2]*0.0+T[2][3]*1.0;
  x[2] = T[0][0]*(-1.0)+T[0][1]*(-1.0)+T[0][2]*0.0+T[0][3]*1.0;
  y[2] = T[1][0]*(-1.0)+T[1][1]*(-1.0)+T[1][2]*0.0+T[1][3]*1.0;
  z[2] = T[2][0]*(-1.0)+T[2][1]*(-1.0)+T[2][2]*0.0+T[2][3]*1.0;

  fprintf(stdout,"( write transform - clip_plane wrap )\n");
  fflush(stdout);
  
  resworld++;
  if (resworld>4)
  {
   resworld=0;
   fprintf(stdout,"( normalization World keep )\n");
  }
  fprintf(stdout,"( echo q )\n");
  fflush(stdout);
  av = (y[1]-y[0])*(z[2]-z[0])-(z[1]-z[0])*(y[2]-y[0]);
  bv = (z[1]-z[0])*(x[2]-x[0])-(x[1]-x[0])*(z[2]-z[0]);
  cv = (x[1]-x[0])*(y[2]-y[0])-(y[1]-y[0])*(x[2]-x[0]);
  dv = av*x[0]+bv*y[0]+cv*z[0];
  sprintf(str, "    %7.5f",-av);
  fl_set_input(Ainput,str);
  sprintf(str, "    %7.5f",-bv);
  fl_set_input(Binput,str);
  sprintf(str, "    %7.5f",-cv);
  fl_set_input(Cinput,str);
  sprintf(str, "    %7.5f",-dv);
  fl_set_input(Dinput,str);
}

void set_info(FL_OBJECT *myinfo)
{
 int i;
 static char *inf[] = {
 "By Daeron Meyer",
 "Copyright (c) 1992",
 "The Geometry Center",
 "www.geomview.org",
 " ",
 "Ginsu is free software and is",
 "designed to be used as a tool",
 "for Geomview. It allows the user",
 "to interactively cut an object",
 "within Geomview and then view",
 "the resulting pieces."
 };
 int nlines = sizeof(inf)/sizeof(char*);
 for (i=0;i<nlines;i++)
  fl_add_browser_line(myinfo, inf[i]);
}

main()
{
 FL_OBJECT *retobj = NULL;
 int c = ' ';
 const char *targ;
 char str[1024], tfname[100], *tmpdir;
 int  update_count=0;
 int  count, xc, yc;
 int  editon = 0;
 float ap,bp,cp,dp;
 float M[4][4];

 tmpdir = getenv("TMPDIR");
 if(tmpdir == NULL) tmpdir = "/tmp";
 sprintf(tfname, "%.88s/ginsu%d", tmpdir, getpid());

 for (yc=0; yc<4; yc++)
  for (xc=0; xc<4; xc++)
  {
    if (yc==xc)
      M[xc][yc] = 1;
    else
      M[xc][yc] = 0;
  }

  /* For XFORMS */
  FL_INITIALIZE("Ginsu");

 create_the_forms();
 set_info(MyBrowser);
 fl_show_form(ClipPanel, FL_PLACE_SIZE, TRUE, "Ginsu");
 fputs(set_clip_plane, stdout);
 fprintf(stdout,"( write transform - clip_plane wrap )\n");
 fprintf(stdout,"( echo q )\n");
 fflush(stdout);

 while (retobj!=ExitButton)
 {
  if (editon)
    retobj = fl_do_forms();
  else
    retobj = fl_check_forms();
  if (retobj==ExitButton)
  {
     fprintf(stdout,"( delete clip_plane )");
     fflush(stdout);
     exit(0);
  }
  else
  if (retobj==InfoButton)
  {
   fl_show_form(InfoPanel, FL_PLACE_SIZE, TRUE, "Info");
  }
  else
  if (retobj==CloseButton)
  {
   fl_hide_form(InfoPanel);
  }
  else
  if (retobj==EditButton)
  {
   editon = fl_get_button(EditButton);
   fl_freeze_form(ClipPanel);
   update_transform(0);
   update_transform(0);
   fl_unfreeze_form(ClipPanel);
   update_count = 0;
  }
  else
  if (retobj == Ainput || retobj == Binput
	|| retobj == Cinput || retobj == Dinput)
  {
    targ = fl_get_input(Ainput);
    sscanf(targ,"%f",&ap);
    sprintf(str, "    %7.5f",ap);
    fl_set_input(Ainput,str);
    targ = fl_get_input(Binput);
    sscanf(targ,"%f",&bp);
    sprintf(str, "    %7.5f",bp);
    fl_set_input(Binput,str);
    targ = fl_get_input(Cinput);
    sscanf(targ,"%f",&cp);
    sprintf(str, "    %7.5f",cp);
    fl_set_input(Cinput,str);
    targ = fl_get_input(Dinput);
    sscanf(targ,"%f",&dp);
    sprintf(str, "    %7.5f",dp);
    fl_set_input(Dinput,str);
    if (!((ap==0.0)&&(bp==0.0)&&(cp==0.0)))
      set_transform(M, ap, bp, cp, dp);
  }
  else
  if (retobj==SliceButton)
  {
   targ = fl_get_input(TargInput);
   if (strlen(targ)&&strcmp(targ,"clip_plane"))
   {
    if ((!strcmp(targ,"World"))||(!strcmp(targ,"g0"))||(!strcmp(targ,"world")))
     fprintf(stdout,"( geometry clip_plane {} )\n");

    fprintf(stdout,"(write geometry \"%s\" %s world)\n", tfname, targ);
    fprintf(stdout,"(echo x)\n");
    fflush(stdout);

    c = ' ';
    while ((c = fgetc(stdin)) != EOF && c != 'x')
	;
    if (c == EOF) exit(0);
    targ = fl_get_input(Ainput);
    sscanf(targ,"%f",&ap);
    targ = fl_get_input(Binput);
    sscanf(targ,"%f",&bp);
    targ = fl_get_input(Cinput);
    sscanf(targ,"%f",&cp);
    targ = fl_get_input(Dinput);
    sscanf(targ,"%f",&dp);
    sprintf(str, "clip -v %g,%g,%g -l %g < %s > %s- && \
	clip -v %g,%g,%g -g %g < %s > %s+",
		ap,bp,cp,dp, tfname, tfname,
		ap,bp,cp,dp, tfname, tfname);
    if(system(str) == 0) {
	targ=fl_get_input(TargInput);
	fprintf(stdout,
	"(progn (delete %s)(new-geometry ginsu.l < %s-)(new-geometry ginsu.g < %s+)(! rm -f %s- %s+))\n",
		targ, tfname, tfname, tfname, tfname);
	fflush(stdout);
    } else {
	fprintf(stderr, "ginsu: Couldn't clip!  Is \"clip\" installed on your UNIX search path?");
	sprintf(str, "%s-", tfname);  unlink(str);
	sprintf(str, "%s+", tfname);  unlink(str);
    }

    unlink(tfname);

    fputs(set_clip_plane, stdout);
    fflush(stdout);
    update_transform(1);
   }
  }
  update_count++;
  csleep(1);
  if (update_count>50 && !editon)
  {
   update_transform(0);
   update_count=0;
  }
 }
}
