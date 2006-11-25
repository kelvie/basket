/***************************************************************************
 *   Copyright (C) 2006 by Sébastien Laoût                                 *
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
#include <qvaluelist.h>
#include <qmap.h>
#include <qdir.h>
#include <ktar.h>
#include <qdom.h>
#include <kmessagebox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "archive.h"
#include "global.h"
#include "bnpview.h"
#include "basket.h"
#include "basketlistview.h"
#include "basketfactory.h"
#include "tag.h"
#include "xmlwork.h"
#include "tools.h"
#include "backgroundmanager.h"
#include "formatimporter.h"

void Archive::save(Basket *basket, bool withSubBaskets, const QString &destination)
{
	QDir dir;

	// Create the temporar folder:
	QString tempFolder = Global::savesFolder() + "temp-archive/";
	dir.mkdir(tempFolder);

	// Create the temporar archive file:
	QString tempDestination = tempFolder + "temp-archive.tar.gz";
	KTar tar(tempDestination, "application/x-gzip");
	tar.open(IO_WriteOnly);
	tar.writeDir("baskets", "", "");

	// Copy the baskets data into the archive:
	QStringList backgrounds;
	saveBasketToArchive(basket, withSubBaskets, &tar, backgrounds);

	// Create a Small baskets.xml Document:
	QDomDocument document("basketTree");
	QDomElement root = document.createElement("basketTree");
	document.appendChild(root);
	Global::bnpView->saveSubHierarchy(Global::bnpView->listViewItemForBasket(basket), document, root, withSubBaskets);
	Basket::safelySaveToFile(tempFolder + "baskets.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + document.toString());
	tar.addLocalFile(tempFolder + "baskets.xml", "baskets/baskets.xml");
	dir.remove(tempFolder + "baskets.xml");

	// Save a Small tags.xml Document:
	QValueList<Tag*> tags;
	listUsedTags(basket, withSubBaskets, tags);
	Tag::saveTagsTo(tags, tempFolder + "tags.xml");
	tar.addLocalFile(tempFolder + "tags.xml", "tags.xml");
	dir.remove(tempFolder + "tags.xml");

	// Save Tag Emblems (in case they are loaded on a computer that do not have those icons):
	QString tempIconFile = tempFolder + "icon.png";
	for (Tag::List::iterator it = tags.begin(); it != tags.end(); ++it) {
		State::List states = (*it)->states();
		for (State::List::iterator it2 = states.begin(); it2 != states.end(); ++it2) {
			State *state = (*it2);
			QPixmap icon = kapp->iconLoader()->loadIcon(state->emblem(), KIcon::Small, 16, KIcon::DefaultState, /*path_store=*/0L, /*canReturnNull=*/true);
			if (!icon.isNull()) {
				icon.save(tempIconFile, "PNG");
				QString iconFileName = state->emblem().replace('/', '_');
				tar.addLocalFile(tempIconFile, "tag-emblems/" + iconFileName);
			}
		}
	}
	dir.remove(tempIconFile);

	// Finish Tar.Gz Exportation:
	tar.close();

	// Computing the File Preview:
	Basket *previewBasket = basket; // FIXME: Use the first non-empty basket!
	QPixmap previewPixmap(previewBasket->visibleWidth(), previewBasket->visibleHeight());
	QPainter painter(&previewPixmap);
	// Save old state, and make the look clean ("smile, you are filmed!"):
	NoteSelection *selection = previewBasket->selectedNotes();
	previewBasket->unselectAll();
	Note *focusedNote = previewBasket->focusedNote();
	previewBasket->setFocusedNote(0);
	previewBasket->doHoverEffects(0, Note::None);
	// Take the screenshot:
	previewBasket->drawContents(&painter, 0, 0, previewPixmap.width(), previewPixmap.height());
	// Go back to the old look:
	previewBasket->selectSelection(selection);
	previewBasket->setFocusedNote(focusedNote);
	previewBasket->doHoverEffects();
	// End and save our splandid painting:
	painter.end();
	QImage previewImage = previewPixmap.convertToImage();
	const int PREVIEW_SIZE = 256;
	previewImage = previewImage.scale(PREVIEW_SIZE, PREVIEW_SIZE, QImage::ScaleMin);
	previewImage.save(tempFolder + "preview.png", "PNG");

	// Finaly Save to the Real Destination file:
	QFile file(destination);
	if (file.open(IO_WriteOnly)) {
		ulong previewSize = QFile(tempFolder + "preview.png").size();
		ulong archiveSize = QFile(tempDestination).size();
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::Latin1);
		stream << "BasKetNP:archive\n"
		       << "version:0.6.1\n"
