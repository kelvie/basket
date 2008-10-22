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

#ifndef VARIOUSWIDGETS_H
#define VARIOUSWIDGETS_H

#include <QWidget>
//Added by qt3to4:
#include <QResizeEvent>
#include <QKeyEvent>
#include <kcombobox.h>
#include <QDialog>
#include <kurllabel.h>
#include <qstring.h>
#include <KDialog>

class QLineEdit;
class QListWidgetItem;

class Basket;

/** A widget to select a command to run,
  * with a QLineEdit and a QPushButton.
  * @author Sébastien Laoût
  */
class RunCommandRequester : public QWidget
{
  Q_OBJECT
  public:
	RunCommandRequester(const QString &runCommand, const QString &message, QWidget *parent = 0, const char *name = 0);
	~RunCommandRequester();
	QString runCommand();
	void setRunCommand(const QString &runCommand);
	QLineEdit *lineEdit() { return m_runCommand; }
  private slots:
	void slotSelCommand();
  private:
	QLineEdit *m_runCommand;
	QString    m_message;
};

/** QComboBox to ask icon size
  * @author Sébastien Laoût
  */
class IconSizeCombo : public QComboBox
{
  Q_OBJECT
  public:
	IconSizeCombo(bool rw, QWidget *parent = 0, const char *name = 0);
	~IconSizeCombo();
	int iconSize();
	void setSize(int size);
};

/** A window that the user resize to graphically choose a new image size
  * TODO: Create a SizePushButton or even SizeWidget
  * @author Sébastien Laoût
  */
class ViewSizeDialog : public QDialog
{
  Q_OBJECT
  public:
	ViewSizeDialog(QWidget *parent, int w, int h);
	~ViewSizeDialog();
  private:
	virtual void resizeEvent(QResizeEvent *);
	QWidget *m_sizeGrip;
};

/** A label displaying a link that, once clicked, offer a What's This messageBox to help users.
  * @author Sébastien Laoût
  */
class HelpLabel : public KUrlLabel
{
  Q_OBJECT
  public:
	HelpLabel(const QString &text, const QString &message, QWidget *parent);
	~HelpLabel();
	QString message()                       { return m_message;    }
  public slots:
	void setMessage(const QString &message) { m_message = message; }
	void showMessage();
  protected:
	void keyPressEvent(QKeyEvent *event);
  private:
	QString m_message;
};

/** A dialog to choose the size of an icon.
  * @author Sébastien Laoût
  */
class IconSizeDialog : public KDialog
{
  Q_OBJECT
  public:
	IconSizeDialog(const QString &caption, const QString &message, const QString &icon, int iconSize, QWidget *parent);
	~IconSizeDialog();
	int iconSize() { return m_iconSize; } /// << @return the choosen icon size (16, 32, ...) or -1 if canceled!
  protected slots:
	void slotCancel();
	void slotSelectionChanged();
	void choose(QListWidgetItem*);
  private:
	QListWidgetItem *m_size16;
	QListWidgetItem *m_size22;
	QListWidgetItem *m_size32;
	QListWidgetItem *m_size48;
	QListWidgetItem *m_size64;
	QListWidgetItem *m_size128;
	int m_iconSize;
};

/**
 * A missing class from KDE (and Qt): a combobox to select a font size!
 */
class FontSizeCombo : public KComboBox
{
  Q_OBJECT
  public:
	FontSizeCombo(bool rw, bool withDefault, QWidget *parent = 0);
	~FontSizeCombo();
	void setFontSize(int size);
	int fontSize();
  protected:
	void keyPressEvent(QKeyEvent *event);
  signals:
	void sizeChanged(int size);
	void escapePressed();
	void returnPressed2();
  private slots:
	void textChangedInCombo(const QString &text);
  private:
	bool m_withDefault;
};

#endif // VARIOUSWIDGETS_H
