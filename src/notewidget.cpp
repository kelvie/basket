#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QFlags>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QTextDocument>

#include <KDebug>

#include "notewidget.h"

#include <libakonadi/item.h>
#include <libakonadi/itemstorejob.h>

using namespace Akonadi;

NoteWidget::NoteWidget( const Item& item, QGraphicsItem* parent ) : QGraphicsTextItem( parent ), mItem( item ) {
	m_isHovered = false;
	m_isDragged = false;

	setAcceptDrops( true );
	setAcceptsHoverEvents( true );

	QFont font( "Helvetica", 16 );
	setFont( font );
	setTextInteractionFlags( Qt::TextEditorInteraction );
	//setHtml( "Hello, world!" );

	//setToolTip( "This is a tooltip :)\n<tooltip>" );
	//setTextWidth( 100 );
	//connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( storeItem() ) );
	if ( !mItem.hasPayload() ) {

	} else {
		mNote = mItem.payload<NotePtr>();
		setPos( mNote->pos() );
		setHtml( mNote->text() );
	}

	QTextDocument *doc = this->document();
	connect( doc, SIGNAL( contentsChanged() ), this, SLOT( contentsChanged() ) );
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
	//kDebug() << "hovered: " << event->pos() << endl;
	//prepareGeometryChange();
	m_isHovered = true;
	update();
}

void NoteWidget::hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) {
	//kDebug() << "unhovered" << event->pos() << endl;
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
	setZValue( 1 );
	update();
	kDebug() << event->pos() << endl;
	if ( event->pos().y() <= 3 && event->button() == Qt::LeftButton ) {
		//TODO setCursor( Qt::ClosedHandCursor );
		event->accept();
		update();
		m_isDragged = true;
		return;
	}
	m_isDragged = false;
	QGraphicsTextItem::mousePressEvent( event );
}

void NoteWidget::mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) {
	setZValue( 0 );
	update();
	if ( m_isDragged ) {
		QGraphicsTextItem::mouseReleaseEvent( event );
		//TODO setCursor( Qt::ArrowCursor );
		update();
		m_isDragged = false;
		return;
	}
	QGraphicsTextItem::mouseReleaseEvent( event );
}

void NoteWidget::mouseMoveEvent( QGraphicsSceneMouseEvent* event ) {
	if ( event->pos().y() <= 3 ) {
		//TODO setCursor( Qt::OpenHandCursor );
		//update();
	}
	if ( m_isDragged ) {
		//TODO setCursor( Qt::ClosedHandCursor );
		QPointF delta = event->pos() - event->lastPos();
		setPos( pos() + delta );
		update();
		return;
	}
	QGraphicsTextItem::mouseReleaseEvent( event );
}

void NoteWidget::keyPressEvent( QKeyEvent* event ) {
	if ( event->matches( QKeySequence::Undo ) ) {
		kDebug() << "undo pressed" << endl;
		//event->accept();
		return;
	}
	if ( event->matches( QKeySequence::Redo ) ) {
		kDebug() << "redo pressed" << endl;
		//event->accept();
		return;
	}

	/*if ( !event->text().isEmpty() ) {
		kDebug() << event->text() << endl;
		event->accept();
		return;
	}*/
	QGraphicsTextItem::keyPressEvent( event );
	/*if ( mNote->text() != toHtml() ) {
		mNote->setText( toHtml() );
		storeItem();
		//emit textChanged( toHtml() );
	}*/
}

void NoteWidget::storeItem() {
	kDebug() << "store item: " << mItem.reference().id() << endl;
	ItemStoreJob* job = new ItemStoreJob( mItem );
	job->storePayload();
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( storeDone( KJob* ) ) );
}

void NoteWidget::storeDone( KJob* job ) {
	if ( job->error() ) {
		kDebug() << "Error" << endl;
	}
}

void NoteWidget::fetchDone( KJob* job ) {
}

void NoteWidget::contentsChanged() {
	kDebug() << "text changed!!! need to set redo/undo operation" << endl;
}