//		       << "read-compatible:0.6.1\n"
//		       << "write-compatible:0.6.1\n"
		       << "preview*:" << previewSize << "\n";
		// Copy the Preview File:
		const Q_ULONG BUFFER_SIZE = 1024;
		char *buffer = new char[BUFFER_SIZE];
		Q_LONG sizeRead;
		QFile previewFile(tempFolder + "preview.png");
		if (previewFile.open(IO_ReadOnly)) {
			while ((sizeRead = previewFile.readBlock(buffer, BUFFER_SIZE)) > 0)
				file.writeBlock(buffer, sizeRead);
		}
		stream << "archive*:" << archiveSize << "\n";
		// Copy the Archive File:
		QFile archiveFile(tempDestination);
		if (archiveFile.open(IO_ReadOnly)) {
			while ((sizeRead = archiveFile.readBlock(buffer, BUFFER_SIZE)) > 0)
				file.writeBlock(buffer, sizeRead);
		}
		// Clean Up:
		delete buffer;
		buffer = 0;
		file.close();
	}

	// Clean Up Everything:
	dir.remove(tempFolder + "preview.png");
	dir.remove(tempDestination);
	dir.rmdir(tempFolder);
}

void Archive::saveBasketToArchive(Basket *basket, bool recursive, KTar *tar, QStringList &backgrounds)
{
	QDir dir;
	// Save basket data:
	tar->addLocalDirectory(basket->fullPath(), "baskets/" + basket->folderName());
	tar->addLocalFile(basket->fullPath() + ".basket", "baskets/" + basket->folderName() + ".basket"); // The hidden files were not added
	// Save basket backgorund image:
	QString imageName = basket->backgroundImageName();
	if (!basket->backgroundImageName().isEmpty() && !backgrounds.contains(imageName)) {
		QString backgroundPath = Global::backgroundManager->pathForImageName(imageName);
		if (!backgroundPath.isEmpty()) {
			// Save the background image:
			tar->addLocalFile(backgroundPath, "backgrounds/" + imageName);
			// Save the preview image:
			QString previewPath = Global::backgroundManager->previewPathForImageName(imageName);
			if (!previewPath.isEmpty())
				tar->addLocalFile(previewPath, "backgrounds/previews/" + imageName);
			// Save the configuration file:
			QString configPath = backgroundPath + ".config";
			if (dir.exists(configPath))
				tar->addLocalFile(configPath, "backgrounds/" + imageName + ".config");
		}
		backgrounds.append(imageName);
	}
	// Recursively save child baskets:
	BasketListViewItem *item = Global::bnpView->listViewItemForBasket(basket);
	if (recursive && item->firstChild()) {
		for (BasketListViewItem *child = (BasketListViewItem*) item->firstChild(); child; child = (BasketListViewItem*) child->nextSibling()) {
			saveBasketToArchive(child->basket(), recursive, tar, backgrounds);
		}
	}
}

void Archive::listUsedTags(Basket *basket, bool recursive, QValueList<Tag*> &list)
{
	basket->listUsedTags(list);
	BasketListViewItem *item = Global::bnpView->listViewItemForBasket(basket);
	if (recursive && item->firstChild()) {
		for (BasketListViewItem *child = (BasketListViewItem*) item->firstChild(); child; child = (BasketListViewItem*) child->nextSibling()) {
			listUsedTags(child->basket(), recursive, list);
		}
	}
}

