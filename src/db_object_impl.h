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

#ifndef _BASKET_DATABASE_OBJECT_IMPL_H_
#define _BASKET_DATABASE_OBJECT_IMPL_H_

#include "basket_export.h"
#include "db_object.h"
#include <QByteArray>
#include <QHash>
#include <QString>


 
// This implements the DatabaseObject
class DatabaseObjectImpl : public DatabaseObject
{
	public:
	virtual QString getProperty(QString name) const;

    virtual void setProperty(QString name, QString value);

	virtual QByteArray data() const;

	virtual void setData(QByteArray data);

	DatabaseObjectImpl(const DatabaseObjectImpl &other);
	DatabaseObjectImpl &operator=(const DatabaseObjectImpl &rhs);

	private:
	QHash<QString, QString> m_props;
	QByteArray m_data;
};

#endif // _BASKET_DATABASE_OBJECT_IMPL_H_
