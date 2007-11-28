#ifndef BASKETCONTENT_H
#define BASKETCONTENT_H

#include <QGraphicsScene>
#include <QHash>

class BasketContent : public QGraphicsScene {
	Q_OBJECT
	public:
		BasketContent( QObject* parent = 0 );
		~BasketContent();
	private:
		QHash<int, int> mapIdToNote;
};

#endif
