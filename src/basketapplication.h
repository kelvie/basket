#ifndef BASKETAPPLICATION_H
#define BASKETAPPLICATION_H

#include <KUniqueApplication>

class BasketMainWindow;
class BasketSystemTray;

class BasketApplication : public KUniqueApplication {
	public:
		BasketApplication();
		virtual ~BasketApplication();

		inline BasketMainWindow *mainWindow() const { return m_mainWindow; }

	private:
		BasketMainWindow *m_mainWindow;
		BasketSystemTray *m_tray;
};

#endif	//BASKETAPPLICATION_H
