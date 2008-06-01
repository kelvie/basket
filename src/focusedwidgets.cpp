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

#include <q3popupmenu.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QKeyEvent>
#include <klocale.h>

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
	// In RichTextFormat mode, [Return] create a new paragraphe.
	// To keep consistency with TextFormat mode (new line on [Return]),
	// we redirect [Return] to simulate [Ctrl+Return] (create a new line in both modes).
	// Create new paragraphes still possible in RichTextFormat mode with [Shift+Enter].
	} else if (event->key() == Qt::Key_Return && event->state() == 0)
		event = new QKeyEvent(QEvent::KeyPress, event->key(), event->ascii(), Qt::ControlModifier,
		                      event->text(), event->isAutoRepeat(), event->count() );
	else if (event->key() == Qt::Key_Return && event->state() & Qt::ControlModifier)
		event = new QKeyEvent(QEvent::KeyPress, event->key(), event->ascii(), Qt::ShiftModifier,
		                      event->text(), event->isAutoRepeat(), event->count() );

	if (m_disableUpdatesOnKeyPress)
		setUpdatesEnabled(false);
	KTextEdit::keyPressEvent(event);
	// Workarround (for ensuring the cursor to be visible): signal not emited when pressing those keys:
	if (event->key() == Qt::Key_Home || event->key() == Qt::Key_End || event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
		int para;
		int index;
		getCursorPosition(&para, &index);
		emit cursorPositionChanged(para, index);
	}
	if (m_disableUpdatesOnKeyPress) {
		setUpdatesEnabled(true);
		if (text().isEmpty())
			;// emit textChanged(); // TODO: DOESN'T WORK: the editor is not resized down to only one line of text
		else
			ensureCursorVisible();
		updateContents();
	}
}

void FocusedTextEdit::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0 && contentsY() > 0) {
		KTextEdit::wheelEvent(event);
		return;
	} else if (event->delta() < 0 && contentsY() + visibleHeight() < contentsHeight()) {
		KTextEdit::wheelEvent(event);
		return;
	} else
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
    case QEvent::KeyPress:
    {
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

#include "focusedwidgets.moc"
