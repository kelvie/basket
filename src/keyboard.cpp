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

#include <kapplication.h> // it define Q_WS_X11

#include "keyboard.h"

/* This file contain modified code from klistbox.cpp
 */

// ShiftMask, ControlMask and Mod1Mask are defined in <X11/X.h>
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xlib.h> // schroder
#include <QX11Info>
#endif

void Keyboard::pressedKeys(bool &shiftPressed, bool &controlPressed)
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	Window root;
	Window child;
	int root_x, root_y, win_x, win_y;
	uint keybstate;
	XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
	               &root_x, &root_y, &win_x, &win_y, &keybstate );

	shiftPressed   = keybstate & ShiftMask;
	controlPressed = keybstate & ControlMask;
#endif
}

/** Same code as pressedKeys(...) but for shift key only
  */
bool Keyboard::shiftPressed()
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	Window root;
	Window child;
	int root_x, root_y, win_x, win_y;
	uint keybstate;
	XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
	               &root_x, &root_y, &win_x, &win_y, &keybstate );

	return (keybstate & ShiftMask) != 0;
#else
	return false;
#endif
}

/** Same code as pressedKeys(...) but for control key only
  */
bool Keyboard::controlPressed()
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	Window root;
	Window child;
	int root_x, root_y, win_x, win_y;
	uint keybstate;
	XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
	               &root_x, &root_y, &win_x, &win_y, &keybstate );

	return (keybstate & ControlMask) != 0;
#else
	return false;
#endif
}

/** Return if Alt key is pressed
  */
bool Keyboard::altPressed()
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	Window root;
	Window child;
	int root_x, root_y, win_x, win_y;
	uint keybstate;
	XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
	               &root_x, &root_y, &win_x, &win_y, &keybstate );

	return (keybstate & Mod1Mask) != 0;
#else
	return false;
#endif
}

/*******************
 * What does KDE 3.1 and later:
 * FIXME: Use this function in KDE 4
 *        (I wasn't knowing it by creating this class)

  / *
   * Returns the currently pressed keyboard modifiers (e.g. shift, control, etc.)
   * Usually you simply want to test for those in key events, in which case
   * QKeyEvent::state() does the job (or QKeyEvent::key() to
   * notice when a modifier is pressed alone).
   * But it can be useful to query for the status of the modifiers at another moment
   * (e.g. some KDE apps do that upon a drop event).
   * @return the keyboard modifiers
   * @since 3.1
    /
uint KApplication::keyboardModifiers()
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &keybstate );
    return keybstate & 0x00ff;
}

 *
 *******************/
