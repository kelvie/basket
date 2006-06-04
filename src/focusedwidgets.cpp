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

#include <qpopupmenu.h>
#include <klocale.h>

#include <iostream>

#include "focusedwidgets.h"

/** class FocusedTextEdit */

FocusedTextEdit::FocusedTextEdit(bool escapeOnReturn, bool disableUpdatesOnKeyPress, QWidget *parent, const char *name)
 : KTextEdit(parent, name),
   m_escapeOnReturn(escapeOnReturn),
   m_disableUpdatesOnKeyPress(disableUpdatesOnKeyPress),
   m_discardNextFocusOut(false)
{
}

FocusedTextEdit::~FocusedTextEdit()
{
}

void FocusedTextEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		emit escapePressed();
		return;
	} else if (m_escapeOnReturn && event->key() == Qt::Key_Return && event->state() == 0) {
		emit escapePressed();
		return;
	// In RichTextFormat mode, [Return] create a new paragraphe.
	// To keep consistency with TextFormat mode (new line on [Return]),
	// we redirect [Return] to simulate [Ctrl+Return] (create a new line in both modes).
	// Create new paragraphes still possible in RichTextFormat mode with [Shift+Enter].
	} else if (event->key() == Qt::Key_Return && event->state() == 0)
		event = new QKeyEvent(QEvent::KeyPress, event->key(), event->ascii(), Qt::ControlButton,
		                      event->text(), event->isAutoRepeat(), event->count() );

	if (m_disableUpdatesOnKeyPress)
		setUpdatesEnabled(false);
	KTextEdit::keyPressEvent(event);
	if (m_disableUpdatesOnKeyPress) {
		setUpdatesEnabled(true);
		if (text().isEmpty())
			;// emit textChanged(); // TODO: DOESN'T WORK: the editor is not resized down to only one line of text
		else
			ensureCursorVisible();
		updateContents();
	}
}

void FocusedTextEdit::focusOutEvent(QFocusEvent *event)
{
	if (m_discardNextFocusOut)
		m_discardNextFocusOut = false;
/*4	else
		emit focusOut();
*/
	KTextEdit::focusOutEvent(event); //
}

#include "global.h"
#include "container.h"
#include "basket.h"

void FocusedTextEdit::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0 && contentsY() > 0) {
		KTextEdit::wheelEvent(event);
		return;
	} else if (event->delta() < 0 && contentsY() + visibleHeight() < contentsHeight()) {
		KTextEdit::wheelEvent(event);
		return;
	} else
		Global::mainContainer->currentBasket()->wheelEvent(event);
}

QPopupMenu* FocusedTextEdit::createPopupMenu(const QPoint &pos)
{
	// Disable the next focusOut event:
	m_discardNextFocusOut = true;
	QPopupMenu *menu = KTextEdit::createPopupMenu(pos);

	int index = 0;
	int id = 0;
	while (true) {
		id = menu->idAt(index);
		if (id == -1)
			break;
		// Disable the spell checking dialog since it would trigger a focusOut event:
		if (menu->text(id) == i18n("Check Spelling..."))
			menu->setItemEnabled(id, false);
		// Disable Spell Check for rich text editors, because it doesn't work anyway:
		if (textFormat() == Qt::RichText && menu->text(id) == i18n("Auto Spell Check"))
			menu->setItemEnabled(id, false);
		// Always enable tabulations!:
		if (menu->text(id) == i18n("Allow Tabulations"))
			menu->setItemEnabled(id, false);
		index++;
	}

	// And return the menu:
	return menu;
}

/** class FocusedComboBox: */

FocusedComboBox::FocusedComboBox(QWidget *parent, const char *name)
 : KComboBox(parent, name)
{
}

FocusedComboBox::~FocusedComboBox()
{
}

void FocusedComboBox::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit escapePressed();
	else if (event->key() == Qt::Key_Return)
		emit returnPressed2();
	else
		KComboBox::keyPressEvent(event);
}

/** class FocusedLineEdit: */

FocusedLineEdit::FocusedLineEdit(QWidget *parent, const char *name)
 : KLineEdit(parent, name)
{
}

FocusedLineEdit::~FocusedLineEdit()
{
}

void FocusedLineEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit escapePressed();
	else
		KLineEdit::keyPressEvent(event);
}

#include "focusedwidgets.moc"
