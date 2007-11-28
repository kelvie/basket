#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>

#include <KDebug>

#include "basketviewcontainer.h"
#include "basketcontent.h"

BasketViewContainer::BasketViewContainer( QWidget* parent ) : QStackedWidget( parent ) {
	QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );

	BasketContent* basket = new BasketContent();

	view->setScene( basket );
}

BasketViewContainer::~BasketViewContainer() {
}

