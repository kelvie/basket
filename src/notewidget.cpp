#include <QPainter>

#include "notewidget.h"

NoteWidget::NoteWidget( QGraphicsItem* parent ) : QGraphicsTextItem( parent ) {
	QFont font( "Helvetica", 20 );
	setFont( font );
	setTextInteractionFlags( Qt::TextEditorInteraction );
	setHtml( "Hello, world!" );
	setTextWidth( 100 );
}

NoteWidget::~NoteWidget() {
}

void NoteWidget::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) {
	painter->setOpacity( 0.5 );

	painter->setBrush( Qt::green );
	painter->drawRoundRect( boundingRect(), 35, 35 );

	//painter->setPen( Qt::NoPen );

	QGraphicsTextItem::paint( painter, option, widget );
}

