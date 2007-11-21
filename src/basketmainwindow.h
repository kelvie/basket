#ifndef BASKETMAINWINDOW_H
#define BASKETMAINWINDOW_H

#include <KXmlGuiWindow>

class QModelIndex;
class QVBoxLayout;

class BasketMainWindow : public KXmlGuiWindow {
	Q_OBJECT
	public:
		BasketMainWindow( QWidget* parent = 0 );
		~BasketMainWindow();

	private:

	private slots:
		void update0( const QModelIndex& index );
};

#endif // BASKETMAINWINDOW_H
