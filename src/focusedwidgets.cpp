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

#include <QMenu>
#include <klocale.h>

#include <iostream>

#include "focusedwidgets.h"
#include "bnpview.h"
#include "global.h"
#include "basket.h"

#ifdef KeyPress
#undef KeyPress
#endif
#include <qevent.h>

/** class FocusedTextEdit */

FocusedTextEdit::FocusedTextEdit(bool disableUpdatesOnKeyPress, QWidget *parent, const char *name)
 : KTextEdit(name, parent),
   m_disableUpdatesOnKeyPress(disableUpdatesOnKeyPress)
{
//FIXME 1.5 dont exist any more	setWFlags(Qt::WNoAutoErase); // Does not work, we still need the disableUpdatesOnKeyPress hack!
}

FocusedTextEdit::~FocusedTextEdit()
{
}

/**
  * Thanks to alex.theel@gmx.net, author of TuxCards
  * Code copied from tuxcards-1.2/src/gui/editor/editor.cpp
  *
  ***
  * Override the regular paste() methode, so that lines are
  * not separated by each other with an blank line.
  */
/*TODO
void FocusedTextEdit::paste()
{
	adaptClipboardText(QClipboard::Selection);
	adaptClipboardText(QClipboard::Clipboard);

	// If we paste a application/x-qrichtext content starting with a "-" or a "*",
	// then auto-bulletting will crash.
	// So we insert a space to be sure what we paste will not trigger the auto-bulleting.

//	enum AutoFormatting { AutoNone = 0, AutoBulletList = 0x00000001, AutoAll = 0xffffffff }
//	uint oldAutoFormating = autoFormatting();
//	setAutoFormatting(AutoNone);

	QClipboard *clipboard = QApplication::clipboard();
	int paragraph;
	int index;
	getCursorPosition(&paragraph, &index);

	bool preventAutoBullet = (index == 0) &&
		(clipboard->data(QClipboard::Selection)->provides("application/x-qrichtext") ||
		 clipboard->data(QClipboard::Clipboard)->provides("application/x-qrichtext")   );

	if (preventAutoBullet)
		insert(" ");

	KTextEdit::paste();

	if (preventAutoBullet) {
		int paragraph2;
		int index2;
		getCursorPosition(&paragraph2, &index2);
		setSelection(paragraph, index, paragraph, index + 1);
		removeSelectedText();
		if (paragraph == paragraph2) // We removed one character in that paragraph, so we should move the cursor back to old position... minus one character
			index2--;
		setCursorPosition(paragraph2, index2);
	}


//	setAutoFormatting(oldAutoFormating);
}*/

/**
  * Thanks to alex.theel@gmx.net, author of TuxCards
  * Code copied from tuxcards-1.2/src/gui/editor/editor.cpp
  *
  ***
  * Auxiliar method that takes the text from the clipboard - using the
  * specified 'mode' -, replaces all '\n' within that text and writes
  * it back to the clipboard.
  */
/* TODO
void FocusedTextEdit::adaptClipboardText(QClipboard::Mode mode)
{
	QClipboard *clipboard = QApplication::clipboard();
	if (!clipboard)
		return;

	if ( (textFormat() == Qt::RichText) && (!clipboard->data(mode)->provides("application/x-qrichtext")) ) {
		QString text = clipboard->text(mode);
		if (text) {
			text = text.replace("\n", QChar(0x2028));
			clipboard->setText(text, mode);
		}
	}
}*/

QTextCursor* FocusedTextEdit::textCursor() const
{
	//TODO return KTextEdit::textCursor();
}


