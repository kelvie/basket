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

#ifndef BASKETFACTORY_H
#define BASKETFACTORY_H

class QColor;
class QString;

class BasketScene;

/** Methods to create various baskets (mkdir, init the properties and load it).
  * @author Sébastien Laoût
  */
namespace BasketFactory
{
/** You should use this method to create a new basket: */
void newBasket(const QString &icon,
               const QString &name,
               const QString &backgroundImage,
               const QColor  &backgroundColor,
               const QColor  &textColor,
               const QString &templateName,
               BasketScene *parent);
/** Internal tool methods to process the method above: */
QString newFolderName();
QString unpackTemplate(const QString &templateName);
}

#endif // BASKETFACTORY_H
