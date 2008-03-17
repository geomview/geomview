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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <ctype.h>
#include "discgrpP.h"
#include "options.h"
#include "complex.h"		/* in case file has elements of SL(2,C) */
#include <string.h>
#include "mgP.h"
#include "streampool.h"
#include "transobj.h"
#include "handleP.h"
#include "projective.h"

int fsaparse();

#define DG_GROUPNAME	1
#define DG_COMMENT	2
#define DG_ATTRIBUTE	3
#define DG_MODEL	4
#define DG_NGENS	5
#define DG_NELS		6
#define DG_GENS		7
#define DG_ELS		8
#define	DG_DIMN		9
#define	DG_CAMGEOM	10
#define	DG_GEOM		11
#define	DG_CAMGEOMFILE	12
#define	DG_GEOMFILE	13
#define	DG_WAFILE	14
#define	DG_MATRIXGROUP	15
#define	DG_CPOINT	16
#define	DG_ENUMDEPTH	17
#define	DG_ENUMDIST	18
#define	DG_DSPYATTR	19
#define DG_SCALE	20
#define DG_C2M		21
#define	DG_DRAWDIST	22
#define DG_NUMKEYWORDS	24	/* 2 redundant keywords */

keytokenpair keytokenlist[] = {
  {"group", 	DG_GROUPNAME},
  {"comment", 	DG_COMMENT},
  {"attribute", 	DG_ATTRIBUTE},
  {"model", 	DG_MODEL},
  {"ngens", 	DG_NGENS},
  {"nels", 	DG_NELS},
  {"gens", 	DG_GENS},
  {"els", 	DG_ELS},
  {"dimn",	DG_DIMN},
  {"dimension",	DG_DIMN},
  {"camgeom",	DG_CAMGEOM},
  {"geom",	DG_GEOM},
  {"camgeomfile",	DG_CAMGEOMFILE},
  {"geomfile",	DG_GEOMFILE},
  {"wafile",	DG_WAFILE},
  {"matrixgroup",	DG_MATRIXGROUP},
  {"mgroup",	DG_MATRIXGROUP},
  {"cpoint",	DG_CPOINT},
  {"enumdepth",	DG_ENUMDEPTH},
  {"enumdist",	DG_ENUMDIST},
  {"drawdist",	DG_DRAWDIST},
  {"display",	DG_DSPYATTR},
  {"scale",	DG_SCALE},
  {"cam2model",	DG_C2M}
};

keytokenpair attr_list[DG_NUM_ATTR] = {
  {"hyperbolic", 	DG_HYPERBOLIC},
  {"euclidean", 	DG_EUCLIDEAN},
  {"spherical", 	DG_SPHERICAL},
  {"finite", 	DG_FINITE},
  {"transposed", 	DG_TRANSPOSED},
  {"conformalball", DG_CONFORMALBALL},
  {"upperhalfspace", DG_UPPERHALFSPACE},
  {"projective", 	DG_PROJECTIVEMODEL}
};

keytokenpair dspyattr_list[DG_NUM_DSPYATTR] = {
  {"centercam",	DG_CENTERCAM},
  {"zcull",	DG_ZCULL},
  {"drawcam",	DG_DRAWCAM},
  {"drawdirdom",	DG_DRAWDIRDOM},
  {"drawgeom",	DG_DRAWGEOM}
};

/* Name can be {GL | SL | SO } ( {n},{k},{R | C} ) */
matrixgroup cgroup = { DG_GENERAL | DG_REAL, 4, 0};

static char delims[] = "%{}();";
static char errfmt[] = "Reading discrete group from \"%s\": %s";

static int
token_from_string(char *s, keytokenpair *kl, int n)
{
  int i;
  for (i=0; i<n; ++i)	{
    if (strcasecmp(s, kl[i].key) == 0)
      return (kl[i].token);
  }
  return (0);
}


/* following gets the next %keyword from file.
   returns 0 if EOF, otherwise 1 */
static int
get_keyword(IOBFILE *fp, char keyword[], char *fname)
{
  switch(iobfnextc(fp, 0)) {
  case EOF:
    return 0;

  case CKET:
    return 0;

  case '(':
    iobfgetc(fp);
    sprintf(keyword, "%.31s", iobfdelimtok(delims, fp, 0));
    return 1;

  default:
    OOGLSyntax(fp,
	       "Reading discrete group from \"%s\": expected (", fname);
    return(0);
  }
}

static int
get_matching_parenthesis(IOBFILE *fp, char *fname)
{
  int t = iobfnextc(fp, 0);
  if (t == EOF) return(0);
  if (t != ')')	{
    OOGLSyntax(fp,"Reading discrete group from \"%s\": expected matching )", fname);
    return(0);
  }
  iobfgetc(fp);
  return(1);
}

static IOBFILE *
included_file(IOBFILE *fp)
{
  char *name;

  if (iobfnextc(fp, 0) == '<') /* read from file */
    {
      name = iobfdelimtok(delims, fp, 0);
      OOGLError(1,
		"Discrete groups: including files "
		"(here: \"%s\") not implemented", name);
      return NULL;
    }
  return NULL;
}

