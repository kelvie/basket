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
#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qdom.h>
//Added by qt3to4:
#include <QTextStream>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

#include "formatimporter.h"
#include "notecontent.h"
#include "notefactory.h"
#include "bnpview.h"
#include "basket.h"
#include "global.h"
#include "xmlwork.h"
#include "tools.h"

#include "kdebug.h"

#include <KIO/CopyJob>

bool FormatImporter::shouldImportBaskets()
{
	// We should import if the application have not successfully loaded any basket...
	if (Global::bnpView->topLevelItemCount()>=0)
		return false;

	// ... And there is at least one folder in the save folder, with a ".basket" file inside that folder.
	QDir dir(Global::savesFolder(), QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoSymLinks);
	QStringList list = dir.entryList();
	for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
		if (*it != "." && *it != ".." && dir.exists(Global::savesFolder() + *it + "/.basket"))
			return true;

	return false;
}

void FormatImporter::copyFolder(const QString &folder, const QString &newFolder)
{
	copyFinished = false;
	KIO::CopyJob *copyJob = KIO::copyAs(KUrl(folder), KUrl(newFolder), /*showProgressInfo=*/false);
	connect(copyJob,  SIGNAL(copyingDone(KIO::Job *, KUrl, KUrl, time_t, bool, bool)),
		this, SLOT(slotCopyingDone(KIO::Job*)));
	while (!copyFinished)
		kapp->processEvents();
}

void FormatImporter::moveFolder(const QString &folder, const QString &newFolder)
{
	copyFinished = false;
	KIO::CopyJob *copyJob = KIO::moveAs(KUrl(folder), KUrl(newFolder), /*showProgressInfo=*/false);
	connect(copyJob,  SIGNAL(copyingDone(KIO::Job *, KUrl, KUrl, time_t, bool, bool)),
		this, SLOT(slotCopyingDone(KIO::Job*)));
	while (!copyFinished)
		kapp->processEvents();
}

void FormatImporter::slotCopyingDone(KIO::Job *)
{
//	kDebug() << "Copy finished of " + from.path() + " to " + to.path();
	copyFinished = true;
}

