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

#include <qstring.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <qptrstack.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>

#include "softwareimporters.h"
#include "basket.h"
#include "basketfactory.h"
#include "notefactory.h"
#include "global.h"
#include "container.h"
#include "xmlwork.h"
#include "tools.h"

/** class TreeImportDialog: */

TreeImportDialog::TreeImportDialog(QWidget *parent)
 : KDialogBase(KDialogBase::Swallow, i18n("Import Hierarchy"), KDialogBase::Ok | KDialogBase::Cancel,
               KDialogBase::Ok, parent, /*name=*/0, /*modal=*/true, /*separator=*/false)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page, /*margin=*/0, spacingHint());

	m_choices = new QVButtonGroup(i18n("How to Import the Notes?"), page);
	new QRadioButton(i18n("&Keep original hierarchy (all notes in separate baskets)"), m_choices);
	new QRadioButton(i18n("First level notes in separate baskets"),                    m_choices);
	new QRadioButton(i18n("All notes in &one basket"),                                 m_choices);
	m_choices->setButton(0);
	topLayout->addWidget(m_choices);
	topLayout->addStretch(10);

	setMainWidget(page);
}

TreeImportDialog::~TreeImportDialog()
{
}

int TreeImportDialog::choice()
{
	return m_choices->selectedId();
}

/** namespace SoftwareImporters: */

