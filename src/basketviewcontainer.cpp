#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>

#include <KDebug>

#include "basketviewcontainer.h"

BasketViewContainer::BasketViewContainer() : QStackedWidget() {
	QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );
	
	QGraphicsScene *scene = new QGraphicsScene();
	QGraphicsTextItem *item = new QGraphicsTextItem();
	item->setHtml( QString("<p>Hello</p>, world!") );
	scene->addItem( item );

	view->setScene( scene );
}

BasketViewContainer::~BasketViewContainer() {
}

