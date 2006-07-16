/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
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

#ifndef BASKETLISTVIEW_H
#define BASKETLISTVIEW_H

#include <klistview.h>
#include <qtimer.h>

class Basket;

class BasketListViewItem : public QListViewItem
{
	public:
	/// CONSTRUCTOR AND DESTRUCTOR:
		BasketListViewItem(QListView     *parent, Basket *basket);
		BasketListViewItem(QListViewItem *parent, Basket *basket);
		BasketListViewItem(QListView     *parent, QListViewItem *after, Basket *basket);
		BasketListViewItem(QListViewItem *parent, QListViewItem *after, Basket *basket);
		~BasketListViewItem();
		///
		bool acceptDrop(const QMimeSource *mime) const;
		void dropped(QDropEvent *event);
		Basket *basket() { return m_basket; }
		void setup();
		int width(const QFontMetrics &fontMetrics, const QListView *listView, int column) const;
		BasketListViewItem* lastChild();
		BasketListViewItem* prevSibling();
		BasketListViewItem* shownItemAbove();
		BasketListViewItem* shownItemBelow();
		QStringList childNamesTree(int deep = 0);
		void moveChildsBaskets();
		void ensureVisible();
		bool isShown();
		bool isCurrentBasket();
		void paintCell(QPainter *painter, const QColorGroup &colorGroup, int column, int width, int align);
		QString escapedName(const QString &string);
		///
		QPixmap circledTextPixmap(const QString &text, int height, const QFont &font, const QColor &color);
		QPixmap foundCountPixmap(bool isLoading, int countFound, bool childsAreLoading, int countChildsFound, const QFont &font, int height);
		bool haveChildsLoading();
		bool haveHiddenChildsLoading();
		bool haveChildsLocked();
		bool haveHiddenChildsLocked();
		int countChildsFound();
		int countHiddenChildsFound();
		///
//	QDragObject* dragObject();
//	bool acceptDrop ( const QMimeSource * mime ) const;
	private:
		Basket *m_basket;
		int     m_width;
};

class BasketTreeListView : public KListView
{
	Q_OBJECT
	public:
		BasketTreeListView(QWidget *parent = 0, const char *name = 0);
		void contentsDragEnterEvent(QDragEnterEvent *event);
		void removeExpands();
		void contentsDragLeaveEvent(QDragLeaveEvent *event);
		void contentsDragMoveEvent(QDragMoveEvent *event);
		void contentsDropEvent(QDropEvent *event);
		void resizeEvent(QResizeEvent *event);
		void paintEmptyArea(QPainter *painter, const QRect &rect);
	protected:
		void focusInEvent(QFocusEvent*);
	private:
		QTimer         m_autoOpenTimer;
		QListViewItem *m_autoOpenItem;
	private slots:
		void autoOpen();
};

#endif // BASKETLISTVIEW_H
