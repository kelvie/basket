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

#include "colorpicker.h"

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

#include <KDE/KColorDialog>

/// ///

/** DektopColorPicker */

/* From Qt documentation:
 * " Note that only visible widgets can grab mouse input.
 *   If isVisible() returns FALSE for a widget, that widget cannot call grabMouse(). "
 * So, we should use an always visible widget to be able to pick a color from screen,
 * even by first hidding the main window (user seldomly want to grab a color from BasKet!)
 * or use a global shortcut (main window can be hidden when hitting that shortcut).
 */

DesktopColorPicker::DesktopColorPicker()
        : QDesktopWidget()
{
    setObjectName("DesktopColorPicker");
    m_gettingColorFromScreen = false;
}

DesktopColorPicker::~DesktopColorPicker()
{
}

void DesktopColorPicker::pickColor()
{
    m_gettingColorFromScreen = true;
//  Global::mainContainer->setActive(false);
    QTimer::singleShot(50, this, SLOT(slotDelayedPick()));
}

/* When firered from basket context menu, and not from menu, grabMouse doesn't work!
 * It's perhapse because context menu call slotColorFromScreen() and then
 * ungrab the mouse (since menus grab the mouse).
 * But why isn't there such bug with normal menus?...
 * By calling this method with a QTimer::singleShot, we are sure context menu code is
 * finished and we can grab the mouse without loosing the grab:
 */
void DesktopColorPicker::slotDelayedPick()
{
    grabKeyboard();
    grabMouse(Qt::CrossCursor);
}

/* Validate the color
 */
void DesktopColorPicker::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_gettingColorFromScreen) {
        m_gettingColorFromScreen = false;
        releaseMouse();
        releaseKeyboard();
        QColor color = KColorDialog::grabColor(event->globalPos());
        emit pickedColor(color);
    } else
        QDesktopWidget::mouseReleaseEvent(event);
}

/* Cancel the mode
 */
void DesktopColorPicker::keyPressEvent(QKeyEvent *event)
{
    if (m_gettingColorFromScreen)
        if (event->key() == Qt::Key_Escape) {
            m_gettingColorFromScreen = false;
            releaseMouse();
            releaseKeyboard();
            emit canceledPick();
        }
    QDesktopWidget::keyPressEvent(event);
}

