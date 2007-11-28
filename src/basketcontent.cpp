#include "basketcontent.h"
#include "notewidget.h"

BasketContent::BasketContent( QObject* parent ) : QGraphicsScene( parent ) {
	//QFont font( "Helvetica", 16, QFont::Bold );
	//addText( "It could be the place for your advertisment!", font );
	NoteWidget* note = new NoteWidget();
	addItem( note );
}

BasketContent::~BasketContent() {
}

