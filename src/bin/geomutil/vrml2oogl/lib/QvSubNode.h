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
#ifndef  _QV_SUB_NODE_
#define  _QV_SUB_NODE_

#include <QvFieldData.h>
#include <QvNode.h>

#define QV_NODE_HEADER(className)					      \
  public:								      \
    className();							      \
    virtual ~className();						      \
    virtual void	traverse(QvState *state);			      \
    virtual void        ToWebOOGL(QvState *state);			      \
  private:								      \
    static QvBool	firstInstance;					      \
    static QvFieldData	*fieldData;					      \
    virtual QvFieldData *getFieldData() { return fieldData; }

#define QV_NODE_SOURCE(className)					      \
    QvFieldData	       *className::fieldData;				      \
    QvBool		className::firstInstance = TRUE;

#define QV_NODE_CONSTRUCTOR(className)					      \
    if (fieldData == NULL)						      \
	fieldData = new QvFieldData;					      \
    else								      \
	firstInstance = FALSE;						      \
    isBuiltIn = FALSE;							      \

#define QV_NODE_IS_FIRST_INSTANCE() (firstInstance == TRUE)

#define QV_NODE_ADD_FIELD(fieldName)					      \
    if (firstInstance)							      \
	fieldData->addField(this, QV__QUOTE(fieldName), &this->fieldName);    \
    this->fieldName.setContainer(this);

#define QV_NODE_DEFINE_ENUM_VALUE(enumType,enumValue)			      \
    if (firstInstance)							      \
	fieldData->addEnumValue(QV__QUOTE(enumType),			      \
				QV__QUOTE(enumValue), enumValue)

#endif /* _QV_SUB_NODE_ */

