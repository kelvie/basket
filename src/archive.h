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

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <qvaluelist.h>
#include <qmap.h>

class Basket;
class Tag;

class QString;
class QStringList;
class QDomNode;
class KTar;
class KProgress;

/**
 * @author Sébastien Laoût <slaout@linux62.org>
 */
class Archive
{
  public:
	static void save(Basket *basket, bool withSubBaskets, const QString &destination);
	static void open(const QString &path);
  private:
	// Convenient Methods for Saving:
	static void saveBasketToArchive(Basket *basket, bool recursive, KTar *tar, QStringList &backgrounds, const QString &tempFolder, KProgress *progress);
	static void listUsedTags(Basket *basket, bool recursive, QValueList<Tag*> &list);
	// Convenient Methods for Loading:
	static void renameBasketFolders(const QString &extractionFolder, QMap<QString, QString> &mergedStates);
	static void renameBasketFolder(const QString &extractionFolder, QDomNode &basketNode, QMap<QString, QString> &folderMap, QMap<QString, QString> &mergedStates);
	static void renameMergedStates(const QString &fullPath, QMap<QString, QString> &mergedStates);
	static void renameMergedStates(QDomNode notes, QMap<QString, QString> &mergedStates);
	static void loadExtractedBaskets(const QString &extractionFolder, QDomNode &basketNode, QMap<QString, QString> &folderMap, Basket *parent);
	static void importTagEmblems(const QString &extractionFolder);
	static void importArchivedBackgroundImages(const QString &extractionFolder);
};

#endif
