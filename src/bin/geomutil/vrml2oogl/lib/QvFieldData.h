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
#ifndef  _QV_FIELD_DATA_
#define  _QV_FIELD_DATA_

#include <QvBasic.h>
#include <QvPList.h>
#include <QvString.h>

class QvField;
class QvInput;
class QvNode;

class QvFieldData {
  public:
    QvFieldData() {}
    ~QvFieldData();

    void		addField(QvNode *defObject, const char *fieldName,
				 const QvField *field);

    int			getNumFields() const	{ return fields.getLength(); }

    const QvName &	getFieldName(int index) const;

    QvField *		getField(const QvNode *object,
				 int index) const;

    void		addEnumValue(const char *typeName,
				     const char *valName, int val);
    void		getEnumData(const char *typeName, int &num,
				    const int *&vals, const QvName *&names);

    QvBool		read(QvInput *in, QvNode *object,
			     QvBool errorOnUnknownField = TRUE) const;

    QvBool		read(QvInput *in, QvNode *object,
			     const QvName &fieldName,
			     QvBool &foundName) const;

    QvBool		readFieldTypes(QvInput *in, QvNode *object);

  private:
    QvPList		fields;
    QvPList		enums;
};    

#endif /* _QV_FIELD_DATA_ */
