/***************************************************************************
 *   Copyright (C) 2010 Brian C. Milco                                     *
 *   bcmilco@gmail.com                                                     *
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

#ifndef HISTORY_H
#define HISTORY_H

#include <QUndoCommand>

class BasketView;

class HistorySetBasket : public QUndoCommand
{
public:
    HistorySetBasket(BasketView *basket, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QString m_folderNameOld;
    QString m_folderNameNew;
};


#endif // HISTORY_H
