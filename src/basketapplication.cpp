#include "basketapplication.h"
#include "basketmainwindow.h"
#include "basketsystemtray.h"

#include <libakonadi/control.h>

BasketApplication::BasketApplication() : KUniqueApplication() {
	// Start akonadi server if it wasn't running yet
	Akonadi::Control::start();

	// Create main window
	mMainWindow = new BasketMainWindow();
	mMainWindow->show();
	
	// Create system tray icon
	mTray = new BasketSystemTray( mainWindow() );
	mTray->setVisible( true );
}

BasketApplication::~BasketApplication() {
}


