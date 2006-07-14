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

#include "mgP.h"
#include "mgbufP.h"


#define WCLIPPLANE 0.001 /* This is as close as we get to the eyeplane */

#define XLEFTCLIP		0.0
#define XRIGHTCLIP		1.0
#define YTOPCLIP		0.0
#define YBOTTOMCLIP		1.0
#define ZNEARCLIP		(-1.0)
#define ZFARCLIP		1.0

static int		 xyz[6], clipvertnum = 0;
static mgbufprim	 *prim1, *prim2, *primtemp;
static CPoint3		 *vts1, *vts2, *vtstemp;

/*
   Function: Xmgr_dividew()
   Description: Divide out w coordinate (*after* clipping against WCLIPPLANE)
   Author: Daeron Meyer
*/

void Xmgr_dividew(void)
{
  CPoint3 *curr;
  float w;
  int   n;

  for (n = 0; n < prim1->numvts; n++) {
    curr = &(vts1[n]);
    w = curr->w; curr->x /= w; curr->y /= w; curr->z /= w;

    curr->z += _mgbufc->znudgeby;

    if (curr->x < XLEFTCLIP) xyz[0]++;
    if (curr->x >= (float)_mgbufc->xsize - XRIGHTCLIP) xyz[1]++;
    if (curr->y < YTOPCLIP) xyz[2]++;
    if (curr->y >= (float)_mgbufc->ysize - YBOTTOMCLIP) xyz[3]++;
    if (curr->z < ZNEARCLIP) xyz[4]++;
    if (curr->z >= ZFARCLIP) xyz[5]++;

  }
}

/*
   Function: Xmgr_cliptoplane()
   Description: Clip polygon against a coordinate axis plane
   Author: Daeron Meyer
*/
void Xmgr_cliptoplane(int coord, float plane, float sign)
{
  CPoint3 *prev, *curr, *dest;
  float i, i1, i2, *c1, *p1, *d1;
  int n;

#ifdef DEBUGCLIP
  fprintf(stderr, "Clip %d, plane=%f, sign=%f\n", coord, plane, sign);
#endif
  prim2->numvts = 0;
  prev = &(vts1[prim1->numvts - 1]);
  i1 = sign * ((float *) prev) [coord] - plane;
  
  for (curr=vts1, n=prim1->numvts; n > 0; n--, prev=curr, i1=i2, curr++)
  {
    i2 = sign * ((float *) curr) [coord] - plane;
    if ((i1 <= 0.0) ^ (i2 <= 0.0)) {
      i =  i1/(i1 - i2);
      dest = &(vts2[prim2->numvts]);
      dest->x = prev->x + i * (curr->x - prev->x);
      dest->y = prev->y + i * (curr->y - prev->y);
      dest->z = prev->z + i * (curr->z - prev->z);
      dest->w = prev->w + i * (curr->w - prev->w);
      if ((i1 <= 0.0) || (prev->drawnext == 0))
	dest->drawnext = 0;
      else
	dest->drawnext = 1;
      c1 = (float *) &(curr->vcol);
      p1 = (float *) &(prev->vcol); 
      d1 = (float *) &(dest->vcol); 
      d1[0] = p1[0] + i * (c1[0] - p1[0]);
      d1[1] = p1[1] + i * (c1[1] - p1[1]);
      d1[2] = p1[2] + i * (c1[2] - p1[2]);
      d1[3] = p1[3] + i * (c1[3] - p1[3]);
      prim2->numvts++;
    }
    if (i2 <= 0.0)
    {
      vts2[prim2->numvts] = *curr;
      prim2->numvts++;
    }
  }
#ifdef DEBUGCLIP
  for (n = 0; n < prim2->numvts; n++)
  {
    fprintf(stderr, "%f, %f, %f, %f: %d\n",
		vts2[n].x, vts2[n].y, vts2[n].z, vts2[n].w, vts2[n].drawnext);
  }
#endif
}


