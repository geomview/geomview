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

/* Implement Geomview's "apply = blend" for non-constant shading
 *
 * See also glTexEnvf(3).
 *
 * Note that we ignore the alpha value of the background color. This
 * is just what OpenGL does, as Geomview never uses GL_INTENSITY as
 * texture format.
 *
 * Note also that the background color is unaffected by lighting. This
 * means: we apply first the normal plastic shader, and then
 * interpolate between the shaded colour and the background color.
 *
 * If the texture has an alpha channel, then Oi = Os * Ot.
 *
 * The additional parameter At interpolates 1 and Ot; the effective
 * alpha contribution from the texture will be (1-At) + At * Ot.
 */
surface
GVblendplastic(float Ka = 1, Kd = .5, Ks = .5, roughness = .1;
	       color specularcolor = 1;
	       string texturename = ""; color bgcolor = 0; float At = 1;)
{
  /* variables used for lighting */
  normal Nf;
  vector V;
  /* texture provided color (luminance) and alpha */
  color Ct;
  float Ot;
  /* number of texture channels,
   * < 3: liminance and possibly alpha, > 2: rgb and possibly alpha
   */
  float channels;

  /* normal plastic shader */
  Ci = Cs;
  Oi = Os;

  Nf = faceforward (normalize(N),I);
  V = -normalize(I);

  Ci = Ci * (Ka*ambient() + Kd*diffuse(Nf)) +
    specularcolor * Ks*specular(Nf,V,roughness);

  /* texture support a la GL_BLEND */
  if (texturename != "" &&
      textureinfo(texturename, "channels", channels) == 1.0) {
    if (channels < 3) {
      Ct = float texture(texturename[0], "width", 0.0);
      Ot = float texture(texturename[1], "fill", 1.0, "width", 0.0);
    } else {
      Ct = color texture(texturename, "width", 0.0);
      Ot = float texture(texturename[3], "fill", 1.0, "width", 0.0);
    }
    Ci  = (1.0 - Ct) * Ci + Ct * bgcolor;
    Ot  = (1.0 - At) + At * Ot;
    Oi *= Ot;
  }

  Ci *= Oi;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