void FormatImporter::importBaskets()
{
	kDebug() << "Import Baskets: Preparing...";

	// Some preliminary preparations (create the destination folders and the basket tree file):
	QDir dirPrep;
	dirPrep.mkdir(Global::savesFolder());
	dirPrep.mkdir(Global::basketsFolder());
	QDomDocument document("basketTree");
	QDomElement root = document.createElement("basketTree");
	document.appendChild(root);

	// First up, establish a list of every baskets, ensure the old order (if any), and count them.
	QStringList baskets;

	// Read the 0.5.0 baskets order:
	QDomDocument *doc = XMLWork::openFile("container", Global::savesFolder() + "container.baskets");
	if (doc != 0) {
		QDomElement docElem = doc->documentElement();
		QDomElement basketsElem = XMLWork::getElement(docElem, "baskets");
		QDomNode n = basketsElem.firstChild();
		while (!n.isNull()) {
			QDomElement e = n.toElement();
			if ((!e.isNull()) && e.tagName() == "basket")
				baskets.append(e.text());
			n = n.nextSibling();
		}
	}

	// Then load the baskets that weren't loaded (import < 0.5.0 ones):
	QDir dir(Global::savesFolder(), QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoSymLinks);
	QStringList list = dir.entryList();
	if (list.count() > 2) // Pass "." and ".."
		for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) // For each folder
			if (*it != "." && *it != ".." && dir.exists(Global::savesFolder() + *it + "/.basket")) // If it can be a basket folder
				if ( baskets.find((*it) + "/") == baskets.end() &&
				     baskets.find(*it)         == baskets.end()    ) // And if it is not already in the imported baskets list
					baskets.append(*it);

	kDebug() << "Import Baskets: Found " << baskets.count() << " baskets to import.";

	// Import every baskets:
	int i = 0;
	for (QStringList::iterator it = baskets.begin(); it != baskets.end(); ++it) {
		++i;
		kDebug() << "Import Baskets: Importing basket " << i << " of " << baskets.count() << "...";

		// Move the folder to the new repository (normal basket) or copy the folder (mirorred folder):
		QString folderName = *it;
		if (folderName.startsWith("/")) { // It was a folder mirror:
			KMessageBox::information(0, i18n("<p>Folder mirroring is not possible anymore (see <a href='http://basket.kde.org/'>basket.kde.org</a> for more information).</p>"
				"<p>The folder <b>%1</b> has been copied for the basket needs. You can either delete this folder or delete the basket, or use both. But remember that "
				"modifying one will not modify the other anymore as they are now separate entities.</p>", folderName), i18n("Folder Mirror Import"),
				"", KMessageBox::AllowLink);
			// Also modify folderName to be only the folder name and not the full path anymore:
			QString newFolderName = folderName;
			if (newFolderName.endsWith("/"))
				newFolderName = newFolderName.left(newFolderName.length() - 1);
			newFolderName = newFolderName.mid(newFolderName.findRev('/') + 1);
			newFolderName = Tools::fileNameForNewFile(newFolderName, Global::basketsFolder());
			FormatImporter f;
			f.copyFolder(folderName, Global::basketsFolder() + newFolderName);
			folderName = newFolderName;
		} else
			dir.rename(Global::savesFolder() + folderName, Global::basketsFolder() + folderName); // Move the folder

		// Import the basket structure file and get the properties (to add them in the tree basket-properties cache):
		QDomElement properties = importBasket(folderName);

		// Add it to the XML document:
		QDomElement basketElement = document.createElement("basket");
		root.appendChild(basketElement);
		basketElement.setAttribute("folderName", folderName);
		basketElement.appendChild(properties);
	}

	// Finalize (write to disk and delete now useless files):
	kDebug() << "Import Baskets: Finalizing...";

	QFile file(Global::basketsFolder() + "baskets.xml");
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		QString xml = document.toString();
		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		stream << xml;
		file.close();
	}

	Tools::deleteRecursively(Global::savesFolder() + ".tmp");
	dir.remove(Global::savesFolder() + "container.baskets");

	kDebug() << "Import Baskets: Finished.";
}

