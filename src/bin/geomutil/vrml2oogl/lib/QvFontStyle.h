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
#ifndef  _QV_FONT_STYLE_
#define  _QV_FONT_STYLE_

#include <QvSFBitMask.h>
#include <QvSFEnum.h>
#include <QvSFFloat.h>
#include <QvSubNode.h>

class QvFontStyle : public QvNode {

    QV_NODE_HEADER(QvFontStyle);

  public:

    enum Family {		// Font family:
	SERIF,				// Serif style (such as TimesRoman)
	SANS,				// Sans-serif style (such as Helvetica)
	TYPEWRITER			// Fixed pitch style (such as Courier)
    };

    enum Style {		// Font style modifications:
	NONE	= 0x00,			// No modifications to family
	BOLD	= 0x01,			// Embolden family
	ITALIC  = 0x02 			// Italicize or slant family
    };

    // Fields
    QvSFFloat		size;		// Font size
    QvSFEnum		family;		// Font family
    QvSFBitMask		style;		// Font style modifications to family
};

#endif /* _QV_FONT_STYLE_ */
