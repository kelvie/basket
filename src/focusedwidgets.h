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

#ifndef FOCUSEDWIDGETS_H
#define FOCUSEDWIDGETS_H

#include <ktextedit.h>
#include <kcolorcombo.h>
#include <kfontcombo.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kapplication.h>

/** The following widgets emit focusOut() signal as soon as they lost focus
  * FocusedTextEdit also emit escapePressed() when Escape key is pressed
  */

// FIXME: emit focusOut() for Escape key in ALL focused widgets ?

class FocusedTextEdit : public KTextEdit
{
  Q_OBJECT
  public:
	FocusedTextEdit(bool escapeOnReturn, bool disableUpdatesOnKeyPress, QWidget *parent = 0, const char *name = 0);
	~FocusedTextEdit();
  protected:
	void keyPressEvent(QKeyEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void wheelEvent(QWheelEvent *event);
	QPopupMenu* createPopupMenu(const QPoint &pos);
  signals:
	void focusOut();
	void escapePressed();
  private:
	bool m_escapeOnReturn;
	bool m_disableUpdatesOnKeyPress;
	bool m_discardNextFocusOut;
};

class FocusedColorCombo : public KColorCombo
{
  Q_OBJECT
  public:
	FocusedColorCombo(QWidget *parent = 0, const char *name = 0)
	 : KColorCombo(parent, name) {}
	~FocusedColorCombo()         {}
  protected:
	void focusOutEvent(QFocusEvent *event)
	{
/*1		if ( kapp->focusWidget() != 0L ) // When color dialog is called, focusWidget == 0L
			emit focusOut();             // FIXME: That's not so accurate but the only way :'-(
*/
		KColorCombo::focusOutEvent(event);
	}
  signals:
	void focusOut();
};

class FocusedFontCombo : public KFontCombo
{
  Q_OBJECT
  public:
	FocusedFontCombo(QWidget *parent = 0, const char *name = 0)
	 : KFontCombo(parent, name) {}
	~FocusedFontCombo()         {}
  protected:
	void focusOutEvent(QFocusEvent *event) { /*2 emit focusOut();*/ KFontCombo::focusOutEvent(event); }
  signals:
	void focusOut();
};

// TODO: Rename to EscapableKComboBox
class FocusedComboBox : public KComboBox
{
  Q_OBJECT
  public:
	FocusedComboBox(QWidget *parent = 0, const char *name = 0);
	~FocusedComboBox();
  protected:
	void keyPressEvent(QKeyEvent *event);
  signals:
	void escapePressed();
	void returnPressed2();
};

// TODO: Rename to EscapableKLineEdit
class FocusedLineEdit : public KLineEdit
{
  Q_OBJECT
  public:
	FocusedLineEdit(QWidget *parent = 0, const char *name = 0);
	~FocusedLineEdit();
  protected:
	void keyPressEvent(QKeyEvent *event);
  signals:
	void escapePressed();
};

#endif // FOCUSEDWIDGETS_H
