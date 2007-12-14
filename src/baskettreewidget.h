#ifndef BASKETTREEWIDGET_H
#define BASKETTREEWIDGET_H

#include <QWidget>

class QModelIndex;

namespace Akonadi {
	class CollectionModel;
	class CollectionFilterProxyModel;
	class CollectionView;
}

class BasketTreeWidget : public QWidget {
	Q_OBJECT

	public:
		BasketTreeWidget( QWidget *parent = 0 );
		~BasketTreeWidget();

	private slots:
		void collectionActivated( const QModelIndex& index );
	
	private:
		Akonadi::CollectionModel *mCollectionModel;
		Akonadi::CollectionFilterProxyModel *mCollectionProxyModel;
		Akonadi::CollectionView *mCollectionTree;
};

#endif //BASKETTREEWIDGET_H
