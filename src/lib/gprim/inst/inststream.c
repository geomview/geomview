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

#include <stdio.h>
#include "instP.h"
#include "streampool.h"
#include "transobj.h"
#include "ntransobj.h"
#include "handleP.h"

static char *locations[] = {
    "none",		/* L_NONE */
    "local",		/* L_LOCAL */
    "global",		/* L_GLOBAL */
    "camera",		/* L_CAMERA */
    "ndc",		/* L_NDC */
    "screen",		/* L_SCREEN */
};

static int getlocation(char *name)
{
    int i;

    if(name == NULL) {
	return -1;
    }
    for(i = COUNT(locations); --i >= 0 && strcasecmp(name, locations[i]) != 0; )
	;
    return i;	/* Return location number, or -1 if not found. */
}

Geom *InstImport(Pool *p)
{
    Inst *inst = NULL;
    IOBFILE *file;
    char *expect = NULL;
    int c;

    if(p == NULL || (file = PoolInputFile(p)) == NULL) {
	return 0;
    }

    if(strcmp(GeomToken(file), "INST")) {
	return 0;
    }

    for(;;) {
	switch((c = iobfnextc(file, 0))) {
	case EOF:
	case CKET:
	    goto done;

	case 'l':
	    if(iobfexpectstr(file, expect = "location")) {
		goto syntax;
	    }
	    if(inst == NULL) {
		inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
	    }
	    inst->location = getlocation( iobfdelimtok("(){}", file, 0) );
	    expect = "location [local|global|camera|ndc|screen]";
	    if(inst->location < 0) {
		goto syntax;
	    }
	    break;

	case 'o':
	    if(iobfexpectstr(file, expect = "origin")) {
		goto syntax;
	    }	    
	    if(inst == NULL) {
		inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
	    }

	    expect = "origin [local|global|camera|ndc|screen] X Y Z";
	    inst->origin = getlocation( iobfdelimtok("(){}", file, 0) );
	    if(inst->origin < 0) {
		goto syntax;
	    }
	    if(iobfgetnf(file, 3, &inst->originpt.x, 0) < 3) {
		goto syntax;
	    }
	    break;
	    
	    
	case 'u':
	    if(iobfexpectstr(file, expect = "unit")) {
		goto syntax;
	    }
	    goto geom;

	case 'g':
	    if(iobfexpectstr(file, expect = "geom"))
		goto syntax;

	geom:
	    if(inst == NULL) {
		inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
	    }
	    expect = "geometry";
	    if(!GeomStreamIn(p, &inst->geomhandle, &inst->geom)) {
		goto failed;
	    }
	    if(inst->geomhandle) {
		HandleRegister(&inst->geomhandle,
			       (Ref *)inst, &inst->geom, HandleUpdRef);
	    }
	    break;

	case 'n': /* ntransform */
	    if(iobfexpectstr(file, (expect = "ntransform"))) {
		goto syntax;
	    }
	    if(inst == NULL) {
		inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
	    }
	    expect = "ntransform matrix";
	    if(!NTransStreamIn(p, &inst->NDaxishandle, &inst->NDaxis)) {
		goto failed;
	    }
	    if(inst->NDaxishandle) {
		HandleRegister(&inst->NDaxishandle,
			       (Ref *)inst, &inst->NDaxis, HandleUpdRef);
	    }
	    break;
	    
	case 't':		/* tlist ... or transform ... */
	    if(inst == NULL) {
		inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
	    }
	    iobfgetc(file);
	    switch((c = iobfgetc(file))) {
	    case 'l':
		if(iobfexpectstr(file, (expect = "tlist")+2)) { /* "tlist" */
		    goto syntax;
		}
	    transforms:
		if(inst == NULL) {
		    inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
		}
		expect = "TLIST object";
		if(!GeomStreamIn(p, &inst->tlisthandle, &inst->tlist)) {
		    goto failed;
		}
		if(inst->tlisthandle) {
		    HandleRegister(&inst->tlisthandle,
				   (Ref *)inst, &inst->tlist, HandleUpdRef);
		}
		break;

	    case 'r':
		if(iobfexpectstr(file, (expect = "transform")+2)) {
		    /* "transform" */
		    goto syntax;
		}
		if(iobfexpectstr(file, "s") == 0) { /* transforms = tlist */
		    goto transforms;
		}
		if(inst == NULL) {
		    inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
		}
		expect = "transform matrix";
		if (!TransStreamIn(p, &inst->axishandle, inst->axis)) {
		    goto failed;
		}
		if (inst->axishandle) {
		    HandleRegister(&inst->axishandle,
				   (Ref *)inst, inst->axis, TransUpdate);
		}
		break;

	    case 'x':
		if (iobfexpectstr(file, (expect = "txtransforms")+2)) {
		    goto syntax;
		}
		if(inst == NULL) {
		    inst = (Inst *)GeomCCreate(NULL, InstMethods(), NULL);
		}
		expect = "TLIST object";
		if(!GeomStreamIn(p, &inst->txtlisthandle, &inst->txtlist)) {
		    goto failed;
		}
		if(inst->txtlisthandle) {
		    HandleRegister(&inst->txtlisthandle,
				   (Ref *)inst, &inst->txtlist, HandleUpdRef);
		}
		break;

	    default:
		expect = "something";
		goto syntax;

	    }
	    break;

	default:
	syntax:
	    OOGLSyntax(file, "Couldn't read INST in \"%s\": "
		       "syntax error, expected %s, got char %c",
		       p->poolname, expect, c);
	    goto bogus;

	failed:
	    OOGLSyntax(file, "Couldn't read INST in \"%s\": expected %s",
		       PoolName(p), expect);

        bogus:
	    GeomDelete((Geom *)inst);
	    return NULL;
	}
    }

 done:

    return (Geom *)inst;
}

int
InstExport(Inst *inst, Pool *pool)
{
    FILE *outf;
    int ok = 1;

    if(inst == NULL || pool == NULL || (outf = PoolOutputFile(pool)) == NULL)
	return 0;

    PoolFPrint(pool, outf, "INST\n");
    if(inst->origin != L_NONE && (unsigned)inst->origin < COUNT(locations)) {
	PoolFPrint(pool, outf, "origin %s ", locations[inst->origin]);
	fputnf(pool->outf, 3, &inst->originpt.x, 0);
	fputc('\n', pool->outf);
    }

    if(inst->location != L_LOCAL && inst->location != L_NONE &&
       (unsigned)inst->location < COUNT(locations))
	PoolFPrint(pool, outf, "location %s\n", locations[inst->location]);

    if(inst->tlist != NULL || inst->tlisthandle != NULL) {
	PoolFPrint(pool, outf, "transforms ");
	ok &= GeomStreamOut(pool, inst->tlisthandle, inst->tlist);
    } else if(inst->tlist != NULL || inst->tlisthandle != NULL) {
	PoolFPrint(pool, outf, "txtransforms ");
	ok &= GeomStreamOut(pool, inst->tlisthandle, inst->tlist);
    } else if (memcmp(inst->axis, TM_IDENTITY, sizeof(Transform)) != 0) {
	PoolFPrint(pool, outf, "");
	ok &= TransStreamOut(pool, inst->axishandle, inst->axis);
    } else if (inst->NDaxis != NULL) {
	PoolFPrint(pool, outf, "");
	ok &= NTransStreamOut(pool, inst->NDaxishandle, inst->NDaxis);
    }

    if(inst->geom != NULL || inst->geomhandle != NULL) {
	PoolFPrint(pool, outf, "geom ");
	ok &= GeomStreamOut(pool, inst->geomhandle, inst->geom);
    }
    return ok;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
