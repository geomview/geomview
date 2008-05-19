/* Copyright (C) 2008 Claus-Justus Heine 
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

#include "mgopenglstipple.h"

#include <GL/gl.h>
#include <string.h>
#include <stdlib.h>

#define NUM_OPACITY_STEPS    32
#define NUM_OPACITY_VARIANTS 128

static GLubyte stippleMasks[NUM_OPACITY_VARIANTS][NUM_OPACITY_STEPS+1][128];
static unsigned char stippleMaskRotation[NUM_OPACITY_STEPS+1];

const GLubyte *mgopengl_get_polygon_stipple(float alpha)
{
  int index;
  int variant;
  
  index = (int)(alpha*(float)NUM_OPACITY_STEPS+0.5);
  variant = stippleMaskRotation[index] =
    (stippleMaskRotation[index] + 1) % NUM_OPACITY_VARIANTS;

  return stippleMasks[variant][index];
}

void mgopengl_init_polygon_stipple(void)
{
  int pat, pix, variant;
  float alpha;

  for (variant = 0; variant < NUM_OPACITY_VARIANTS; variant++) {
    memset(stippleMasks[variant][0], 0, 128);
    memset(stippleMasks[variant][NUM_OPACITY_STEPS], ~0, 128);
    for (pat = 1; pat < NUM_OPACITY_STEPS; pat++) {
      memset(stippleMasks[pat], 0, 128);
      alpha = 1.0 / (float)NUM_OPACITY_STEPS * (float)pat;
      srand(pat*variant);
      for (pix = 0; pix < 32*32; pix++) {
	float prob;
	int byte = pix / 8;
	int bit  = pix % 8;
	
	prob = (float)rand() / (float)RAND_MAX;
	if (prob >= (1.0 - alpha)) {
	  stippleMasks[variant][pat][byte] |= 1 << bit;
	}
      }
    }
  }
}
