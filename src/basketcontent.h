#ifndef BASKETCONTENT_H
#define BASKETCONTENT_H

#include <QGraphicsScene>

class BasketContent : public QGraphicsScene {
	Q_OBJECT
	public:
		BasketContent( QObject* parent = 0 );
		~BasketConent();
	private:
		QHash<int, int> mapIdToNote;
};

#endif
