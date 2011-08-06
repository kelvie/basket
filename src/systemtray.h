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

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <KDE/KSystemTrayIcon>

/** A thin wrapper around KSystemTrayIcon until the old SystemTray is ported.
 * As things are ported, items should
 * @author Kelvie Wong
 */
class SystemTray : public KSystemTrayIcon
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemTray);

public:
    SystemTray(QWidget *parent = 0);
    ~SystemTray();

public slots:
    void updateDisplay();

signals:
    void showPart();

private:
    QSize m_iconSize;
    QIcon m_icon;
    QIcon m_lockedIcon;
};

#ifdef USE_OLD_SYSTRAY

/** This class provide a personalized system tray icon.
  * @author Sébastien Laoût
  */
class SystemTray2 : public SystemTray
{
    Q_OBJECT
public:
    explicit SystemTray2(QWidget *parent = 0, const char *name = 0);
    ~SystemTray2();
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

private:
    QTimer    *m_showTimer;
    QTimer    *m_autoShowTimer;
    bool       m_canDrag;
    QPoint     m_pressPos;
};
#endif // USE_OLD_SYSTRAY

#endif // SYSTEMTRAY_H
