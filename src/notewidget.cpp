#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QFlags>
#include <QGraphicsSceneMouseEvent>

#include <KDebug>

#include "notewidget.h"

NoteWidget::NoteWidget( QGraphicsItem* parent ) : QGraphicsTextItem( parent ) {
	m_isHovered = false;
	m_isDragged = false;

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

	QRectF rect = boundingRect();

	painter->setBrush( QColor( 50, 50, 255 ) );
	painter->drawRoundRect( rect, 10, 10 );

	painter->setBrush( Qt::red );
	painter->drawEllipse( QRectF( rect.x() + 10, rect.y() + 5, rect.width() - 20, 6 ) );

	//painter->setPen( Qt::NoPen );
	
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>( option );
	style->state &= ~( QStyle::State_Selected | QStyle::State_HasFocus );

	QGraphicsTextItem::paint( painter, style, widget );
}

QRectF NoteWidget::boundingRect() const {
	QRectF rect = QGraphicsTextItem::boundingRect();
	/*if ( m_isHovered ) {
		return QRectF( rect.x() - 10, rect.y() - 10, rect.width() + 20, rect.height() + 20  );
	}*/
	qreal penWidth = 1;
	return QRectF( rect.x() - penWidth / 2, rect.y() - 10 - penWidth / 2, rect.width() + 10 + penWidth, rect.height() + 10 + penWidth );
}

void NoteWidget::hoverEnterEvent( QGraphicsSceneHoverEvent* event ) {
	kDebug() << "hovered" << endl;
	//prepareGeometryChange();
	m_isHovered = true;
	update();
}

void NoteWidget::hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) {
	kDebug() << "unhovered" << endl;
	//prepareGeometryChange();
	m_isHovered = false;
	update();
}

QPainterPath NoteWidget::shape() const {
	QPainterPath path;
	path.addRect( boundingRect() );
	return path;
}

void NoteWidget::mousePressEvent( QGraphicsSceneMouseEvent* event ) {
	kDebug() << event->pos() << endl;
	if ( event->pos().y() <= 3 && event->button() == Qt::LeftButton ) {
		event->accept();
		update();
		m_isDragged = true;
		return;
	}
	m_isDragged = false;
	QGraphicsTextItem::mousePressEvent( event );
}

void NoteWidget::mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) {
	if ( m_isDragged ) {
		QGraphicsTextItem::mouseReleaseEvent( event );
		update();
		m_isDragged = false;
		return;
	}
	QGraphicsTextItem::mouseReleaseEvent( event );
}

void NoteWidget::mouseMoveEvent( QGraphicsSceneMouseEvent* event ) {
	if ( m_isDragged ) {
		QPointF delta = event->pos() - event->lastPos();
		setPos( pos() + delta );
		update();
		return;
	}
	QGraphicsTextItem::mouseReleaseEvent( event );
}

