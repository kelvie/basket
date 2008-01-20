#ifndef BASKETMAINWINDOW_H
#define BASKETMAINWINDOW_H

#include <KXmlGuiWindow>

class BasketViewContainer;
class BasketTreeWidget;
class QModelIndex;

/*
 *	Basket main window class
 */
class BasketMainWindow : public KXmlGuiWindow {
	Q_OBJECT

	public:
		BasketMainWindow( QWidget* parent = 0 );
		~BasketMainWindow();

		void setCurrentBasket( int newBasketId );

	private slots:
		void createNewBasket();
		void undo();
		void redo();
		void paste();
		void showSettingsDialog();

	private:
		void init();

		void setupMenus();
		void setupActions();
		void setupDockWidgets();

		BasketViewContainer* mViewContainer;
		BasketTreeWidget* mTreeWidget;
};

#endif // BASKETMAINWINDOW_H