QDomElement FormatImporter::importBasket(const QString &folderName)
{
	// Load the XML file:
	QDomDocument *document = XMLWork::openFile("basket", Global::basketsFolder() + folderName + "/.basket");
	if (!document) {
		kDebug() << "Import Baskets: Failed to read the basket file!";
		return QDomElement();
	}
	QDomElement docElem = document->documentElement();

	// Import properties (change <background color=""> to <appearance backgroundColor="">, and figure out if is a checklist or not):
	QDomElement properties = XMLWork::getElement(docElem, "properties");
	QDomElement background = XMLWork::getElement(properties, "background");
	QColor backgroundColor = QColor(background.attribute("color"));
	if (backgroundColor.isValid() && (backgroundColor != kapp->palette().color(QPalette::Base))) { // Use the default color if it was already that color:
		QDomElement appearance = document->createElement("appearance");
		appearance.setAttribute("backgroundColor", backgroundColor.name());
		properties.appendChild(appearance);
	}
	QDomElement disposition = document->createElement("disposition");
	disposition.setAttribute("mindMap",     "false");
	disposition.setAttribute("columnCount", "1");
	disposition.setAttribute("free",        "false");
	bool isCheckList = XMLWork::trueOrFalse( XMLWork::getElementText(properties, "showCheckBoxes", false) );

	// Insert all notes in a group (column): 1/ rename "items" to "group", 2/ add "notes" to root, 3/ move "group" into "notes"
	QDomElement column = XMLWork::getElement(docElem, "items");
	column.setTagName("group");
	QDomElement notes = document->createElement("notes");
	notes.appendChild(column);
	docElem.appendChild(notes);

	// Import notes from older representations:
	QDomNode n = column.firstChild();
	while ( ! n.isNull() ) {
		QDomElement e = n.toElement();
		if (!e.isNull()) {
			e.setTagName("note");
			QDomElement content = XMLWork::getElement(e, "content");
			// Add Check tag:
			if (isCheckList) {
				bool isChecked = XMLWork::trueOrFalse(e.attribute("checked", "false"));
				XMLWork::addElement(*document, e, "tags", (isChecked ? "todo_done" : "todo_unchecked"));
			}
			// Import annotations as folded groups:
			QDomElement parentE = column;
			QString annotations = XMLWork::getElementText(e, "annotations", "");
			if (!annotations.isEmpty()) {
				QDomElement annotGroup = document->createElement("group");
				column.insertBefore(annotGroup, e);
				annotGroup.setAttribute("folded", "true");
				annotGroup.appendChild(e);
				parentE = annotGroup;
				// Create the text note and add it to the DOM tree:
				QDomElement annotNote = document->createElement("note");
				annotNote.setAttribute("type", "text");
				annotGroup.appendChild(annotNote);
				QString annotFileName = Tools::fileNameForNewFile("annotations1.txt", Basket::fullPathForFolderName(folderName));
				QString annotFullPath = Basket::fullPathForFolderName(folderName) + "/" + annotFileName;
				QFile file(annotFullPath);
				if (file.open(QIODevice::WriteOnly)) {
					QTextStream stream(&file);
					stream << annotations;
					file.close();
				}
				XMLWork::addElement(*document, annotNote, "content", annotFileName);
				n = annotGroup;
			}
			// Import Launchers from 0.3.x, 0.4.0 and 0.5.0-alphas:
			QString runCommand = e.attribute("runcommand"); // Keep compatibility with 0.4.0 and 0.5.0-alphas versions
			runCommand = XMLWork::getElementText(e, "action", runCommand); // Keep compatibility with 0.3.x versions
			if ( ! runCommand.isEmpty() ) { // An import should be done
				// Prepare the launcher note:
				QString title = content.attribute("title", "");
				QString icon  = content.attribute("icon",  "");
				if (title.isEmpty()) title = runCommand;
				if (icon.isEmpty())  icon  = NoteFactory::iconForCommand(runCommand);
				// Import the launcher note:
				// Adapted version of "QString launcherName = NoteFactory::createNoteLauncherFile(runCommand, title, icon, this)":
				QString launcherContent = QString(
					"[Desktop Entry]\n"
					"Exec=%1\n"
					"Name=%2\n"
					"Icon=%3\n"
					"Encoding=UTF-8\n"
					"Type=Application\n").arg(runCommand, title, icon.isEmpty() ? QString("exec") : icon);
				QString launcherFileName = Tools::fileNameForNewFile("launcher.desktop", Global::basketsFolder() + folderName /*+ "/"*/);
				QString launcherFullPath = Global::basketsFolder() + folderName /*+ "/"*/ + launcherFileName;
				QFile file(launcherFullPath);
				if (file.open(QIODevice::WriteOnly)) {
					QTextStream stream(&file);
					stream.setEncoding(QTextStream::UnicodeUTF8);
					stream << launcherContent;
					file.close();
				}
				// Add the element to the DOM:
				QDomElement launcherElem = document->createElement("note");
				parentE.insertBefore(launcherElem, e);
				launcherElem.setAttribute("type", "launcher");
				XMLWork::addElement(*document, launcherElem, "content", launcherFileName);
			}
			// Import unknown ns to 0.6.0:
			if (e.attribute("type") == "unknow")
				e.setAttribute("type", "unknown");
			// Import links from version < 0.5.0:
			if (!content.attribute("autotitle").isEmpty() && content.attribute("autoTitle").isEmpty())
				content.setAttribute("autoTitle", content.attribute("autotitle"));
			if (!content.attribute("autoicon").isEmpty() && content.attribute("autoIcon").isEmpty())
				content.setAttribute("autoIcon", content.attribute("autoicon"));
		}
		n = n.nextSibling();
	}

	// Save the resulting XML file:
	QFile file(Global::basketsFolder() + folderName + "/.basket");
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
//		QString xml = document->toString();
//		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
//		stream << xml;
		stream << document->toString(); // Document is ALREADY using UTF-8
		file.close();
	} else
		kDebug() << "Import Baskets: Failed to save the basket file!";

	// Return the newly created properties (to put in the basket tree):
	return properties;
}

