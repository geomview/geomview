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
#ifndef  _QV_SF_ENUM_
#define  _QV_SF_ENUM_

#include <QvString.h>
#include <QvSubField.h>

class QvSFEnum : public QvSField {
  public:
    int value;
    QV_SFIELD_HEADER(QvSFEnum);

    // Sets up value/name correspondances
    void setEnums(int num, const int vals[], const QvName names[])
	{ numEnums = num; enumValues = vals; enumNames = names; }

    int			numEnums;	// Number of enumeration values
    const int		*enumValues;	// Enumeration values
    const QvName	*enumNames;	// Mnemonic names of values

    // Looks up enum name, returns value. Returns FALSE if not found.
    QvBool		findEnumValue(const QvName &name, int &val) const;
};

#define	QV_NODE_SET_SF_ENUM_TYPE(fieldName, enumType)			      \
    do {								      \
	int _so_sf_enum_num;						      \
	const int *_so_sf_enum_vals;					      \
	const QvName *_so_sf_enum_names;				      \
	fieldData->getEnumData(QV__QUOTE(enumType),			      \
			       _so_sf_enum_num,				      \
			       _so_sf_enum_vals,			      \
			       _so_sf_enum_names);			      \
	fieldName.setEnums(_so_sf_enum_num,				      \
			   _so_sf_enum_vals,				      \
			   _so_sf_enum_names);				      \
    } while (0)

#endif /* _QV_SF_ENUM_ */
