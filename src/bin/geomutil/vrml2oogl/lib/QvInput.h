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
#ifndef  _QV_INPUT_
#define  _QV_INPUT_

#include <QvDict.h>
#include <QvString.h>

class QvNode;
class QvDB;

class QvInput {
 public:

    QvInput();
    ~QvInput();

    static float	isASCIIHeader(const char *string);
    void		setFilePointer(FILE *newFP);
    FILE *		getCurFile() const { return fp; }
    float		getVersion();
    QvBool		get(char &c);
    QvBool		read(char	    &c);
    QvBool		read(QvString       &s);
    QvBool		read(QvName	    &n, QvBool validIdent = FALSE);
    QvBool		read(int	    &i);
    QvBool		read(unsigned int   &i);
    QvBool		read(short	    &s);
    QvBool		read(unsigned short &s);
    QvBool		read(long	    &l);
    QvBool		read(unsigned long  &l);
    QvBool		read(float	    &f);
    QvBool		read(double	    &d);
    QvBool		eof() const;
    void		getLocationString(QvString &string) const;
    void		putBack(char c);
    void		putBack(const char *string);
    void		addReference(const QvName &name, QvNode *node);
    QvNode *		findReference(const QvName &name) const;

  private:
    FILE		*fp;		// File pointer
    int			lineNum;	// Number of line currently reading
    float		version;	// Version number of file
    QvBool		readHeader;	// TRUE if header was checked for A/B
    QvBool		headerOk;	// TRUE if header was read ok
    QvDict		refDict;	// Node reference dictionary
    QvString		backBuf;
    int			backBufIndex;		

    QvBool		checkHeader();

    QvBool		skipWhiteSpace();

    QvBool		readInteger(long &l);
    QvBool		readUnsignedInteger(unsigned long &l);
    QvBool		readReal(double &d);
    QvBool		readUnsignedIntegerString(char *str);
    int			readDigits(char *string);
    int			readHexDigits(char *string);
    int			readChar(char *string, char charToRead);

friend class QvNode;
friend class QvDB;
};

#endif /* _QV_INPUT_ */
