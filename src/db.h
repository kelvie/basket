/*  Copyright (C) 2009 Kelvie Wong <kelvie@ieee.org>

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

#ifndef _BASKET_DATABASE_H_
#define _BASKET_DATABASE_H_

#include "basket_export.h"
#include "db.h"
#include "db_object.h"

#include <QString>

/// This is the abstract class for the database that will all
// DatabaseObject obj(database->getObject("asdfads"));
// QByteArray data = obj.data()
// // do something with data

class BASKET_EXPORT BasketDatabase
{
    Q_DISABLE_COPY(BasketDatabase);

public:
    virtual ~BasketDatabase() {};

    virtual DatabaseObject getObject(QString key) const = 0;

    // Returns the key (string) of the object
    virtual QString addObject(DatabaseObject obj) = 0;

    // Deletes the object with the key `key'
    virtual void removeObject(QString key) = 0;

    // Returns true if there is an object with the key 'key' in the database
    virtual bool hasObject(QString key) const = 0;

    // The root object is the object that is a list of all of the top level
    // baskets.
    virtual void setRootObject(DatabaseObject obj) = 0;
    virtual DatabaseObject getRootObject() const = 0;

protected:
    BasketDatabase() {};
};


#endif // _BASKET_DATABASE_H_