void Archive::open(const QString &path)
{
	// Create the temporar folder:
	QString tempFolder = Global::savesFolder() + "temp-archive/";
	QDir dir;
	dir.mkdir(tempFolder);
	const Q_ULONG BUFFER_SIZE = 1024;

	QFile file(path);
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::Latin1);
		QString line = stream.readLine();
		if (line != "BasKetNP:archive") {
			KMessageBox::error(0, i18n("This file is not a basket archive."), i18n("Basket Archive Error"));
			file.close();
			Tools::deleteRecursively(tempFolder);
			return;
		}
		QString version;
		QStringList readCompatibleVersions;
		QStringList writeCompatibleVersions;
		while (!stream.atEnd()) {
			// Get Key/Value Pair From the Line to Read:
			line = stream.readLine();
			int index = line.find(':');
			QString key;
			QString value;
			if (index >= 0) {
				key = line.left(index);
				value = line.right(line.length() - index - 1);
			} else {
				key = line;
				value = "";
			}
			if (key == "version") {
				version = value;
			} else if (key == "read-compatible") {
				readCompatibleVersions = QStringList::split(value, ";");
			} else if (key == "write-compatible") {
				writeCompatibleVersions = QStringList::split(value, ";");
			} else if (key == "preview*") {
				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					KMessageBox::error(0, i18n("This file is corrupted. It can not be opened."), i18n("Basket Archive Error"));
					file.close();
					Tools::deleteRecursively(tempFolder);
					return;
				}
				// Get the preview file:
//FIXME: We do not need the preview for now
//				QFile previewFile(tempFolder + "preview.png");
//				if (previewFile.open(IO_WriteOnly)) {
					char *buffer = new char[BUFFER_SIZE];
					Q_LONG sizeRead;
					while ((sizeRead = file.readBlock(buffer, QMIN(BUFFER_SIZE, size))) > 0) {
//						previewFile.writeBlock(buffer, sizeRead);
						size -= sizeRead;
					}
//					previewFile.close();
					delete buffer;
//				}
			} else if (key == "archive*") {
				if (version != "0.6.1" && readCompatibleVersions.contains("0.6.1") && !writeCompatibleVersions.contains("0.6.1")) {
					KMessageBox::information(
						0,
						i18n("This file was created with a recent version of %1. "
						     "It can be opened but not every information will be available to you. "
						     "For instance, some notes may be missing because they are of a type only available in new versions. "
						     "When saving the file back, consider to save it to another file, to preserve the original one.")
							.arg(kapp->aboutData()->programName()),
						i18n("Basket Archive Error")
					);
				}
				if (version != "0.6.1" && !readCompatibleVersions.contains("0.6.1") && !writeCompatibleVersions.contains("0.6.1")) {
					KMessageBox::error(
						0,
						i18n("This file was created with a recent version of %1. Please upgrade to a newer version to be able to open that file.")
							.arg(kapp->aboutData()->programName()),
						i18n("Basket Archive Error")
					);
					file.close();
					Tools::deleteRecursively(tempFolder);
					return;
				}

				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					KMessageBox::error(0, i18n("This file is corrupted. It can not be opened."), i18n("Basket Archive Error"));
					file.close();
					Tools::deleteRecursively(tempFolder);
					return;
				}
				// Get the archive file:
				QString tempArchive = tempFolder + "temp-archive.tar.gz";
				QFile archiveFile(tempArchive);
				if (archiveFile.open(IO_WriteOnly)) {
					char *buffer = new char[BUFFER_SIZE];
					Q_LONG sizeRead;
					while ((sizeRead = file.readBlock(buffer, QMIN(BUFFER_SIZE, size))) > 0) {
						archiveFile.writeBlock(buffer, sizeRead);
						size -= sizeRead;
					}
					archiveFile.close();
					delete buffer;

					// Extract the Archive:
					QString extractionFolder = tempFolder + "extraction/";
					QDir dir;
					dir.mkdir(extractionFolder);
					KTar tar(tempArchive, "application/x-gzip");
					tar.open(IO_ReadOnly);
					tar.directory()->copyTo(extractionFolder);
					tar.close();

					// Import the Tags:
					importTagEmblems(extractionFolder); // Import and rename tag emblems BEFORE loading them!
					QMap<QString, QString> mergedStates = Tag::loadTags(extractionFolder + "tags.xml");
					QMap<QString, QString>::Iterator it;
					if (mergedStates.count() > 0) {
						Tag::saveTags();
					}

					// Import the Background Images:
					importArchivedBackgroundImages(extractionFolder);

					// Import the Baskets:
					renameBasketFolders(extractionFolder, mergedStates);

				}
			} else if (key.endsWith("*")) {
				// We do not know what it is, but we should read the embedded-file in order to discard it:
				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					KMessageBox::error(0, i18n("This file is corrupted. It can not be opened."), i18n("Basket Archive Error"));
					file.close();
					Tools::deleteRecursively(tempFolder);
					return;
				}
				// Get the archive file:
				char *buffer = new char[BUFFER_SIZE];
				Q_LONG sizeRead;
				while ((sizeRead = file.readBlock(buffer, QMIN(BUFFER_SIZE, size))) > 0) {
					size -= sizeRead;
				}
				delete buffer;
			} else {
				// We do not know what it is, and we do not care.
			}
			// Analyse the Value, if Understood:
		}
		file.close();
	}
	Tools::deleteRecursively(tempFolder);
}

