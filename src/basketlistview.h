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

#ifndef BASKETLISTVIEW_H
#define BASKETLISTVIEW_H

#include <QtCore/QTimer>
#include <QtGui/QTreeWidget>

class QPixmap;
class QResizeEvent;
class QDragEnterEvent;
class QDropEvent;
class QDragMoveEvent;
class QFocusEvent;
class QDragLeaveEvent;

class BasketView;


class BasketListViewItem : public QTreeWidgetItem
{
public:
    /// CONSTRUCTOR AND DESTRUCTOR:
    BasketListViewItem(QTreeWidget *parent, BasketView *basket);
    BasketListViewItem(QTreeWidgetItem *parent, BasketView *basket);
    BasketListViewItem(QTreeWidget *parent, QTreeWidgetItem *after, BasketView *basket);
    BasketListViewItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, BasketView *basket);
    ~BasketListViewItem();

    BasketView *basket() {
        return m_basket;
    }
    void setup();
    BasketListViewItem* lastChild();
    QStringList childNamesTree(int deep = 0);
    void moveChildsBaskets();
    void ensureVisible();
    bool isShown();
    bool isCurrentBasket();
    bool isUnderDrag();
    QString escapedName(const QString &string);

    QPixmap circledTextPixmap(const QString &text, int height, const QFont &font, const QColor &color);
    QPixmap foundCountPixmap(bool isLoading, int countFound, bool childrenAreLoading, int countChildsFound, const QFont &font, int height);
    bool haveChildsLoading();
    bool haveHiddenChildsLoading();
    bool haveChildsLocked();
    bool haveHiddenChildsLocked();
    int countChildsFound();
    int countHiddenChildsFound();

    void setUnderDrag(bool);
    bool isAbbreviated();
    void setAbbreviated(bool b);

private:
    BasketView *m_basket;
    int     m_width;
    bool m_isUnderDrag;
    bool m_isAbbreviated;
};

Q_DECLARE_METATYPE(BasketListViewItem *);

class BasketTreeListView : public QTreeWidget
{
    Q_OBJECT
public:
    BasketTreeListView(QWidget *parent = 0);
    void dragEnterEvent(QDragEnterEvent *event);
    void removeExpands();
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeEvent(QResizeEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    Qt::DropActions supportedDropActions() const;

    static QString TREE_ITEM_MIME_STRING;
protected:
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QList<QTreeWidgetItem *> items) const;
    bool event(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void focusInEvent(QFocusEvent*);
private:
    QTimer         m_autoOpenTimer;
    QTreeWidgetItem *m_autoOpenItem;
signals:
    void contextMenuRequested(const QPoint &);
private slots:
    void autoOpen();
private:
    void setItemUnderDrag(BasketListViewItem* item);
    BasketListViewItem* m_itemUnderDrag;
    QPoint m_dragStartPosition;
};

#endif // BASKETLISTVIEW_H
