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

#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qwhatsthis.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qbuttongroup.h>
#include <kstringhandler.h>

#include <ksqueezedtextlabel.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qinputdialog.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <qiconset.h>
#include <kaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kedittoolbar.h>
#include <kdebug.h>
#include <qsignalmapper.h>
#include <qstringlist.h>

#include <qpainter.h>
#include <qstyle.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <qtimer.h>
#include <qaction.h>
#include <kstdaccel.h>
#include <kglobalaccel.h>
#include <kkeydialog.h>
#include <kpassivepopup.h>
#include <kconfig.h>
#include <kcolordialog.h>
#include <kaboutdata.h>

#include <kdeversion.h>
#include <qdesktopwidget.h>
#include <kwin.h>

#include <kprogress.h>

#include "mainwindow.h"
#include "basket.h"
#include "basketproperties.h"
#include "note.h"
#include "settings.h"
#include "global.h"
//#include "addbasketwizard.h"
#include "newbasketdialog.h"
#include "basketfactory.h"
#include "popupmenu.h"
#include "xmlwork.h"
#include "debugwindow.h"
#include "notefactory.h"
#include "notedrag.h"
#include "tools.h"
#include "tag.h"
#include "formatimporter.h"
#include "softwareimporters.h"
#include "regiongrabber.h"
#include "password.h"
#include "bnpview.h"
#include "systemtray.h"
#include "clickablelabel.h"
#include "basketstatusbar.h"
#include <iostream>
#include <ksettings/dialog.h>

/** Container */

MainWindow::MainWindow(QWidget *parent, const char *name)
	: KMainWindow(parent, name != 0 ? name : "MainWindow"), m_settings(0), m_quit(false)
{
	DEBUG_WIN << "Baskets are loaded from " + Global::basketsFolder();

	BasketStatusBar* bar = new BasketStatusBar(statusBar());
	m_baskets = new BNPView(this, "BNPViewApp", this, actionCollection(), bar);
	setCentralWidget(m_baskets);

	setupGlobalShortcuts();
	setupActions();
	statusBar()->show();
	statusBar()->setSizeGripEnabled(true);

	setAutoSaveSettings(/*groupName=*/QString::fromLatin1("MainWindow"), /*saveWindowSize=*/false);

	m_actShowToolbar->setChecked(   toolBar()->isShown()   );
	m_actShowStatusbar->setChecked( statusBar()->isShown() );

	m_tryHideTimer = new QTimer(this);
	m_hideTimer    = new QTimer(this);
	connect( m_tryHideTimer, SIGNAL(timeout()), this, SLOT(timeoutTryHide()) );
	connect( m_hideTimer,    SIGNAL(timeout()), this, SLOT(timeoutHide())    );
	connect( m_baskets,      SIGNAL(setWindowCaption(const QString &)), this, SLOT(setCaption(const QString &)));

	createGUI("basketui.rc");
}

MainWindow::~MainWindow()
{
	delete m_settings;
}

void MainWindow::setupGlobalShortcuts()
{
	/* Global shortcuts */
	KGlobalAccel *globalAccel = Global::globalAccel; // Better for the following lines

	globalAccel->insert( "global_show_hide_main_window", i18n("Show/hide main window"),
							i18n("Allows you to show main Window if it is hidden, and to hide it if it is shown."),
							Qt::CTRL+Qt::SHIFT+Qt::Key_W, Qt::CTRL+Qt::SHIFT+Qt::Key_W,
							this, SLOT(changeActive()),             true, true );
}

