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

#ifndef _BASKET_DATABASE_FACTORY_H_
#define _BASKET_DATABASE_FACTORY_H_

#include "basket_export.h"
#include "db.h"

class BASKET_EXPORT BasketDatabaseFactory
{
public:
    // Returns a database of the given type.  Right now, the type does nothing,
    // and is included for future implementations of the BasketDatabase
    static BasketDatabase *getDatabase(QString type=QString());
};

#endif // _BASKET_DATABASE_FACTORY_H_
