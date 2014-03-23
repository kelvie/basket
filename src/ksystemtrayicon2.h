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

#ifndef K_SYSTEM_TRAY_ICON
#define K_SYSTEM_TRAY_ICON

#include <QWidget>
#include <KSystemTrayIcon>

/** Convenient class to develop the displayCloseMessage() dialog
  * hopefuly integrated in KDE 3.4
  * @author Sébastien Laoût
  */
class KSystemTray2 : public KSystemTrayIcon, public QWidget
{
    Q_OBJECT
public:
    explicit KSystemTray2(QWidget *parent = 0, const char *name = 0);
    ~KSystemTray2();
    /**
      * Call this method when the user clicked the close button of the window
      * (the [x]) to inform him that the application sit in the system tray
      * and willn't be closed (as he is used to).
      *
      * You usually call it from reimplemented KMainWindow::queryClose()
      *
      * @since 3.4
      */
    void displayCloseMessage(QString fileMenu = "");
};


#endif // K_SYSTEM_TRAY_ICON
