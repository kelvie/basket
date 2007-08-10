/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
 *   slaout@linux62.org                                                    *
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

/** KSystemTray2 */

// To draw the systray screenshot image:
#include <QDockWidget>
#include <qmovie.h>
#include <qvariant.h>
#include "linklabel.h"
#include "note.h"

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
#include "systemtray.h"
#include "basket.h"
#include "settings.h"
#include "global.h"
#include "tools.h"
#include <QWheelEvent>
#include <kdialog.h>
#include <QIcon>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <QX11Info>
KSystemTray2::KSystemTray2(QWidget *parent)
 : KSystemTrayIcon(parent)
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
	QPoint g = parentWidget()->mapToGlobal(parentWidget()->pos());
	int desktopWidth  = kapp->desktop()->width();
	int desktopHeight = kapp->desktop()->height();
	int tw = geometry().width();
	int th = geometry().height();

	// We are triying to make a live screenshot of the systray icon to circle it
	//  and show it to the user. If no systray is used or if the icon is not visible,
	//  we should not show that screenshot but only a text!

	// 1. Determine if the user use a system tray area or not:
	QByteArray screenstr;
	screenstr.setNum(QX11Info::appScreen ());
	QByteArray trayatom = "_NET_SYSTEM_TRAY_S" + screenstr;
	bool useSystray = (KSelectionWatcher(trayatom.data()).owner() != 0L);

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
			).arg(KGlobal::mainComponent().aboutData()->programName());
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
		painter.setPen( QPen(KApplication::palette().dark(), CIRCLE_WIDTH) );
		painter.drawArc(ax + SHADOW_OFFSET, ay + SHADOW_OFFSET,
		                tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
		painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, CIRCLE_WIDTH) );
		painter.drawArc(ax, ay, tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
#if 1
		// Draw the pixmap over the screenshot in case a window hide the icon:
		painter.drawPixmap(g.x() - x, g.y() - y + 1, shot);
#endif
		painter.end();

		// Then, we add a border arround the image to make it more visible:
		QPixmap finalShot(w + 2*IMAGE_BORDER, h + 2*IMAGE_BORDER);
		finalShot.fill(KApplication::palette().color(QPalette::Foreground));
		painter.begin(&finalShot);
		painter.drawPixmap(IMAGE_BORDER, IMAGE_BORDER, shot);
		painter.end();

		// Associate source to image and show the dialog:
//FIXME 1.5		QMimeSourceFactory::defaultFactory()->setPixmap("systray_shot", finalShot);
		KMessageBox::information(kapp->activeWindow(),
			message + "<p><center><img source=\"systray_shot\"></center></p>",
			i18n("Docking in System Tray"), "hideOnCloseInfo");
// FIXME 1.5		QMimeSourceFactory::defaultFactory()->setData("systray_shot", 0L);
	} else {
		KMessageBox::information(kapp->activeWindow(),
			message,
			i18n("Docking in System Tray"), "hideOnCloseInfo");
	}
}

/** SystemTray */

SystemTray::SystemTray(QWidget *parent)
 : KSystemTray2(parent), m_showTimer(0), m_autoShowTimer(0)
{
	setObjectName("SystemTray");
// FIXME 1.5	setAcceptDrops(true);

	m_showTimer = new QTimer(this);
	connect( m_showTimer, SIGNAL(timeout()), Global::bnpView, SLOT(setActive()) );

	m_autoShowTimer = new QTimer(this);
	connect( m_autoShowTimer, SIGNAL(timeout()), Global::bnpView, SLOT(setActive()) );

	// Create pixmaps for the icon:
	m_iconPixmap              = KIcon("basket").pixmap(32);
//	FIXME: When main window is shown at start, the icon is loaded 1 pixel too high
//	       and then reloaded instantly after at the right position.
//	setPixmap(m_iconPixmap); // Load it the sooner as possible to avoid flicker
	QImage  lockedIconImage   = m_iconPixmap.toImage();
	QPixmap lockOverlayPixmap = loadIcon("lockoverlay").pixmap(32);// FIXME 1.5
	QImage  lockOverlayImage  = lockOverlayPixmap.toImage();
	KIconEffect::overlay(lockedIconImage, lockOverlayImage);
	m_lockedIconPixmap.fromImage(lockedIconImage);

	updateToolTip(); // Set toolTip AND icon
}

