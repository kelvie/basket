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

#include <qstring.h>
#include <qdir.h>
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <QStack>
#include <qlayout.h>
#include <qradiobutton.h>
#include <kmessagebox.h>
#include <QTextEdit>
#include <QDomDocument>

#include "softwareimporters.h"
#include "basket.h"
#include "basketfactory.h"
#include "notefactory.h"
#include "global.h"
#include "bnpview.h"
#include "xmlwork.h"
#include "tools.h"
#include <QGroupBox>

/** class TreeImportDialog: */

TreeImportDialog::TreeImportDialog(QWidget *parent)
     : KDialog(parent)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page);

	// KDialog options
	setCaption(i18n("Import Hierarchy"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName("ImportHeirachy");
	setModal(true);
	showButtonSeparator(false);

	m_choices = new QGroupBox(i18n("How to Import the Notes?"), page);
	m_choiceLayout = new QVBoxLayout();
	m_choices->setLayout(m_choiceLayout);

	m_hierarchy_choice = new QRadioButton(i18n("&Keep original hierarchy (all notes in separate baskets)"), m_choices);
	m_separate_baskets_choice = new QRadioButton(i18n("&First level notes in separate baskets"),            m_choices);
	m_one_basket_choice = new QRadioButton(i18n("&All notes in one basket"),                                m_choices);

	m_hierarchy_choice->setChecked(true);
	m_choiceLayout->addWidget(m_hierarchy_choice);
	m_choiceLayout->addWidget(m_separate_baskets_choice);
	m_choiceLayout->addWidget(m_one_basket_choice);

	topLayout->addWidget(m_choices);
	topLayout->addStretch(10);

	setMainWidget(page);
}

TreeImportDialog::~TreeImportDialog()
{
}

int TreeImportDialog::choice()
{
	if (m_hierarchy_choice->isChecked())
		return 1;
	else if (m_separate_baskets_choice->isChecked())
		return 2;
	else
		return 3;
}

/** class TextFileImportDialog: */

TextFileImportDialog::TextFileImportDialog(QWidget *parent)
     : KDialog(parent)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page);

	// KDialog options
	setCaption(i18n("Import Text File"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName("ImportTextFile");
	setModal(true);
	showButtonSeparator(false);


	m_choices = new QGroupBox(i18n("Format of the Text File"), page);
	m_choiceLayout = new QVBoxLayout;
	m_choices->setLayout(m_choiceLayout);

	m_emptyline_choice = new QRadioButton(i18n("Notes separated by an &empty line"), m_choices);
	m_newline_choice = new QRadioButton(i18n("One &note per line"),                  m_choices);
	m_dash_choice = new QRadioButton(i18n("Notes begin with a &dash (-)"),           m_choices);
	m_star_choice = new QRadioButton(i18n("Notes begin with a &star (*)"),           m_choices);
	m_anotherSeparator = new QRadioButton(i18n("&Use another separator:"),           m_choices);

	m_choiceLayout->addWidget(m_emptyline_choice);
	m_choiceLayout->addWidget(m_newline_choice);
	m_choiceLayout->addWidget(m_dash_choice);
	m_choiceLayout->addWidget(m_star_choice);
	m_choiceLayout->addWidget(m_anotherSeparator);

	QWidget *indentedTextEdit = new QWidget(m_choices);
	m_choiceLayout->addWidget(indentedTextEdit);

	QHBoxLayout *hLayout = new QHBoxLayout(indentedTextEdit);
	hLayout->addSpacing(20);
	m_customSeparator = new QTextEdit(indentedTextEdit);
	hLayout->addWidget(m_customSeparator);

	m_all_in_one_choice = new QRadioButton(i18n("&All in one note"),                  m_choices);
	m_choiceLayout->addWidget(m_all_in_one_choice);

	m_emptyline_choice->setChecked(true);
	topLayout->addWidget(m_choices);

	connect( m_customSeparator, SIGNAL(textChanged()), this, SLOT(customSeparatorChanged()) );

	setMainWidget(page);
}

TextFileImportDialog::~TextFileImportDialog()
{
}

QString TextFileImportDialog::separator()
{
	if (m_emptyline_choice->isChecked())
		return "\n\n";
	else if (m_newline_choice->isChecked())
		return "\n";
	else if (m_dash_choice->isChecked())
		return "\n-";
	else if (m_star_choice->isChecked())
		return "\n*";
	else if (m_anotherSeparator->isChecked())
		return m_customSeparator->toPlainText();
	else if (m_all_in_one_choice->isChecked())
		return "";
	else
		return "\n\n";
}

void TextFileImportDialog::customSeparatorChanged()
{
	if (!m_anotherSeparator->isChecked())
		m_anotherSeparator->toggle();
}

/** namespace SoftwareImporters: */

QString SoftwareImporters::fromICS(const QString &ics)
{
	QString result = ics;

	// Remove escaped '\' characters and append the text to the body
	int pos = 0;
	while ( (pos = result.indexOf('\\', pos)) != -1 ) {
		if (pos == result.length() - 1) // End of string
			break;
		if (result[pos+1] == 'n') {
			result.replace(pos, 2, '\n');
		} else if (result[pos+1] == 'r') {
			result.replace(pos, 2, '\r');
		} else if (result[pos+1] == 't') {
			result.replace(pos, 2, '\t');
		} else if (result[pos] == '\\') {
			result.remove(pos, 1); // Take care of "\\", "\,", "\;" and other escaped characters I haven't noticed
			++pos;
		}
	}

	return result;
}

QString SoftwareImporters::fromTomboy(QString tomboy)
{
	// The first line is the note title, and we already have it, so we remove it (yes, that's pretty stupid to duplicate it in the content...):
	tomboy = tomboy.mid(tomboy.indexOf("\n")).trimmed();

	// Font styles and decorations:
	tomboy.replace("<bold>",           "<b>");
	tomboy.replace("</bold>",          "</b>");
	tomboy.replace("<italic>",         "<i>");
	tomboy.replace("</italic>",        "</i>");
	tomboy.replace("<strikethrough>",  "<span style='text-decoration: line-through'>");
	tomboy.replace("</strikethrough>", "</span>");

	// Highlight not supported by QTextEdit:
	tomboy.replace("<highlight>",      "<span style='color:#ff0080'>");
	tomboy.replace("</highlight>",     "</span>");

	// Font sizes:
	tomboy.replace("<size:small>",     "<span style='font-size: 7pt'>");
	tomboy.replace("</size:small>",    "</span>");
	tomboy.replace("<size:large>",     "<span style='font-size: 16pt'>");
	tomboy.replace("</size:large>",    "</span>");
	tomboy.replace("<size:huge>",      "<span style='font-size: 20pt'>");
	tomboy.replace("</size:huge>",     "</span>");

	// Internal links to other notes aren't supported yet by BasKet Note Pads:
	tomboy.replace("<link:internal>",  "");
	tomboy.replace("</link:internal>", "");

	// In the Tomboy file, new lines are "\n" and not "<br>":
	tomboy.replace("\n", "<br>\n");

	// Preserve consecutive spaces:
	return "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" + tomboy + "</body></html>";
}

Note* SoftwareImporters::insertTitledNote(Basket *parent, const QString &title, const QString &content, Qt::TextFormat format/* = Qt::PlainText*/, Note *parentNote/* = 0*/)
{
	Note *nGroup = new Note(parent);

	Note *nTitle = NoteFactory::createNoteText(title, parent);
	nTitle->addState(Tag::stateForId("title"));

	Note *nContent;
	if (format == Qt::PlainText)
		nContent = NoteFactory::createNoteText(content, parent);
	else
		nContent = NoteFactory::createNoteHtml(content, parent);

	if (parentNote == 0)
		parentNote = parent->firstNote(); // In the first column!
	parent->insertNote(nGroup,   parentNote, Note::BottomColumn, QPoint(), /*animate=*/false);
	parent->insertNote(nTitle,   nGroup,     Note::BottomColumn, QPoint(), /*animate=*/false);
	parent->insertNote(nContent, nTitle,     Note::BottomInsert, QPoint(), /*animate=*/false);

	return nGroup;
}

void SoftwareImporters::finishImport(Basket *basket)
{
	// Unselect the last inserted group:
	basket->unselectAll();

	// Focus the FIRST note (the last inserted note is currently focused!):
	basket->setFocusedNote(basket->firstNoteShownInStack());

	// Relayout every notes at theire new place and simulate a load animation (because already loaded just after the creation).
	// Without a relayouting, notes on the bottom would comes from the top (because they were inserted on top) and clutter the animation load:
	basket->relayoutNotes(/*animate=*/false);
	basket->animateLoad();
	basket->save();
}



void SoftwareImporters::importKJots()
{
	QString dirPath = KStandardDirs::locateLocal("appdata","") + "/../kjots/"; // I think the assumption is good
	QDir dir(dirPath, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

	QStringList list = dir.entryList();
	if (list.isEmpty())
		return;

	BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/i18n("From KJots"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
	Basket *kjotsBasket = Global::bnpView->currentBasket();

	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each file
		QFile file(dirPath + *it);
		if (file.open(QIODevice::ReadOnly)) {
			QTextStream stream(&file);
			QString buf = stream.readLine();

			// IT IS A NOTEBOOK FILE, AT THE VERION 0.6.x and older:
			if ( !buf.isNull() && buf.left(9) == "\\NewEntry") {

				// First create a basket for it:
				BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/KUrl(file.fileName()).fileName(), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/kjotsBasket);
				Basket *basket = Global::bnpView->currentBasket();
				basket->load();

				QString title, body;
				bool haveAnEntry = false;
				while (1) {
					if (buf.left(9) == "\\NewEntry") {
						if (haveAnEntry) // Do not add note the first time
							insertTitledNote(basket, title, Tools::stripEndWhiteSpaces(body));
						title = buf.mid(10, buf.length());          // Problem : basket will be saved
						body = ""; // New note will then be created //  EACH time a note is imported
						haveAnEntry = true;
					} else if (buf.left(3) != "\\ID") { // Don't care of the ID
						// Remove escaped '\' characters and append the text to the body
						int pos = 0;
						while ( (pos = buf.indexOf('\\', pos)) != -1 )
							if (buf[++pos] == '\\')
								buf.remove(pos, 1);
						body.append(buf + "\n");
					}
					buf = stream.readLine();
					if (buf.isNull()) // OEF
						break;
				}
				// Add the ending note (there isn't any other "\\NewEntry" to do it):
				if (haveAnEntry)
					insertTitledNote(basket, title, Tools::stripEndWhiteSpaces(body));
				finishImport(basket);

			// IT IS A NOTEBOOK XML FILE, AT THE VERION 0.7.0 and later:
			} else if ( (*it).endsWith(".book") /*&& !buf.isNull() && (buf.left(2) == "<!" / *<!DOCTYPE...* / || buf.left(2) == "<?" / *<?xml...* /)*/) {

				QDomDocument *doc = XMLWork::openFile("KJots", dirPath + *it);
				if (doc == 0)
					continue;

				QString bookTitle = XMLWork::getElementText(doc->documentElement(), "KJotsBook/Title");

				// First create a basket for it:
				BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/bookTitle, /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/kjotsBasket);
				Basket *basket = Global::bnpView->currentBasket();
				basket->load();

				QDomElement docElem = XMLWork::getElement(doc->documentElement(), "KJotsBook");
				for ( QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling() ) {
					QDomElement e = n.toElement();
					if ( (!e.isNull()) && e.tagName() == "KJotsPage" )
						insertTitledNote(basket, XMLWork::getElementText(e, "Title"), XMLWork::getElementText(e, "Text"));
				}
				finishImport(basket);

			}

			file.close();
		}
	}
}

