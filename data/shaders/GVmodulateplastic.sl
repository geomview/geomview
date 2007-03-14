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

/* Implement Geomview's "apply = modulate" for constant shading. */
surface
GVmodulateplastic(float Ka = 1, Kd = .5, Ks = .5, roughness = .1;
		  color specularcolor = 1;
		  string texturename = "";)
{
  normal Nf;
  vector V;

  Ci = Cs;
  Oi = Os;
  if (texturename != "") {
    Ci *= color texture (texturename);
    if (float texture (texturename[3], "fill", 1.0, "width", 0.0) == 0.0) {
      Oi = 0.0;
    } 
  }

  Nf = faceforward (normalize(N),I);
  V = -normalize(I);

  Ci = Ci * (Ka*ambient() + Kd*diffuse(Nf)) +
    specularcolor * Ks*specular(Nf,V,roughness);
  Ci *= Os;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
