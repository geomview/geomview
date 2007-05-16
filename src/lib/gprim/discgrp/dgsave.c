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

#include "discgrpP.h"

extern keytokenpair attr_list[], dspyattr_list[]; 

DiscGrp           *
DiscGrpFSave(DiscGrp *dg, FILE *fp, char *fname)
{

	int i;

	fprintf(fp,"DISCGRP\n");
	if (dg->name) fprintf(fp,"(group \" %s \" )\n",dg->name);
	if (dg->comment) fprintf(fp,"(comment \" %s \" )\n",dg->comment);
	for (i=0; i<DG_NUM_ATTR; ++i)	{
	    if (dg->attributes & attr_list[i].token)
		fprintf(fp,"(attribute %s )\n",attr_list[i].key);
	    }
	for (i=0; i<DG_NUM_DSPYATTR; ++i)	{
	    if (dg->flag & dspyattr_list[i].token)
		fprintf(fp,"(display %s )\n",dspyattr_list[i].key);
	    }
	fprintf(fp,"(dimn %d )\n",dg->dimn);
	fprintf(fp,"(ngens %d )\n",dg->gens->num_el);

	fprintf(fp,"(gens\n");
	for (i=0; i<dg->gens->num_el; ++i)	{
	    fprintf(fp,"%s\n",dg->gens->el_list[i].word);
	    fputtransform(fp, 1, (float *)dg->gens->el_list[i].tform, 0);
	    }
	fprintf(fp,")\n");

    if (dg->flag & DG_SAVEBIGLIST && dg->big_list)	{
	fprintf(fp,"(nels %d )\n",dg->big_list->num_el);
	fprintf(fp,"(els\n");
	for (i=0; i<dg->big_list->num_el; ++i)	{
	    fprintf(fp,"# %s\n",dg->big_list->el_list[i].word);
	    fputtransform(fp, 1, (float *)dg->big_list->el_list[i].tform, 0);
	    }
	fprintf(fp,")\n");
	}

    fprintf(fp,"(cpoint %f %f %f %f )\n",
	dg->cpoint.x, dg->cpoint.y, dg->cpoint.z, dg->cpoint.w);

    if (dg->c2m)	{
	fprintf(fp,"(c2m ");
	fputtransform(fp, 1, (float *) dg->c2m, 0);
	fprintf(fp,")\n");
	}

    fprintf(fp,"(enumdepth %d )\n",dg->enumdepth);
    fprintf(fp,"(enumdist %g )\n",dg->enumdist);
    fprintf(fp,"(drawdist %g )\n",dg->drawdist);
    fprintf(fp,"(scale %g )\n",dg->scale);

    if (dg->geom && dg->geom != dg->ddgeom)	{
	fprintf(fp,"(geom\n");
	GeomFSave(dg->geom, fp, fname);
	fprintf(fp,")\n");
	}

    else if (dg->ddgeom  && dg->flag & DG_SAVEDIRDOM )	{
	fprintf(fp,"(geom\n");
	GeomFSave(dg->ddgeom, fp, fname);
	fprintf(fp,")\n");
	}

    if (dg->camgeom)	{
	fprintf(fp,"(camgeom\n");
	GeomFSave(dg->camgeom, fp, fname);
	fprintf(fp,")\n");
	}

	return(dg);
}


DiscGrp* DiscGrpSave(DiscGrp *dg, char *name)
{
	DiscGrp           *tdg;
	FILE           *outf;

	outf = fopen(name, "w");
	if (!outf)
	{
		OOGLError(1,"Unable to open file %s\n",name);
		return NULL;
	}
	tdg = DiscGrpFSave(dg, outf, name);
	fclose(outf);

	return (tdg);
}
