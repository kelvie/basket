#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QGraphicsTextItem>

class NoteWidget : public QGraphicsTextItem {
	Q_OBJECT

	public:
		NoteWidget( QGraphicsItem* parent = 0 );
		~NoteWidget();

		QRectF boundingRect() const;

	public slots:
		void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );
		void hoverEnterEvent( QGraphicsSceneHoverEvent* event );
		void hoverLeaveEvent( QGraphicsSceneHoverEvent* event );

	private:
		bool m_isHovered;

};

#endif
