/***************************************************************************
 *   Copyright (C) 2003 by S�astien Lao�t                                 *
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


#include "xmlwork.h"

#include <QDomDocument>
#include <QDomElement>
#include <qdom.h>
#include <qstringlist.h>
#include <qfile.h>

#include <kdebug.h>
#include <kmessagebox.h>

QDomDocument* XMLWork::openFile(const QString &name, const QString &filePath)
{
	QDomDocument *doc = new QDomDocument(name);
	QFile file(filePath);
	kDebug() << filePath << endl;
	if ( ! file.open( QIODevice::ReadOnly ) ) {
		//FIXME 0 -> this
		//KMessageBox::information(0, "Load an XML file", "Error : un-openable file");
		kDebug() << "Error!!!, Load an XML file" << endl;
		delete doc;
		return 0;
	}
	if ( ! doc->setContent(&file) ) {
		//KMessageBox::information(0, "Load an XML file", "Error : malformed content");
		kDebug() << "Error!!!, Bad content of XML file" << endl;
		file.close();
		delete doc;
		return 0;
	}
	file.close();
	return doc;
}

QDomElement XMLWork::getElement(const QDomElement &startElement, const QString &elementPath)
{
	QStringList elements = elementPath.split("/",  QString::KeepEmptyParts,Qt::CaseInsensitive);
	QDomNode n = startElement.firstChild();
	for (unsigned int i = 0; i < elements.count(); ++i) {               // For each elements
		while ( ! n.isNull() ) {                                        // Browse theire sub elements
			QDomElement e = n.toElement();                              //  and search the good one
			if ( (!e.isNull()) && e.tagName() == elements.at(i) ) {    // If found
				if ( i + 1 == elements.count() )                        // And if it is the asked element
					return e;                                           // Return the first corresponding
				else {                                                  // Or if it is an intermediate element
					n = e.firstChild();                                 // Continue with the next sub element
					break;
				}
			}
			n = n.nextSibling();
		}
	}
	return QDomElement();                                               // Not found !
}

QString XMLWork::getElementText(const QDomElement &startElement, const QString &elementPath, const QString &defaultTxt)
{
	QDomElement e = getElement(startElement, elementPath);
	if (e.isNull())
		return defaultTxt;
	else
		return e.text();
}

void XMLWork::addElement(QDomDocument &document, QDomElement &parent, const QString &name, const QString &text)
{
	QDomElement tag = document.createElement(name);
	parent.appendChild(tag);
	QDomText content = document.createTextNode(text);
	tag.appendChild(content);
}

bool XMLWork::trueOrFalse(const QString &value, bool defaultValue)
{
	if ( value == "true"  || value == "1" || value == "on"  || value == "yes" )
		return true;
	if ( value == "false" || value == "0" || value == "off" || value == "no"  )
		return false;
	return defaultValue;
}

QString XMLWork::trueOrFalse(bool value)
{
	return value ? "true" : "false";
}

QString XMLWork::innerXml(QDomElement &element)
{
	QString inner;
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.isCharacterData())
			inner += n.toCharacterData().data();
		else if (n.isElement()) {
			QDomElement e = n.toElement();
			inner += "<" + e.tagName() + ">" + innerXml(e) + "</" + e.tagName() + ">";
		}
	return inner;
}
