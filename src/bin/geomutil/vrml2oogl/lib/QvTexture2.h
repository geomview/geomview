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
#ifndef  _QV_TEXTURE_2_
#define  _QV_TEXTURE_2_

#include <QvSFEnum.h>
#include <QvSFImage.h>
#include <QvSFString.h>
#include <QvSubNode.h>

class QvTexture2 : public QvNode {

    QV_NODE_HEADER(QvTexture2);

  public:
    enum Wrap {			// Texture wrap type
	REPEAT,				// Repeats texture outside 0-1
					//  texture coordinate range
	CLAMP				// Clamps texture coordinates
					//  to lie within 0-1 range
    };

    // Fields.
    QvSFString		filename;	// file to read texture from
    QvSFImage		image;		// The texture
    QvSFEnum		wrapS;
    QvSFEnum		wrapT;

    virtual QvBool	readInstance(QvInput *in);
    QvBool		readImage();
};

#endif /* _QV_TEXTURE_2_ */
