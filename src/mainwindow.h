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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <kmainwindow.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <ksystemtray.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qclipboard.h>
#include <kaction.h>
#include <qpixmap.h>
#include <qdesktopwidget.h>
#include <qtimer.h>
#include <qsplitter.h>

class QWidget;
class QPoint;
class KAction;
class KToggleAction;
class QPopupMenu;
class QSignalMapper;
class QStringList;
class QToolTipGroup;
class KPassivePopup;
class Basket;
class DecoratedBasket;
class Container;
class RegionGrabber;
class NoteSelection;
class BNPView;
class ClickableLabel;
namespace KSettings { class Dialog; };


/** The window that contain baskets, organized by tabs.
  * @author S�astien Laot
  */
class MainWindow : public KMainWindow
{
  Q_OBJECT
  public:
	/** Construtor, initializer and destructor */
	MainWindow(QWidget *parent = 0, const char *name = 0);
	~MainWindow();
  private:
	void setupActions();
  public slots:
	bool askForQuit();
	/** Settings **/
	void toggleToolBar();
	void toggleStatusBar();
	void showShortcutsSettingsDialog();
	void showGlobalShortcutsSettingsDialog();
	void configureToolbars();
	void configureNotifications();
	void showSettingsDialog();
	void minimizeRestore();
	void quit();
	void changeActive();
	void slotNewToolbarConfig();

  protected:
	bool queryExit();
	bool queryClose();
	virtual void resizeEvent(QResizeEvent*);
	virtual void moveEvent(QMoveEvent*);
  public:
	void polish();

  private:
	// Settings actions :
	KToggleAction *m_actShowToolbar;
	KToggleAction *m_actShowStatusbar;
	KAction       *actQuit;
	KAction       *actConfigGlobalShortcuts;
	KAction       *actAppConfig;
	QPtrList<KAction> actBasketsList;

  private:
	QVBoxLayout        *m_layout;
	BNPView            *m_baskets;
	bool                m_startDocked;
	KSettings::Dialog  *m_settings;
	bool                m_quit;
};

#endif // CONTAINER_H
