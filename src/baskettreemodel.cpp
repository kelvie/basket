#include "baskettreemodel.h"

#include <QPixmap>

#include <KIconLoader>

#include <libakonadi/collectionmodel.h>

BasketTreeModel::BasketTreeModel( QObject* parent ) : Akonadi::CollectionModel( parent ) {
}

BasketTreeModel::~BasketTreeModel() {
}

QVariant BasketTreeModel::data( const QModelIndex &index, int role ) const {
	if ( !index.isValid() )
		return QVariant();

	if ( role == Qt::DecorationRole ) {
		if ( index.column() == 0 ) {
			return SmallIcon( QLatin1String( "basket" ) );
		}
	}

	return Akonadi::CollectionModel::data( index, role );
}


