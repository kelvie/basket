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

#include <KDE/KDialog>

class QString;
class QGroupBox;
class QDomElement;
class QRadioButton;
class KTextEdit;
class QVBoxLayout;

class BasketScene;
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
    QVBoxLayout* m_choiceLayout;
    QRadioButton *m_hierarchy_choice;
    QRadioButton *m_separate_baskets_choice;
    QRadioButton *m_one_basket_choice;
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
    QVBoxLayout* m_choiceLayout;
    QRadioButton  *m_emptyline_choice;
    QRadioButton  *m_newline_choice;
    QRadioButton  *m_dash_choice;
    QRadioButton  *m_star_choice;
    QRadioButton  *m_all_in_one_choice;
    QRadioButton  *m_anotherSeparator;
    KTextEdit     *m_customSeparator;
};

/** Functions that import data from other softwares.
  * @author Sébastien Laoût
  */
namespace SoftwareImporters
{
// Useful methods to design importers:
QString fromICS(const QString &ics);
QString fromTomboy(QString tomboy);
//! Get first of the <tags> to be used as basket name. Strip 'system:notebook:' part
QString getFirstTomboyTag(const QDomElement& docElem);
Note* insertTitledNote(BasketScene *parent, const QString &title, const QString &content, Qt::TextFormat format = Qt::PlainText, Note *parentNote = 0);
void finishImport(BasketScene *basket);

// The importers in themselves:
void importKNotes();
void importKJots();
void importKnowIt();
void importTuxCards();
void importStickyNotes();
void importTomboy();
void importJreepadFile();
void importTextFile();

//
void importTuxCardsNode(const QDomElement &element, BasketScene *parentBasket, Note *parentNote, int remainingHierarchy);
}

#endif // SOFTWAREIMPORTERS_H
