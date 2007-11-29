#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#include "notewidget.h"

NoteWidget::NoteWidget( QGraphicsItem* parent ) : QGraphicsTextItem( parent ) {
	QFont font( "Helvetica", 20 );
	setFont( font );
	setTextInteractionFlags( Qt::TextEditorInteraction );
	setHtml( "Hello, world!" );
	//setTextWidth( 100 );
}

NoteWidget::~NoteWidget() {
}

void NoteWidget::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) {
	painter->setOpacity( 0.5 );

	painter->setBrush( Qt::green );
	painter->drawRoundRect( boundingRect() );

	//painter->setPen( Qt::NoPen );
	
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>( option );
	style->state &= ~( QStyle::State_Selected | QStyle::State_HasFocus );

	QGraphicsTextItem::paint( painter, style, widget );
}

