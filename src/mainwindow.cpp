/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
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

#include "mainwindow.h"

#include <QtGui/QMoveEvent>
#include <QtGui/QResizeEvent>

#include <KDE/KAction>
#include <KDE/KApplication>
#include <KDE/KLocale>
#include <KDE/KEditToolBar>
#include <KDE/KStatusBar>
#include <KDE/KDebug>
#include <KDE/KMessageBox>
#include <KDE/KConfig>
#include <KDE/KAboutData>
#include <KDE/KShortcutsDialog>
#include <KDE/KActionCollection>
#include <KDE/KToggleAction>

#include <KDE/KSettings/Dialog>

#include <kdeversion.h>

#include <QtGui/QDesktopWidget>

#include "settings.h"
#include "global.h"
#include "bnpview.h"
#include "basketstatusbar.h"

/** Container */

MainWindow::MainWindow(QWidget *parent)
        : KXmlGuiWindow(parent), m_settings(0), m_quit(false)
{
    BasketStatusBar* bar = new BasketStatusBar(statusBar());
    m_baskets = new BNPView(this, "BNPViewApp", this, actionCollection(), bar);
    setCentralWidget(m_baskets);

    setupActions();
    statusBar()->show();
    statusBar()->setSizeGripEnabled(true);

    setAutoSaveSettings(/*groupName=*/QString::fromLatin1("MainWindow"), /*saveWindowSize=*//*FIXME:false:Why was it false??*/true);

//  m_actShowToolbar->setChecked(   toolBar()->isVisible()   );
    m_actShowStatusbar->setChecked(statusBar()->isVisible());
    connect(m_baskets,      SIGNAL(setWindowCaption(const QString &)), this, SLOT(setCaption(const QString &)));

//  InlineEditors::instance()->richTextToolBar();
    setStandardToolBarMenuEnabled(true);

    createGUI("basketui.rc");
    KConfigGroup group = KGlobal::config()->group(autoSaveGroup());
    applyMainWindowSettings(group);
}

MainWindow::~MainWindow()
{
    KConfigGroup group = KGlobal::config()->group(autoSaveGroup());
    saveMainWindowSettings(group);
    delete m_settings;
    delete m_baskets;
}

void MainWindow::setupActions()
{
    actQuit         = KStandardAction::quit(this, SLOT(quit()), actionCollection());
    KAction *a = NULL;
    a = actionCollection()->addAction("minimizeRestore", this,
                                      SLOT(minimizeRestore()));
    a->setText(i18n("Minimize"));
    a->setIcon(KIcon(""));
    a->setShortcut(0);

    /** Settings : ************************************************************/
//  m_actShowToolbar   = KStandardAction::showToolbar(   this, SLOT(toggleToolBar()),   actionCollection());
    m_actShowStatusbar = KStandardAction::showStatusbar(this, SLOT(toggleStatusBar()), actionCollection());

//  m_actShowToolbar->setCheckedState( KGuiItem(i18n("Hide &Toolbar")) );

    (void) KStandardAction::keyBindings(this, SLOT(showShortcutsSettingsDialog()), actionCollection());

    (void) KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    //KAction *actCfgNotifs = KStandardAction::configureNotifications(this, SLOT(configureNotifications()), actionCollection() );
    //actCfgNotifs->setEnabled(false); // Not yet implemented !

    actAppConfig = KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
}

/*void MainWindow::toggleToolBar()
{
    if (toolBar()->isVisible())
        toolBar()->hide();
    else
        toolBar()->show();

    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}*/

void MainWindow::toggleStatusBar()
{
    if (statusBar()->isVisible())
        statusBar()->hide();
    else
        statusBar()->show();

    KConfigGroup group = KGlobal::config()->group(autoSaveGroup());
    saveMainWindowSettings(group);
}

void MainWindow::configureToolbars()
{
    KConfigGroup group = KGlobal::config()->group(autoSaveGroup());
    saveMainWindowSettings(group);

    KEditToolBar dlg(actionCollection());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
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
    createGUI("basketui.rc"); // TODO: Reconnect tags menu aboutToShow() ??
    if (!Global::bnpView->isPart())
        Global::bnpView->connectTagsMenu(); // The Tags menu was created again!
    // TODO: Does this do anything?
    plugActionList(QString::fromLatin1("go_baskets_list"), actBasketsList);
    KConfigGroup group = KGlobal::config()->group(autoSaveGroup());
    applyMainWindowSettings(group);
}

