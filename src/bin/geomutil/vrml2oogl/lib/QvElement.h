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
#ifndef  _QV_ELEMENT_
#define  _QV_ELEMENT_

#include <QvBasic.h>

class QvNode;

//////////////////////////////////////////////////////////////////////////////
//
// The base class element; the data is a pointer to some QvNode. The
// type of the node can be inferred from the use of the element
// instance in a particular stack in the state. In some cases, the
// "type" field is used to distinguish among various possible node
// types within a single stack.
//
//////////////////////////////////////////////////////////////////////////////

class QvElement {

  public:

    enum NodeType {
	// Fallback case
	Unknown,

	// Types of cameras in camera stack
	OrthographicCamera,
	PerspectiveCamera,

	// Types of lights in light stack
	DirectionalLight,
	PointLight,
	SpotLight,

	// Types of transformations in transformation stack
	NoOpTransform,		// For QvTransformSeparator
	MatrixTransform,
	Rotation,
	Scale,
	Transform,
	Translation,

	// This has to be last!!!
	NumNodeTypes
    };

    static const char *nodeTypeNames[NumNodeTypes];	// Names of node types

    int		depth;		// Depth of element in state
    QvElement	*next;		// Next element in stack
    QvNode	*data;		// Pointer to node containing data
    NodeType	type;		// Type of data node

    QvElement();
    virtual ~QvElement();

    // Prints contents for debugging, mostly
    virtual void	print();
};

#endif /* _QV_ELEMENT_ */
