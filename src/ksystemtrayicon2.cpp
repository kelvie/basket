/***************************************************************************
 *   Copyright (C) 2008 by Kelvie Wong                                     *
 *   kelvie@ieee.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ksystemtrayicon2.h"

// To draw the systray screenshot image:
#include <q3dockwindow.h>
#include <qmovie.h>
#include <qvariant.h>
//Added by qt3to4:
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDragEnterEvent>

#include <qdesktopwidget.h>
#include <qmime.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpixmap.h>
// To know the program name:
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kiconeffect.h>
// Others:
#include <kmessagebox.h>
#include <kmanagerselection.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kdebug.h>

KSystemTray2::KSystemTray2(QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
}

KSystemTray2::~KSystemTray2()
{
}

void KSystemTray2::displayCloseMessage(QString fileMenu)
{
	/* IDEAS OF IMPROVEMENTS:
	*  - Use queuedMessageBox() but it need a dontAskAgainName parameter
	*    and image in QMimeSourceFactory shouldn't be removed.
	*  - Sometimes the systray icon is covered (a passive popup...)
	*    Use XComposite extension, if available, to get the kicker pixmap.
	*  - Perhapse desaturate the area around the proper SysTray icon,
	*    helping bring it into sharper focus. Or draw the cicle with XOR
	*    brush.
	*  - Perhapse add the icon in the text (eg. "... in the
	*    system tray ([icon])."). Add some clutter to the dialog.
	*/
#if KDE_IS_VERSION( 3, 1, 90 )
	// Don't do all the computations if they are unneeded:
	if ( ! KMessageBox::shouldBeShownContinue("hideOnCloseInfo") )
		return;
#endif
	// "Default parameter". Here, to avoid a i18n() call and dependancy in the .h
	if (fileMenu.isEmpty())
		fileMenu = i18n("File");

	// Some values we need:
	QPoint g = mapToGlobal(pos());
	int desktopWidth  = kapp->desktop()->width();
	int desktopHeight = kapp->desktop()->height();
	int tw = width();
	int th = height();

	// We are triying to make a live screenshot of the systray icon to circle it
	//  and show it to the user. If no systray is used or if the icon is not visible,
	//  we should not show that screenshot but only a text!

	// 1. Determine if the user use a system tray area or not:
	QByteArray screenstr;
	screenstr.setNum(qt_xscreen());
	QByteArrray trayatom = "_NET_SYSTEM_TRAY_S" + screenstr;
	bool useSystray = (KSelectionWatcher(trayatom).owner() != 0L);

	// 2. And then if the icon is visible too (eg. this->show() has been called):
	useSystray = useSystray && isVisible();

	// 3. Kicker (or another systray manager) can be visible but masked out of
	//    the screen (ie. on right or on left of it). We check if the icon isn't
	//    out of screen.
	if (useSystray) {
		QRect deskRect(0, 0, desktopWidth, desktopHeight);
		if ( !deskRect.contains(g.x(), g.y()) ||
		     !deskRect.contains(g.x() + tw, g.y() + th) )
			useSystray = false;
	}

	// 4. We raise the window containing the systray icon (typically the kicker) to
	//    have the most chances it is visible during the capture:
/*	if (useSystray) {
		// We are testing if one of the corners is hidden, and if yes, we would enter
		// a time consuming process (raise kicker and wait some time):
//		if (kapp->widgetAt(g) != this ||
//		    kapp->widgetAt(g + QPoint(tw-1, 0)) != this ||
//		    kapp->widgetAt(g + QPoint(0, th-1)) != this ||
//		    kapp->widgetAt(g + QPoint(tw-1, th-1)) != this) {
			int systrayManagerWinId = topLevelWidget()->winId();
			KWindowSystem::forceActiveWindow(systrayManagerWinId);
			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
//			KWindowSystem::activateWindow(systrayManagerWinId);
//			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
//				KWindowSystem::raiseWindow(systrayManagerWinId);
//			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
			sleep(1);
			// TODO: Re-verify that at least one corner is now visible
//		}
	}*/

//	KMessageBox::information(this, QString::number(g.x()) + ":" + QString::number(g.y()) + ":" +
//	                         QString::number((int)(kapp->widgetAt(g+QPoint(1,1)))));

	QString message = i18n(
		"<p>Closing the main window will keep %1 running in the system tray. "
		"Use <b>Quit</b> from the <b>Basket</b> menu to quit the application.</p>"
			,KGlobal::instance()->aboutData()->programName());
	// We are sure the systray icon is visible: ouf!
	if (useSystray) {
		// Compute size and position of the pixmap to be grabbed:
		int w = desktopWidth / 4;
		int h = desktopHeight / 9;
		int x = g.x() + tw/2 - w/2; // Center the rectange in the systray icon
		int y = g.y() + th/2 - h/2;
		if (x < 0)                 x = 0; // Move the rectangle to stay in the desktop limits
		if (y < 0)                 y = 0;
		if (x + w > desktopWidth)  x = desktopWidth - w;
		if (y + h > desktopHeight) y = desktopHeight - h;

		// Grab the desktop and draw a circle arround the icon:
		QPixmap shot = QPixmap::grabWindow(QX11Info::appRootWindow(), x, y, w, h);
		QPainter painter(&shot);
		const int CIRCLE_MARGINS = 6;
		const int CIRCLE_WIDTH   = 3;
		const int SHADOW_OFFSET  = 1;
		const int IMAGE_BORDER   = 1;
		int ax = g.x() - x - CIRCLE_MARGINS - 1;
		int ay = g.y() - y - CIRCLE_MARGINS - 1;
		painter.setPen( QPen(KApplication::palette().active().dark(), CIRCLE_WIDTH) );
		painter.drawArc(ax + SHADOW_OFFSET, ay + SHADOW_OFFSET,
		                tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
		painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, CIRCLE_WIDTH) );
		painter.drawArc(ax, ay, tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
#if 1
		// Draw the pixmap over the screenshot in case a window hide the icon:
		painter.drawPixmap(g.x() - x, g.y() - y + 1, *pixmap());
#endif
		painter.end();

		// Then, we add a border arround the image to make it more visible:
		QPixmap finalShot(w + 2*IMAGE_BORDER, h + 2*IMAGE_BORDER);
		finalShot.fill(KApplication::palette().active().foreground());
		painter.begin(&finalShot);
		painter.drawPixmap(IMAGE_BORDER, IMAGE_BORDER, shot);
		painter.end();

		// Associate source to image and show the dialog:
		Q3MimeSourceFactory::defaultFactory()->setPixmap("systray_shot", finalShot);
		KMessageBox::information(kapp->activeWindow(),
			message + "<p><center><img source=\"systray_shot\"></center></p>",
			i18n("Docking in System Tray"), "hideOnCloseInfo");
		Q3MimeSourceFactory::defaultFactory()->setData("systray_shot", 0L);
	} else {
		KMessageBox::information(kapp->activeWindow(),
			message,
			i18n("Docking in System Tray"), "hideOnCloseInfo");
	}
}
