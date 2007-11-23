#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>

#include <KDebug>

#include "basketviewcontainer.h"
#include "plasma/corona.h"
#include "plasma/containment.h"

BasketViewContainer::BasketViewContainer() : QStackedWidget() {
	QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );
	
	Plasma::Corona *corona = new Plasma::Corona();
	//Plasma::Containment *cont = corona->addContainment( "context" );
	Plasma::Containment *cont = corona->addContainment( "content" );
	cont->setScreen( 0 );
	cont->setFormFactor( Plasma::Planar );

	view->setScene( corona );
}

BasketViewContainer::~BasketViewContainer() {
}

