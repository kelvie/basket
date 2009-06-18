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

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMouseEvent>

/** Class to pick a color on the screen
  * @author Sébastien Laoût
  */
class DesktopColorPicker : public QDesktopWidget
{
    Q_OBJECT
public:
    /** Construtor, initializer and destructor */
    DesktopColorPicker();
    ~DesktopColorPicker();
public slots:
    /** Begin color picking.
      * This function returns immediatly, and pickedColor() is emitted if user has
      * choosen a color, and not canceled the process (by pressing Escape).
      */
    void pickColor();
signals:
    /** When user picked a color, this signal is emitted.
      */
    void pickedColor(const QColor &color);
    /** When user cancel a picking (by pressing Escape), this signal is emitted.
      */
    void canceledPick();
protected slots:
    void slotDelayedPick();
protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool m_gettingColorFromScreen;
};

#endif // COLORPICKER_H
