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
#ifndef _QV_PLIST_
#define _QV_PLIST_

#include <QvBasic.h>

class QvPList {
  public:
    QvPList();
    ~QvPList();
    void	append(void * ptr)
	{ if (nPtrs + 1 > ptrsSize) expand(nPtrs + 1);
	  ptrs[nPtrs++] = ptr; }
    int		find(const void *ptr) const;
    void	remove(int which);
    int		getLength() const		{ return (int) nPtrs;	}
    void	truncate(int start)
	{ nPtrs = start; }
    void *&	operator [](int i) const	{ return ptrs[i]; }

  private:
    void **	ptrs;
    int		nPtrs;
    int		ptrsSize;		
    void	setSize(int size)
	{ if (size > ptrsSize) expand(size); nPtrs = size; }
    void	expand(int size);
};

#endif /* _QV_PLIST_ */
