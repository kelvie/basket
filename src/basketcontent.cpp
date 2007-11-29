#include <QLinearGradient>

#include <KDebug>

#include "basketcontent.h"
#include "notewidget.h"

BasketContent::BasketContent( QObject* parent ) : QGraphicsScene( parent ) {
	//QFont font( "Helvetica", 16, QFont::Bold );
	//addText( "It could be the place for your advertisment!", font );
	
	QLinearGradient gradient( -100, -100, 100, 100 );
	gradient.setColorAt( 0, QColor( 200, 200, 200 ) );
	gradient.setColorAt( 1, QColor( 100, 100, 100 ) );
	setBackgroundBrush( QBrush( gradient ) );

	//setBackgroundBrush( QColor( 200, 200, 200 ) );
	NoteWidget* note = new NoteWidget();
	addItem( note );
}

BasketContent::~BasketContent() {
}

