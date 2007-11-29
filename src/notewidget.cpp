#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include "notewidget.h"

NoteWidget::NoteWidget( QGraphicsItem* parent ) : QGraphicsTextItem( parent ) {
	m_isHovered = false;

	setAcceptDrops( true );
	setAcceptsHoverEvents( true );

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

QRectF NoteWidget::boundingRect() const {
	QRectF rect = QGraphicsTextItem::boundingRect();
	if ( m_isHovered ) {
		return QRectF( rect.x() - 10, rect.y() - 10, rect.width() + 20, rect.height() + 20  );
	}
	return rect;

}

void NoteWidget::hoverEnterEvent( QGraphicsSceneHoverEvent* event ) {
	kDebug() << "hovered" << endl;
	prepareGeometryChange();
	m_isHovered = true;
	update();
}

void NoteWidget::hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) {
	kDebug() << "unhovered" << endl;
	prepareGeometryChange();
	m_isHovered = false;
	update();
}

