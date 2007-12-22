#ifndef BASKETTREEWIDGET_H
#define BASKETTREEWIDGET_H

#include <QWidget>

class QModelIndex;
class BasketMainWindow;

namespace Akonadi {
	class CollectionModel;
	class CollectionFilterProxyModel;
	class CollectionView;
}

/*
 *	Class represents widget for tree of baskets
 */
class BasketTreeWidget : public QWidget {
	Q_OBJECT

	public:
		BasketTreeWidget( BasketMainWindow* mainWindow, QWidget *parent = 0 );
		~BasketTreeWidget();

	private slots:
		void collectionActivated( const QModelIndex& index );
	
	private:
		Akonadi::CollectionModel *mCollectionModel;
		Akonadi::CollectionFilterProxyModel *mCollectionProxyModel;
		Akonadi::CollectionView *mCollectionTree;

		BasketMainWindow* mMainWindow;
};

#endif //BASKETTREEWIDGET_H