/*
   Function: Xmgr_primclip()
   Description: Clip a polygon to fit in the viewing volume
   Author: Daeron Meyer
*/
int Xmgr_primclip(mgbufprim *aprim)
{
   static mgbufprim	 clip;
   static int		 mykind, n;
   static vvec		 clipverts;
   static int		 intersectw;

   xyz[0]=0; xyz[1]=0; xyz[2]=0; xyz[3]=0; xyz[4]=0; xyz[5]=0;

   /* give ourselves more space for vertex data if needed */
   if ((aprim->numvts * 2) > clipvertnum) {
     if (!clipvertnum)
       VVINIT(clipverts, CPoint3, clipvertnum);

     clipvertnum = aprim->numvts * 2;
     vvneeds(&clipverts, clipvertnum);
   }

  /* We might need more space for the final polygon since
     clipping can produce a polygon with more vertices than
     it had to begin with. We'll err on the side of safety: */

  if (aprim->numvts > _mgbufc->pvertnum)
  {
    _mgbufc->pvertnum *= 2;
    vvneeds(&(_mgbufc->pverts), _mgbufc->pvertnum);
  }

  mykind = aprim->mykind;
  prim1 = aprim; prim2 = &clip;
  vts1 =  &(VVEC(_mgbufc->pverts, CPoint3)[prim1->index]);
  vts2 =  &(VVEC(clipverts, CPoint3)[0]);

  if ((mykind==PRIM_LINE) || (mykind==PRIM_SLINE))
      vts1[prim1->numvts-1].drawnext = 0;

  prim2->index = 0;
  prim2->numvts = prim1->numvts;

#define CLIP_POLYS_AND_SWAP(a, b, c) { \
Xmgr_cliptoplane(a, b, c); \
if (prim2->numvts == 0) { aprim->numvts = 0; return PRIM_INVIS; } \
vtstemp = vts2; vts2 = vts1; vts1 = vtstemp; \
primtemp = prim2; prim2 = prim1; prim1 = primtemp; \
}

#ifdef DEBUGCLIP
  fprintf(stderr, "BEGIN-CLIP\n");
  fprintf(stderr, "INITIAL POLY:\n");
  for (n=0; n < aprim->numvts; n++)
  {
    fprintf(stderr, "%f, %f, %f, %f: %d\n",
		vts1[n].x, vts1[n].y, vts1[n].z, vts1[n].w, vts1[n].drawnext);
  }
  fprintf(stderr,"===========================\n");
#endif

  intersectw = 0;
  for (n = 0; (n < aprim->numvts) && !intersectw; n++)
  {
    if (vts1[n].w < WCLIPPLANE) intersectw = 1;
  }

  if (intersectw) CLIP_POLYS_AND_SWAP(3, -WCLIPPLANE, -1.0);

  Xmgr_dividew();
  n = prim1->numvts;

  if (!intersectw && (xyz[0]+xyz[1]+xyz[2]+xyz[3]+xyz[4]+xyz[5] == 0))
    return mykind;
  else
    if (xyz[0] == n || xyz[1] == n || xyz[2] == n ||
		xyz[3] == n || xyz[4] == n || xyz[5] == n)
	return PRIM_INVIS;

  if (xyz[0]) CLIP_POLYS_AND_SWAP(0, -XLEFTCLIP, -1.0);
  if (xyz[1]) CLIP_POLYS_AND_SWAP(0, (float)_mgbufc->xsize-XRIGHTCLIP, 1.0);
  if (xyz[2]) CLIP_POLYS_AND_SWAP(1, -YTOPCLIP, -1.0);
  if (xyz[3]) CLIP_POLYS_AND_SWAP(1, (float)_mgbufc->ysize-YBOTTOMCLIP, 1.0);
  if (xyz[4]) CLIP_POLYS_AND_SWAP(2, -ZNEARCLIP, -1.0);
  if (xyz[5]) CLIP_POLYS_AND_SWAP(2, ZFARCLIP, 1.0);


  if (aprim == prim2) /* If the vertex data ended up in the local 
			vertex list, then we need to copy it back to the
			global vertex list */
  {
    prim2->numvts = prim1->numvts;
/*
    for (n=0; n < aprim->numvts; n++)
    {
      vts2[n] = vts1[n];
    }
*/
    memcpy(vts2, vts1, sizeof(CPoint3) * prim1->numvts);

  }

#ifdef DEBUGCLIP
  fprintf(stderr, "FINAL POLY:\n");
  for (n=0; n < aprim->numvts; n++)
  {
    fprintf(stderr, "%f, %f, %f, %f: %d\n",
		vts1[n].x, vts1[n].y, vts1[n].z, vts1[n].w, vts1[n].drawnext);
  }
  fprintf(stderr, "END-CLIP\n");
#endif

  return mykind;
}