SystemTray::~SystemTray()
{
}

void SystemTray::mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {          // Prepare drag
		m_pressPos = event->globalPos();
		m_canDrag  = true;
		event->accept();
	} else if (event->button() & Qt::MidButton) {    // Paste
		Global::bnpView->currentBasket()->setInsertPopupMenu();
		Global::bnpView->currentBasket()->pasteNote(QClipboard::Selection);
		Global::bnpView->currentBasket()->cancelInsertPopupMenu();
		if (Settings::usePassivePopup())
			Global::bnpView->showPassiveDropped(i18n("Pasted selection to basket <i>%1</i>"));
		event->accept();
	} else if (event->button() & Qt::RightButton) { // Popup menu
		KMenu menu(parentWidget());
		setContextMenu(qobject_cast<QMenu*>(&menu)); 
		menu.setTitle( KCmdLineArgs::aboutData( )->programName() );
		menu.setIcon(KIcon("basket"));
		menu.addAction(Global::bnpView->actNewBasket);
		menu.addAction(Global::bnpView->actNewSubBasket);
		menu.addAction(Global::bnpView->actNewSiblingBasket);
			menu.insertSeparator(menu.actions().last());
		menu.addAction(Global::bnpView->m_actPaste);
		menu.addAction(Global::bnpView->m_actGrabScreenshot);
		menu.addAction(Global::bnpView->m_actColorPicker);

		if(!Global::bnpView->isPart())
		{
			QAction* action;

			menu.insertSeparator(menu.actions().last());

			action = Global::bnpView->actionCollection()->action("options_configure_global_keybinding");
			if(action)
				menu.addAction(action);

			action = Global::bnpView->actionCollection()->action("options_configure");
			if(action)
				menu.addAction(action);

			menu.insertSeparator(menu.actions().last());

			// Minimize / restore : since we manage the popup menu by ourself, we should do that work :
			action = Global::bnpView->actionCollection()->action("minimizeRestore");
			if(action)
			{
				if (Global::mainWindow()->isVisible())
					action->setText(i18n("&Minimize"));
				else
					action->setText(i18n("&Restore"));
				menu.addAction(action);
			}

			action = Global::bnpView->actionCollection()->action("file_quit");
			if(action)
				menu.addAction(action);
		}

		Global::bnpView->currentBasket()->setInsertPopupMenu();
		connect( &menu, SIGNAL(aboutToHide()), Global::bnpView->currentBasket(), SLOT(delayedCancelInsertPopupMenu()) );
		menu.exec(event->globalPos());
		event->accept();
	} else
		event->ignore();
}

void SystemTray::mouseMoveEvent(QMouseEvent *event)
{
	event->ignore();
}

void SystemTray::mouseReleaseEvent(QMouseEvent *event)
{
	m_canDrag = false;
	if (event->button() == Qt::LeftButton)         // Show / hide main window
		if ( parentWidget()->rect().contains(event->pos()) ) {     // Accept only if released in systemTray
			toggleActive();
			emit showPart();
			event->accept();
		} else
			event->ignore();
}

void SystemTray::dragEnterEvent(QDragEnterEvent *event)
{
	m_showTimer->setSingleShot(true);
	m_showTimer->start( Settings::dropTimeToShow() * 100 );
	Global::bnpView->currentBasket()->showFrameInsertTo();
///	m_parentContainer->setStatusBarDrag(); // FIXME: move this line in Basket::showFrameInsertTo() ?
	Basket::acceptDropEvent(event);
}

void SystemTray::dragMoveEvent(QDragMoveEvent *event)
{
	Basket::acceptDropEvent(event);
}

void SystemTray::dragLeaveEvent(QDragLeaveEvent*)
{
	m_showTimer->stop();
	m_canDrag = false;
	Global::bnpView->currentBasket()->resetInsertTo();
	Global::bnpView->updateStatusBarHint();
}

#include <iostream>

