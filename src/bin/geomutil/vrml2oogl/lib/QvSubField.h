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
#ifndef  _QV_SUB_FIELD_
#define  _QV_SUB_FIELD_

#include <stdlib.h>
#include <QvField.h>
#include <QvInput.h>

/////////////////////////////////////////////////////////////////////////////

#define QV_SFIELD_HEADER(className)					      \
  public:								      \
    className();							      \
    virtual ~className();						      \
    virtual QvBool readValue(QvInput *in)

/////////////////////////////////////////////////////////////////////////////

#define QV_MFIELD_HEADER(className)					      \
  public:								      \
    className();							      \
    virtual ~className();						      \
    virtual QvBool	read1Value(QvInput *in, int index);		      \
    void		allocValues(int newNum);			      \
  protected:								      \
    static QvBool	canUseMalloc

/////////////////////////////////////////////////////////////////////////////

#define QV_SFIELD_SOURCE(className)					      \
									      \
className::className()							      \
{									      \
}									      \
className::~className()							      \
{									      \
}

/////////////////////////////////////////////////////////////////////////////

// Note: field types whose values require constructors to be called
// should pass TRUE for valueHasConstructor. Otherwise, pass FALSE.

#ifdef __SUNPRO_CC
#define QV_MFIELD_SOURCE(className, valueType, numValues, valueHasConstructor)\
									      \
QvBool className::canUseMalloc;						      \
									      \
className::className()							      \
{									      \
    canUseMalloc = ! valueHasConstructor;				      \
    values = NULL;							      \
    /* Make room for 1 value to start */				      \
    allocValues(1);							      \
}									      \
									      \
className::~className()							      \
{									      \
    if (values != NULL) {						      \
	if (canUseMalloc)						      \
	    free((char *) values);					      \
	else								      \
	    delete [] values;						      \
    }									      \
}									      \
									      \
void									      \
className::allocValues(int newNum)					      \
{									      \
    if (values == NULL) {						      \
	if (newNum > 0) {						      \
	    if (canUseMalloc)						      \
		values = (valueType *)					      \
		    malloc(numValues * sizeof(valueType) * newNum);	      \
	    else							      \
		values = new valueType[numValues * newNum];		      \
	}								      \
    }									      \
    else {								      \
	if (newNum > 0) {						      \
	    if (canUseMalloc)						      \
		values = (valueType *)					      \
		    realloc((malloc_t)values, numValues * sizeof(valueType) * newNum);  \
	    else {							      \
		valueType *oldValues = values;				      \
		values = new valueType[numValues * newNum];		      \
		for (int i = 0; i < num && i < newNum; i++)		      \
		    values[i] = oldValues[i];				      \
		delete [] oldValues;					      \
	    }								      \
	}								      \
	else {								      \
	    if (canUseMalloc)						      \
		free((char *) values);					      \
	    else							      \
		delete [] values;					      \
	    values = NULL;						      \
	}								      \
    }									      \
    num = maxNum = newNum;						      \
}
#else

#define QV_MFIELD_SOURCE(className, valueType, numValues, valueHasConstructor)\
									      \
QvBool className::canUseMalloc;						      \
									      \
className::className()							      \
{									      \
    canUseMalloc = ! valueHasConstructor;				      \
    values = NULL;							      \
    /* Make room for 1 value to start */				      \
    allocValues(1);							      \
}									      \
									      \
className::~className()							      \
{									      \
    if (values != NULL) {						      \
	if (canUseMalloc)						      \
	    free((char *) values);					      \
	else								      \
	    delete [] values;						      \
    }									      \
}									      \
									      \
void									      \
className::allocValues(int newNum)					      \
{									      \
    if (values == NULL) {						      \
	if (newNum > 0) {						      \
	    if (canUseMalloc)						      \
		values = (valueType *)					      \
		    malloc(numValues * sizeof(valueType) * newNum);	      \
	    else							      \
		values = new valueType[numValues * newNum];		      \
	}								      \
    }									      \
    else {								      \
	if (newNum > 0) {						      \
	    if (canUseMalloc)						      \
		values = (valueType *)					      \
		    realloc(values, numValues * sizeof(valueType) * newNum);  \
	    else {							      \
		valueType *oldValues = values;				      \
		values = new valueType[numValues * newNum];		      \
		for (int i = 0; i < num && i < newNum; i++)		      \
		    values[i] = oldValues[i];				      \
		delete [] oldValues;					      \
	    }								      \
	}								      \
	else {								      \
	    if (canUseMalloc)						      \
		free((char *) values);					      \
	    else							      \
		delete [] values;					      \
	    values = NULL;						      \
	}								      \
    }									      \
    num = maxNum = newNum;						      \
}

#endif

#endif /* _QV_SUB_FIELD_ */