QString SoftwareImporters::fromICS(const QString &ics)
{
	QString result = ics;

	// Remove escaped '\' characters and append the text to the body
	int pos = 0;
	while ( (pos = result.find('\\', pos)) != -1 ) {
		if ((uint)pos == result.length() - 1) // End of string
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
	tomboy = tomboy.mid(tomboy.find("\n")).stripWhiteSpace();

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

void SoftwareImporters::insertTitledNote(Basket *parent, const QString &title, const QString &content, Qt::TextFormat format/* = Qt::PlainText*/)
{
	Note *nGroup = new Note(parent);

	Note *nTitle = NoteFactory::createNoteText(title, parent);
	nTitle->addState(Tag::stateForId("title"));

	Note *nContent;
	if (format == Qt::PlainText)
		nContent = NoteFactory::createNoteText(content, parent);
	else
		nContent = NoteFactory::createNoteHtml(content, parent);

	parent->insertNote(nGroup,   parent->firstNote(), Note::BottomColumn, QPoint(), /*animate=*/false);
	parent->insertNote(nTitle,   nGroup,              Note::BottomColumn, QPoint(), /*animate=*/false);
	parent->insertNote(nContent, nTitle,              Note::BottomInsert, QPoint(), /*animate=*/false);
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
	QString dirPath = locateLocal("appdata","") + "/../kjots/"; // I think the assumption is good
	QDir dir(dirPath, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

	QStringList list = dir.entryList();
	if (list.isEmpty())
		return;

	BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/i18n("From KJots"), /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
	Basket *kjotsBasket = Global::basketTree->currentBasket();

	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each file
		QFile file(dirPath + *it);
		if (file.open(IO_ReadOnly)) {
			QTextStream stream(&file);
			stream.setEncoding(QTextStream::Locale);
			QString buf = stream.readLine();

			// IT IS A NOTEBOOK FILE, AT THE VERION 0.6.x and older:
			if ( !buf.isNull() && buf.left(9) == "\\NewEntry") {

				// First create a basket for it:
				BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/KURL(file.name()).fileName(), /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/kjotsBasket);
				Basket *basket = Global::basketTree->currentBasket();
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
						while ( (pos = buf.find('\\', pos)) != -1 )
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
				BasketFactory::newBasket(/*icon=*/"kjots", /*name=*/bookTitle, /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/kjotsBasket);
				Basket *basket = Global::basketTree->currentBasket();
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
	QString dirPath = locateLocal("appdata","") + "/../knotes/"; // I thing the assumption is good
	QDir dir(dirPath, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

	QStringList list = dir.entryList();
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each file
		if ( ! (*it).endsWith(".ics") ) // Don't process *.ics~ and otehr files
			continue;
		QFile file(dirPath + *it);
		if (file.open(IO_ReadOnly)) {
			QTextStream stream(&file);
			stream.setEncoding(QTextStream::UnicodeUTF8);

			// First create a basket for it:
			BasketFactory::newBasket(/*icon=*/"knotes", /*name=*/i18n("From KNotes"), /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
			Basket *basket = Global::basketTree->currentBasket();
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
					isRichText = XMLWork::trueOrFalse(buf.mid(22, buf.length() - 22).stripWhiteSpace(), "false");
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
	QDir dir(QDir::home().absPath(), QString::null, QDir::Name | QDir::IgnoreCase,
	         QDir::Dirs | QDir::NoSymLinks | QDir::Hidden);
	QStringList founds;

	QStringList list = dir.entryList();
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) { // For each folder
		if ( (*it).contains("gnome", false) ) {
			QString fullPath = QDir::home().absPath() + "/" + (*it) + "/stickynotes_applet";
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
		BasketFactory::newBasket(/*icon=*/"gnome", /*name=*/i18n("From Sticky Notes"), /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
		Basket *basket = Global::basketTree->currentBasket();
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
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		QString text;
		text = stream.read();
		file.close();
		return text;
	} else
		return "";
}


void SoftwareImporters::importTomboy()
{
	QString dirPath = QDir::home().absPath() + "/.tomboy/"; // I thing the assumption is good
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
			BasketFactory::newBasket(/*icon=*/"tomboy", /*name=*/i18n("From Tomboy"), /*backgroundColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
			basket = Global::basketTree->currentBasket();
			basket->load();
		}

		QDomElement docElem = doc->documentElement();
		QString title = XMLWork::getElementText(docElem, "title");

		// DOES NOT REALLY WORKS:
		//QDomElement contentElement = XMLWork::getElement(docElem, "text/note-content");
		//QString content = XMLWork::innerXml(contentElement);

		// Isolate "<note-content version="0.1">CONTENT</note-content>"!
		QString xml = loadUtf8FileToString(dirPath + *it);
		xml = xml.mid(xml.find("<note-content "));
		xml = xml.mid(xml.find(">") + 1);
		xml = xml.mid(0, xml.find("</note-content>"));

		if (!title.isEmpty() && !/*content*/xml.isEmpty())
			insertTitledNote(basket, title, fromTomboy(xml/*content*/), Qt::RichText);
	}

	if (basket)
		finishImport(basket);
}

/** @author Petri Damsten <petri.damsten@iki.fi>
  */
void SoftwareImporters::importKnowIt()
{
	KURL url = KFileDialog::getOpenURL(":ImportKnowIt",
	                                   "*.kno|KnowIt files\n*.*|All files");
	if (!url.isEmpty())
	{
		QFile file(url.path());
		QFileInfo info(url.path());
		Basket* basket = 0;
		Basket* master = 0;
		QString text;

		BasketFactory::newBasket(/*icon=*/"knowit",
		                         /*name=*/info.baseName(),
		                         /*backgroundColor=*/QColor(),
		                         /*templateName=*/"1column",
		                         /*createIn=*/0);
		master = Global::basketTree->currentBasket();
		master->load();

		if(file.open(IO_ReadOnly))
		{
			QTextStream stream(&file);
			uint level;
			QString name;
			QString line;
			QStringList links;
			QStringList descriptions;

			stream.setEncoding(QTextStream::UnicodeUTF8);
			while(1)
			{
				line = stream.readLine();

				if(line.startsWith("\\NewEntry") ||
				   line.startsWith("\\CurrentEntry") || stream.atEnd())
				{
					if(!name.isEmpty())
					{
						if(level == 0)
						{
							BasketFactory::newBasket(/*icon=*/"knowit",
							                         /*name=*/name,
							                         /*backgroundColor=*/QColor(),
							                         /*templateName=*/"1column",
							                         /*createIn=*/master);
							basket = Global::basketTree->currentBasket();
							basket->load();
						}

						if(!text.stripWhiteSpace().isEmpty())
						{
							insertTitledNote(basket, name, text, Qt::RichText);
						}
						for(uint j = 0; j < links.count(); ++j)
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

					int i = line.find("Entry") + 6;
					int n = line.find(' ', i);
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
	QString fileName = KFileDialog::getOpenFileName(":ImportTuxCards",  "*.*|All files");
	if (fileName.isEmpty())
		return;

	int hierarchy = TreeImportDialog(0).exec();
}

#include "softwareimporters.h"
