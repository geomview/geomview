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
#ifndef  _QV_WWW_ANCHOR_
#define  _QV_WWW_ANCHOR_

#include <QvSFEnum.h>
#include <QvSFString.h>
#include <QvGroup.h>

class QvWWWAnchor : public QvGroup {

    QV_NODE_HEADER(QvWWWAnchor);

  public:

    enum Map {			// Map types:
	NONE,				// Leave URL name alone
	POINT				// Add object coords to URL name
    };

    // Fields
    QvSFString		name;		// URL name
    QvSFString		description;	// Useful description of scene
    QvSFEnum		map;		// How to map pick to URL name
};

#endif /* _QV_WWW_ANCHOR_ */
