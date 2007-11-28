#ifndef BASKETMAINWINDOW_H
#define BASKETMAINWINDOW_H

#include <KXmlGuiWindow>

class QModelIndex;
class QVBoxLayout;
class QMenuBar;
class KMenu;

class BasketMainWindow : public KXmlGuiWindow {
	Q_OBJECT
	public:
		BasketMainWindow( QWidget* parent = 0 );
		~BasketMainWindow();

	private slots:
		void update0( const QModelIndex& index );

		void createNewBasket();

	private:
		void init();

		void setupMenus();
		void setupActions();
		void setupDockWidgets();
};

#endif // BASKETMAINWINDOW_H
