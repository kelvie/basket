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

#ifndef TRANSPARENTWIDGET_H
#define TRANSPARENTWIDGET_H

#include <QWidget>

class BasketScene;
class QPaintEvent;
class QMouseEvent;
class QObject;

class TransparentWidget : public QWidget
{
    Q_OBJECT
public:
    TransparentWidget(BasketScene *basket);
    void setPosition(int x, int y);
    //void reparent(QWidget *parent, Qt::WFlags f, const QPoint &p, bool showIt = FALSE);
protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
private:
    BasketScene *m_basket;
    int     m_x;
    int     m_y;
};

#endif // TRANSPARENTWIDGET_H