static void
parse_group_name(char *gname)
{
  (void)gname;
  cgroup.attributes = DG_GENERAL | DG_REAL;	/* type of entry */
  cgroup.dimn = 4;		/* dimension of matrices */
  cgroup.sig = 0;		/* signature of quadratic form */

}

static ColorA white = {1,1,1,.75};

static void
get_el_list( DiscGrp *discgrp, DiscGrpElList *dgellist, IOBFILE *fp, char *fname)
{
  int i;
  char *name, c;

  if (included_file(fp))	/* read from file */
    {
    }

  for (i=0; i<dgellist->num_el; ++i)	{
    dgellist->el_list[i].attributes = 0;
    dgellist->el_list[i].color = white;
    dgellist->el_list[i].inverse = NULL;
    c = iobfnextc(fp, 0);
    /* get the name if it's alphabetic */
    if ( c >= 'A' && c <= 'z' )
      {
	name = iobfdelimtok(delims, fp, 0);
	if (strlen(name) > DG_WORDLENGTH) {
	  OOGLSyntax(fp,"Reading discrete group from \"%s\": Words limited to length %d", fname, DG_WORDLENGTH);
	  return;
	}
	strcpy(dgellist->el_list[i].word, name);
      }
    else  {	/* make up a name for this gen */
      dgellist->el_list[i].word[0] = 'a' + i;
      dgellist->el_list[i].word[1] = 0;
    }


    switch(discgrp->attributes & DG_MODEL_BITS){
    case DG_CONFORMALBALL:
      OOGLSyntax(fp,errfmt,fname,"Unimplemented conformal model");
      break;

    case DG_UPPERHALFSPACE:
      {
	sl2c_matrix mylf;
	proj_matrix mypm;
	int k,m;
	for (k=0; k<2; ++k) for (m=0; m<2; ++m) {
	  iobfgetnd(fp, 1, &mylf[k][m].real, 0);
	  iobfgetnd(fp, 1, &mylf[k][m].imag, 0);
	}
	sl2c_to_proj(mylf, mypm);
	for (k=0; k<4; ++k) for (m=0; m<4; ++m)
	  dgellist->el_list[i].tform[k][m] = mypm[k][m];
      }
      break;

    default:
      if (iobfgettransform(fp,1,(float *)dgellist->el_list[i].tform,0) != 1) {
	OOGLSyntax(fp,errfmt,fname,"Error reading generator");
	return;
      }
      if (discgrp->attributes & DG_TRANSPOSED)
	TmTranspose(dgellist->el_list[i].tform, discgrp->big_list->el_list[i].tform);
      break;
    }
  }
  discgrp->attributes &= ~DG_UPPERHALFSPACE;
}