void MainWindow::setupActions()
{
	actQuit         = KStdAction::quit( this, SLOT(quit()), actionCollection() );
	new KAction(i18n("Minimize"), "", 0,
				this, SLOT(minimizeRestore()), actionCollection(), "minimizeRestore" );
	/** Settings : ************************************************************/
	m_actShowToolbar   = KStdAction::showToolbar(   this, SLOT(toggleToolBar()),   actionCollection());
    m_actShowStatusbar = KStdAction::showStatusbar( this, SLOT(toggleStatusBar()), actionCollection());

	m_actShowToolbar->setCheckedState( KGuiItem(i18n("Hide &Toolbar")) );

	(void) KStdAction::keyBindings( this, SLOT(showShortcutsSettingsDialog()), actionCollection() );

	actConfigGlobalShortcuts = KStdAction::keyBindings(this, SLOT(showGlobalShortcutsSettingsDialog()),
	                                                   actionCollection(), "options_configure_global_keybinding");
	actConfigGlobalShortcuts->setText(i18n("Configure &Global Shortcuts..."));

	(void) KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection() );

	//KAction *actCfgNotifs = KStdAction::configureNotifications(this, SLOT(configureNotifications()), actionCollection() );
	//actCfgNotifs->setEnabled(false); // Not yet implemented !

	actAppConfig = KStdAction::preferences( this, SLOT(showSettingsDialog()), actionCollection() );
}

void MainWindow::toggleToolBar()
{
	if (toolBar()->isVisible())
		toolBar()->hide();
	else
		toolBar()->show();

	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MainWindow::toggleStatusBar()
{
	if (statusBar()->isVisible())
		statusBar()->hide();
	else
		statusBar()->show();

	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MainWindow::configureToolbars()
{
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );

	KEditToolbar dlg(actionCollection());
	connect( &dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()) );
	dlg.exec();
}

void MainWindow::configureNotifications()
{
	// TODO
	// KNotifyDialog *dialog = new KNotifyDialog(this, "KNotifyDialog", false);
	// dialog->show();
}

