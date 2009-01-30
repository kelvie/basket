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

#ifndef _BASKET_DATABASE_IMPL_H_
#define _BASKET_DATABASE_IMPL_H_

#include "db.h"

#include <QDir>

/// This implements the BasketDatabase
/// The storage mechanism for the database is as follows:
///
/// Three directories are created within the user's data directory: 
/// data, obj, and rc
/// `data' is the actual raw data of each object
/// `obj' holds the hash of the data, as well as any other metadata each object
///     has
/// `rc' contains the reference counts for each object, in case there are
///    duplicate objects that are being pointed by by different places.  Perhaps
///    better implementations would be able to avoid this requirement, as it
///    does seem like an unnecessary disk access point, but nonetheless it is
///    included.
/// Within each of these directories will just be a bunch of files named after
/// the SHA1 hash of its object file (e.g. the file in `obj').  Any particular
/// hash is linked with its file in every other directory, (e.g. the object with
/// the hash 928be... will have its data in data/928be... and its reference
/// count in data/928be...
class BasketDatabaseImpl : public BasketDatabase
{
public:
    virtual DatabaseObject getObject(QString key) const;

    // Returns the hash of the object
    virtual QString addObject(DatabaseObject obj);

    // Deletes the object with the key 'key'
    virtual void removeObject(QString key);

    virtual bool hasObject(QString key) const;

    virtual DatabaseObject getRootObject() const;

    virtual void setRootObject(DatabaseObject obj);

    virtual ~BasketDatabaseImpl();

protected:
    BasketDatabaseImpl();
    friend class BasketDatabaseFactory;

private: // functions
    // This changes the reference count of the item with the key `key' by the
    // difference `d' (generally +/- 1)
    void changeRC(QString key, int d);

private: // members
    QHash<QString, QString> m_index;
    QDir m_dataDir;
    QDir m_objDir;
    QDir m_rcDir;
    QString m_root;
};


#endif // _BASKET_DATABASE_IMPL_H_
