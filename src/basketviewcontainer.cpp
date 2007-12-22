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
//#include <libakonadi/datareference.h>

using namespace Akonadi;

BasketViewContainer::BasketViewContainer( QWidget* parent ) : QWidget( parent ) {

	mStackedWidget = new QStackedWidget( this );

	// Monitor will be needed in the future
	mMonitor = new Monitor( this );
	mMonitor->monitorMimeType( "basket/note" );
	connect( mMonitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ), this, SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ));

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
		view->setScene( new BasketContent( basketId, this ) );
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


