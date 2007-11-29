#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#include "notewidget.h"

NoteWidget::NoteWidget( QGraphicsItem* parent ) : QGraphicsTextItem( parent ) {
	setAcceptDrops( true );

	QFont font( "Helvetica", 20 );
	setFont( font );
	setTextInteractionFlags( Qt::TextEditorInteraction );
	setHtml( "Hello, world!" );
	//setTextWidth( 100 );
}

NoteWidget::~NoteWidget() {
}

void NoteWidget::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) {
	painter->setOpacity( 0.7 );

	painter->setRenderHint( QPainter::Antialiasing );
	painter->setRenderHint( QPainter::TextAntialiasing );

	painter->setBrush( QColor( 100, 100, 255 ) );
	painter->drawRoundRect( boundingRect(), 10, 10 );
	//painter->drawRoundRect( boundingRect() );

	//painter->setPen( Qt::NoPen );
	
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>( option );
	style->state &= ~( QStyle::State_Selected | QStyle::State_HasFocus );

	QGraphicsTextItem::paint( painter, style, widget );
}

