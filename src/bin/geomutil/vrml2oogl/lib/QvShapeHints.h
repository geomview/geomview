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
#ifndef  _QV_SHAPE_HINTS_
#define  _QV_SHAPE_HINTS_

#include <QvSFEnum.h>
#include <QvSFFloat.h>
#include <QvSubNode.h>

class QvShapeHints : public QvNode {

    QV_NODE_HEADER(QvShapeHints);

  public:
    enum VertexOrdering {
	UNKNOWN_ORDERING,
	CLOCKWISE,
	COUNTERCLOCKWISE
    };

    enum ShapeType {
	UNKNOWN_SHAPE_TYPE,
	SOLID
    };

    enum FaceType {
	UNKNOWN_FACE_TYPE,
	CONVEX
    };

    // Fields
    QvSFEnum		vertexOrdering;	// Ordering of face vertices
    QvSFEnum		shapeType;	// Info about shape geometry
    QvSFEnum		faceType;	// Info about face geometry
    QvSFFloat		creaseAngle;	// Smallest angle for sharp edge
};

#endif /* _QV_SHAPE_HINTS_ */
