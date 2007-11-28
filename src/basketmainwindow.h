#ifndef BASKETMAINWINDOW_H
#define BASKETMAINWINDOW_H

#include <KXmlGuiWindow>

class BasketViewContainer;
class BasketTreeView;
class QModelIndex;
class QVBoxLayout;
class QMenuBar;
class KMenu;

class BasketMainWindow : public KXmlGuiWindow {
	Q_OBJECT
	public:
		BasketMainWindow( QWidget* parent = 0 );
		~BasketMainWindow();

	public slots:
		void setCurrentBasket( int basketId );

	private slots:
		void update0( const QModelIndex& index );

		void createNewBasket();

	private:
		void init();

		void setupMenus();
		void setupActions();
		void setupDockWidgets();

		BasketViewContainer* m_viewContainer;
		BasketTreeView* m_treeView;
};

#endif // BASKETMAINWINDOW_H
