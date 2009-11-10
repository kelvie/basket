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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <KDE/KMainWindow>
#include <QTabBar>
#include <QTabWidget>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QPoint>
#include <QClipboard>
#include <KDE/KAction>
#include <QPixmap>
#include <QDesktopWidget>
#include <QTimer>
#include <QSplitter>
#include <KXmlGuiWindow>

class QWidget;
class QPoint;
class KAction;
class KToggleAction;
class QSignalMapper;
class QStringList;
class QToolTipGroup;
class KPassivePopup;
class BasketView;
class DecoratedBasket;
class Container;
class RegionGrabber;
class NoteSelection;
class BNPView;
class KActionCollection;

namespace KSettings
{
class Dialog;
};


/** The window that contain baskets, organized by tabs.
  * @author Sébastien Laoût
  */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /** Construtor, initializer and destructor */
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void setupActions();
public slots:
    bool askForQuit();
    /** Settings **/
//  void toggleToolBar();
    void toggleStatusBar();
    void showShortcutsSettingsDialog();
    void configureToolbars();
    void configureNotifications();
    void showSettingsDialog();
    void minimizeRestore();
    void quit();
    void slotNewToolbarConfig();

protected:
    bool queryExit();
    bool queryClose();
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);

public:
    void ensurePolished();

private:
    // Settings actions :
//  KToggleAction *m_actShowToolbar;
    KToggleAction *m_actShowStatusbar;
    KAction       *actQuit;
    KAction       *actAppConfig;
    QList<QAction *> actBasketsList;

private:
    QVBoxLayout        *m_layout;
    BNPView            *m_baskets;
    bool                m_startDocked;
    KSettings::Dialog  *m_settings;
    bool                m_quit;
};

#endif // CONTAINER_H
