#include <KSystemTrayIcon>

#include "basketsystemtray.h"

BasketSystemTray::BasketSystemTray( QWidget *parent ) : KSystemTrayIcon( parent ) {
	baseIcon = KSystemTrayIcon::loadIcon( "basket" );
	setIcon( baseIcon );
}

BasketSystemTray::~BasketSystemTray() {
}
