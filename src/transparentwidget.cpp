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
+ ***************************************************************************/

#include "basketscene.h"
#include "transparentwidget.h"

#include <QtGui/QGraphicsView>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>

/** Class TransparentWidget */

//TODO: Why was Qt::WNoAutoErase used here?
TransparentWidget::TransparentWidget(BasketScene *basket)
        : QWidget(basket->graphicsView()->viewport()), m_basket(basket)
{
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true); // To receive mouseMoveEvents

    basket->graphicsView()->viewport()->installEventFilter(this);
}

/*void TransparentWidget::reparent(QWidget *parent, Qt::WFlags f, const QPoint &p, bool showIt)
{
    QWidget::reparent(parent, Qt::WNoAutoErase, p, showIt);
}*/

void TransparentWidget::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
}

void TransparentWidget::paintEvent(QPaintEvent*event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

//  painter.save();
    painter.translate(-m_x, -m_y);
    //m_basket->drawContents(&painter, m_x, m_y, width(), height());

//  painter.restore();
//  painter.setPen(Qt::blue);
//  painter.drawRect(0, 0, width(), height());
}

void TransparentWidget::mouseMoveEvent(QMouseEvent *event)
{
//    QMouseEvent *translated = new QMouseEvent(QEvent::MouseMove, event->pos() + QPoint(m_x, m_y), event->button(), event->buttons(), event->modifiers());
//    m_basket->contentsMouseMoveEvent(translated);
//    delete translated;
}

bool TransparentWidget::eventFilter(QObject */*object*/, QEvent *event)
{
    // If the parent basket viewport has changed, we should change too:
    if (event->type() == QEvent::Paint)
        update();

    return false; // Event not consumed, in every cases (because it's only a notification)!
}
