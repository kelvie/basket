#include "baskettreemodel.h"

BasketTreeModel::BasketTreeModel( QObject* parent ) : QAbstractItemModel( parent ) {
}

BasketTreeModel::~BasketTreeModel() {
}

QVariant BasketTreeModel::data( const QModelIndex &index, int rolte = Qt::DisplayRole ) const {

}

int BasketTreeModel::columnCount( const QModelIndex &parent = QModelIndex() ) const {
	return 1;
}

