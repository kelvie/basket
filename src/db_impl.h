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

#include "db_object.h"
#include "db.h"

/// This implements the BasketDatabase
class BasketDatabaseImpl : public BasketDatabase
{
	public:
	virtual DatabaseObject *getObject(QString hash) const;

	// Returns the hash of the object
	virtual QString addObject(DatabaseObject obj);

	// Deletes the object with the hash `hash'
	virtual void removeObject(QString hash);

	virtual bool hasObject(QString hash) const;

	virtual DatabaseObject getRootObject() const;

	virtual void setRootObject(DatabaseObject obj);

	protected:
	virtual BasketDatabase() {};
};


#endif // _BASKET_DATABASE_H_
