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
#include <kcmultidialog.h>

/** Container */

MainWindow::MainWindow(QWidget *parent, const char *name)
	: KMainWindow(parent, name != 0 ? name : "MainWindow"), m_settings(0), m_quit(false)
{
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
	createGUI(); // TODO: Reconnect tags menu aboutToShow() ??
	if (!Global::runInsideKontact())
		Global::bnpView->connectTagsMenu(); // The Tags menu was created again!
	plugActionList( QString::fromLatin1("go_baskets_list"), actBasketsList);
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MainWindow::showSettingsDialog()
{
	if(m_settings == 0)
		m_settings = new KSettings::Dialog(kapp->activeWindow());
	if (Global::mainWindow()) {
		m_settings->dialog()->showButton(KDialogBase::Help,    false); // Not implemented!
		m_settings->dialog()->showButton(KDialogBase::Default, false); // Not implemented!
		m_settings->dialog()->exec();
	} else
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
		Global::systemTray->displayCloseMessage(i18n("Basket"));
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

void MainWindow::changeActive()
{
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
	kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
	Global::systemTray->toggleActive();
#else
	setActive( ! isActiveWindow() );
#endif
}

#include "mainwindow.moc"
