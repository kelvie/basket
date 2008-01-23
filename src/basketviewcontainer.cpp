#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <KDebug>

#include "basketviewcontainer.h"
#include "basketcontent.h"

#include <libakonadi/monitor.h>
#include <libakonadi/collection.h>
#include <libakonadi/item.h>
#include <libakonadi/session.h>

using namespace Akonadi;

BasketViewContainer::BasketViewContainer( QWidget* parent ) : QWidget( parent ) {

	mStackedWidget = new QStackedWidget( this );

	// Monitor all changes on the server (Akonadi)
	mMonitor = new Monitor( this );
	// Need to ignore, because we don't want to get notifications about local changes
	// we can handle them locally
	mMonitor->ignoreSession( Akonadi::Session::defaultSession() );
	// monitor for all notes
	mMonitor->monitorMimeType( "basket/note" );
	connect( mMonitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ), this, SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ));
	//TODO add monitoring for collections, etc

	/*QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );

	BasketContent* basket = new BasketContent();

	view->setScene( basket );*/

	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->addWidget( mStackedWidget );
	setLayout( layout );
}

BasketViewContainer::~BasketViewContainer() {
}
	
void BasketViewContainer::setCurrentBasket( int basketId ) {
	kDebug() << "set: " << basketId << endl;
	QHash<int, QGraphicsView*>::const_iterator i = mBasketIdToViewWidget.find( basketId );
	if ( i == mBasketIdToViewWidget.constEnd() ) {
		QGraphicsView* view = new QGraphicsView( this );
		//view->setFrameShape( QFrame::NoFrame );
		view->setScene( new BasketContent( basketId, this ) );
		view->setInteractive( true );
		view->setAcceptDrops( true );
		view->setDragMode( QGraphicsView::RubberBandDrag );

		mBasketIdToViewWidget[basketId] = view;
		mStackedWidget->addWidget( view );
		mStackedWidget->setCurrentWidget( view );
	} else {
		mStackedWidget->setCurrentWidget( i.value() );
	}
}

void BasketViewContainer::itemAdded( const Akonadi::Item& item, const Akonadi::Collection& collection ) {
	kDebug() << "-----------" << endl;
	kDebug() << item.reference().id() << endl;
	kDebug() << collection.id() << endl;
	kDebug() << "-----------" << endl;

	QHash<int, QGraphicsView*>::iterator i = mBasketIdToViewWidget.find( collection.id() );
	if ( i == mBasketIdToViewWidget.end() ) return;
	
	BasketContent* basket = dynamic_cast<BasketContent*>(i.value()->scene());
	basket->itemAdded( item );
}

void BasketViewContainer::fontChanged( const QString& text ) {
}

void BasketViewContainer::toggleFormatTextBold() {
	if ( !mStackedWidget->currentWidget() ) {
		kDebug() << "bug: button toggle while there are no baskets" << endl;
		return;
	}
	QGraphicsView *view = qobject_cast<QGraphicsView*>( mStackedWidget->currentWidget() );
	BasketContent *content = qobject_cast<BasketContent*>( view->scene() );
	content->toggleFormatTextBold();
}

