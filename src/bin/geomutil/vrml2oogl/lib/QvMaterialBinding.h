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
#ifndef  _QV_MATERIAL_BINDING_
#define  _QV_MATERIAL_BINDING_

#include <QvSFEnum.h>
#include <QvSubNode.h>

class QvMaterialBinding : public QvNode {

    QV_NODE_HEADER(QvMaterialBinding);

  public:
    enum Binding {
	DEFAULT,
	OVERALL,
	PER_PART,
	PER_PART_INDEXED,
	PER_FACE,
	PER_FACE_INDEXED,
	PER_VERTEX,
	PER_VERTEX_INDEXED
    };

    // Fields:
    QvSFEnum		value;			
};

#endif /* _QV_MATERIAL_BINDING_ */
