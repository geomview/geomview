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
#ifndef  _QV_STATE_
#define  _QV_STATE_

#include <QvElement.h>

class QvState {

  public:

    // Stack indices, based on type of elements in them:
    enum StackIndex {
	CameraIndex,
	Coordinate3Index,
	FontStyleIndex,
	LightIndex,
	MaterialBindingIndex,
	MaterialIndex,
	NormalBindingIndex,
	NormalIndex,
	ShapeHintsIndex,
	Texture2Index,
	Texture2TransformationIndex,
	TextureCoordinate2Index,
	TransformationIndex,

	// This has to be last!!!
	NumStacks
    };

    static const char *stackNames[NumStacks];	// Names of stacks

    int		depth;		// Current state depth
    QvElement	**stacks;	// Stacks of elements

    QvState();
    ~QvState();

    // Adds an element instance to the indexed stack
    void	addElement(StackIndex stackIndex, QvElement *elt);

    // Returns top element on a stack
    QvElement *	getTopElement(StackIndex stackIndex)
	{ return stacks[stackIndex]; }

    // Pushes/pops the stacks
    void	push();
    void	pop();

    // Pops top element off one stack
    void	popElement(StackIndex stackIndex);

    // Prints contents for debugging, mostly
    void	print();
};

#endif /* _QV_STATE_ */
