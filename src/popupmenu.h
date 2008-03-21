//Added by qt3to4:
#include <Q3PopupMenu>
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

#ifndef POPUPMENU_H
#define POPUPMENU_H

class Q3PopupMenu;
class QRect;

/** QPopupMenu/KPopupMenu doesn't provide metod to exec a menu
  * at a given rectangle !
  * eg, popup at bottom of a rectangle, and at top if not possible...
  * @author Sébastien Laoût
  */
namespace PopupMenu
{
	/** Show the popup menu centered into rect.
	  */
	void execAtRectCenter(Q3PopupMenu &menu, const QRect &rect);

	/** Show the popup menu at left-bottom of rect, or at right-bottom
	  * if not possible (not enought place).
	  * If it isn't possible to show it at bottom, it will be shown on
	  * top of rect (top-left if possible, if not it will be top-right).
	  * If center is true, it will try to horizontaly center the popup with
	  * rect, so it will try two positions : bottom center and then top center.
	  */
	void execAtRectBottom(Q3PopupMenu &menu, const QRect &rect, bool centered = false);

	/** Idem execAtRectBottom but on the right or left sides,
	  * prior aligned with the top of the rect, and at the bottom
	  * if not possible.
	  * If center is true, it will try to vertically center the popup with
	  * rect, so it will try two positions : right center and then left center.
	  */
	void execAtRectRight(Q3PopupMenu &menu, const QRect &rect, bool centered = false);
}

/** Test window of PopupMenu methods.
  * Just include popupmenu.h in a main Qt application and call
  * new PopupMenuTest();
  * Click the window for more explications.
  * Resize it to test particular cases.
  * (Comment the class, if it isn't done yet to do not compile it :-) ).
  * @author Sébastien Laoût
  */

/*****

#include <qwidget.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qpen.h>

c l a s s   P o p u p M e n u T e s t   :   p u b l i c   Q W i d g e t
{
  Q _ O B J E C T
  p u b l i c:
	PopupMenuTest()
	 : QWidget(0)
	{
		setCaption("Click to test!");
		show();
	}

	void mousePressEvent(QMouseEvent *event)
	{
		QPopupMenu menu;
		QRect rect( mapToGlobal(QPoint(0,0)), size() );

		menu.insertItem("A test of popup menu!");
		menu.insertItem("This menu contain some items");
		menu.insertItem("Resize the window as you want and:");
		menu.insertItem("- click : execAtRectCenter");
		menu.insertItem("- right click : execAtRectBottom");
		menu.insertItem("- middle click : execAtRectRight");
		menu.insertItem("- Shift + right click : execAtRectBottom centered");
		menu.insertItem("- Shift + middle click : execAtRectRight centered");

		if (event->button() & Qt::LeftButton)
			PopupMenu::execAtRectCenter(menu, rect);
		else if ((event->button() & Qt::RightButton) && (event->state() & Qt::ShiftButton))
			PopupMenu::execAtRectBottom(menu, rect, true);
		else if (event->button() & Qt::RightButton)
			PopupMenu::execAtRectBottom(menu, rect);
		else if ((event->button() & Qt::MidButton) && (event->state() & Qt::ShiftButton))
			PopupMenu::execAtRectRight(menu, rect, true);
		else if (event->button() & Qt::MidButton)
			PopupMenu::execAtRectRight(menu, rect);
	}

	void paintEvent(QPaintEvent*)
	{
		QPainter paint(this);
		paint.setPen(paletteBackgroundColor());
		paint.drawRect(rect());
		paint.drawWinFocusRect(rect());
		paint.setPen( QPen(Qt::black, 1) );
		paint.drawLine( rect().topLeft(), rect().bottomRight() );
		paint.drawLine( rect().topRight(), rect().bottomLeft() );
	}
};

*****/

#endif // POPUPMENU_H
