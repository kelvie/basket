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
class KTextEdit;
class QRadioButton;
class QTextEdit;
class QVBoxLayout;
class QCheckBox;

class BasketView;
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
    QTextEdit     *m_customSeparator;
};

/** The dialog to ask how to import OneNote data.
  * @author Suchitra Subbakrishna
  */
class OneNoteImportDialog : public KDialog
{
    Q_OBJECT
public:
    OneNoteImportDialog(QWidget *parent = 0);
    ~OneNoteImportDialog();
    int choice();
    bool addAuthorNote();
private:
    QGroupBox *m_choices;
    QVBoxLayout* m_choiceLayout;
    QRadioButton *m_root_new_choice;
    QRadioButton *m_child_new_choice;
    QRadioButton *m_childsub_new_choice;
    QCheckBox *m_add_author_meta_choice;
};

/** Functions that import data from other softwares.
  * @author Sébastien Laoût
  */
namespace SoftwareImporters
{
// Useful methods to design importers:
QString fromICS(const QString &ics);
QString fromTomboy(QString tomboy);
Note* insertTitledNote(BasketView *parent, const QString &title, const QString &content, Qt::TextFormat format = Qt::PlainText, Note *parentNote = 0);
void finishImport(BasketView *basket);

// The importers in themselves:
void importKNotes();
void importKJots();
void importKnowIt();
void importOneNoteXml();
void importTuxCards();
void importStickyNotes();
void importTomboy();
void importTextFile();

//
void importTuxCardsNode(const QDomElement &element, BasketView *parentBasket, Note *parentNote, int remainingHierarchy);
void importOneNoteNode(const QDomElement &element, BasketView *parentBasket, bool isRichText);
int collectOneNoteSectionContents(const QDomElement &element, QString *content, const QString childStyle, int entryType, int tabs);
void collectOneNoteTableContents(const QDomElement &element, QString *content, const QString childStyle, int tabs);
}

#endif // SOFTWAREIMPORTERS_H
