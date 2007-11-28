#include "baskettreeview.h"

#include <QTreeWidgetItem>
#include <QStringList>
#include <QPalette>
#include <QModelIndex>

#include <KDebug>

BasketTreeView::BasketTreeView( QWidget *parent ) : QTreeWidget( parent ) {
	setAcceptDrops( true );
	setUniformRowHeights( true );
	setSelectionMode( QAbstractItemView::SingleSelection );
	//setEditTriggers( QAbstractItemView::NoEditTriggers );
	//setDropIndicatorShown( false );
	//setDragDropMode( QAbstractItemView::DragDrop );

	/*viewport()->setAttribute( Qt::WA_Hover );

	QPalette palette = viewoprt()->palette();
	palette.setColor( Qt::red, Qt::transparent );
	viewport()->setPalette( palette );*/

	QList<QTreeWidgetItem*> items;
	for (int i = 0; i < 10; i++) {
		items.append( new QTreeWidgetItem( (QTreeWidget*)0, QStringList( QString("item: %1").arg(i) ) ) );
	}
	insertTopLevelItems(0, items);
	setHeaderItem(0);
	setHeaderLabel( "General" );
}

BasketTreeView::~BasketTreeView() {
}