void FocusedTextEdit::keyPressEvent(QKeyEvent *event)
{
//	if (event->key() == Qt::Key_Escape) {
//		emit escapePressed();
//		return;
//	// In RichTextFormat mode, [Return] create a new paragraphe.
//	// To keep consistency with TextFormat mode (new line on [Return]),
//	// we redirect [Return] to simulate [Ctrl+Return] (create a new line in both modes).
//	// Create new paragraphes still possible in RichTextFormat mode with [Shift+Enter].
//	} else if (event->key() == Qt::Key_Return && event->state() == 0)
//		event = new QKeyEvent(QEvent::KeyPress, event->key(), event->ascii(), Qt::ControlModifier,
//		                      event->text(), event->isAutoRepeat(), event->count() );
//	else if (event->key() == Qt::Key_Return && event->state() & Qt::ControlModifier)
//		event = new QKeyEvent(QEvent::KeyPress, event->key(), event->ascii(), Qt::ShiftModifier,
//		                      event->text(), event->isAutoRepeat(), event->count() );
//
//	if (m_disableUpdatesOnKeyPress)
//		setUpdatesEnabled(false);
//	KTextEdit::keyPressEvent(event);
//	// Workarround (for ensuring the cursor to be visible): signal not emited when pressing those keys:
//	if (event->key() == Qt::Key_Home || event->key() == Qt::Key_End || event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
//		int para;
//		int index;
//		getCursorPosition(&para, &index);
//		emit cursorPositionChanged(para, index);
//	}
//	if (m_disableUpdatesOnKeyPress) {
//		setUpdatesEnabled(true);
//		if (text().isEmpty())
//			;// emit textChanged(); // TODO: DOESN'T WORK: the editor is not resized down to only one line of text
//		else
//			ensureCursorVisible();
//		updateContents();
//	}
}

void FocusedTextEdit::wheelEvent(QWheelEvent *event)
{
//	if (event->delta() > 0 && contentsY() > 0) {
//		KTextEdit::wheelEvent(event);
//		return;
//	} else if (event->delta() < 0 && contentsY() + visibleHeight() < contentsHeight()) {
//		KTextEdit::wheelEvent(event);
//		return;
//	} else
//		Global::bnpView->currentBasket()->wheelEvent(event);
}

void FocusedTextEdit::enterEvent(QEvent *event)
{
//	emit mouseEntered();
//	KTextEdit::enterEvent(event);
}

QMenu* FocusedTextEdit::createPopupMenu(const QPoint &pos)
{
//	QMenu *menu = KTextEdit::createPopupMenu(pos);
//
//	int index = 0;
//	int id = 0;
//	while (true) {
//		id = menu->idAt(index);
//		if (id == -1)
//			break;
//		// Disable Spell Check for rich text editors, because it doesn't work anyway:
//		if (textFormat() == Qt::RichText && (menu->text(id) == i18n("Auto Spell Check") || menu->text(id) == i18n("Check Spelling...")))
//			menu->setItemEnabled(id, false);
//		// Always enable tabulations!:
//		if (menu->text(id) == i18n("Allow Tabulations"))
//			menu->setItemEnabled(id, false);
//		index++;
//	}
//
//	// And return the menu:
//	return menu;
}

/** class FocusedColorCombo: */

FocusedColorCombo::FocusedColorCombo(QWidget *parent, const char *name)
 //TODO : KColorCombo(parent, name)
{
}

FocusedColorCombo::~FocusedColorCombo()
{
}

void FocusedColorCombo::keyPressEvent(QKeyEvent *event)
{
//	if (event->key() == Qt::Key_Escape)
//		emit escapePressed();
//	else if (event->key() == Qt::Key_Return)
//		emit returnPressed2();
//	else
//		KColorCombo::keyPressEvent(event);
}

/** class FocusedFontCombo: */

FocusedFontCombo::FocusedFontCombo(QWidget *parent, const char *name)
 //TODO : QFontComboBox(parent, name)
{
}

FocusedFontCombo::~FocusedFontCombo()
{
}

//TODO
void FocusedFontCombo::keyPressEvent(QKeyEvent *event)
{
//	if (event->key() == Qt::Key_Escape)
//		emit escapePressed();
//	else if (event->key() == Qt::Key_Return)
//		emit returnPressed2();
//	else
//		KQFontComboBox::keyPressEvent(event);
}

/** class FocusedComboBox: */

FocusedComboBox::FocusedComboBox(QWidget *parent, const char *name)
//TODO : KComboBox(parent, name)
{
}

FocusedComboBox::~FocusedComboBox()
{
}

void FocusedComboBox::keyPressEvent(QKeyEvent *event)
{
//	if (event->key() == Qt::Key_Escape)
//		emit escapePressed();
//	else if (event->key() == Qt::Key_Return)
//		emit returnPressed2();
//	else
//		KComboBox::keyPressEvent(event);
}

/** class FocusedLineEdit: */

FocusedLineEdit::FocusedLineEdit(QWidget *parent, const char *name)
	: KLineEdit( parent )
//FIXME 1.5 : KLineEdit(parent, name)
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

void FocusedLineEdit::enterEvent(QEvent *event)
{
	emit mouseEntered();
	KLineEdit::enterEvent(event);
}

#include "focusedwidgets.moc"
