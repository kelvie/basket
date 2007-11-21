#include "basketapplication.h"
#include "basketmainwindow.h"
#include "basketsystemtray.h"

BasketApplication::BasketApplication() : KUniqueApplication() {
	m_mainWindow = new BasketMainWindow();
	m_mainWindow->show();
	
	m_tray = new BasketSystemTray( mainWindow() );
	m_tray->setVisible( true );
}

BasketApplication::~BasketApplication() {
	//delete m_mainWindow;
	//m_mainWindow = 0;
}


