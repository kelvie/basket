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

#include "history.h"
#include "global.h"

#include "bnpview.h"
#include "basketview.h"

HistorySetBasket::HistorySetBasket(BasketView *basket, QUndoCommand *parent)
        :QUndoCommand(parent)
{
    setText(i18n("Set current basket to %1").arg(basket->basketName()));
    m_folderNameOld = Global::bnpView->currentBasket()->folderName();
    m_folderNameNew = basket->folderName();
}

void HistorySetBasket::undo()
{
    BasketView *oldBasket = Global::bnpView->basketForFolderName(m_folderNameOld);
    Global::bnpView->setCurrentBasket(oldBasket);
}

void HistorySetBasket::redo()
{
    BasketView *curBasket = Global::bnpView->basketForFolderName(m_folderNameNew);
    Global::bnpView->setCurrentBasket(curBasket);
}

