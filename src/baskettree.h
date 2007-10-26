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

#ifndef BASKETTREE_H
#define BASKETTREE_H

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <qtimer.h>
#include <QMimeData>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class Basket;

class BasketTreeItem : public QTreeWidgetItem
{
	public:
	//  CONSTRUCTOR AND DESTRUCTOR:
		BasketTreeItem(QTreeWidgetItem     *parent, Basket *basket);
		BasketTreeItem(QTreeWidgetItem     *parent, QTreeWidgetItem *after, Basket *basket);
		~BasketTreeItem();

		bool acceptDrop(const QMimeData *mime) const;
		void dropped(QDropEvent *event);
		Basket *basket() { return m_basket; }
		void setup();
		int width(const QFontMetrics &fontMetrics, const QTreeWidgetItem *listView, int column) const;
		BasketTreeItem* firstChild(){return (BasketTreeItem*) child(0);}
		BasketTreeItem* lastChild();
		BasketTreeItem* prevSibling();
		BasketTreeItem* shownItemAbove();
		BasketTreeItem* shownItemBelow();
		QStringList childNamesTree(int deep = 0);
		void moveChildsBaskets();
		void ensureVisible();
		bool isVisible();
		bool isCurrentBasket();
		void paintCell(QPainter *painter, const QPalette &colorGroup, int column, int width, int align);
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

		void setUnderDrag(bool);
		bool isAbbreviated();
		///
//	QDragObject* dragObject();
//	bool acceptDrop ( const QMimeSource * mime ) const;
	private:
		Basket *m_basket;
		int     m_width;
		bool m_isUnderDrag;
		bool m_isAbbreviated;
};

class BasketTree : public QTreeWidget
{
	Q_OBJECT
	public:
		BasketTree(QWidget *parent = 0, const char *name = 0);
		void contentsDragEnterEvent(QDragEnterEvent *event);
		void removeExpands();
		void contentsDragLeaveEvent(QDragLeaveEvent *event);
		void contentsDragMoveEvent(QDragMoveEvent *event);
		void contentsDropEvent(QDropEvent *event);
		void resizeEvent(QResizeEvent *event);
		void paintEmptyArea(QPainter *painter, const QRect &rect);
		BasketTreeItem* firstChild(){return (BasketTreeItem*) topLevelItem(0);}
	protected:
		void focusInEvent(QFocusEvent*);
	private:
		QTimer         m_autoOpenTimer;
		QTreeWidgetItem *m_autoOpenItem;
	private slots:
		void autoOpen();
	private:
		void setItemUnderDrag(BasketTreeItem* item);
		BasketTreeItem* m_itemUnderDrag;

};

#endif // BASKETTREE_H
