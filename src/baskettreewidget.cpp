#include "baskettreewidget.h"
//#include "baskettreemodel.h"

#include <QStringList>
#include <QPalette>
#include <QModelIndex>

#include <KDebug>
#include <KIcon>

#include <libakonadi/collection.h>
#include <libakonadi/collectionview.h>
#include <libakonadi/collectionfilterproxymodel.h>
#include <libakonadi/collectionmodel.h>
#include <libakonadi/collectionmodifyjob.h>
#include <libakonadi/collectioncreatejob.h>
#include <libakonadi/session.h>

using namespace Akonadi;

BasketTreeWidget::BasketTreeWidget( QWidget *parent ) : QWidget( parent ) {
	mCollectionTree = new Akonadi::CollectionView( this );
	connect( mCollectionTree, SIGNAL( clicked( QModelIndex ) ), SLOT( collectionActivated( QModelIndex ) ) );

	mCollectionModel = new Akonadi::CollectionModel( this );
	mCollectionProxyModel = new Akonadi::CollectionFilterProxyModel( this );
	mCollectionProxyModel->setSourceModel( mCollectionModel );
	//mCollectionProxyModel->addMimeType( QString( "basket/basket" ) );
	//mCollectionProxyModel->addMimeType( QString( "message/rfc822" ) );
	//mCollectionProxyModel->addMimeType( QString( "inode/directory" ) );
	mCollectionProxyModel->addMimeType( QString( "basket/basket" ) );
	mCollectionTree->setModel( mCollectionProxyModel );

	//Collection* col = Collection( index.internalId() );
	//kDebug() << index.internalId() << endl;
	//kDebug() << Collection::root().id() << endl;

	//CollectionCreateJob* job = new CollectionCreateJob( Collection::root() , "first", new Session() );
	//connect( job, SIGNAL(), this, SLOT() );
}

BasketTreeWidget::~BasketTreeWidget() {
}

void BasketTreeWidget::collectionActivated( const QModelIndex& index ) {
	//kDebug() << "activated" << endl;
	/*QModelIndex root = mCollectionModel->index( 0, 0 );
	QModelIndex index0 = mCollectionModel->index( 0, 0, root );
	kDebug() << root.data( CollectionModel::CollectionIdRole ).toInt() << endl;
	kDebug() << index0.data( CollectionModel::CollectionIdRole ).toInt() << endl;*/
	//kDebug() << index.data( CollectionModel::CollectionIdRole ).toInt() << endl;
	/*int parentId = index.data( CollectionModel::CollectionIdRole ).toInt();
	CollectionCreateJob* job = new CollectionCreateJob( Collection( parentId ), "test" );
	QStringList list; list << "basket/basket";
	job->setContentTypes( list );
	job->start();*/
}

