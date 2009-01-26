/*  Copyright (C) 2009 Kelvie Wong <kelvie@ieee.org>
              (C) 2009 Maranatha Luckanachai <maranatha.myrrh@gmail.com>

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

#include <QByteArray>
#include <QHash>
#include <QString>

class DatabaseObjectPrivate;

// This is the basic object that will be passed to and from the database.  Its
// only members consist of raw data (in a QByteArray) and a set of properties
// (with QString keys and values).  The point of this class is so that data
// retrieval/storage may be changed in the future (i.e. buffered reads and what
// not).  Perhaps in the future, we may return a QDatastream rather than a byte
// array.
class BASKET_EXPORT DatabaseObject
{
public:
    DatabaseObject();
    virtual ~DatabaseObject();

    virtual QString getProperty(QString name) const;

    virtual QHash<QString, QString> properties() const;

    virtual void setProperty(QString name, QString value);

    virtual QByteArray data() const;

    virtual void setData(QByteArray data);

    DatabaseObject(const DatabaseObject &other);
    DatabaseObject &operator=(const DatabaseObject &rhs);

private:
    DatabaseObjectPrivate *p;
};

#endif // _BASKET_DATABASE_OBJECT_H_
