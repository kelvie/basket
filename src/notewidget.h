#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QGraphicsTextItem>

class NoteWidget : public QGraphicsTextItem {
	Q_OBJECT

	public:
		NoteWidget( QGraphicsItem* parent = 0 );
		~NoteWidget();

	public slots:
		void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

	private:

};

#endif
