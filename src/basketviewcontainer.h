#ifndef BASKETVIEWCONTAINER_H
#define BASKETVIEWCONTAINER_H

#include <QStackedWidget>

class BasketViewContainer : public QStackedWidget {
	Q_OBJECT

public:
	BasketViewContainer( QWidget* parent = 0 );
	~BasketViewContainer();

private:
};

#endif //BASKETVIEWCONTAINER_H