void MainWindow::slotNewToolbarConfig() // This is called when OK or Apply is clicked
{
	// ...if you use any action list, use plugActionList on each here...
	createGUI();
	plugActionList( QString::fromLatin1("go_baskets_list"), actBasketsList);
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MainWindow::showSettingsDialog()
{
	if(m_settings == 0)
		m_settings = new KSettings::Dialog( this );
	m_settings->show();
}

void MainWindow::showShortcutsSettingsDialog()
{
	KKeyDialog::configure(actionCollection(), "basketui.rc");
	//.setCaption(..)
	//actionCollection()->writeSettings();
}

void MainWindow::showGlobalShortcutsSettingsDialog()
{
	KKeyDialog::configure(Global::globalAccel);
	//.setCaption(..)
	Global::globalAccel->writeSettings();
}

void MainWindow::changedSelectedNotes()
{
//	tabChanged(0); // FIXME: NOT OPTIMIZED
}

/*void MainWindow::areSelectedNotesCheckedChanged(bool checked)
{
	m_actCheckNotes->setChecked(checked && currentBasket()->showCheckBoxes());
}*/

void MainWindow::polish()
{
	bool shouldSave = false;

	// If position and size has never been set, set nice ones:
	//  - Set size to sizeHint()
	//  - Keep the window manager placing the window where it want and save this
	if (Settings::mainWindowSize().isEmpty()) {
//		std::cout << "Main Window Position: Initial Set in show()" << std::endl;
		int defaultWidth  = kapp->desktop()->width()  * 5 / 6;
		int defaultHeight = kapp->desktop()->height() * 5 / 6;
		resize(defaultWidth, defaultHeight); // sizeHint() is bad (too small) and we want the user to have a good default area size
		shouldSave = true;
	} else {
//		std::cout << "Main Window Position: Recall in show(x="
//		          << Settings::mainWindowPosition().x() << ", y=" << Settings::mainWindowPosition().y()
//		          << ", width=" << Settings::mainWindowSize().width() << ", height=" << Settings::mainWindowSize().height()
//		          << ")" << std::endl;
		move(Settings::mainWindowPosition());
		resize(Settings::mainWindowSize());
	}

	KMainWindow::polish();

	if (shouldSave) {
//		std::cout << "Main Window Position: Save size and position in show(x="
//		          << pos().x() << ", y=" << pos().y()
//		          << ", width=" << size().width() << ", height=" << size().height()
//		          << ")" << std::endl;
		Settings::setMainWindowPosition(pos());
		Settings::setMainWindowSize(size());
		Settings::saveConfig();
	}
}

void MainWindow::resizeEvent(QResizeEvent*)
{
//	std::cout << "Main Window Position: Save size in resizeEvent(width=" << size().width() << ", height=" << size().height() << ") ; isMaximized="
//	          << (isMaximized() ? "true" : "false") << std::endl;
	Settings::setMainWindowSize(size());
	Settings::saveConfig();
}

void MainWindow::moveEvent(QMoveEvent*)
{
//	std::cout << "Main Window Position: Save position in moveEvent(x=" << pos().x() << ", y=" << pos().y() << ")" << std::endl;
	Settings::setMainWindowPosition(pos());
	Settings::saveConfig();
}

bool MainWindow::queryExit()
{
	hide();
	return true;
}

#include <qdesktopwidget.h>
#include <qmime.h>
#include <qpainter.h>
// To know the program name:
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

/** Scenario of "Hide main window to system tray icon when mouse move out of the window" :
  * - At enterEvent() we stop m_tryHideTimer
  * - After that and before next, we are SURE cursor is hovering window
  * - At leaveEvent() we restart m_tryHideTimer
  * - Every 'x' ms, timeoutTryHide() seek if cursor hover a widget of the application or not
  * - If yes, we musn't hide the window
  * - But if not, we start m_hideTimer to hide main window after a configured elapsed time
  * - timeoutTryHide() continue to be called and if cursor move again to one widget of the app, m_hideTimer is stopped
  * - If after the configured time cursor hasn't go back to a widget of the application, timeoutHide() is called
  * - It then hide the main window to systray icon
  * - When the user will show it, enterEvent() will be called the first time he enter mouse to it
  * - ...
  */

/** Why do as this ? Problems with the use of only enterEvent() and leaveEvent() :
  * - Resize window or hover titlebar isn't possible : leave/enterEvent
  *   are
  *   > Use the grip or Alt+rightDND to resize window
  *   > Use Alt+DND to move window
  * - Each menu trigger the leavEvent
  */

void MainWindow::enterEvent(QEvent*)
{
	m_tryHideTimer->stop();
	m_hideTimer->stop();
}

void MainWindow::leaveEvent(QEvent*)
{
	if (Settings::useSystray() && Settings::hideOnMouseOut())
		m_tryHideTimer->start(50);
}

void MainWindow::timeoutTryHide()
{
	// If a menu is displayed, do nothing for the moment
	if (kapp->activePopupWidget() != 0L)
		return;

	if (kapp->widgetAt(QCursor::pos()) != 0L)
		m_hideTimer->stop();
	else if ( ! m_hideTimer->isActive() ) // Start only one time
		m_hideTimer->start(Settings::timeToHideOnMouseOut() * 100, true);

	// If a sub-dialog is oppened, we musn't hide the main window:
	if (kapp->activeWindow() != 0L && kapp->activeWindow() != this)
		m_hideTimer->stop();
}

void MainWindow::timeoutHide()
{
	// We check that because the setting can have been set to off
	if (Settings::useSystray() && Settings::hideOnMouseOut())
		m_baskets->setActive(false);
	m_tryHideTimer->stop();
}

void MainWindow::quit()
{
	m_quit = true;
	close();
}

bool MainWindow::queryClose()
{
/*	if (m_shuttingDown) // Set in askForQuit(): we don't have to ask again
	return true;*/

	if (kapp->sessionSaving()) {
		Settings::setStartDocked(false); // If queryClose() is called it's because the window is shown
		Settings::saveConfig();
		return true;
	}

	if (Settings::useSystray() && !m_quit) {
		Global::tray->displayCloseMessage(i18n("Basket"));
		hide();
		return false;
	} else
		return askForQuit();
}

bool MainWindow::askForQuit()
{
	QString message = i18n("<p>Do you really want to quit %1?</p>").arg(kapp->aboutData()->programName());
	if (Settings::useSystray())
		message += i18n("<p>Notice that you do not have to quit the application before ending your KDE session: "
				"it will be reloaded the next time you log in.</p>");

	int really = KMessageBox::warningContinueCancel( this, message, i18n("Quit Confirm"),
			KStdGuiItem::quit(), "confirmQuitAsking" );

	if (really == KMessageBox::Cancel)
	{
		m_quit = false;
		return false;
	}

	return true;
}

void MainWindow::minimizeRestore()
{
	if(isVisible())
		hide();
	else
		show();
}

#include "mainwindow.moc"
