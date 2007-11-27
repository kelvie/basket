#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>

#include <KDebug>

#include "basketviewcontainer.h"

BasketViewContainer::BasketViewContainer() : QStackedWidget() {
	QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );
	
	view->setScene( new QGraphicsScene );
}

BasketViewContainer::~BasketViewContainer() {
}

