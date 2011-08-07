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

#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QtGui/QWidget>

class QVBoxLayout;
class QTextBrowser;
class QString;
class QCloseEvent;

/**A simple window that display text through debugging messages.
  *@author Sébastien Laoût
  */

class DebugWindow : public QWidget
{
    Q_OBJECT
public:
    /** Construtor and destructor */
    DebugWindow(QWidget *parent = 0);
    ~DebugWindow();
    /** Methods to post a message to the debug window */
    void postMessage(const QString msg);
    DebugWindow& operator<<(const QString msg);
    void insertHLine();
protected:
    virtual void closeEvent(QCloseEvent *event);
private:
    QVBoxLayout  *layout;
    QTextBrowser *textBrowser;
};

#define DEBUG_WIN if (Global::debugWindow) *Global::debugWindow

#endif // DEBUGWINDOW_H