void SoftwareImporters::importKNotes()
{
	QString dirPath = KStandardDirs::locateLocal("appdata","") + "/../knotes/"; // I thing the assumption is good
	QDir dir(dirPath, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

	QStringList list = dir.entryList();
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each file
		if ( ! (*it).endsWith(".ics") ) // Don't process *.ics~ and otehr files
			continue;
		QFile file(dirPath + *it);
		if (file.open(QIODevice::ReadOnly)) {
			QTextStream stream(&file);
			stream.setCodec("UTF-8");

			// First create a basket for it:
			BasketFactory::newBasket(/*icon=*/"knotes", /*name=*/i18n("From KNotes"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
			Basket *basket = Global::bnpView->currentBasket();
			basket->load();

			bool inVJournal    = false;
			bool inDescription = false;
			bool isRichText    = false;
			QString title, body;
			QString buf;
			while (1) {
				buf = stream.readLine();
				if (buf.isNull()) // OEF
					break;

				if ( !buf.isNull() && buf == "BEGIN:VJOURNAL") {
					inVJournal = true;
				} else if (inVJournal && buf.startsWith("SUMMARY:")) {
					title = buf.mid(8, buf.length());
				} else if (inVJournal && buf.startsWith("DESCRIPTION:")) {
					body = buf.mid(12, buf.length());
					inDescription = true;
				} else if (inDescription && buf.startsWith(" ")) {
					body += buf.mid(1, buf.length());
				} else if (buf.startsWith("X-KDE-KNotes-RichText:")) {
					isRichText = XMLWork::trueOrFalse(buf.mid(22, buf.length() - 22).trimmed(), "false");
				} else if (buf == "END:VJOURNAL") {
					insertTitledNote(basket, fromICS(title), fromICS(body), (isRichText ? Qt::RichText : Qt::PlainText));
					inVJournal    = false;
					inDescription = false;
					isRichText    = false;
					title = "";
					body = "";
				} else
					inDescription = false;
			}

			// Bouh : duplicate code
			// In case of unvalide ICAL file!
			if ( ! body.isEmpty() ) // Add the ending note
				insertTitledNote(basket, fromICS(title), fromICS(body), (isRichText ? Qt::RichText : Qt::PlainText));
			file.close();
			finishImport(basket);
		}
	}
}

void SoftwareImporters::importStickyNotes()
{
	// Sticky Notes file is usually located in ~/.gnome2/stickynotes_applet
	// We will search all directories in "~/" that contain "gnome" in the name,
	// and will search for "stickynotes_applet" file (that should be XML file with <stickynotes> root.
	QDir dir(QDir::home().absolutePath(), QString::null, QDir::Name | QDir::IgnoreCase,
	         QDir::Dirs | QDir::NoSymLinks | QDir::Hidden);
	QStringList founds;

	QStringList list = dir.entryList();
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each folder
		if ( (*it).contains("gnome", Qt::CaseInsensitive) ) {
			QString fullPath = QDir::home().absolutePath() + "/" + (*it) + "/stickynotes_applet";
			if (dir.exists(fullPath))
				founds += fullPath;
		}
	}

	for ( QStringList::Iterator it = founds.begin(); it != founds.end(); ++it ) { // For each file
		QFile file(*it);
		QDomDocument *doc = XMLWork::openFile("stickynotes", *it);
		if (doc == 0)
			continue;

		// First create a basket for it:
		BasketFactory::newBasket(/*icon=*/"gnome", /*name=*/i18n("From Sticky Notes"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
		Basket *basket = Global::bnpView->currentBasket();
		basket->load();

		QDomElement docElem = doc->documentElement();
		for ( QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling() ) {
			QDomElement e = n.toElement();
			if ( (!e.isNull()) && e.tagName() == "note" )
				insertTitledNote(basket, e.attribute("title"), e.text());
		}
		finishImport(basket);
	}
}



// TODO: FIXME: Code duplicated from notecontent.cpp but with UTF-8 encoding.
// TODO: FIXME: Later, merge!
QString loadUtf8FileToString(const QString &fileName)
{
	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		QString text;
		text = stream.readAll();
		file.close();
		return text;
	} else
		return "";
}


void SoftwareImporters::importTomboy()
{
	QString dirPath = QDir::home().absolutePath() + "/.tomboy/"; // I thing the assumption is good
	QDir dir(dirPath, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

	Basket *basket = 0; // Create the basket ONLY if we found at least one note to add!

	QStringList list = dir.entryList();
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each file
		if ( ! (*it).endsWith(".note") )
			continue;
		QDomDocument *doc = XMLWork::openFile("note", dirPath + *it);
		if (doc == 0)
			continue;

		if (basket == 0) {
			// First create a basket for it:
			BasketFactory::newBasket(/*icon=*/"tomboy", /*name=*/i18n("From Tomboy"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
			basket = Global::bnpView->currentBasket();
			basket->load();
		}

		QDomElement docElem = doc->documentElement();
		QString title = XMLWork::getElementText(docElem, "title");

		// DOES NOT REALLY WORKS:
		//QDomElement contentElement = XMLWork::getElement(docElem, "text/note-content");
		//QString content = XMLWork::innerXml(contentElement);

		// Isolate "<note-content version="0.1">CONTENT</note-content>"!
		QString xml = loadUtf8FileToString(dirPath + *it);
		xml = xml.mid(xml.indexOf("<note-content "));
		xml = xml.mid(xml.indexOf(">") + 1);
		xml = xml.mid(0, xml.indexOf("</note-content>"));

		if (!title.isEmpty() && !/*content*/xml.isEmpty())
			insertTitledNote(basket, title, fromTomboy(xml/*content*/), Qt::RichText);
	}

	if (basket)
		finishImport(basket);
}

void SoftwareImporters::importTextFile()
{
	QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///:ImportTextFile"),  "*|All files");
	if (fileName.isEmpty())
		return;

	TextFileImportDialog dialog;
	if (dialog.exec() == QDialog::Rejected)
		return;
	QString separator = dialog.separator();


	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		QString content = stream.readAll();
		QStringList list = (separator.isEmpty()
			? QStringList(content)
			: content.split(separator)
		);

		// First create a basket for it:
		QString title = i18nc("From TextFile.txt", "From %1",KUrl(fileName).fileName());
		BasketFactory::newBasket(/*icon=*/"txt", title, /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
		Basket *basket = Global::bnpView->currentBasket();
		basket->load();

		// Import every notes:
		for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
			Note *note = NoteFactory::createNoteFromText((*it).trimmed(), basket);
			basket->insertNote(note, basket->firstNote(), Note::BottomColumn, QPoint(), /*animate=*/false);
		}

		// Finish the export:
		finishImport(basket);
	}
}

/** @author Petri Damsten <petri.damsten@iki.fi>
  */
void SoftwareImporters::importKnowIt()
{
	KUrl url = KFileDialog::getOpenUrl(KUrl("kfiledialog:///:ImportKnowIt"),
	                                   "*.kno|KnowIt files\n*|All files");
	if (!url.isEmpty())
	{
		QFile file(url.path());
		QFileInfo info(url.path());
		Basket* basket = 0;
		QStack<Basket*> baskets;
		QString text;
		int hierarchy = 0;

		TreeImportDialog dialog;
		if (dialog.exec() == QDialog::Rejected)
			return;

		hierarchy = dialog.choice();

		BasketFactory::newBasket(/*icon=*/"knowit",
		                         /*name=*/info.baseName(),
		                         /*backgroundImage=*/"",
		                         /*backgroundColor=*/QColor(),
		                         /*textColor=*/QColor(),
		                         /*templateName=*/"1column",
		                         /*createIn=*/0);
		basket = Global::bnpView->currentBasket();
		basket->load();
		baskets.push(basket);

		if(file.open(QIODevice::ReadOnly))
		{
			QTextStream stream(&file);
			int level = 0;
			QString name;
			QString line;
			QStringList links;
			QStringList descriptions;

			stream.setCodec("UTF-8");
			while(1)
			{
				line = stream.readLine();

				if(line.startsWith("\\NewEntry") ||
				   line.startsWith("\\CurrentEntry") || stream.atEnd())
				{
					while(level + 1 < baskets.size()-baskets.count(0))
						baskets.pop();
					if(level + 1 > baskets.size()-baskets.count(0))
						baskets.push(basket);

					if(!name.isEmpty())
					{
						if((level == 0 && hierarchy == 1) ||
							(hierarchy == 0))
						{
							BasketFactory::newBasket(/*icon=*/"knowit",
							                         /*name=*/name,
							                         /*backgroundImage=*/"",
							                         /*backgroundColor=*/QColor(),
							                         /*textColor=*/QColor(),
							                         /*templateName=*/"1column",
							                         /*createIn=*/baskets.top());
							basket = Global::bnpView->currentBasket();
							basket->load();
						}

						if(!text.trimmed().isEmpty() ||
							hierarchy == 2 ||
							(hierarchy == 1 && level > 0))
						{
							insertTitledNote(basket, name, text, Qt::RichText);
						}
						for(int j = 0; j < links.count(); ++j)
						{
							Note* link;
							if(descriptions.count() < j+1 || descriptions[j].isEmpty())
								link = NoteFactory::createNoteLink(links[j], basket);
							else
								link = NoteFactory::createNoteLink(links[j],
							descriptions[j], basket);
							basket->insertCreatedNote(link);
						}
						finishImport(basket);
					}
					if(stream.atEnd())
						break;

					int i = line.indexOf("Entry") + 6;
					int n = line.indexOf(' ', i);
					level = line.mid(i, n - i).toInt();
					name = line.mid(n+1);
					text = "";
					links.clear();
					descriptions.clear();
				}
				else if(line.startsWith("\\Link"))
				{
					links.append(line.mid(6));
				}
				else if(line.startsWith("\\Descr"))
				{
					while(descriptions.count() < links.count() - 1)
						descriptions.append("");
					descriptions.append(line.mid(7));
				}
				else
				{
					text += line + "\n";
				}
			}
			file.close();
		}
	}
}

void SoftwareImporters::importTuxCards()
{
	QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///:ImportTuxCards"),  "*|All files");
	if (fileName.isEmpty())
		return;

	TreeImportDialog dialog;
	if (dialog.exec() == QDialog::Rejected)
		return;

	int hierarchy = dialog.choice();

	QDomDocument *document = XMLWork::openFile("tuxcards_data_file"/*"InformationCollection"*/, fileName);
	if (document == 0) {
		KMessageBox::error(0, i18n("Can not import that file. It is either corrupted or not a TuxCards file."), i18n("Bad File Format"));
		return;
	}

	QDomElement collection = document->documentElement();
	int remainingHierarchy = (hierarchy == 0 ? 65000 : 3 - hierarchy);
	importTuxCardsNode(collection, /*parentBasket=*/0, /*parentNote=*/0, remainingHierarchy);
}

// TODO: <InformationElement isOpen="true" isEncripted="false"

void SoftwareImporters::importTuxCardsNode(const QDomElement &element, Basket *parentBasket, Note *parentNote, int remainingHierarchy)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (e.isNull() || e.tagName() != "InformationElement") // Cannot handle that!
			continue;

		QString icon        = e.attribute("iconFileName");
		QString name        = XMLWork::getElementText(e, "Description");
		QString content     = XMLWork::getElementText(e, "Information");
		bool    isRichText  = (e.attribute("informationFormat") == "RTF");
		bool    isEncrypted = (e.attribute("isEncripted") == "true");
		if (icon.isEmpty() || icon == "none")
			icon = "tuxcards";
		Note *nContent;

		if (isEncrypted) {
			KMessageBox::information(0, i18n("A note is encrypted. The importer does not yet support encrypted notes. Please remove the encryption with TuxCards and re-import the file."), i18n("Encrypted Notes not Supported Yet"));
			isRichText = true;
			content = i18n("<font color='red'><b>Encrypted note.</b><br>The importer do not support encrypted notes yet. Please remove the encryption with TuxCards and re-import the file.</font>");
		}

		if (remainingHierarchy > 0) {
			BasketFactory::newBasket(icon, name, /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", parentBasket);
			Basket *basket = Global::bnpView->currentBasket();
			basket->load();

			if (isRichText)
				nContent = NoteFactory::createNoteHtml(content, basket);
			else
				nContent = NoteFactory::createNoteText(content, basket);
			basket->insertNote(nContent, basket->firstNote(), Note::BottomColumn, QPoint(), /*animate=*/false);

			importTuxCardsNode(e, basket, 0, remainingHierarchy - 1);
			finishImport(basket);
		} else {
			Note *nGroup = insertTitledNote(parentBasket, name, content, (isRichText ? Qt::RichText : Qt::PlainText), parentNote);
			importTuxCardsNode(e, parentBasket, nGroup, remainingHierarchy - 1);
		}
	}
}

