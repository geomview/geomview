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

#include "meshP.h"

static GeomClass *aMeshMethods = NULL;

int MeshPresent = 1;

static char meshName[] = "mesh";

extern Mesh      *MeshCreate( va_list );
extern Mesh      *MeshDelete( Mesh * );
extern Mesh      *MeshCopy( Mesh * );

char *
MeshName()
{
  return meshName;
}

GeomClass *MeshMethods(void)
{
  if (!aMeshMethods) {
    aMeshMethods = GeomClassCreate(meshName);

    aMeshMethods->name = MeshName;
    aMeshMethods->methods = (GeomMethodsFunc *) MeshMethods;
    aMeshMethods->create = (GeomCreateFunc *) MeshCreate;
    aMeshMethods->Delete = (GeomDeleteFunc *) MeshDelete;
    aMeshMethods->copy = (GeomCopyFunc *) MeshCopy;
    aMeshMethods->fload = (GeomFLoadFunc *) MeshFLoad;
    aMeshMethods->fsave = (GeomFSaveFunc *) MeshFSave;
    aMeshMethods->bound = (GeomBoundFunc *) MeshBound;
    aMeshMethods->boundsphere = (GeomBoundSphereFunc *) MeshBoundSphere;
    aMeshMethods->evert = (GeomEvertFunc *) MeshEvert;
    aMeshMethods->draw = (GeomDrawFunc *) MeshDraw;
    aMeshMethods->bsptree = (GeomBSPTreeFunc *) MeshBSPTree;
    aMeshMethods->pick = (GeomPickFunc *) MeshPick;
    aMeshMethods->transform = (GeomTransformFunc *) MeshTransform;
    aMeshMethods->transformto = (GeomTransformToFunc *) MeshTransform;
  }
  return aMeshMethods;
}
