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
#ifndef  _QV_FIELD_
#define  _QV_FIELD_

#include <stdlib.h>
#include <QvString.h>

class QvInput;
class QvNode;

class QvField {
  public:
    virtual ~QvField();

    QvBool		isDefault() const	{ return flags.hasDefault; }

    QvNode *		getContainer() const	{ return container; }

    void		setDefault(QvBool def)	{ flags.hasDefault = def; }
    void		setContainer(QvNode *cont);
    QvBool		read(QvInput *in, const QvName &name);

    QvField()		{ flags.hasDefault = TRUE; }

  public:

  private:
    struct {
	unsigned int hasDefault		: 1; // Field is set to default value
    }			flags;

    QvNode		*container;

    static QvField *	createInstanceFromName(const QvName &className);
    virtual QvBool	readValue(QvInput *in) = 0;

friend class QvFieldData;
};

class QvSField : public QvField {
  public:
    virtual ~QvSField();

  protected:
    QvSField();

  private:
    virtual QvBool	readValue(QvInput *in) = 0;
};

class QvMField : public QvField {

  public:
    int			num;		// Number of values
    int			maxNum;		// Number of values allocated

    // Destructor
    virtual ~QvMField();

  protected:
    QvMField();
    virtual void	makeRoom(int newNum);

  private:
    virtual void	allocValues(int num) = 0;
    virtual QvBool	readValue(QvInput *in);
    virtual QvBool	read1Value(QvInput *in, int index) = 0;
};

#endif /* _QV_FIELD_ */
