/***************************************************************************
 *   Copyright (C) 2003 by S�bastien Lao�t                                 *
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

#ifndef XMLWORKXMLWORK_H
#define XMLWORKXMLWORK_H

#include <QString>
class QDomDocument;
class QDomElement;

/** All related functions to manage XML files and trees
  * @author S�bastien Lao�t
  */

namespace XMLWork
{
	// Manage XML files :
	QDomDocument* openFile(const QString &name, const QString &filePath);
	// Manage XML trees :
	QDomElement   getElement(const QDomElement &startElement, const QString &elementPath);
	QString       getElementText(const QDomElement &startElement, const QString &elementPath, const QString &defaultTxt = QString());
	void          addElement(QDomDocument &document, QDomElement &parent, const QString &name, const QString &text);
	QString       innerXml(QDomElement &element);
	// Not directly related to XML :
	bool          trueOrFalse(const QString &value, bool defaultValue = true);
	QString       trueOrFalse(bool value);
}

#endif // XMLWORKXMLWORK_H
