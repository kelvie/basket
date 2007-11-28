#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QString>

#include <KDebug>

#include "basketviewcontainer.h"

BasketViewContainer::BasketViewContainer( QWidget* parent ) : QStackedWidget( parent ) {
	QGraphicsView *view = new QGraphicsView();
	this->addWidget( view );
	QGraphicsScene* scene = new QGraphicsScene();
	QFont font( "Helvetica", 16, QFont::Bold );
	scene->addText( "It could be the place for your advertisment!", font );
	view->setScene( scene );
}

BasketViewContainer::~BasketViewContainer() {
}

