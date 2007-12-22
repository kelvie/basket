#include "baskettreewidget.h"
#include "baskettreemodel.h"
#include "basketmainwindow.h"

#include <QStringList>
#include <QPalette>
#include <QModelIndex>
#include <QVBoxLayout>

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

BasketTreeWidget::BasketTreeWidget( BasketMainWindow* mainWindow, QWidget *parent ) : QWidget( parent ), mMainWindow( mainWindow ) {
	mCollectionTree = new Akonadi::CollectionView( this );
	connect( mCollectionTree, SIGNAL( clicked( QModelIndex ) ), SLOT( collectionActivated( QModelIndex ) ) );

	//mCollectionModel = new Akonadi::CollectionModel( this );
	mCollectionModel = new BasketTreeModel( this );
	mCollectionProxyModel = new Akonadi::CollectionFilterProxyModel( this );
	mCollectionProxyModel->setSourceModel( mCollectionModel );
	//mCollectionProxyModel->addMimeType( QString( "basket/basket" ) );
	//mCollectionProxyModel->addMimeType( QString( "message/rfc822" ) );
	//mCollectionProxyModel->addMimeType( QString( "inode/directory" ) );
	mCollectionProxyModel->addMimeType( QString( "basket/note" ) );
	mCollectionTree->setModel( mCollectionProxyModel );

	//Collection* col = Collection( index.internalId() );
	//kDebug() << index.internalId() << endl;
	//kDebug() << Collection::root().id() << endl;

	//CollectionCreateJob* job = new CollectionCreateJob( Collection::root() , "first", new Session() );
	//connect( job, SIGNAL(), this, SLOT() );
	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->addWidget( mCollectionTree );
	this->setLayout( layout );
}

BasketTreeWidget::~BasketTreeWidget() {
}

void BasketTreeWidget::collectionActivated( const QModelIndex& index ) {
	kDebug() << "activated basket: " << index.data( CollectionModel::CollectionIdRole ).toInt() << endl;
	mMainWindow->setCurrentBasket( index.data( CollectionModel::CollectionIdRole ).toInt() );

	//kDebug() << "activated" << endl;
	/*QModelIndex root = mCollectionModel->index( 0, 0 );
	QModelIndex index0 = mCollectionModel->index( 0, 0, root );
	kDebug() << root.data( CollectionModel::CollectionIdRole ).toInt() << endl;
	kDebug() << index0.data( CollectionModel::CollectionIdRole ).toInt() << endl;*/
	//kDebug() << index.data( CollectionModel::CollectionIdRole ).toInt() << endl;
	/*int parentId = index.data( CollectionModel::CollectionIdRole ).toInt();
	CollectionCreateJob* job = new CollectionCreateJob( Collection( parentId ), "Basket" );
	QStringList list; list << "basket/note";
	job->setContentTypes( list );
	job->start();*/
}

