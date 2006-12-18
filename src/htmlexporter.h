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

#ifndef HTMLEXPORTER_H
#define HTMLEXPORTER_H

#include <qstring.h>
#include <qtextstream.h>

class Basket;
class Note;

/**
 * @author Sébastien Laoût <slaout@linux62.org>
 */
class HTMLExporter
{
  public:
	HTMLExporter(Basket *basket);
	~HTMLExporter();
  private:
	void prepareExport(Basket *basket, const QString &fullPath);
	void exportBasket(Basket *basket, bool isSubBasket);
	void exportNote(Note *note, int indent);
	void writeBasketTree(Basket *currentBasket);
	void writeBasketTree(Basket *currentBasket, Basket *basket, int indent);

  public:
	QString copyIcon(const QString &iconName, int size);
	QString copyFile(const QString &srcPath, bool createIt);

  public:
	// Absolute path of the file name the user choosen:
	QString filePath;          // eg.: "/home/seb/foo.html"
	QString fileName;          // eg.: "foo.html"

	// Absolute & relative paths for the current basket to be exported:
	QString basketFilePath;    // eg.: "/home/seb/foo.html" or "/home/seb/foo.html_files/baskets/basketN.html"
	QString filesFolderPath;   // eg.: "/home/seb/foo.html_files/"
	QString filesFolderName;   // eg.: "foo.html_files/" or "../"
	QString iconsFolderPath;   // eg.: "/home/seb/foo.html_files/icons/"
	QString iconsFolderName;   // eg.: "foo.html_files/icons/" or "../icons/"
	QString imagesFolderPath;  // eg.: "/home/seb/foo.html_files/images/"
	QString imagesFolderName;  // eg.: "foo.html_files/images/" or "../images/"
	QString dataFolderPath;    // eg.: "/home/seb/foo.html_files/data/" or "/home/seb/foo.html_files/baskets/basketN-data/"
	QString dataFolderName;    // eg.: "foo.html_files/data/" or "basketN-data/"
	QString basketsFolderPath; // eg.: "/home/seb/foo.html_files/baskets/"
	QString basketsFolderName; // eg.: "foo.html_files/baskets/" or ""

	// Various properties of the currently exporting basket:
	QString backgroundColorName;

	// Variables used by every export methods:
	QTextStream stream;
	Basket *exportedBasket;
	bool withBasketTree;
};

#endif // HTMLEXPORTER_H
