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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef LANG_H
#define LANG_H

extern char geomview_version[];

typedef enum {
  NOT_A_KEYWORD = -1,
  NO_KEYWORD=0,
  YES_KEYWORD,
  ON_KEYWORD,
  OFF_KEYWORD,
  ZERO_KEYWORD,
  ONE_KEYWORD,
  NONE_KEYWORD,
  EACH_KEYWORD,
  KEEP_KEYWORD,
  ALL_KEYWORD,
  EUCLIDEAN_KEYWORD,
  HYPERBOLIC_KEYWORD,
  SPHERICAL_KEYWORD,
  VIRTUAL_KEYWORD,
  PROJECTIVE_KEYWORD,
  CONFORMALBALL_KEYWORD,
  TIFF_KEYWORD,
  FRAME_KEYWORD,
  CAMERA_KEYWORD,
  GEOM_KEYWORD,
  TRANSFORM_KEYWORD,
  NTRANSFORM_KEYWORD,
  COMMAND_KEYWORD,
  WINDOW_KEYWORD,
  IMAGE_KEYWORD,
  APPEARANCE_KEYWORD,
  TRANSLATE_KEYWORD,
  E_TRANSLATE_KEYWORD,
  H_TRANSLATE_KEYWORD,
  S_TRANSLATE_KEYWORD,
  TRANSLATE_SCALED_KEYWORD,
  E_TRANSLATE_SCALED_KEYWORD,
  H_TRANSLATE_SCALED_KEYWORD,
  S_TRANSLATE_SCALED_KEYWORD,
  ROTATE_KEYWORD,
  SCALE_KEYWORD,
  ZOOM_KEYWORD,
  HORIZONTAL_KEYWORD,
  VERTICAL_KEYWORD,
  COLORED_KEYWORD,
  TOGGLE_KEYWORD,
  SMOOTH_KEYWORD,
  BBOX_CENTER_KEYWORD,
  ORIGIN_KEYWORD,
  FOCUS_CHANGE_KEYWORD,
  MOUSE_CROSS_KEYWORD,
  MOTION_INERTIA_KEYWORD,
  MOTION_CONSTRAIN_KEYWORD,
  MOTION_OWN_COORDS_KEYWORD
} Keyword;

void lang_init();
bool boolval(char *s, Keyword keyword);
HandleOps *keyword2ops(Keyword keyword);

#endif
