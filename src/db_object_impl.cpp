/*  Copyright (C) 2009 Maranatha Luckanachai <maranatha.myrrh@gmail.com>

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

#include "db_object_impl.h"

virtual QString getProperty(QString name) const
{
    return m_props[name];
}

virtual void setProperty(QString name, QString value)
{
    m_props[name] = value;
}

virtual QByteArray data() const
{
    return m_data;
}

virtual void setData(QByteArray data)
{
    m_data = data;
}

DatabaseObjectImpl(const DatabaseObjectImpl &other)
{
    *this = other;
}

DatabaseObjectImpl &operator=(const DatabaseObjectImpl &rhs)
{
    m_props = other.mprops;
    m_data = other.data;
}

