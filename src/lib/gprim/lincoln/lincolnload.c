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

/*
 * Load a polylist in the Lincoln file format.
 */

#include	"polylistP.h"
#include	"lincoln.h"

#define CONV(ptr,list)	if (ptr) (ptr) = list + ((long)(ptr) - 1);


PolyList *
LincolnFLoad(IOBFILE *inf)
{	
	struct data	header;
	struct vertex	*vertex_list;
	struct edge	*edge_list;
	struct face	*face_list;
	struct vertex *vp;
	struct edge *ep;
	struct face *fp;
	int i,j;

	Vertex	*v_list;
	PolyList *new;

	if(iobfnextc(inf,0) != '\0')	/* Demand null title string */
	    return NULL;
	if(iobfread(&header,sizeof(struct data),1,inf) <= 0)
		return NULL;

	vertex_list = OOGLNewNE(struct vertex, header.nvertices, "Lincoln vertices");
	edge_list = OOGLNewNE(struct edge, header.nedges, "Lincoln edges");
	face_list = OOGLNewNE(struct face, header.nfaces, "Lincoln faces");

	if(iobfread(vertex_list,sizeof(struct vertex),header.nvertices,inf) <= 0
	  || iobfread(edge_list,sizeof(struct edge),header.nedges,inf) <= 0
	  || iobfread(face_list,sizeof(struct face),header.nfaces,inf) <= 0)
		return NULL;

	for (vp = vertex_list; vp<vertex_list+header.nvertices; vp++) {
		CONV(vp->e,edge_list);
	}
	for (ep = edge_list; ep<edge_list+header.nedges; ep++) {
		CONV(ep->f0,face_list);
		CONV(ep->f1,face_list);
		CONV(ep->v0,vertex_list);
		CONV(ep->v1,vertex_list);
		CONV(ep->e00,edge_list);
		CONV(ep->e01,edge_list);
		CONV(ep->e10,edge_list);
		CONV(ep->e11,edge_list);
	}
	for (fp= face_list; fp < face_list+header.nfaces; fp++) {
		CONV(fp->e,edge_list);
	}

	new = OOGLNewE(PolyList, "Lincoln");
	/* should do a PolyListCreate here but time's short...*/
	GGeomInit(new, PolyListMethods(), PLMAGIC, NULL);
	v_list = new->vl = OOGLNewNE(Vertex, header.nvertices, "Lincoln verts");
	new->p = OOGLNewNE(Poly, header.nfaces, "Lincoln faces");

	memset(v_list, 0, header.nvertices * sizeof(Vertex));

	new->geomflags = PL_HASPCOL; /* We have a color-per-face, no normals */
	new->n_polys = header.nfaces;
	new->n_verts = header.nvertices;
	v_list = new->vl;

	for (i = 0; i < header.nfaces; i++) {
		Poly *p;

		fp = face_list + i;
		ep = fp->e;
		if (ep->f1 == fp)
			vp = ep->v0;
		else
			vp = ep->v1;
		p = &new->p[i];
		p->n_vertices = fp->order;
		p->v = OOGLNewNE(Vertex *, fp->order, "Lincoln face");
		p->pcol.r = ((fp->chars.color>>16) &0xFF) / 255.0;
		p->pcol.g = ((fp->chars.color>>8)  &0xFF) / 255.0;
		p->pcol.b = ((fp->chars.color)     &0xFF) / 255.0;
		p->pcol.a = (fp->chars.color&0xFF000000) 
				? (((fp->chars.color>>24) & 0xFF) / 255.0)
				: 1.0;

		for (j = 0; j < fp->order; j++) {
			Vertex *v = &v_list[vp - vertex_list];

			p->v[j]= v;
			v->pt.x = vp->pos.x;
			v->pt.y = vp->pos.y;
			v->pt.z = vp->pos.z;
			if (ep->v0 == vp) {
				vp = ep->v1;
				ep = ((ep->f0==fp) ? (ep->e01) : (ep->e11)) ;
			}
			else {
				vp = ep->v0;
				ep = ((ep->f0==fp) ? (ep->e00) : (ep->e10)) ;
			}
		}
	}

	OOGLFree((char*)vertex_list);
	OOGLFree((char*)edge_list);
	OOGLFree((char*)face_list);

	return new;
}
