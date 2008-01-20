#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QGraphicsTextItem>

#include <libakonadi/item.h>

#include "note.h"
#include "basketcontent.h"

class NoteWidget : public QGraphicsTextItem {
	Q_OBJECT

	public:
		NoteWidget( const Akonadi::Item& item, QGraphicsItem* parent = 0 );
		~NoteWidget();

		QRectF boundingRect() const;
		QPainterPath shape() const;

		inline BasketContent* basket() const { return static_cast<BasketContent*>( scene() ); }

	public slots:
		void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );
		void hoverEnterEvent( QGraphicsSceneHoverEvent* event );
		void hoverLeaveEvent( QGraphicsSceneHoverEvent* event );
		void mousePressEvent( QGraphicsSceneMouseEvent* event );
		void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
		void mouseMoveEvent( QGraphicsSceneMouseEvent* event );

		void keyPressEvent( QKeyEvent* event );

		void storeItem();
		void storeDone( KJob* job );

		void fetchDone( KJob* job );

		void contentsChanged();

	signals:
		//void textChanged( const QString& text );

	private:
		bool m_isHovered;
		bool m_isDragged;

		NotePtr mNote;
		Akonadi::Item mItem;
};

#endif