/**
 * When opening a basket archive that come from another computer,
 * it can contains tags that use icons (emblems) that are not present on that computer.
 * Fortunately, basket archives contains a copy of every used icons.
 * This method check for every emblems and import the missing ones.
 * It also modify the tags.xml copy for the emblems to point to the absolute path of the impported icons.
 */
void Archive::importTagEmblems(const QString &extractionFolder)
{
	QDomDocument *document = XMLWork::openFile("basketTags", extractionFolder + "tags.xml");
	if (document == 0)
		return;
	QDomElement docElem = document->documentElement();

	QDir dir;
	dir.mkdir(Global::savesFolder() + "tag-emblems/");
	FormatImporter copier; // Only used to copy files synchronously

	QDomNode node = docElem.firstChild();
	while (!node.isNull()) {
		QDomElement element = node.toElement();
		if ( (!element.isNull()) && element.tagName() == "tag" ) {
			QDomNode subNode = element.firstChild();
			while (!subNode.isNull()) {
				QDomElement subElement = subNode.toElement();
				if ( (!subElement.isNull()) && subElement.tagName() == "state" ) {
					QString emblemName = XMLWork::getElementText(subElement, "emblem");
					if (!emblemName.isEmpty()) {
						QPixmap emblem = kapp->iconLoader()->loadIcon(emblemName, KIcon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/true);
						// The icon does not exists on that computer, import it:
						if (emblem.isNull()) {
							// Of the emblem path was eg. "/home/seb/emblem.png", it was exported as "tag-emblems/_home_seb_emblem.png".
							// So we need to copy that image to "~/.kde/share/apps/basket/tag-emblems/emblem.png":
							int slashIndex = emblemName.findRev("/");
							QString emblemFileName = (slashIndex < 0 ? emblemName : emblemName.right(slashIndex - 2));
							QString source      = extractionFolder + "tag-emblems/" + emblemName.replace('/', '_');
							QString destination = Global::savesFolder() + "tag-emblems/" + emblemFileName;
							if (!dir.exists(destination))
								copier.copyFolder(source, destination);
							// Replace the emblem path in the tags.xml copy:
							QDomElement emblemElement = XMLWork::getElement(subElement, "emblem");
							subElement.removeChild(emblemElement);
							XMLWork::addElement(*document, subElement, "emblem", destination);
						}
					}
				}
				subNode = subNode.nextSibling();
			}
		}
		node = node.nextSibling();
	}
	Basket::safelySaveToFile(extractionFolder + "tags.xml", document->toString());
}

void Archive::importArchivedBackgroundImages(const QString &extractionFolder)
{
	FormatImporter copier; // Only used to copy files synchronously
	QString destFolder = KGlobal::dirs()->saveLocation("data", "basket/backgrounds/");

	QDir dir(extractionFolder + "backgrounds/", /*nameFilder=*/"*.png", /*sortSpec=*/QDir::Name | QDir::IgnoreCase, /*filterSpec=*/QDir::Files | QDir::NoSymLinks);
	QStringList files = dir.entryList();
	for (QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
		QString image = *it;
		if (!Global::backgroundManager->exists(image)) {
			// Copy images:
			QString imageSource = extractionFolder + "backgrounds/" + image;
			QString imageDest   = destFolder + image;
			copier.copyFolder(imageSource, imageDest);
			// Copy configuration file:
			QString configSource = extractionFolder + "backgrounds/" + image + ".config";
			QString configDest   = destFolder + image;
			if (dir.exists(configSource))
				copier.copyFolder(configSource, configDest);
			// Copy preview:
			QString previewSource = extractionFolder + "backgrounds/previews/" + image;
			QString previewDest   = destFolder + "previews/" + image;
			if (dir.exists(previewSource)) {
				dir.mkdir(destFolder + "previews/"); // Make sure the folder exists!
				copier.copyFolder(previewSource, previewDest);
			}
			// Append image to database:
			Global::backgroundManager->addImage(imageDest);
		}
	}
}

void Archive::renameBasketFolders(const QString &extractionFolder, QMap<QString, QString> &mergedStates)
{
	QDomDocument *doc = XMLWork::openFile("basketTree", extractionFolder + "baskets/baskets.xml");
	if (doc != 0) {
		QMap<QString, QString> folderMap;
		QDomElement docElem = doc->documentElement();
		QDir dir;
		QDomNode node = docElem.firstChild();
		renameBasketFolder(extractionFolder, node, folderMap, mergedStates);
		loadExtractedBaskets(extractionFolder, node, folderMap, 0);
	}
}

