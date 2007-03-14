/* Copyright (C) 2007 Claus-Justus Heine
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

/* Implement Geomview's "apply = decal" for (non-)constant
 * shading. This means the alpha channel of the texture interpolates
 * between the texture and Cs, but Os remains unchanged. Note that the
 * "fill == 1" parameter automatically results in the "correct"
 * (i.e. OpenGL) behaviour when the texture does not carry an
 * alpha-channel.
 *
 * "apply = decal" is constant shaded, so we simply ignore all of the
 * plastic parameters.
 */
surface
GVdecalplastic(float Ka = 1, Kd = .5, Ks = .5, roughness = .1;
	       color specularcolor = 1;
	       string texturename = "";)
{
  Ci = Cs;
  Oi = Os;
  if (texturename != "") {
    float s = float texture (texturename[0], "fill", 1.0, "width", 0.0);
    Ci = (1 - s) * Ci + s * color texture (texturename);
  }
  Ci *= Os;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
