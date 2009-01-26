/*  Copyright (C) 2009 Maranatha Luckanachai <maranatha.myrrh@gmail.com>
                  2009 Kelvie Wong <kelvie@ieee.org>

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

#include "db_object.h"
#include "db_object_p.h"

DatabaseObject::DatabaseObject()
    : p(new DatabaseObjectPrivate)
{

}

DatabaseObject::~DatabaseObject()
{
    delete p;
}

QString DatabaseObject::getProperty(QString name) const
{
    return p->props[name];
}

QHash<QString, QString> DatabaseObject::properties() const
{
    return p->props;
}

void DatabaseObject::setProperty(QString name, QString value)
{
    p->props[name] = value;
}

QByteArray DatabaseObject::data() const
{
    return p->data;
}

void DatabaseObject::setData(QByteArray data)
{
    p->data = data;
}

DatabaseObject::DatabaseObject(const DatabaseObject &other)
    : p(new DatabaseObjectPrivate(*other.p))
{
    // pass
}

DatabaseObject &DatabaseObject::operator=(const DatabaseObject &rhs)
{
    delete p;
    p = new DatabaseObjectPrivate(*rhs.p);
    return *this;
}

