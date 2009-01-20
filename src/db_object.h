/*  Copyright (C) 2009 Kelvie Wong <kelvie@ieee.org>
                       Maranatha Luckanachai <maranatha.myrrh@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _BASKET_DATABASE_OBJECT_H_
#define _BASKET_DATABASE_OBJECT_H_

#include "basket_export.h"
#include <QString>
#include <QByteArray> 

class BASKET_EXPORT DatabaseObject
{
	public:

	// These get and set various properties (meta-data) for the database object
	// Database objects all have a "type" property, and this property is either
	// "basket", "note", or "list", that define the semantic meaning of what
	// data() returns.
	virtual QString getProperty(QString name) const = 0;
    virtual void setProperty(QString name, QString value) = 0;

	// This will generally read the data as this function is called, and may
	// block.
	virtual QByteArray data() const = 0;

	virtual void setData(QByteArray data) = 0;
	
	// The copy constructors; we will primarily be using copies of the
	// DatabaseObject, rather than dynamically allocated pointers, so we have to
	// keep the implementation memory friendly.
	virtual DatabaseObject(const DatabaseObject &other) = 0;
	virtual DatabaseObject &operator= (const DatabaseObject &rhs) const = 0;
};

#endif // _BASKET_DATABASE_OBJECT_H_
