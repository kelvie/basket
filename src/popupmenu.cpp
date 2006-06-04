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

#include <kapplication.h>
#include <qdesktopwidget.h>
#include <qpopupmenu.h>
#include <qrect.h>

#include "popupmenu.h"

#if KDE_IS_VERSION( 3, 2, 90 )   // KDE 3.3.x
  #define MENU_Y_OFFSET 1
#else
  #define MENU_Y_OFFSET 2
#endif

/** NOTE 1 : This implementation forget BIDI support (eg RightToLeft languages
  *          expect to try to popup the menu first at bottom-right
  *          and not at bottom-left.
  * NOTE 2 : This implementation do not support virtual desktop with more than
  *          one screen. Pehrapse QPopupMenu solve it by itself but I can't
  *          try : I have only one screen.
  * => Have those methods directly in Qt (or KDE) would be a great benefits !
  */

void PopupMenu::execAtRectCenter(QPopupMenu &menu, const QRect &rect)
{
	// Compute point where to popup the menu that should be centered :
	QSize menuSize = menu.sizeHint();
	QSize menuHalfSize = menuSize / 2;
	QPoint point = rect.center() - QPoint(menuHalfSize.width(), menuHalfSize.height());

	// Very strange : menu.exec(point) do that clipping (but not when exec() by mouse !!! ) :
	// If menu is partially out of the screen, move it :
/*	int desktopWidth  = kapp->desktop()->width();
	int desktopHeight = kapp->desktop()->height();
	if (point.x() + menuSize.width() > desktopWidth)       point.setX(desktopWidth - menuSize.width());
	if (point.y() + menuSize.height() - 2 > desktopHeight) point.setY(desktopHeight - menuSize.height() + 2);
	if (point.x() < 0)                                     point.setX(0);
	if (point.y() < 0)                                     point.setY(0);*/

	// And show the menu :
	menu.exec( point + QPoint(0, MENU_Y_OFFSET) ); // Stupid offset (Qt bug ?) : we should show the menus 2 pixels more bottom !
}

// Needed on debug to draw the passed global rectangle :
//#include <qpainter.h>
//#include <qpen.h>

void PopupMenu::execAtRectBottom(QPopupMenu &menu, const QRect &rect, bool centered)
{
	QSize menuSize = menu.sizeHint() - QSize(1, 1); // A size is [1..n] => We want two lengths that are [0..(n-1)]
	int desktopWidth  = kapp->desktop()->width();   //  to be compared/added/substracted with QRects/QPoints...
	int desktopHeight = kapp->desktop()->height();

	/** Paint the rect on the screen (desktop).
	  * For test purpose only (to be sure the passed rectangle is right).
	  * Comment this (and the qpainter and qpen includes) for a non-debug version.
	  */
	/*QPainter paint(kapp->desktop(), kapp->desktop(), true);
	paint.setPen( QPen(Qt::black, 1) );
	paint.drawRect(rect);
	paint.end();*/

	// rect.bottomLeft() and rect.bottomRight() must be VISIBLE :
	//  show the menu 1 pixel more BOTTOM (add 1 in Y) :
	QPoint point = rect.bottomLeft() + QPoint(0, 1);
	if (point.y() + menuSize.height() < desktopHeight) { // First try at bottom
		if (centered)
			point = QPoint( rect.center().x() - menuSize.width() / 2, point.y() );
		else if (point.x() + menuSize.width() < desktopWidth) //   Then, try at bottom-left
			/*point is already set*/;
		else                                             //   Overwise, at bottom-right
			point = rect.bottomRight() - QPoint(menuSize.width(), - 1);
	// Idem : rect.topLeft() and rect.topRight() must be VISIBLE :
	//  show the menu 1 pixel more TOP (substract 1 in Y) :
	} else {                                             // Overwize, try at top
		if (centered)
			point = QPoint( rect.center().x() - menuSize.width() / 2, rect.top() - menuSize.height() - 1 );
		else if (point.x() + menuSize.width() < desktopWidth) //   Then, try at top-left
			point = rect.topLeft()  - QPoint(0, menuSize.height() + 1);
		else                                             //   Overwise, at top-right
			point = rect.topRight() - QPoint(menuSize.width(), menuSize.height() + 1);
	}

	// No need to clip : it will be done by menu.exec(...)

	// And show the menu :
	menu.exec( point + QPoint(0, MENU_Y_OFFSET) ); // Stupid offset (Qt bug ?) : we should show the menus 2 pixels more bottom !
}

void PopupMenu::execAtRectRight(QPopupMenu &menu, const QRect &rect, bool centered)
{
	QSize menuSize = menu.sizeHint() - QSize(1, 1); // A size is [1..n] => We want two lengths that are [0..(n-1)]
	int desktopWidth  = kapp->desktop()->width();   //  to be compared/added/substracted with QRects/QPoints...
	int desktopHeight = kapp->desktop()->height();

	/** Paint the rect on the screen (desktop).
	  * For test purpose only (to be sure the passed rectangle is right).
	  * Comment this (and the qpainter and qpen includes) for a non-debug version.
	  */
	/*QPainter paint(kapp->desktop(), kapp->desktop(), true);
	paint.setPen( QPen(Qt::black, 1) );
	paint.drawRect(rect);
	paint.end();*/

	// rect.topRight() and rect.topLeft() must be VISIBLE :
	//  show the menu 1 pixel more RIGHT (add 1 in X) :
	QPoint point = rect.topRight() + QPoint(1, 0);
	if (point.x() + menuSize.width() < desktopWidth) { // First try at right
		if (centered)
			point = QPoint( point.x(), rect.center().y() - menuSize.height() / 2 );
		else if (point.y() + menuSize.height() < desktopHeight) //   Then, try at top-right
			/*point is already set*/;
		else                                             //   Overwise, at top-left
			point = rect.bottomRight() - QPoint(-1, menuSize.height());
	// Idem : rect.topLeft() and rect.bottomLeft() must be VISIBLE :
	//  show the menu 1 pixel more LEFT (substract 1 in X) :
	} else {                                             // Overwize, try at top
		if (centered)
			point = QPoint( rect.left() - menuSize.width() - 1, rect.center().y() - menuSize.height() / 2 );
		else if (point.y() + menuSize.height() < desktopHeight) //   Then, try at top-left
			point = rect.topLeft()  - QPoint(menuSize.width() + 1, 0);
		else                                             //   Overwise, at bottom-left
			point = rect.bottomLeft() - QPoint(menuSize.width() + 1, menuSize.height());
	}

	// No need to clip : it will be done by menu.exec(...)

	// And show the menu :
	menu.exec( point + QPoint(0, MENU_Y_OFFSET) ); // Stupid offset (Qt bug ?) : we should show the menus 2 pixels more bottom !
}

// # i n  c l u d e   " p o p u p m e n u . m o c " // Comment this if you don't compile PopupMenuTest class
