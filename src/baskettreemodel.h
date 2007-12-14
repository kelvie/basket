#ifndef BASKETTREEMODEL_H
#define BASKETTREEMODEL_H

#include <QAbstractItemModel>

class BasketTreeModel : public QAbstractItemModel {
	Q_OBJECT

	public:
		BasketTreeModel( QObject* parent = 0 );
		virtual ~BasketTreeModel();

		virtual QVariant data( const QModelIndex &index, int rolte = Qt::DisplayRole ) const;

		virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

		/*TODO QModelIndex indexForItem( const BasketTreeItem &item ) const;
		BasketTreeItem itemForIndex( const QModelIndex &index ) const;
		QModelIndex indexForId( int basketId ) const;*/

		//TODO void itemChanged( const QModelIndex& index );

		virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
		virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

		virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

	private:


};

#endif