void SystemTray::dropEvent(QDropEvent *event)
{
	m_showTimer->stop();

	Global::bnpView->currentBasket()->blindDrop(event);

/*	Basket *basket = Global::bnpView->currentBasket();
	if (!basket->isLoaded()) {
		Global::bnpView->showPassiveLoading(basket);
		basket->load();
	}
	basket->contentsDropEvent(event);
	std::cout << (long) basket->selectedNotes() << std::endl;

	if (Settings::usePassivePopup())
		Global::bnpView->showPassiveDropped(i18n("Dropped to basket <i>%1</i>"));*/
}

/* This function comes directly from JuK: */

/*
 * This function copies the entirety of src into dest, starting in
 * dest at x and y.  This function exists because I was unable to find
 * a function like it in either QImage or kdefx
 */
static bool copyImage(QImage &dest, QImage &src, int x, int y)
{
	if(dest.depth() != src.depth())
		return false;
	if((x + src.width()) >= dest.width())
		return false;
	if((y + src.height()) >= dest.height())
		return false;

	// We want to use KIconEffect::overlay to do this, since it handles
	// alpha, but the images need to be the same size.  We can handle that.

	QImage large_src(dest);

	// It would perhaps be better to create large_src based on a size, but
	// this is the easiest way to make a new image with the same depth, size,
	// etc.

	large_src.detach();

	// However, we do have to specifically ensure that setAlphaBuffer is set
	// to false

//FIXME 1.5	large_src.setAlphaBuffer(false);
	large_src.fill(0); // All transparent pixels
//FIXME 1.5	large_src.setAlphaBuffer(true);

	int w = src.width();
	int h = src.height();
	for(int dx = 0; dx < w; dx++)
		for(int dy = 0; dy < h; dy++)
			large_src.setPixel(dx + x, dy + y, src.pixel(dx, dy));

	// Apply effect to image

	KIconEffect::overlay(dest, large_src);

	return true;
}

void SystemTray::updateToolTip()
{
//	return; /////////////////////////////////////////////////////

	Basket *basket = Global::bnpView->currentBasket();
	if (!basket)
		return;

	if (basket->icon().isEmpty() || basket->icon() == "basket" || ! Settings::showIconInSystray())
		setIcon(basket->isLocked() ? m_lockedIconPixmap : m_iconPixmap);
	else {
		// Code that comes from JuK:
		QPixmap bgPix = loadIcon("basket").pixmap(32);//FIXME 1.5 find the icon curent size
		QPixmap fgPix = SmallIcon(basket->icon());

		QImage bgImage = bgPix.toImage(); // Probably 22x22
		QImage fgImage = fgPix.toImage(); // Should be 16x16
		QImage lockOverlayImage = loadIcon("lockoverlay").pixmap(32).toImage();//FIXME 1.5 find the icon curent size

		KIconEffect::semiTransparent(bgImage);
		copyImage(bgImage, fgImage, (bgImage.width() - fgImage.width()) / 2,
				(bgImage.height() - fgImage.height()) / 2);
		if (basket->isLocked())
			KIconEffect::overlay(bgImage, lockOverlayImage);

		bgPix.fromImage(bgImage);
		setIcon(bgPix);
	}

	//QTimer::singleShot( Container::c_delayTooltipTime, this, SLOT(updateToolTipDelayed()) );
	// No need to delay: it's be called when notes are changed:
	updateToolTipDelayed();
}

void SystemTray::updateToolTipDelayed()
{
	Basket *basket = Global::bnpView->currentBasket();

	QString tip = "<p><nobr>" + ( basket->isLocked() ? KDialog::makeStandardCaption(i18n("%1 (Locked)"))
	                                                 : KDialog::makeStandardCaption(     "%1")          )
	                            .arg(Tools::textToHTMLWithoutP(basket->basketName()));

	this->setToolTip(tip);
}

void SystemTray::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0)
		Global::bnpView->goToPreviousBasket();
	else
		Global::bnpView->goToNextBasket();

	if (Settings::usePassivePopup())
		Global::bnpView->showPassiveContent();
}

void SystemTray::enterEvent(QEvent*)
{
	if (Settings::showOnMouseIn()){
	m_autoShowTimer->setSingleShot(true);
	m_autoShowTimer->start(Settings::timeToShowOnMouseIn() * 100);
	}
}

void SystemTray::leaveEvent(QEvent*)
{
	m_autoShowTimer->stop();
}

#include "systemtray.moc"
