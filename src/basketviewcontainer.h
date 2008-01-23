#ifndef BASKETVIEWCONTAINER_H
#define BASKETVIEWCONTAINER_H

#include <QWidget>
#include <QHash>

class QStackedWidget;
class BasketContent;
class QGraphicsView;

namespace Akonadi {
	class Monitor;
	class Item;
	class Collection;
}

/*
 *	Widget that contains baskets, notes etc
 */
class BasketViewContainer : public QWidget {
	Q_OBJECT

public:
	BasketViewContainer( QWidget* parent = 0 );
	~BasketViewContainer();

public slots:
	void setCurrentBasket( int basketId );
	void itemAdded( const Akonadi::Item& item, const Akonadi::Collection& collection );
	void fontChanged( const QString& text );

	void toggleFormatTextBold();

private:
	//QHash<int, BasketContent*> mBasketIdToBasketContent;
	QHash<int, QGraphicsView*> mBasketIdToViewWidget;

	QStackedWidget* mStackedWidget;
	int mCurrentBasketId;

	Akonadi::Monitor* mMonitor;

};

#endif //BASKETVIEWCONTAINER_H
