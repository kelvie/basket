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

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <ksystemtrayicon.h>

class MainWindow;

/** Convenient class to develop the displayCloseMessage() dialog
  * hopefuly integrated in KDE 3.4
  * @author S�astien Laot
  */
class KSystemTray2 : public KSystemTrayIcon
{
  Q_OBJECT
  public:
	KSystemTray2(QWidget *parent = 0, const char *name = 0);
	~KSystemTray2();
	/**
	  * Call this method when the user clicked the close button of the window
	  * (the [x]) to inform him that the application sit in the system tray
	  * and willn't be closed (as he is used to).
	  *
	  * You usualy call it from reimplemented KMainWindow::queryClose()
	  *
	  * @since 3.4
	  */
	void displayCloseMessage(QString fileMenu = "");
};

/** This class provide a personalized system tray icon.
  * @author S�astien Laot
  */
class SystemTray : public KSystemTray2
{
  Q_OBJECT
  public:
	SystemTray(QWidget *parent = 0, const char *name = 0);
	~SystemTray();
  protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent*);
	virtual void dropEvent(QDropEvent *event);
	void wheelEvent(QWheelEvent *event);
	void enterEvent(QEvent*);
	void leaveEvent(QEvent*);
  public slots:
	void updateToolTip();
  protected slots:
	void updateToolTipDelayed();
  signals:
	void showPart();
  private:
	QTimer    *m_showTimer;
	QTimer    *m_autoShowTimer;
	bool       m_canDrag;
	QPoint     m_pressPos;
	QPixmap    m_iconPixmap;
	QPixmap    m_lockedIconPixmap;
};

#endif // SYSTEMTRAY_H
