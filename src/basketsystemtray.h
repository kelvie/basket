#ifndef BASKETSYSTEMTRAY_H
#define BASKETSYSTEMTRAY_H

#include <KSystemTrayIcon>

class QWidget;
class QIcon;

class BasketSystemTray : public KSystemTrayIcon {
	public:
		BasketSystemTray( QWidget *parent = 0 );
		~BasketSystemTray();
	private:
		QIcon baseIcon;
};

#endif // BASKETSYSTRAY_H
