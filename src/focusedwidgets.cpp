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

#include <QWheelEvent>
#include <QKeyEvent>
#include <KDE/KLocale>

#include "focusedwidgets.h"
#include "bnpview.h"
#include "global.h"
#include "basket.h"

#ifdef KeyPress
#undef KeyPress
#endif
#include <qevent.h>

/** class FocusedTextEdit */

FocusedTextEdit::FocusedTextEdit(bool disableUpdatesOnKeyPress,
                                 QWidget *parent)
        : KTextEdit(parent),
        m_disableUpdatesOnKeyPress(disableUpdatesOnKeyPress)
{
    // pass
}

FocusedTextEdit::~FocusedTextEdit()
{
}

void FocusedTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit escapePressed();
        return;
    }

    if (m_disableUpdatesOnKeyPress)
        setUpdatesEnabled(false);

    KTextEdit::keyPressEvent(event);

    // Workaround (for ensuring the cursor to be visible): signal not emited when pressing those keys:
    if (event->key() == Qt::Key_Home
            || event->key() == Qt::Key_End
            || event->key() == Qt::Key_PageUp
            || event->key() == Qt::Key_PageDown)
        emit cursorPositionChanged();


    if (m_disableUpdatesOnKeyPress) {
        setUpdatesEnabled(true);
        if (!document()->isEmpty())
            ensureCursorVisible();
    }
}

void FocusedTextEdit::wheelEvent(QWheelEvent *event)
{
    // If we're already scrolled all the way to the top or bottom, we pass the
    // wheel event onto the basket.
    QScrollBar *sb = verticalScrollBar();
    if ((event->delta() > 0 && sb->value() > sb->minimum())
            || (event->delta() < 0 && sb->value() < sb->maximum()))
        KTextEdit::wheelEvent(event);
    else
        Global::bnpView->currentBasket()->wheelEvent(event);
}

void FocusedTextEdit::enterEvent(QEvent *event)
{
    emit mouseEntered();
    KTextEdit::enterEvent(event);
}

/** class FocusWidgetFilter */
FocusWidgetFilter::FocusWidgetFilter(QWidget *parent)
        : QObject(parent)
{
    if (parent)
        parent->installEventFilter(this);
}

bool FocusWidgetFilter::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        switch (ke->key()) {
        case Qt::Key_Return:
            emit returnPressed();
            return true;
        case Qt::Key_Escape:
            emit escapePressed();
            return true;
        default:
            return false;
        };
    }
    case QEvent::Enter:
        emit mouseEntered();
        // pass through
    default:
        return false;
    };
}
