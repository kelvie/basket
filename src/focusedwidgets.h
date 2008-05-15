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
#include <qclipboard.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>
#include <QWheelEvent>

class FocusedTextEdit : public KTextEdit
{
  Q_OBJECT
  public:
	FocusedTextEdit(bool disableUpdatesOnKeyPress, QWidget *parent = 0, const char *name = 0);
	~FocusedTextEdit();
	void paste();
	QTextCursor* textCursor() const;
  protected:
	void adaptClipboardText(QClipboard::Mode mode);
	void keyPressEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);
	void enterEvent(QEvent *event);
	KMenu* createPopupMenu(const QPoint &pos);
  signals:
	void escapePressed();
	void mouseEntered();
  private:
	bool m_disableUpdatesOnKeyPress;
};

// TODO: Rename to EscapableKColorCombo
class FocusedColorCombo : public KColorCombo
{
  Q_OBJECT
  public:
	FocusedColorCombo(QWidget *parent = 0, const char *name = 0);
	~FocusedColorCombo();
  protected:
	void keyPressEvent(QKeyEvent *event);
  signals:
	void escapePressed();
	void returnPressed2();
};

// TODO: Rename to EscapableKFontCombo
class FocusedFontCombo : public KFontCombo
{
  Q_OBJECT
  public:
	FocusedFontCombo(QWidget *parent = 0, const char *name = 0);
	~FocusedFontCombo();
  protected:
	void keyPressEvent(QKeyEvent *event);
  signals:
	void escapePressed();
	void returnPressed2();
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
	void enterEvent(QEvent *event);
  signals:
	void escapePressed();
	void mouseEntered();
};

#endif // FOCUSEDWIDGETS_H
