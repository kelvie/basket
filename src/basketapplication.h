#ifndef BASKETAPPLICATION_H
#define BASKETAPPLICATION_H

#include <KUniqueApplication>

class BasketMainWindow;
class BasketSystemTray;

class BasketApplication : public KUniqueApplication {
	Q_OBJECT

	public:
		BasketApplication();
		virtual ~BasketApplication();

		inline BasketMainWindow* mainWindow() const { return mMainWindow; }

	private:
		BasketMainWindow* mMainWindow;
		BasketSystemTray* mTray;
};

#endif	//BASKETAPPLICATION_H
