/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
 *   slaout@linux62.org                                                    *
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

#ifndef DECORATEDBASKET_H
#define DECORATEDBASKET_H

class QString;
class Basket;
class QVBoxLayout;

#include <QWidget>
#include "filter.h"

/** This class handle Basket and add a FilterWidget on top of it.
  * @author Sébastien Laoût
  */
class DecoratedBasket : public QWidget
{
    Q_OBJECT
public:
    DecoratedBasket(QWidget *parent, const QString &folderName, Qt::WFlags fl = 0);
    ~DecoratedBasket();
    void setFilterBarPosition(bool onTop);
    void resetFilter();
    void setFilterBarVisible(bool show, bool switchFocus = true);
    bool isFilterBarVisible()        {
        return m_filter->isVisible();
    }
    const FilterData& filterData() {
        return m_filter->filterData();
    }
    FilterBar* filterBar()         {
        return m_filter;
    }
    Basket*    basket()            {
        return m_basket;
    }
private:
    QVBoxLayout *m_layout;
    FilterBar   *m_filter;
    Basket      *m_basket;
};
#endif // DECORATEDBASKET_H
