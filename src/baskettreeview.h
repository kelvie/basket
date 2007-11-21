#ifndef BASKETTREEVIEW_H
#define BASKETTREEVIEW_H

#include <QTreeWidget>

class QModelIndex;

class BasketTreeView : public QTreeWidget {
	Q_OBJECT

	public:
		BasketTreeView( QWidget *parent = 0 );
		~BasketTreeView();

};

#endif //BASKETTREEVIEW_H