void Archive::renameBasketFolder(const QString &extractionFolder, QDomNode &basketNode, QMap<QString, QString> &folderMap, QMap<QString, QString> &mergedStates)
{
	QDomNode n = basketNode;
	while ( ! n.isNull() ) {
		QDomElement element = n.toElement();
		if ( (!element.isNull()) && element.tagName() == "basket" ) {
			QString folderName = element.attribute("folderName");
			if (!folderName.isEmpty()) {
				// Find a folder name:
				QString newFolderName = BasketFactory::newFolderName();
				folderMap[folderName] = newFolderName;
				// Reserve the folder name:
				QDir dir;
				dir.mkdir(Global::basketsFolder() + newFolderName);
				// Rename the merged tag ids:
				if (mergedStates.count() > 0) {
					renameMergedStates(extractionFolder + "baskets/" + folderName + ".basket", mergedStates);
				}
				// Child baskets:
				QDomNode node = element.firstChild();
				renameBasketFolder(extractionFolder, node, folderMap, mergedStates);
			}
		}
		n = n.nextSibling();
	}
}

void Archive::renameMergedStates(const QString &fullPath, QMap<QString, QString> &mergedStates)
{
	QDomDocument *doc = XMLWork::openFile("basket", fullPath);
	if (doc == 0)
		return;
	QDomElement docElem = doc->documentElement();
	QDomElement notes = XMLWork::getElement(docElem, "notes");
	renameMergedStates(notes, mergedStates);
	Basket::safelySaveToFile(fullPath, /*"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + */doc->toString());
}

void Archive::renameMergedStates(QDomNode notes, QMap<QString, QString> &mergedStates)
{
	QDomNode n = notes.firstChild();
	while ( ! n.isNull() ) {
		QDomElement element = n.toElement();
		if (!element.isNull()) {
			if (element.tagName() == "group" ) {
				renameMergedStates(n, mergedStates);
			} else if (element.tagName() == "note") {
				QString tags = XMLWork::getElementText(element, "tags");
				if (!tags.isEmpty()) {
					QStringList tagNames = QStringList::split(";", tags);
					for (QStringList::Iterator it = tagNames.begin(); it != tagNames.end(); ++it) {
						QString &tag = *it;
						if (mergedStates.contains(tag)) {
							tag = mergedStates[tag];
						}
					}
					QString newTags = tagNames.join(";");
					QDomElement tagsElement = XMLWork::getElement(element, "tags");
					element.removeChild(tagsElement);
					QDomDocument document = element.ownerDocument();
					XMLWork::addElement(document, element, "tags", newTags);
				}
			}
		}
		n = n.nextSibling();
	}
}

void Archive::loadExtractedBaskets(const QString &extractionFolder, QDomNode &basketNode, QMap<QString, QString> &folderMap, Basket *parent)
{
	bool basketSetAsCurrent = (parent != 0);
	QDomNode n = basketNode;
	while ( ! n.isNull() ) {
		QDomElement element = n.toElement();
		if ( (!element.isNull()) && element.tagName() == "basket" ) {
			QString folderName = element.attribute("folderName");
			if (!folderName.isEmpty()) {
				// Move the basket folder to its destination, while renaming it uniquely:
				QString newFolderName = folderMap[folderName];
				FormatImporter copier;
				// The folder has been "reserved" by creating it. Avoid asking the user to override:
				QDir dir;
				dir.rmdir(Global::basketsFolder() + newFolderName);
				copier.moveFolder(extractionFolder + "baskets/" + folderName, Global::basketsFolder() + newFolderName);
				// Append and load the basket in the tree:
				Basket *basket = Global::bnpView->loadBasket(newFolderName);
				BasketListViewItem *basketItem = Global::bnpView->appendBasket(basket, (basket && parent ? Global::bnpView->listViewItemForBasket(parent) : 0));
				basketItem->setOpen(!XMLWork::trueOrFalse(element.attribute("folded", "false"), false));
				basket->loadProperties(XMLWork::getElement(element, "properties"));
				// Open the first basket of the archive:
				if (!basketSetAsCurrent) {
					Global::bnpView->setCurrentBasket(basket);
					basketSetAsCurrent = true;
				}
				QDomNode node = element.firstChild();
				loadExtractedBaskets(extractionFolder, node, folderMap, basket);
			}
		}
		n = n.nextSibling();
	}
}
