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

#ifndef SOFTWAREIMPORTERS_H
#define SOFTWAREIMPORTERS_H

#include <qnamespace.h>
#include <kdialog.h>
#include <QRadioButton>

class QString;
class QGroupBox;
class QDomElement;
class KTextEdit;

class Basket;
class Note;

/** The dialog to ask how to import hierarchical data.
  * @author Sébastien Laoût
  */
class TreeImportDialog : public KDialog
{
  Q_OBJECT
  public:
	TreeImportDialog(QWidget *parent = 0);
	~TreeImportDialog();
	int choice();
  private:
	QGroupBox *m_choices;
};

/** The dialog to ask how to import text files.
  * @author Sébastien Laoût
  */
class TextFileImportDialog : public KDialog
{
  Q_OBJECT
  public:
	TextFileImportDialog(QWidget *parent = 0);
	~TextFileImportDialog();
	QString separator();
  protected slots:
	void customSeparatorChanged();
  private:
	QGroupBox *m_choices;
	QRadioButton  *m_anotherSeparator;
	QTextEdit     *m_customSeparator;
};

/** Functions that import data from other softwares.
  * @author Sébastien Laoût
  */
namespace SoftwareImporters
{
	// Useful methods to design importers:
	QString fromICS(const QString &ics);
	QString fromTomboy(QString tomboy);
	Note* insertTitledNote(Basket *parent, const QString &title, const QString &content, Qt::TextFormat format = Qt::PlainText, Note *parentNote = 0);
	void finishImport(Basket *basket);

	// The importers in themselves:
	void importKNotes();
	void importKJots();
	void importKnowIt();
	void importTuxCards();
	void importStickyNotes();
	void importTomboy();
	void importTextFile();

	//
	void importTuxCardsNode(const QDomElement &element, Basket *parentBasket, Note *parentNote, int remainingHierarchy);
}

#endif // SOFTWAREIMPORTERS_H