Geom *
DiscGrpImport(Pool *p)
{
  char *name, *fname;
  char keyword[DG_KEYWORDSIZE];
  DiscGrp *discgrp;
  IOBFILE *fp;
  FILE *wafp;
  char *expect;

  if(p == NULL || (fp = PoolInputFile(p)) == NULL)
    return 0;

  /* check for 'DISCGRP' at head of file */
  if(strcmp(GeomToken(fp), "DISCGRP"))
    return(NULL);

  /* now the parentheses begin */
  if (iobfnextc(fp, 0) != '(')
    return(NULL);

  discgrp = (DiscGrp*)GeomCreate("discgrp",CR_END);

  while (get_keyword(fp, keyword,p->poolname))	{

    switch ( token_from_string(keyword, keytokenlist,sizeof(keytokenlist)/sizeof(keytokenpair) ))	{

    case DG_WAFILE:
      name = iobfdelimtok(delims, fp, 0);
      fname = findfile(PoolName(p), name);
      if(fname == NULL || (wafp = fopen(fname, "rb")) == NULL) {
	OOGLSyntax(fp,
		   "Reading discrete group from \"%s\": can't open wafile \"%s\"",
		   p->poolname, name);
	return(NULL);
      }
      discgrp->fsa = OOGLNew(wa);
      fsaparse(wafp, discgrp->fsa);
      fclose(wafp);
      break;

    case DG_DSPYATTR:
      name = iobfdelimtok(delims, fp, 0);
      discgrp->flag |= token_from_string(name, dspyattr_list,sizeof(dspyattr_list)/sizeof(keytokenpair));
      break;

    case DG_ATTRIBUTE:
    case DG_MODEL:
      name = iobfdelimtok(delims, fp, 0);
      discgrp->attributes |= token_from_string(name, attr_list,sizeof(attr_list)/sizeof(keytokenpair));
      break;

    case DG_COMMENT:
      discgrp->comment = strdup(iobfdelimtok(delims, fp, 0));
      break;

    case DG_MATRIXGROUP:
      parse_group_name(iobfdelimtok(delims, fp, 0));
      break;

    case DG_SCALE:
      if(iobfgetnf(fp, 1, &discgrp->scale, 0) <= 0) {
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid scale");
	return(NULL);
      }
      break;


    case DG_C2M:
      discgrp->c2m = (float (*)[4])OOGLNewNE(float, 16, "Transform");
      if (iobfgettransform(fp,1,(float *)discgrp->c2m,0) != 1) {
	OOGLSyntax(fp,errfmt,p->poolname,"Error reading cam2model");
	return(NULL);
      }
      break;

    case DG_ENUMDEPTH:
      if(iobfgetni(fp, 1, &discgrp->enumdepth, 0) <= 0) {
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid enumdepth");
	return(NULL);
      }
      break;

    case DG_ENUMDIST:
      if(iobfgetnf(fp, 1, &discgrp->enumdist, 0) <= 0) {
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid enumdist");
	return(NULL);
      }
      break;

    case DG_DRAWDIST:
      if(iobfgetnf(fp, 1, &discgrp->drawdist, 0) <= 0) {
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid drawdist");
	return(NULL);
      }
      break;

    case DG_CPOINT:
      if(iobfgetnf(fp, 4, (float *)&discgrp->cpoint, 0) <= 0) {
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid Cpoint");
	return(NULL);
      }
      break;

    case DG_CAMGEOM:
      expect = "camgeometry";
      if(!GeomStreamIn(p, &discgrp->camgeomhandle, &discgrp->camgeom))
	goto failed;
      if(discgrp->camgeomhandle)
	HandleRegister(&discgrp->camgeomhandle, (Ref *)discgrp,
		       &discgrp->camgeom, HandleUpdRef);

      break;

    case DG_ELS:
      discgrp->big_list->mgroup = cgroup;
      get_el_list(discgrp, discgrp->big_list, fp, p->poolname);
      discgrp->flag |= DG_SAVEBIGLIST;
      break;

    case DG_GROUPNAME:
      discgrp->name = strdup(iobfdelimtok(delims, fp, 0));
      break;

    case DG_GENS:
      {
	int i;
	static char name[2] = "a";
	discgrp->gens->mgroup = cgroup;
	get_el_list(discgrp, discgrp->gens, fp, p->poolname);
	/* make up names for the generators if not given */
	if (strcmp(discgrp->gens->el_list[0].word, "") == 0) {
	  for (i=0; i<discgrp->gens->num_el; ++i)	{
	    strcpy(discgrp->gens->el_list[i].word,name);
	    name[0]++;
	  }
	}
      }

      break;

    case DG_GEOM:
      expect = "geometry";
      if(!GeomStreamIn(p, &discgrp->geomhandle, &discgrp->geom))
	goto failed;
      if(discgrp->geomhandle)
	HandleRegister(&discgrp->geomhandle, (Ref *)discgrp,
		       &discgrp->geom, HandleUpdRef);

      break;

    case DG_DIMN:
      if(iobfgetni(fp, 1, &discgrp->dimn, 0) <= 0 ||
	 discgrp->dimn > 4) 	{
	OOGLSyntax(fp,errfmt, p->poolname, "Invalid Dimension");
	return(NULL);
      }
      cgroup.dimn = discgrp->dimn+1;  /* default matrix group */
      break;

    case DG_NGENS:
      {
	int ngens;
	if(iobfgetni(fp, 1, &ngens, 0) <= 0 || ngens <= 0) {
	  OOGLSyntax(fp,errfmt,p->poolname, "Invalid generator count");
	  return(NULL);
	}
	discgrp->gens = OOGLNewE(DiscGrpElList, "DiscGrp gens");
	discgrp->gens->num_el = ngens;
	discgrp->gens->el_list = OOGLNewNE(DiscGrpEl,
					   discgrp->gens->num_el, "DiscGrp gens elem list");
      }
      break;

    case DG_NELS:
      {
	int nels;
	if(iobfgetni(fp, 1, &nels, 0) <= 0 || nels <= 0) {
	  OOGLSyntax(fp,errfmt,p->poolname, "Invalid generator count");
	  return(NULL);
	}
	discgrp->big_list = OOGLNewE(DiscGrpElList, "DiscGrp el_list");
	discgrp->big_list->num_el = nels;
	discgrp->big_list->el_list = OOGLNewNE(DiscGrpEl,
					       discgrp->big_list->num_el, "DiscGrp elem list");
      }
      break;
    failed:
      OOGLSyntax(fp, "Couldn't read DISCGRP in \"%s\": expected %s",
		 PoolName(p), expect);
      break;

    default:
      OOGLError(1,"Bad keyword DiscGrpFLoad %s",keyword);
      break;
    }
    if ( !(get_matching_parenthesis(fp,p->poolname))) break;
  }

  /* make sure the generator list includes all inverses */
  DiscGrpAddInverses(discgrp);

  /*
    if (discgrp->geom == NULL)
    {
    discgrp->flag |= DG_DRAWDIRDOM;
    DiscGrpSetupDirdom(discgrp);
    discgrp->ddgeom = DiscGrpDirDom(discgrp);
    }
  */

  if (discgrp->big_list == NULL)
    {
      DiscGrpInitStandardConstraint(discgrp->enumdepth, discgrp->enumdist, discgrp->enumdist);
      discgrp->big_list = DiscGrpEnum(discgrp, DiscGrpStandardConstraint);
    }

  return(( Geom *) discgrp);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
