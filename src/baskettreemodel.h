#ifndef BASKETTREEMODEL_H
#define BASKETTREEMODEL_H

#include <QAbstractItemModel>

#include <libakonadi/collectionmodel.h>

class BasketTreeModel : public Akonadi::CollectionModel {
	Q_OBJECT

	public:
		explicit BasketTreeModel( QObject* parent = 0 );

		virtual ~BasketTreeModel();

		virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;


};

#endif
