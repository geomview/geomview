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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef CR_DEF
#define CR_DEF

#include <stdarg.h>	/* So we only have to do it once, here. */

/* This stuff should be an enum, not a define */

	/* Generic attributes */
#define	CR_END		0	/* Marks end of create list (any object) */
#define CR_COPY		1	/* no argument  (any object)		*/
#define CR_NOCOPY	2	/* no argument  (any object)		*/

	/* Generic Geom attributes */
#define CR_APPEAR	8	/* marks beginning of Appearance create list) */

#define CR_POINT	9	/* Point3 []	(any Geom with vertices) */
#define CR_POINT4	18	/* HPoint3 []	(any Geom with 4D vertices) */
#define CR_NORMAL	10	/* Point3 []	(any Geom w/per-vtx normals) */
#define CR_COLOR	11	/* ColorA []	(any Geom w/per-vtx colors) */
#define CR_OPACITY	12	/* XXX Not used yet XXX		*/

#define CR_FLAG		13	/* int flags	(patch,Mesh,PolyList,Quad) */
#define	CR_FLAGMERGE	14	/* int flags, int flagmask		*/
	/* Control of 'override' mask for mergeable things (Appearance)	*/
#define	CR_OVERMERGE	15	/* int override, int ovmask		*/

	/* BBox attributes */
#define CR_MAX		16	/* Point3 *	(BBox)			*/
#define CR_MIN		17	/* Point3 *	(BBox)			*/
#define CR_4MAX		1492	/* HPoint3 *	(BBox)			*/
#define CR_4MIN		1066	/* HPoint3 *	(BBox)			*/
#define CR_NMIN		2048    /* HPointN *	(BBox)			*/
#define CR_NMAX		2049    /* HPointN *	(BBox)			*/
#define CR_NCENTER      2050	/* HPointN *    (return NDcenter of BBox) */

#define CR_4D		19	/* is this object a true 4D item?	*/

	/* Hierarchy object (Inst, List, TList) attributes */
#define CR_AXIS		20	/* Transform	(Inst)			*/
#define	CR_AXISHANDLE	28	/* Handle * to TransObj (for Inst)	*/
#define CR_NDAXIS	202	/* Transform	(Inst)			*/
#define	CR_NDAXISHANDLE	203	/* Handle * to NTransObj (for Inst)	*/
#define CR_GEOM		21	/* Geom *child	(Inst,List)		*/
#define	CR_GEOMHANDLE	22	/* Handle *childname (Inst,List)	*/
#define CR_TLIST	23	/* Geom *tlist	   (Inst,TList)		*/
#define	CR_TLISTHANDLE	24	/* Handle *tlistname (Inst,TList) 	*/
#define CR_TXTLIST	204	/* Geom *txtlist	   (Inst)	*/
#define	CR_TXTLISTHANDLE 205	/* Handle *txtlistname (Inst) 		*/
#define CR_ELEM		25	/* Transform []	(TList)			*/
#define CR_NELEM	26	/* int nxforms(TList), int nquads (Quad)*/
#define CR_CDR		27	/* List *	(List) (obsolescent?)	*/
#define CR_HANDLE_GEOM  29	/* Handle *, Geom *	(Inst, List)	*/
#define	CR_CAR		CR_GEOM	/* Geom *	(List) (obsolescent)	*/
#define	CR_UNIT		CR_GEOM /* Geom *	(Inst) (obsolescent)	*/
#define	CR_LOCATION	201	/* int		(Inst) L_LOCAL, L_CAMERA, ...*/

	/* Mesh attributes */
#define CR_NU		30	/* int nu	(Mesh)			*/
#define CR_NV		31	/* int nv	(Mesh)			*/
#define	CR_UWRAP	32	/* int uwrapped	(Mesh)			*/
#define	CR_VWRAP	33	/* int vwrapped	(Mesh)			*/
#define CR_U		34	/* Point3 texturecoords[] (Mesh)	*/

#define CR_UMIN		35	/* int umin	(Mesh) [submesh]	*/
#define CR_UMAX		36	/* int umax	(Mesh)			*/
#define CR_VMIN		37	/* int vmin	(Mesh) 			*/
#define CR_VMAX		38	/* int vmax	(Mesh)			*/

	/* Patch attributes */
#define CR_DEGU		40	/* int u_degree	(patches)		*/
#define CR_DEGV		41	/* int v_degree	(patches)		*/
#define CR_MESH		42	/* Mesh *	(patches)		*/
#define CR_DIM		43	/* int vertex_dim (= 3 or 4) (patches)	*/
#define	CR_ST		44	/* struct { float s, t; } stcoords[4];	*/
				/* (texture coords at corners (Mesh,Patch) */
#define	CR_MESHDIM	45	/* mesh surface dimension (typ. 2)	*/
#define	CR_MESHSIZE	46	/* int meshsize[meshdim] 	 	*/

	/* Vect attributes */
#define CR_NVECT	48	/* int  nvectors (Vect)			*/
#define CR_NVERT	49	/* int nvertices (Vect)			*/
#define CR_NCOLR	50	/* int ncolors  (Vect)			*/
#define CR_VECTC	51	/* short nverts_per_vector[nvect] (Vect)*/
#define CR_COLRC	52	/* short ncolors_per_vector[nvect] (Vect)*/

	/* PolyList attributes */
#define CR_NPOLY        56	/* int npolygons	(PolyList)	*/
/*      CR_NVERT	(48)	   int nvertperpol[npoly]		*/
#define CR_VERT         57	/* int vertindex[sum(nvertperpol[])]	*/
#define CR_POLYNORMAL   58	/* Point3 facenormal[npoly]		*/
#define CR_POLYCOLOR    59	/* ColorA facecolor[npoly]		*/
#define CR_POLYFLAGS   300      /* per poly concave/not-flat flags      */

	/* Sphere attributes */
#define CR_CENTER		60	/* HPoint3 *center (also bbox)  */
#define CR_RADIUS		61	/* float  radius		*/
#define CR_SPACE		64	/* Space - TM_EUCLIDEAN is default */
#define CR_NENCOMPASS_POINTS	65	/* int - Number of points to be
					 * put inside sphere */
#define CR_ENCOMPASS_POINTS	66	/* Hpoint3 * - points to be inside
					 * sphere (for bounding spheres */
/*      CR_AXIS			20	Transform to apply to points before
					making bounding sphere */
#define CR_SPHERETX             67      /* texture mapping for sphere */

/* tokens for Camera moved to camera.h */

/* tokens for Appearances, Lights, Materials moved to shade/appearance.h */

#define	CR_POLICE	62	/* enforce strong checking on input data? */
#define	CR_NOPOLICE	63	/* enforce strong checking on input data? */



#endif
