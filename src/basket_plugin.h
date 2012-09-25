/***************************************************************************
 *   Copyright (C) 2009 by Robert Marmorstein                              *
 *   robert@narnia.homeunix.com                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef BASKET_PLUGIN_H
#define BASKET_PLUGIN_H

#include <KontactInterface/Plugin>

namespace KParts {
    class ReadOnlyPart;
}

class BasketPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    BasketPlugin(KontactInterface::Core *core, const QVariantList &);
    ~BasketPlugin();

    virtual void readProperties(const KConfigGroup &config);
    virtual void saveProperties(KConfigGroup &config);

private slots:
    void showPart();

protected:
    KParts::ReadOnlyPart *createPart();
};

#endif