void MainWindow::showSettingsDialog()
{
    if (m_settings == 0)
        m_settings = new KSettings::Dialog(kapp->activeWindow());
    if (Global::mainWindow()) {
        m_settings->showButton(KDialog::Help,    false); // Not implemented!
        m_settings->showButton(KDialog::Default, false); // Not implemented!
        m_settings->exec();
    } else
        m_settings->show();
}

void MainWindow::showShortcutsSettingsDialog()
{
    KShortcutsDialog::configure(actionCollection());
    //.setCaption(..)
    //actionCollection()->writeSettings();
}

void MainWindow::ensurePolished()
{
    bool shouldSave = false;

    // If position and size has never been set, set nice ones:
    //  - Set size to sizeHint()
    //  - Keep the window manager placing the window where it want and save this
    if (Settings::mainWindowSize().isEmpty()) {
//      kDebug() << "Main Window Position: Initial Set in show()";
        int defaultWidth  = kapp->desktop()->width()  * 5 / 6;
        int defaultHeight = kapp->desktop()->height() * 5 / 6;
        resize(defaultWidth, defaultHeight); // sizeHint() is bad (too small) and we want the user to have a good default area size
        shouldSave = true;
    } else {
//      kDebug() << "Main Window Position: Recall in show(x="
//                << Settings::mainWindowPosition().x() << ", y=" << Settings::mainWindowPosition().y()
//                << ", width=" << Settings::mainWindowSize().width() << ", height=" << Settings::mainWindowSize().height()
//                << ")";
        //move(Settings::mainWindowPosition());
        //resize(Settings::mainWindowSize());
    }

    KXmlGuiWindow::ensurePolished();

    if (shouldSave) {
//      kDebug() << "Main Window Position: Save size and position in show(x="
//                << pos().x() << ", y=" << pos().y()
//                << ", width=" << size().width() << ", height=" << size().height()
//                << ")";
        Settings::setMainWindowPosition(pos());
        Settings::setMainWindowSize(size());
        Settings::saveConfig();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
//  kDebug() << "Main Window Position: Save size in resizeEvent(width=" << size().width() << ", height=" << size().height() << ") ; isMaximized="
//            << (isMaximized() ? "true" : "false");
    Settings::setMainWindowSize(size());
    Settings::saveConfig();

    // Added to make it work (previous lines do not work):
    //saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KXmlGuiWindow::resizeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
//  kDebug() << "Main Window Position: Save position in moveEvent(x=" << pos().x() << ", y=" << pos().y() << ")";
    Settings::setMainWindowPosition(pos());
    Settings::saveConfig();

    // Added to make it work (previous lines do not work):
    //saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KXmlGuiWindow::moveEvent(event);
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
    /*  if (m_shuttingDown) // Set in askForQuit(): we don't have to ask again
        return true;*/

    if (kapp->sessionSaving()) {
        Settings::setStartDocked(false); // If queryClose() is called it's because the window is shown
        Settings::saveConfig();
        return true;
    }

    if (Settings::useSystray()
            && !m_quit
            && Global::systemTray->parentWidgetTrayClose()) {
        hide();
        return false;
    } else
        return askForQuit();
}

bool MainWindow::askForQuit()
{
    QString message = i18n("<p>Do you really want to quit %1?</p>", KGlobal::mainComponent().aboutData()->programName());
    if (Settings::useSystray())
        message += i18n("<p>Notice that you do not have to quit the application before ending your KDE session. "
                        "If you end your session while the application is still running, the application will be reloaded the next time you log in.</p>");

    int really = KMessageBox::warningContinueCancel(this, message,
                 i18n("Quit Confirm"),
                 KStandardGuiItem::quit(),
                 KStandardGuiItem::cancel(),
                 "confirmQuitAsking");

    if (really == KMessageBox::Cancel) {
        m_quit = false;
        return false;
    }

    return true;
}

void MainWindow::minimizeRestore()
{
    if (isVisible())
        hide();
    else
        show();
}

